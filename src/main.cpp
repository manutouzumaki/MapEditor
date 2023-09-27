
#include <stdio.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include <windows.h>
#include <windowsX.h>
#include <dwmapi.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include "defines.h"
#include "math.h"
#include "types.h"

// TODO: see is there are a better way to handle this globas

// gobals to handle general app and window state
static bool gRunning;
static i32 gWindowX;
static i32 gWindowY;
static u32 gResizeWidth;
static u32 gResizeHeight;
static f32 gCurrentWindowWidth = WINDOW_WIDTH;
static f32 gCurrentWindowHeight = WINDOW_HEIGHT;

// gobals to handle app view distibution state
static f32 gDivisorX = 0.5f;
static f32 gDivisorY = 0.5f;
static f32 gNewDivisorX;
static f32 gNewDivisorY;
static f32 gSeparation = 6;
static bool gModifyViewsX;
static bool gModifyViewsY;

// global variables for use during rendering
static Shader gColShader;
static Shader gTexShader;
static ConstBuffer gConstBuffer;
static VertexBuffer gVertexBuffer;
static DynamicVertexBuffer gDynamicVertexBuffer;
static Vertex gQuad[] = {
    // Face 1
    {{-0.5f, -0.5f, 0}, {0.8, 0.8, 0.8, 1}, {0, 1}},
    {{-0.5f,  0.5f, 0}, {0.8, 0.8, 0.8, 1}, {0, 0}},
    {{ 0.5f, -0.5f, 0}, {0.8, 0.8, 0.8, 1}, {1, 1}},
    // Face 2
    {{ 0.5f, -0.5f, 0}, {0.8, 0.8, 0.8, 1}, {1, 1}},
    {{-0.5f,  0.5f, 0}, {0.8, 0.8, 0.8, 1}, {0, 0}},
    {{ 0.5f,  0.5f, 0}, {0.8, 0.8, 0.8, 1}, {1, 0}}
};

#include "input.cpp"
#include "win32.cpp"
#include "line.cpp"
#include "view.cpp"

#include "mainView.cpp"
#include "frontView.cpp"
#include "topView.cpp"
#include "sideView.cpp"

bool MouseInDivisorX(Rect clientRect, f32 fixWidth)
{
    // handle the vertical line
    f32 xpos = clientRect.w * gDivisorX;
    RectMinMax vRect = {xpos-gSeparation*0.5f, 0, xpos+gSeparation*0.5f, clientRect.h};

    i32 mouseRelX = MouseX() - fixWidth;
    i32 mouseRelY = MouseY();
    
    if(mouseRelX >= vRect.minX && mouseRelX <= vRect.maxX &&
       mouseRelY >= vRect.minY && mouseRelY <= vRect.maxY)
    {
        return true;
    }
    return false;
}

bool MouseInDivisorY(Rect clientRect, f32 fixWidth)
{
    // handle the horizontal line
    f32 ypos = clientRect.h * gDivisorY;
    RectMinMax vRect = {0, ypos-gSeparation*0.5f, clientRect.w, ypos+gSeparation*0.5f};

    i32 mouseRelX = MouseX() - fixWidth;
    i32 mouseRelY = MouseY();
    
    if(mouseRelX >= vRect.minX && mouseRelX <= vRect.maxX &&
       mouseRelY >= vRect.minY && mouseRelY <= vRect.maxY)
    {
        return true;
    }
    return false;
}

void RecalculateViewDimensions(View *views, Rect clientRect)
{
    f32 viewWidth  = clientRect.w * gDivisorX;
    f32 viewHeight = clientRect.h * gDivisorY;
    if(viewWidth-gSeparation > 1 && viewHeight-gSeparation > 1)
    {
        ViewResize(views + 0,
                   viewWidth*0.5f,
                   (clientRect.h*(1.0f-gDivisorY))+viewHeight*0.5f,
                   viewWidth-gSeparation, viewHeight-gSeparation);
    }

    viewWidth  = clientRect.w * (1.0f-gDivisorX);
    viewHeight = clientRect.h * gDivisorY;
    if(viewWidth-gSeparation > 1 && viewHeight-gSeparation > 1)
    {
        ViewResize(views + 1,
                   (clientRect.w*gDivisorX)+viewWidth*0.5f,
                   (clientRect.h*(1.0f-gDivisorY))+viewHeight*0.5f,
                   viewWidth-gSeparation, viewHeight-gSeparation);
    }

    viewWidth  = clientRect.w * gDivisorX;
    viewHeight = clientRect.h * (1.0f-gDivisorY);
    if(viewWidth-gSeparation > 1 && viewHeight-gSeparation > 1)
    {
        ViewResize(views + 2,
                   viewWidth*0.5f,
                   viewHeight*0.5f,
                   viewWidth-gSeparation, viewHeight-gSeparation);
    }

    viewWidth  = clientRect.w * (1.0f-gDivisorX);
    viewHeight = clientRect.h * (1.0f-gDivisorY);
    if(viewWidth-gSeparation > 1 && viewHeight-gSeparation > 1)
    {
        ViewResize(views + 3,
                   (clientRect.w*gDivisorX)+viewWidth*0.5f,
                   viewHeight*0.5f,
                   viewWidth-gSeparation, viewHeight-gSeparation);
    }
}

int main()
{
    HINSTANCE instace = GetModuleHandle(0);
    HWND window = InitWindow(instace);
    InitD3D11(window);
    LineRendererInitialize(200);

    // Set up Dear imgui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsClassic(); // StyleColorsDark StyleColorsLight StyleColorsClassic

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(device, deviceContext);

    bool show_demo_window = false;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(116.0f/255.0f, 116.0f/255.0f, 245.0f/255.0f, 1.00f);


    // Load Shader and Create Input Layout
    gColShader = LoadShader("../src/shaders/colVert.hlsl", "../src/shaders/colFrag.hlsl");
    gTexShader = LoadShader("../src/shaders/texVert.hlsl", "../src/shaders/texFrag.hlsl");
    
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
        gColShader.vertexShaderCompiled->GetBufferPointer(),
        gColShader.vertexShaderCompiled->GetBufferSize(),
        &layout);

    // Load Vertex Buffer
    gVertexBuffer = LoadVertexBuffer(gQuad, ARRAY_LENGTH(gQuad), layout);
    gDynamicVertexBuffer = LoadDynamicVertexBuffer(13770*sizeof(Vertex), layout); // TODO: set the size better

    // Load FrameBuffer
    f32 fixWidth = 200.0f;
    f32 clientWidth = (WINDOW_WIDTH - fixWidth);
    Rect uiRect = {0, 0, fixWidth, WINDOW_HEIGHT};
    Rect clientRect = {fixWidth, 0, clientWidth, WINDOW_HEIGHT};
    
    View views[4];

    f32 viewWidth  = clientRect.w * gDivisorX;
    f32 viewHeight = clientRect.h * gDivisorY;
    views[0] = ViewCreate(viewWidth*0.5f,
                          (clientRect.h*(1.0f-gDivisorY))+viewHeight*0.5f,
                          viewWidth-gSeparation, viewHeight-gSeparation,
                          PROJ_TYPE_PERSP,
                          SetupMainView, ProcessMainView, RenderMainView);

    viewWidth  = clientRect.w * (1.0f-gDivisorX);
    viewHeight = clientRect.h * gDivisorY;
    views[1] = ViewCreate((clientRect.w*gDivisorX)+viewWidth*0.5f,
                          (clientRect.h*(1.0f-gDivisorY))+viewHeight*0.5f,
                          viewWidth-gSeparation, viewHeight-gSeparation,
                          PROJ_TYPE_ORTHO,
                          SetupTopView, ProcessTopView, RenderTopView);

    viewWidth  = clientRect.w * gDivisorX;
    viewHeight = clientRect.h * (1.0f-gDivisorY);
    views[2] = ViewCreate(viewWidth*0.5f,
                          viewHeight*0.5f,
                          viewWidth-gSeparation, viewHeight-gSeparation,
                          PROJ_TYPE_ORTHO,
                          SetupFrontView, ProcessFrontView, RenderFrontView);

    viewWidth  = clientRect.w * (1.0f-gDivisorX);
    viewHeight = clientRect.h * (1.0f-gDivisorY);
    views[3] = ViewCreate((clientRect.w*gDivisorX)+viewWidth*0.5f,
                          viewHeight*0.5f,
                          viewWidth-gSeparation, viewHeight-gSeparation,
                          PROJ_TYPE_ORTHO,
                          SetupSideView, ProcessSideView, RenderSideView);

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
    Vec3 pos = {0, 0, -50};
    Vec3 tar = {0, 0, 0};
    CBuffer cbuffer{};
    cbuffer.view = Mat4LookAt(pos, tar, up);
    cbuffer.world = Mat4Identity();
    cbuffer.proj = Mat4Ortho(0.0f, (f32)WINDOW_WIDTH, 0.0f, (f32)WINDOW_HEIGHT, 0.0f, 100.0f);
    gConstBuffer = LoadConstBuffer((void *)&cbuffer, sizeof(CBuffer), 0);

    ShowWindow(window, SW_MAXIMIZE);

    for(i32 i = 0; i < 4; ++i)
    {
        ViewSetup(views + i);
    }

    gRunning = true;
    while(gRunning)
    {
        InputPrepareForFrame();
        FlushEvents(window);

        if(MouseJustDown(MOUSE_BUTTON_LEFT))
        {
            if(MouseInDivisorX(clientRect, fixWidth)) gModifyViewsX = true;
            if(MouseInDivisorY(clientRect, fixWidth)) gModifyViewsY = true;
        }

        if(MouseJustUp(MOUSE_BUTTON_LEFT))
        {
            if(gModifyViewsX) { gDivisorX = gNewDivisorX; gModifyViewsX = false; }
            if(gModifyViewsY) { gDivisorY = gNewDivisorY; gModifyViewsY = false; }
            RecalculateViewDimensions(views, clientRect);
        }

        if(gModifyViewsX)
        {
            i32 mouseRelX = MouseX() - fixWidth;
            gNewDivisorX = Clamp(mouseRelX / clientRect.w, 0.1f, 0.9f);
        }

        if(gModifyViewsY)
        {
            i32 mouseRelY = MouseY();
            gNewDivisorY = Clamp(mouseRelY / clientRect.h, 0.1f, 0.9f);
        }

        if(gResizeWidth != 0 && gResizeHeight != 0)
        {
            ResizeD3D11();
            cbuffer.proj = Mat4Ortho(0.0f, (f32)gResizeWidth, 0.0f, (f32)gResizeHeight, 0.0f, 100.0f);
            UpdateConstBuffer(&gConstBuffer, (void *)&cbuffer);
           
            clientWidth = (gResizeWidth - fixWidth);
            if(clientWidth > 0)
            {
                uiRect = {0, 0, fixWidth, (f32)gResizeHeight};
                clientRect = {fixWidth, 0, clientWidth, (f32)gResizeHeight};
                RecalculateViewDimensions(views, clientRect);
            }

            gCurrentWindowWidth = gResizeWidth;
            gCurrentWindowHeight = gResizeHeight;
            gResizeWidth = gResizeHeight = 0;
        }        

        for(i32 i = 0; i < 4; ++i)
        {
            ViewProcess(views + i);
            ViewRender(views + i);
        }

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

            ImGui::SetWindowSize(ImVec2(fixWidth, gCurrentWindowHeight));
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
        u32 stride = sizeof(Vertex);
        u32 offset = 0;
        deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        deviceContext->IASetInputLayout(gVertexBuffer.layout);
        deviceContext->IASetVertexBuffers(0, 1, &gVertexBuffer.GPUBuffer, &stride, &offset);
        deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
        
        float clearColor[] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        deviceContext->ClearRenderTargetView(renderTargetView, clearColor);
        deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

        deviceContext->VSSetShader(gTexShader.vertex, 0, 0);
        deviceContext->PSSetShader(gTexShader.fragment, 0, 0);

        // Render the for Views
        for(i32 i = 0; i < 4; ++i)
        {
            deviceContext->PSSetShaderResources(0, 1, &views[i].fb.shaderResourceView);
            cbuffer.world = Mat4Translate(clientRect.x + views[i].x,
                                          clientRect.y + views[i].y, 1) * Mat4Scale(views[i].w, views[i].h, 1);
            UpdateConstBuffer(&gConstBuffer, (void *)&cbuffer);
            deviceContext->Draw(gVertexBuffer.verticesCount, 0);
        }

        if(gModifyViewsX)
        {
            // draw the mouse position 
            deviceContext->VSSetShader(gColShader.vertex, 0, 0);
            deviceContext->PSSetShader(gColShader.fragment, 0, 0);

            // Horizontal line
            cbuffer.world = Mat4Translate(clientRect.x + clientRect.w * gNewDivisorX,
                                          clientRect.y + clientRect.h*0.5f, 0.0f) * Mat4Scale(3, clientRect.h, 1);
            UpdateConstBuffer(&gConstBuffer, (void *)&cbuffer);
            deviceContext->Draw(gVertexBuffer.verticesCount, 0);
        }
        if(gModifyViewsY)
        {
            // draw the mouse position 
            deviceContext->VSSetShader(gColShader.vertex, 0, 0);
            deviceContext->PSSetShader(gColShader.fragment, 0, 0);
            // Vertical line
            cbuffer.world = Mat4Translate(clientRect.x + clientRect.w*0.5f,
                                          clientRect.y + clientRect.h * (1.0f-gNewDivisorY), 0.0f) * Mat4Scale(clientRect.w, 3, 1);
            UpdateConstBuffer(&gConstBuffer, (void *)&cbuffer);
            deviceContext->Draw(gVertexBuffer.verticesCount, 0);
        }

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        
        swapChain->Present(1, 0);

    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    UnloadVertexBuffer(&gVertexBuffer);
    UnloadDynamicVertexBuffer(&gDynamicVertexBuffer);
    UnloadShader(&gColShader);
    UnloadShader(&gTexShader);
    UnloadConstBuffer(&gConstBuffer);
    
    for(i32 i = 0; i < 4; ++i)
    {
        ViewDestroy(views + i);
    }
 
    if(layout) layout->Release();
    if(samplerState) samplerState->Release();

    LineRendererShutdown();
    ShutdownD3D11();

    return 0;
}

