
#include <stdio.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include "defines.h"
#include "math.h"

static bool running;

#include "win32.cpp"

struct Vertex
{
    Vec3 position;
    Vec4 color;
    Vec2 uv;
};

struct Shader
{
    ID3D11VertexShader *vertex;
    ID3D11PixelShader *fragment;
    ID3DBlob *vertexShaderCompiled;
    ID3DBlob *fragmentShaderCompiled;
};

struct ConstBuffer
{
    ID3D11Buffer *buffer;
    u32 index;
};

struct VertexBuffer
{
    ID3D11Buffer *GPUBuffer;
    u32 verticesCount;

    ID3D11InputLayout *layout;
};

struct FrameBuffer
{
    f32 x;
    f32 y;
    f32 width;
    f32 height;
    DXGI_FORMAT format;
     
    ID3D11Texture2D *texture;
    ID3D11RenderTargetView *renderTargetView;
    ID3D11ShaderResourceView *shaderResourceView;
};

struct File
{
    void *data;
    size_t size;
};

struct CBuffer
{
    Mat4 proj;
    Mat4 view;
    Mat4 world;
};

struct Rect
{
    f32 x, y;
    f32 w, h;
};

static File ReadFile(char *filepath)
{
    File result;
    memset(&result, 0, sizeof(File));

    HANDLE hFile = CreateFileA(filepath, GENERIC_READ,
            FILE_SHARE_READ, 0, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, 0);

    if(hFile == INVALID_HANDLE_VALUE)
    {
        printf("Error reading file: %s\n", filepath);
        ASSERT(!"INVALID_CODE_PATH");
    }

    LARGE_INTEGER bytesToRead;
    GetFileSizeEx(hFile, &bytesToRead);

    void *data =  malloc(bytesToRead.QuadPart + 1);

    size_t bytesReaded = 0;
    if(!ReadFile(hFile, data, bytesToRead.QuadPart, (LPDWORD)&bytesReaded, 0))
    {
        printf("Error reading file: %s\n", filepath);
        ASSERT(!"INVALID_CODE_PATH");
    }

    char *end = ((char *)data) + bytesToRead.QuadPart;
    end[0] = '\0';

    CloseHandle(hFile);

    result.data = data;
    result.size = bytesReaded;

    return result;
}

static Shader LoadShader(char *vertpath, char *fragpath)
{
    Shader shader = {};

    File vertfile = ReadFile(vertpath);
    File fragfile = ReadFile(fragpath);
    
    HRESULT result = 0;
    ID3DBlob *errorVertexShader = 0;
    result = D3DCompile(vertfile.data, vertfile.size,
                        0, 0, 0, "vs_main", "vs_4_0",
                        D3DCOMPILE_ENABLE_STRICTNESS, 0,
                        &shader.vertexShaderCompiled,
                        &errorVertexShader);
    if(errorVertexShader != 0)
    {
        char *errorString = (char *)errorVertexShader->GetBufferPointer();
        printf("error compiling vertex shader (%s): %s", vertpath, errorString);
        errorVertexShader->Release();
        ASSERT(!"INVALID_CODE_PATH");
    }

    ID3DBlob *errorFragmentShader = 0;
    result = D3DCompile(fragfile.data, fragfile.size,
                        0, 0, 0, "fs_main", "ps_4_0",
                        D3DCOMPILE_ENABLE_STRICTNESS, 0,
                        &shader.fragmentShaderCompiled,
                        &errorFragmentShader);
    if(errorFragmentShader)
    {
        char *errorString = (char *)errorFragmentShader->GetBufferPointer();
        printf("error compiling fragment shader (%s): %s", fragpath, errorString);
        errorFragmentShader->Release();
        ASSERT(!"INVALID_CODE_PATH")
    }

    // create the vertex and fragment shader
    result = device->CreateVertexShader(
            shader.vertexShaderCompiled->GetBufferPointer(),
            shader.vertexShaderCompiled->GetBufferSize(), 0,
            &shader.vertex);
    result = device->CreatePixelShader(
            shader.fragmentShaderCompiled->GetBufferPointer(),
            shader.fragmentShaderCompiled->GetBufferSize(), 0,
            &shader.fragment);

    free(vertfile.data);
    free(fragfile.data);

    return shader;
}

void UnloadShader(Shader *shader)
{
    if(shader->vertex) shader->vertex->Release();
    if(shader->fragment) shader->fragment->Release();
    if(shader->vertexShaderCompiled) shader->vertexShaderCompiled->Release();
    if(shader->fragmentShaderCompiled) shader->fragmentShaderCompiled->Release();
}

ConstBuffer LoadConstBuffer(void *bufferData, size_t bufferSize, u32 index)
{
    ConstBuffer constBuffer = {};

    D3D11_BUFFER_DESC constBufferDesc;
    ZeroMemory(&constBufferDesc, sizeof(constBufferDesc));
    constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufferDesc.ByteWidth = bufferSize;
    constBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    HRESULT result = device->CreateBuffer(&constBufferDesc, 0, &constBuffer.buffer);
    if(FAILED(result))
    {
        printf("error creating const buffer\n");
        ASSERT(!"INVALID_CODE_PATH");
    }
    deviceContext->UpdateSubresource(constBuffer.buffer, 0, 0, bufferData, 0, 0);
    deviceContext->VSSetConstantBuffers(index, 1, &constBuffer.buffer);
    deviceContext->PSSetConstantBuffers(index, 1, &constBuffer.buffer);
    constBuffer.index = index;

    return constBuffer;
}

void UpdateConstBuffer(ConstBuffer *constBuffer, void *bufferData)
{
    deviceContext->UpdateSubresource(constBuffer->buffer, 0, 0, bufferData, 0, 0);
    deviceContext->VSSetConstantBuffers(constBuffer->index, 1, &constBuffer->buffer);
    deviceContext->PSSetConstantBuffers(constBuffer->index, 1, &constBuffer->buffer);
}

void UnloadConstBuffer(ConstBuffer *constBuffer)
{
    if(constBuffer->buffer) constBuffer->buffer->Release();
    constBuffer->index = -1;
}

VertexBuffer LoadVertexBuffer(Vertex *vertices, size_t count, ID3D11InputLayout *layout)
{
    VertexBuffer buffer = {};
    buffer.verticesCount = count;
    buffer.layout = layout;

    D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory(&resourceData, sizeof(resourceData));

    D3D11_BUFFER_DESC vertexDesc;
    ZeroMemory(&vertexDesc, sizeof(vertexDesc));
    vertexDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDesc.ByteWidth = sizeof(Vertex) * count;
    resourceData.pSysMem = vertices;

    HRESULT result = device->CreateBuffer(&vertexDesc, &resourceData, &buffer.GPUBuffer);
    if(FAILED(result))
    {
        printf("error loading vertex buffer\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    return buffer;
}

void UnloadVertexBuffer(VertexBuffer *vertexBuffer)
{
    if(vertexBuffer->GPUBuffer) vertexBuffer->GPUBuffer->Release();
    vertexBuffer->verticesCount = 0;
}

FrameBuffer LoadFrameBuffer(f32 x, f32 y, f32 width, f32 height, DXGI_FORMAT format)
{
    FrameBuffer frameBuffer = {};
    frameBuffer.x = x;
    frameBuffer.y = y;
    frameBuffer.width = width;
    frameBuffer.height = height;
    frameBuffer.format = format;
    
    // create texture 2d
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = format;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    if(FAILED(device->CreateTexture2D(&texDesc, 0, &frameBuffer.texture)))
    {
        printf("Error creating FrameBuffer Texture\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    // create render target view
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    if(FAILED(device->CreateRenderTargetView(frameBuffer.texture, &rtvDesc, &frameBuffer.renderTargetView)))
    {
        printf("Error creating FrameBuffer rtv\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    // create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    if (FAILED(device->CreateShaderResourceView(frameBuffer.texture, &srvDesc, &frameBuffer.shaderResourceView)))
    {
        printf("Error creating FrameBuffer srv\n");
        ASSERT(!"INVALID_CODE_PATH");
    }
    
    return frameBuffer;

}

void UnloadFrameBuffer(FrameBuffer *frameBuffer)
{
    if(frameBuffer->texture) frameBuffer->texture->Release(); frameBuffer->texture = 0;
    if(frameBuffer->renderTargetView) frameBuffer->renderTargetView->Release(); frameBuffer->renderTargetView = 0;
    if(frameBuffer->shaderResourceView) frameBuffer->shaderResourceView->Release(); frameBuffer->shaderResourceView = 0;
}

void ResizeFrameBuffer(FrameBuffer *frameBuffer, f32 x, f32 y, f32 width, f32 height)
{
    UnloadFrameBuffer(frameBuffer);
    *frameBuffer = LoadFrameBuffer(x, y, width, height, frameBuffer->format);
}

static Vertex quad[] = {
    // Face 1
    {{-0.5f, -0.5f, 0}, {1, 1, 0, 1}, {0, 0}},
    {{-0.5f,  0.5f, 0}, {1, 0, 0, 1}, {0, 1}},
    {{ 0.5f, -0.5f, 0}, {0, 1, 0, 1}, {1, 0}},
    // Face 2
    {{ 0.5f, -0.5f, 0}, {0, 1, 0, 1}, {1, 0}},
    {{-0.5f,  0.5f, 0}, {1, 0, 0, 1}, {0, 1}},
    {{ 0.5f,  0.5f, 0}, {0, 0, 1, 1}, {1, 1}}
};


int main()
{
    HINSTANCE instace = GetModuleHandle(0);
    HWND window = InitWindow(instace);
    InitD3D11(window);

    // Set up Dear imgui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsClassic();
    //ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(device, deviceContext);

    bool show_demo_window = false;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.2f, 1.00f);


    // Load Shader and Create Input Layout
    Shader colShader = LoadShader("../src/shaders/colVert.hlsl", "../src/shaders/colFrag.hlsl");
    Shader texShader = LoadShader("../src/shaders/texVert.hlsl", "../src/shaders/texFrag.hlsl");
    
    ID3D11InputLayout *layout = 0;
    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
         0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
         0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
         0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    i32 totalLayoutElements = ARRAY_LENGTH(inputLayoutDesc);
    device->CreateInputLayout(inputLayoutDesc,
        totalLayoutElements,
        colShader.vertexShaderCompiled->GetBufferPointer(),
        colShader.vertexShaderCompiled->GetBufferSize(),
        &layout);

    // Load Vertex Buffer
    VertexBuffer vertexBuffer = LoadVertexBuffer(quad, ARRAY_LENGTH(quad), layout);

    // Load FrameBuffer
    f32 fixWidth = 200.0f;
    f32 clientWidth = (1264.0f - fixWidth);
    Rect uiRect = {0, 0, fixWidth, 681.0f};
    Rect clientRect = {fixWidth, 0, clientWidth, 681.0f};

    f32 frameWidth = clientRect.w*0.5f;
    f32 frameHeight = clientRect.h*0.5f;
    FrameBuffer frameBuffer0 = LoadFrameBuffer(             frameWidth*0.5f, frameHeight + frameHeight*0.5f, frameWidth-2, frameHeight-2, DXGI_FORMAT_R8G8B8A8_UNORM);
    FrameBuffer frameBuffer1 = LoadFrameBuffer(frameWidth + frameWidth*0.5f,               frameHeight*0.5f, frameWidth-2, frameHeight-2, DXGI_FORMAT_R8G8B8A8_UNORM);
    FrameBuffer frameBuffer2 = LoadFrameBuffer(             frameWidth*0.5f,               frameHeight*0.5f, frameWidth-2, frameHeight-2, DXGI_FORMAT_R8G8B8A8_UNORM);
    FrameBuffer frameBuffer3 = LoadFrameBuffer(frameWidth + frameWidth*0.5f, frameHeight + frameHeight*0.5f, frameWidth-2, frameHeight-2, DXGI_FORMAT_R8G8B8A8_UNORM);

    // Create Sampler State
    ID3D11SamplerState *samplerState;
    D3D11_SAMPLER_DESC colorMapDesc = {};
    colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP; //D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP; //D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP; //D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; //D3D11_FILTER_MIN_MAG_MIP_LINEAR | D3D11_FILTER_MIN_MAG_MIP_POINT
    colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;
    if(FAILED(device->CreateSamplerState(&colorMapDesc, &samplerState)))
    {
        printf("Error: Failed Creating sampler state\n");
        ASSERT(!"INVALID_CODE_PATH");
    }
    deviceContext->PSSetSamplers(0, 1, &samplerState);


    // create the const buffer data to be pass to the gpu
    Vec3 up  = {0, 1,  0};
    Vec3 pos = {0, 0, -2};
    Vec3 tar = {0, 0, 0};
    CBuffer cbuffer{};
    cbuffer.view = Mat4LookAt(pos, tar, up);
    cbuffer.world = Mat4Identity();;
    //cbuffer.proj = Mat4Perspective(60, (f32)WINDOW_WIDTH/(f32)WINDOW_HEIGHT, 0.01f, 100.0f);
    cbuffer.proj = Mat4Ortho((f32)WINDOW_WIDTH*-0.5f, (f32)WINDOW_WIDTH*0.5f,
                             (f32)WINDOW_HEIGHT*-0.5f, (f32)WINDOW_HEIGHT*0.5f,
                             0.01f, 100.0f);
    ConstBuffer constBuffer = LoadConstBuffer((void *)&cbuffer, sizeof(CBuffer), 0);

    ShowWindow(window, 1);
    running = true;
    while(running)
    {
        FlushEvents(window);
        static f32 lastWidth = 1264.0f;
        static f32 lastHeight = 681.0f;
        if(resizeWidth != 0 && resizeHeight != 0)
        {
            ResizeD3D11();
            //cbuffer.proj = Mat4Perspective(60, (f32)resizeWidth/(f32)resizeHeight, 0.01f, 100.0f);
            cbuffer.proj = Mat4Ortho(0.0f, (f32)resizeWidth, 0.0f, (f32)resizeHeight, 0.0f, 100.0f);
            UpdateConstBuffer(&constBuffer, (void *)&cbuffer);

            clientWidth = (resizeWidth - fixWidth);
            if(clientWidth > 0)
            {
                uiRect = {0, 0, fixWidth, (f32)resizeHeight};
                clientRect = {fixWidth, 0, clientWidth, (f32)resizeHeight};

                frameWidth = clientRect.w*0.5f;
                frameHeight = clientRect.h*0.5f;
                ResizeFrameBuffer(&frameBuffer0,              frameWidth*0.5f, frameHeight + frameHeight*0.5f, frameWidth-2, frameHeight-2);
                ResizeFrameBuffer(&frameBuffer1, frameWidth + frameWidth*0.5f,               frameHeight*0.5f, frameWidth-2, frameHeight-2);
                ResizeFrameBuffer(&frameBuffer2,              frameWidth*0.5f,               frameHeight*0.5f, frameWidth-2, frameHeight-2);
                ResizeFrameBuffer(&frameBuffer3, frameWidth + frameWidth*0.5f, frameHeight + frameHeight*0.5f, frameWidth-2, frameHeight-2);
            }

            lastWidth = resizeWidth;
            lastHeight = resizeHeight;
            resizeWidth = resizeHeight = 0;
        }        

        u32 stride = sizeof(Vertex);
        u32 offset = 0;
        deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        deviceContext->IASetInputLayout(vertexBuffer.layout);
        deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer.GPUBuffer, &stride, &offset);

        deviceContext->VSSetShader(colShader.vertex, 0, 0);
        deviceContext->PSSetShader(colShader.fragment, 0, 0);


        // update viewport
        D3D11_VIEWPORT viewport;
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = (f32)frameBuffer0.width;
        viewport.Height = (f32)frameBuffer0.height;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        deviceContext->RSSetViewports(1, &viewport);

        // update proj matrix
        Mat4 proj = cbuffer.proj;

        static f32 angle = 0;
        angle += 0.01f;


        cbuffer.proj = Mat4Perspective(60, (f32)frameBuffer0.width/(f32)frameBuffer0.height, 0.01f, 100.0f);
        
        f32 clearColor_0[] = { 0.6, 0.1, 0.1, 1 };
        deviceContext->ClearRenderTargetView(frameBuffer0.renderTargetView, clearColor_0);
        deviceContext->OMSetRenderTargets(1, &frameBuffer0.renderTargetView, 0);
        cbuffer.world = Mat4RotateY(angle) * Mat4RotateX(angle);
        UpdateConstBuffer(&constBuffer, (void *)&cbuffer);
        deviceContext->Draw(vertexBuffer.verticesCount, 0);

        cbuffer.proj = Mat4Ortho(frameBuffer1.width*-0.5f, frameBuffer1.width*0.5f,
                                 frameBuffer1.height*-0.5f, frameBuffer1.height*0.5f,
                                 0.01f, 100.0f);

        f32 clearColor_1[] = { 0.6, 0.6, 0.1, 1 };
        deviceContext->ClearRenderTargetView(frameBuffer1.renderTargetView, clearColor_1);
        deviceContext->OMSetRenderTargets(1, &frameBuffer1.renderTargetView, 0);
        cbuffer.world = Mat4Translate(-sinf(angle) * 250, cosf(angle) * 100, 0) * Mat4Scale(300, 300, 1) * Mat4RotateZ(angle);
        UpdateConstBuffer(&constBuffer, (void *)&cbuffer);
        deviceContext->Draw(vertexBuffer.verticesCount, 0);

        f32 clearColor_2[] = { 0.6, 0.1, 0.6, 1 };
        deviceContext->ClearRenderTargetView(frameBuffer2.renderTargetView, clearColor_2);
        deviceContext->OMSetRenderTargets(1, &frameBuffer2.renderTargetView, 0);
        cbuffer.world = Mat4Translate(sinf(angle) * 250, -cosf(angle) * 100, 0) * Mat4Scale(300, 300, 1);
        UpdateConstBuffer(&constBuffer, (void *)&cbuffer);
        deviceContext->Draw(vertexBuffer.verticesCount, 0);

        f32 clearColor_3[] = { 0.2, 0.6, 0.2, 1 };
        deviceContext->ClearRenderTargetView(frameBuffer3.renderTargetView, clearColor_3);
        deviceContext->OMSetRenderTargets(1, &frameBuffer3.renderTargetView, 0);
        cbuffer.world = Mat4Translate(-sinf(angle) * 250, -cosf(angle) * 100, 0) * Mat4Scale(300, 300, 1);
        UpdateConstBuffer(&constBuffer, (void *)&cbuffer);
        deviceContext->Draw(vertexBuffer.verticesCount, 0);


        // reset proj matrix
        cbuffer.proj = proj;
        
        // reset the viewport
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = (f32)lastWidth;
        viewport.Height = (f32)lastHeight;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        deviceContext->RSSetViewports(1, &viewport);

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!", 0, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoCollapse); // Create a window called "Hello, world!" and append into it.

            // TODO: try change size and position
            ImGui::SetWindowSize(ImVec2(fixWidth, lastHeight));
            ImGui::SetWindowPos(ImVec2(0, 0));

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        ImGui::Render();
        
        //  render to main render target output
        deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
        
        deviceContext->VSSetShader(texShader.vertex, 0, 0);
        deviceContext->PSSetShader(texShader.fragment, 0, 0);
        
        
        float clearColor1[] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        deviceContext->ClearRenderTargetView(renderTargetView, clearColor1);
        deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

        deviceContext->PSSetShaderResources(0, 1, &frameBuffer0.shaderResourceView);
        cbuffer.world = Mat4Translate(clientRect.x + frameBuffer0.x,
                                      clientRect.y + frameBuffer0.y, 0) * Mat4Scale(frameBuffer0.width, frameBuffer0.height, 1);
        UpdateConstBuffer(&constBuffer, (void *)&cbuffer);
        deviceContext->Draw(vertexBuffer.verticesCount, 0);

        deviceContext->PSSetShaderResources(0, 1, &frameBuffer1.shaderResourceView);
        cbuffer.world = Mat4Translate(clientRect.x + frameBuffer1.x,
                                      clientRect.y + frameBuffer1.y, 0) * Mat4Scale(frameBuffer0.width, frameBuffer0.height, 1);
        UpdateConstBuffer(&constBuffer, (void *)&cbuffer);
        deviceContext->Draw(vertexBuffer.verticesCount, 0);

        deviceContext->PSSetShaderResources(0, 1, &frameBuffer2.shaderResourceView);
        cbuffer.world = Mat4Translate(clientRect.x + frameBuffer2.x,
                                      clientRect.y + frameBuffer2.y, 0) * Mat4Scale(frameBuffer0.width, frameBuffer0.height, 1);
        UpdateConstBuffer(&constBuffer, (void *)&cbuffer);
        deviceContext->Draw(vertexBuffer.verticesCount, 0);

        deviceContext->PSSetShaderResources(0, 1, &frameBuffer3.shaderResourceView);
        cbuffer.world = Mat4Translate(clientRect.x + frameBuffer3.x,
                                      clientRect.y + frameBuffer3.y, 0) * Mat4Scale(frameBuffer0.width, frameBuffer0.height, 1);
        UpdateConstBuffer(&constBuffer, (void *)&cbuffer);
        deviceContext->Draw(vertexBuffer.verticesCount, 0);
        
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        
        swapChain->Present(1, 0);

    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    UnloadVertexBuffer(&vertexBuffer);
    UnloadShader(&colShader);
    UnloadShader(&texShader);
    UnloadConstBuffer(&constBuffer);
    UnloadFrameBuffer(&frameBuffer0);
    UnloadFrameBuffer(&frameBuffer1);
    UnloadFrameBuffer(&frameBuffer2);
    UnloadFrameBuffer(&frameBuffer3);

    if(layout) layout->Release();
    if(samplerState) samplerState->Release();

    ShutdownD3D11();

    return 0;
}

