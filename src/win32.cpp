// global variables to handle D3D11 state
static ID3D11Device* device;
static ID3D11DeviceContext* deviceContext;
static IDXGISwapChain* swapChain;
static ID3D11RenderTargetView* renderTargetView;
static ID3D11DepthStencilView* depthStencilView;
static ID3D11RasterizerState* wireFrameRasterizer;
static ID3D11RasterizerState* fillRasterizerCullBack;
static ID3D11RasterizerState* fillRasterizerCullFront;
static ID3D11RasterizerState* fillRasterizerCullNone;
static ID3D11DepthStencilState* depthStencilOn;
static ID3D11DepthStencilState* depthStencilOff;
static ID3D11BlendState* alphaBlendEnable;
static ID3D11BlendState* alphaBlendDisable;

static ID3D11SamplerState *gSamplerState;

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK WndProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
        return true;

    LRESULT result = 0;
    switch(Msg)
    {

    case WM_CLOSE:
    {
        gRunning = false;
    } break;

    case WM_MOVE:
    {
        gWindowX = (i32)LOWORD(lParam);
        gWindowY = (i32)HIWORD(lParam);
    } break;

    case WM_SIZE:
    {
        if (wParam == SIZE_MINIMIZED)
            return 0;
        gResizeWidth = (u32)LOWORD(lParam); // Queue resize
        gResizeHeight = (u32)HIWORD(lParam);
    } break;

    default:
    {
        result = DefWindowProcA(hWnd, Msg, wParam, lParam);
    } break;

    }

    return result;
}

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

static HWND InitWindow(HINSTANCE instace)
{
    // Create Win32 Window
    WNDCLASSA wndclass;
    wndclass.style = CS_HREDRAW|CS_VREDRAW;
    wndclass.lpfnWndProc = WndProcA;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = instace;
    wndclass.hIcon = 0;
    wndclass.hCursor = 0;
    wndclass.hbrBackground = 0;
    wndclass.lpszMenuName = 0;
    wndclass.lpszClassName = WINDOW_TITLE;
    RegisterClassA(&wndclass);
    
    RECT wr;
    wr.left = 0;
    wr.right = WINDOW_WIDTH;
    wr.top = 0;
    wr.bottom = WINDOW_HEIGHT;
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, 0);

    HWND window = CreateWindowExA(
        0, WINDOW_TITLE, WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left,
        wr.bottom - wr.top,
        0, 0, instace, 0);

    BOOL value = TRUE;
    ::DwmSetWindowAttribute(window, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

    return window;
}

static void InitD3D11(HWND window)
{
    // Init directx 11
    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_SOFTWARE
    };
    i32 driverTypesCount = ARRAY_LENGTH(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] = 
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    i32 featureLevelsCount = ARRAY_LENGTH(featureLevels);

    // create the d3d11 device swapchain and device context
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    memset(&swapChainDesc, 0, sizeof(DXGI_SWAP_CHAIN_DESC));
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = WINDOW_WIDTH;
    swapChainDesc.BufferDesc.Height = WINDOW_HEIGHT;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = window;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_DRIVER_TYPE driverType;
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT result = 0;

    for(u32 driver = 0; driver < driverTypesCount; ++driver)
    {
        result = D3D11CreateDeviceAndSwapChain(0, driverTypes[driver], 0, 0, 
                                               featureLevels, featureLevelsCount,
                                               D3D11_SDK_VERSION,
                                               &swapChainDesc, &swapChain,
                                               &device, &featureLevel,
                                               &deviceContext);
        if(SUCCEEDED(result))
        {
            driverType = driverTypes[driver];
            break;
        }
    }
    
    // create render target view
    ID3D11Texture2D *backBufferTexture = 0;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&backBufferTexture);
    device->CreateRenderTargetView(backBufferTexture, 0, &renderTargetView);
    if(backBufferTexture)
    {
        backBufferTexture->Release();
    }

    // set up the viewport
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = (f32)WINDOW_WIDTH;
    viewport.Height = (f32)WINDOW_HEIGHT;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);

    // create the depth stencil texture
    ID3D11Texture2D* depthStencilTexture = 0;
    D3D11_TEXTURE2D_DESC depthStencilTextureDesc;
    depthStencilTextureDesc.Width = WINDOW_WIDTH;
    depthStencilTextureDesc.Height = WINDOW_HEIGHT;
    depthStencilTextureDesc.MipLevels = 1;
    depthStencilTextureDesc.ArraySize = 1;
    depthStencilTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilTextureDesc.SampleDesc.Count = 1;
    depthStencilTextureDesc.SampleDesc.Quality = 0;
    depthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilTextureDesc.CPUAccessFlags = 0;
    depthStencilTextureDesc.MiscFlags = 0;
 
    // create the depth stencil view
    result = device->CreateTexture2D(&depthStencilTextureDesc, NULL, &depthStencilTexture);
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    result = device->CreateDepthStencilView(depthStencilTexture, &descDSV, &depthStencilView);
    if (depthStencilTexture)
    {
        depthStencilTexture->Release();
    }

    // create depth stencil states
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    // Depth test parameters
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    // Stencil test parameters
    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;
    // Stencil operations if pixel is front-facing
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    // Stencil operations if pixel is back-facing
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    device->CreateDepthStencilState(&depthStencilDesc, &depthStencilOn);
    depthStencilDesc.DepthEnable = false;
    depthStencilDesc.StencilEnable = false;
    device->CreateDepthStencilState(&depthStencilDesc, &depthStencilOff);

    // Alpha blending
    D3D11_BLEND_DESC blendStateDesc = {};
    blendStateDesc.RenderTarget[0].BlendEnable = true;
    blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device->CreateBlendState(&blendStateDesc, &alphaBlendEnable);

    blendStateDesc.RenderTarget[0].BlendEnable = false;
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device->CreateBlendState(&blendStateDesc, &alphaBlendDisable);

    // Create Rasterizers Types
    D3D11_RASTERIZER_DESC fillRasterizerFrontDesc = {};
    fillRasterizerFrontDesc.FillMode = D3D11_FILL_SOLID;
    fillRasterizerFrontDesc.CullMode = D3D11_CULL_FRONT;
    fillRasterizerFrontDesc.DepthClipEnable = true;
    device->CreateRasterizerState(&fillRasterizerFrontDesc, &fillRasterizerCullFront);

    D3D11_RASTERIZER_DESC fillRasterizerBackDesc = {};
    fillRasterizerBackDesc.FillMode = D3D11_FILL_SOLID;
    fillRasterizerBackDesc.CullMode = D3D11_CULL_BACK;
    fillRasterizerBackDesc.DepthClipEnable = true;
    device->CreateRasterizerState(&fillRasterizerBackDesc, &fillRasterizerCullBack);

    D3D11_RASTERIZER_DESC fillRasterizerNoneDesc = {};
    fillRasterizerNoneDesc.FillMode = D3D11_FILL_SOLID;
    fillRasterizerNoneDesc.CullMode = D3D11_CULL_NONE;
    fillRasterizerNoneDesc.DepthClipEnable = true;
    device->CreateRasterizerState(&fillRasterizerNoneDesc, &fillRasterizerCullNone);

    D3D11_RASTERIZER_DESC wireFrameRasterizerDesc = {};
    wireFrameRasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
    wireFrameRasterizerDesc.CullMode = D3D11_CULL_NONE;
    wireFrameRasterizerDesc.DepthClipEnable = true;
    device->CreateRasterizerState(&wireFrameRasterizerDesc, &wireFrameRasterizer);

    // Create Sampler State
    D3D11_SAMPLER_DESC colorMapDesc = {};
    colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP; //D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP; //D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP; //D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; //D3D11_FILTER_MIN_MAG_MIP_LINEAR | D3D11_FILTER_MIN_MAG_MIP_POINT
    colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;
    if(FAILED(device->CreateSamplerState(&colorMapDesc, &gSamplerState)))
    {
        printf("Error: Failed Creating sampler state\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
    deviceContext->OMSetDepthStencilState(depthStencilOn, 1);
    deviceContext->OMSetBlendState(alphaBlendEnable, 0, 0xffffffff);
    deviceContext->RSSetState(fillRasterizerCullNone);
    deviceContext->PSSetSamplers(0, 1, &gSamplerState);
}

static void ShutdownD3D11()
{
    if(device) device->Release();
    if(deviceContext) deviceContext->Release();
    if(swapChain) swapChain->Release();
    if(renderTargetView) renderTargetView->Release();
    if(depthStencilView) depthStencilView->Release();

    if(wireFrameRasterizer) wireFrameRasterizer->Release();
    if(fillRasterizerCullBack) fillRasterizerCullBack->Release();
    if(fillRasterizerCullFront) fillRasterizerCullFront->Release();
    if(fillRasterizerCullNone) fillRasterizerCullNone->Release();
    if(depthStencilOn) depthStencilOn->Release();
    if(depthStencilOff) depthStencilOff->Release();
    if(alphaBlendEnable) alphaBlendEnable->Release();
    if(alphaBlendDisable) alphaBlendDisable->Release();
    if(gSamplerState) gSamplerState->Release();
}

static void FlushEvents(HWND window)
{
    MSG msg;
    while(PeekMessageA(&msg, window, 0, 0, PM_REMOVE) != 0)
    {
        switch(msg.message)
        {
            // TODO: handle important messages
            case WM_MOUSEMOVE:
            {
                gInput.x = (i32)GET_X_LPARAM(msg.lParam);
                gInput.y = (i32)GET_Y_LPARAM(msg.lParam);
            } break;
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            {
                gInput.buttons[0].down = ((msg.wParam & MK_LBUTTON) != 0); 
                gInput.buttons[1].down = ((msg.wParam & MK_MBUTTON) != 0); 
                gInput.buttons[2].down = ((msg.wParam & MK_RBUTTON) != 0); 
            } break;
            case WM_MOUSEWHEEL:
            {
                gInput.wheelDelta = GET_WHEEL_DELTA_WPARAM(msg.wParam) / 120;
            } break;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

static void ResizeD3D11()
{
    if(renderTargetView) renderTargetView->Release(); renderTargetView = 0;
    if(depthStencilView) depthStencilView->Release(); depthStencilView = 0;
    
    swapChain->ResizeBuffers(0, gResizeWidth, gResizeHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    
    // re create render target view
    ID3D11Texture2D *backBufferTexture = 0;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&backBufferTexture);
    device->CreateRenderTargetView(backBufferTexture, 0, &renderTargetView);
    if(backBufferTexture)
    {
        backBufferTexture->Release();
    }
    // create the depth stencil texture
    ID3D11Texture2D* depthStencilTexture = 0;
    D3D11_TEXTURE2D_DESC depthStencilTextureDesc;
    depthStencilTextureDesc.Width = gResizeWidth;
    depthStencilTextureDesc.Height = gResizeHeight;
    depthStencilTextureDesc.MipLevels = 1;
    depthStencilTextureDesc.ArraySize = 1;
    depthStencilTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilTextureDesc.SampleDesc.Count = 1;
    depthStencilTextureDesc.SampleDesc.Quality = 0;
    depthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilTextureDesc.CPUAccessFlags = 0;
    depthStencilTextureDesc.MiscFlags = 0;
 
    // create the depth stencil view
    device->CreateTexture2D(&depthStencilTextureDesc, NULL, &depthStencilTexture);
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    device->CreateDepthStencilView(depthStencilTexture, &descDSV, &depthStencilView);
    if (depthStencilTexture)
    {
        depthStencilTexture->Release();
    }

    deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

    // set up the viewport
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = (f32)gResizeWidth;
    viewport.Height = (f32)gResizeHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);
}

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

DynamicVertexBuffer LoadDynamicVertexBuffer(size_t size, ID3D11InputLayout *layout)
{
    DynamicVertexBuffer buffer = {};
    buffer.size = size;
    buffer.used = 0;
    buffer.verticesCount = 0;
    buffer.layout = layout;

    D3D11_BUFFER_DESC vertexDesc;
    ZeroMemory(&vertexDesc, sizeof(vertexDesc));
    vertexDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexDesc.ByteWidth = size;

    HRESULT result = device->CreateBuffer(&vertexDesc, NULL, &buffer.GPUBuffer);
    buffer.CPUBuffer = (Vertex *)malloc(size);

    return buffer;
}

void PushToGPUDynamicVertexBuffer(DynamicVertexBuffer *vertexBuffer)
{
    D3D11_MAPPED_SUBRESOURCE bufferData;
    ZeroMemory(&bufferData, sizeof(bufferData));
    deviceContext->Map(vertexBuffer->GPUBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
    memcpy(bufferData.pData, vertexBuffer->CPUBuffer, vertexBuffer->used);
    deviceContext->Unmap(vertexBuffer->GPUBuffer, 0);
}

void UnloadDynamicVertexBuffer(DynamicVertexBuffer *vertexBuffer)
{
    if(vertexBuffer->GPUBuffer) vertexBuffer->GPUBuffer->Release();
    if(vertexBuffer->CPUBuffer) free(vertexBuffer->CPUBuffer);
    vertexBuffer->verticesCount = 0;
    vertexBuffer->size = 0;
    vertexBuffer->used = 0;
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

    // create the depth stencil texture
    ID3D11Texture2D* depthStencilTexture = 0;
    D3D11_TEXTURE2D_DESC depthStencilTextureDesc;
    depthStencilTextureDesc.Width = width;
    depthStencilTextureDesc.Height = height;
    depthStencilTextureDesc.MipLevels = 1;
    depthStencilTextureDesc.ArraySize = 1;
    depthStencilTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilTextureDesc.SampleDesc.Count = 1;
    depthStencilTextureDesc.SampleDesc.Quality = 0;
    depthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilTextureDesc.CPUAccessFlags = 0;
    depthStencilTextureDesc.MiscFlags = 0;
 
    // create the depth stencil view
    if(FAILED(device->CreateTexture2D(&depthStencilTextureDesc, NULL, &depthStencilTexture)))
    {
        printf("Error creating FrameBuffer depthStencilTexture\n");
        ASSERT(!"INVALID_CODE_PATH");
    }
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    if(FAILED(device->CreateDepthStencilView(depthStencilTexture, &descDSV, &frameBuffer.depthStencilView)))
    {
        printf("Error creating FrameBuffer dsv\n");
        ASSERT(!"INVALID_CODE_PATH");
    }
    if (depthStencilTexture)
    {
        depthStencilTexture->Release();
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

void LoadTextureAtlasGpuData(TextureAtlas *atlas)
{
    // Create the GPU stuff
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = atlas->w;
    texDesc.Height = atlas->h;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DYNAMIC;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    texDesc.MiscFlags = 0;
    if(FAILED(device->CreateTexture2D(&texDesc, 0, &atlas->gpuPixels)))
    {
        printf("Error creating Texture Atlas GPU Texture\n");
        ASSERT(!"INVALID_CODE_PATH");
    }

    // create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    if (FAILED(device->CreateShaderResourceView(atlas->gpuPixels, &srvDesc, &atlas->srv)))
    {
        printf("Error creating Atlas srv\n");
        ASSERT(!"INVALID_CODE_PATH");
    }
}

void UnloadTextureAtlasGpuData(TextureAtlas *atlas)
{
    if(atlas->gpuPixels) atlas->gpuPixels->Release(); atlas->gpuPixels = 0;
    if(atlas->srv) atlas->srv->Release(); atlas->srv = 0;
}

TextureAtlas LoadTextureAtlas(i32 width, i32 height)
{
    TextureAtlas atlas = {};

    // Create the CPU stuff
    atlas.w = width;
    atlas.h = height;
    atlas.textures = 0;
    atlas.cpuPixels = (u32 *)malloc(sizeof(u32)*width*height);

    LoadTextureAtlasGpuData(&atlas);

    return atlas;
}

void UnloadTextureAtlas(TextureAtlas *atlas)
{
    if(atlas->cpuPixels) free(atlas->cpuPixels);
    UnloadTextureAtlasGpuData(atlas);

}

void SortTextureByHeightInTextuerAtlas(TextureAtlas *atlas)
{
    bool swapped;
    for(i32 i = 0; i < DarraySize(atlas->textures) - 1; ++i)
    {
        swapped = false;
        for(i32 j = 0; j < DarraySize(atlas->textures) - i - 1; ++j)
        {
            Texture a = atlas->textures[j + 0];
            Texture b = atlas->textures[j + 1];
            if(a.h < b.h)
            {
                Texture tmp = a;
                a = b;
                b = tmp;
                swapped = true;
            }
        }

        if(swapped == false)
            break;
    }
}

void CopyTextureToAtlas(u32 *dst, i32 xDst, i32 yDst, i32 pDst,
                        u32 *src, i32 xSrc, i32 ySrc, i32 pSrc, i32 wSrc, i32 hSrc)
{
    for(i32 y = 0; y < hSrc; ++y)
    {
        for(i32 x = 0; x < wSrc; ++x)
        {
            dst[(yDst + y) * pDst + (xDst + x)] = src[(ySrc + y) * pSrc + (xSrc + x)];
        }
    }
}

void CopyTextureToAtlas(u32 *dst, i32 xDst, i32 yDst, i32 pDst,
                        u32 *src, i32 pSrc, i32 wSrc, i32 hSrc)
{
    for(i32 y = 0; y < hSrc; ++y)
    {
        for(i32 x = 0; x < wSrc; ++x)
        {
            dst[(yDst + y) * pDst + (xDst + x)] = src[y * pSrc + x];
        }
    }
}

void AddTextureToTextureAtlas(TextureAtlas *atlas, char *filepath)
{
    stbi_set_flip_vertically_on_load(false);
    // fisrt load the new texture to be added to the atlas
    i32 width, height, nrComponents;
    u8 *pixels = stbi_load(filepath, &width, &height, &nrComponents, 0);

    if(pixels == 0)
    {
        printf("Error loading texture: %s\n", filepath);
        ASSERT(!"INVALID_CODE_PATH");
    }

    Texture newTexture = {};
    newTexture.x = -1;
    newTexture.y = -1;
    newTexture.w = width;
    newTexture.h = height;
    newTexture.pich = width;
    newTexture.pixels = (u32 *)pixels;
    newTexture.lastAdded = true;
   
    DarrayPush(atlas->textures, newTexture, Texture);

    SortTextureByHeightInTextuerAtlas(atlas);


    // update texture atlas
    // create a copy of the old pixel we need to save the pixels value to copy them
    u32 *tmpPixels = (u32 *)malloc(sizeof(u32)*atlas->w*atlas->h);
    memset(tmpPixels, 0, sizeof(u32)*atlas->w*atlas->h);
    //memcpy(tmpPixels, atlas->cpuPixels, sizeof(u32)*atlas->w*atlas->h);

    i32 oldAtlasW = atlas->w;
    i32 oldAtlasH = atlas->h;

    // for each texture in the atlas we copy it to the new position
    // after the sort
    i32 xPos = 0; // start at one because of the offset
    i32 yPos = 0;
    i32 yOffset = atlas->textures[0].h;
    for(i32 i = 0; i < DarraySize(atlas->textures); ++i)
    {
        Texture *texture = atlas->textures + i;
        i32 xOffset = texture->w;

        if(xPos + xOffset <= atlas->w)
        {
            if(texture->lastAdded)
                CopyTextureToAtlas(tmpPixels, xPos, yPos, atlas->w, 
                                   (u32 *)pixels, texture->pich, texture->w, texture->h); 
            else
                CopyTextureToAtlas(tmpPixels, xPos, yPos, atlas->w,
                                   atlas->cpuPixels, texture->x, texture->y, texture->pich, texture->w, texture->h); 

            texture->x = xPos;
            texture->y = yPos;
            texture->pich = atlas->w;
            texture->pixels = tmpPixels + yPos * atlas->w + xPos;

            xPos += xOffset;
        }
        else
        {
            xPos = 0;
            yPos += yOffset;
            yOffset = texture->h;

            if(yPos + texture->h <= atlas->h)
            {
                if(texture->lastAdded)
                    CopyTextureToAtlas(tmpPixels, xPos, yPos, atlas->w, 
                                       (u32 *)pixels, texture->pich, texture->w, texture->h); 
                else
                    CopyTextureToAtlas(tmpPixels, xPos, yPos, atlas->w,
                                       atlas->cpuPixels, texture->x, texture->y, texture->pich, texture->w, texture->h); 

                texture->x = xPos;
                texture->y = yPos;
                texture->pich = atlas->w;
                texture->pixels = tmpPixels + yPos * atlas->w + xPos;

                xPos += xOffset;
            }
            else
            {
                // resize the buffer
                tmpPixels = (u32 *)realloc(tmpPixels, sizeof(u32)*(atlas->w*2)*(atlas->h)*2);
                memset(tmpPixels, 0, sizeof(u32)*(atlas->w*2)*(atlas->h)*2);
                // restart the copy
                xPos = 0;
                yPos = 0;
                yOffset = atlas->textures[0].h;
                i =  -1;
                atlas->w = atlas->w*2;
                atlas->h = atlas->h*2;

            }
        }  
    }

    stbi_image_free(pixels);
    free(atlas->cpuPixels);
    atlas->cpuPixels = tmpPixels;

    // chack if the atlas change size
    if(atlas->w != oldAtlasW || atlas->h != oldAtlasH)
    {
        // if change size reacreate the GPU pixels 
        UnloadTextureAtlasGpuData(atlas);
        LoadTextureAtlasGpuData(atlas);
        printf("Texture Atlas Reallocated!!!\n");
    }

    // copy the pixels from the cpu to the gpu
    D3D11_MAPPED_SUBRESOURCE bufferData;
    ZeroMemory(&bufferData, sizeof(bufferData));
    deviceContext->Map(atlas->gpuPixels, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
    memcpy(bufferData.pData, atlas->cpuPixels, sizeof(u32)*atlas->w*atlas->h);
    deviceContext->Unmap(atlas->gpuPixels, 0);

    for(i32 i = 0; i < DarraySize(atlas->textures); ++i)
    {
        Texture *texture = atlas->textures + i;
        texture->lastAdded = false;
    }
}

