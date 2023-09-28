#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include <windows.h>
#include <windowsX.h>
#include <dwmapi.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include "defines.h"
#include "darray.h"
#include "math.h"
#include "types.h"

// gobals to handle general app and window state
static bool gRunning;
static i32 gWindowX;
static i32 gWindowY;
static u32 gResizeWidth;
static u32 gResizeHeight;
static f32 gCurrentWindowWidth = WINDOW_WIDTH;
static f32 gCurrentWindowHeight = WINDOW_HEIGHT;
static f32 gFixWidth = 200.0f;
static f32 gUnitSize = 64.0f;

// global variables for use during rendering
static Shader gColShader;
static Shader gTexShader;
static ConstBuffer gConstBuffer;
static VertexBuffer gVertexBuffer;
static DynamicVertexBuffer gDynamicVertexBuffer;
static Vertex gQuad[] = {
    // Face 1
    {{-0.5f, -0.5f, 0}, {0, 0, 1}, {0.8, 0.8, 0.8, 1}, {0, 1}},
    {{-0.5f,  0.5f, 0}, {0, 0, 1}, {0.8, 0.8, 0.8, 1}, {0, 0}},
    {{ 0.5f, -0.5f, 0}, {0, 0, 1}, {0.8, 0.8, 0.8, 1}, {1, 1}},
    // Face 2
    {{ 0.5f, -0.5f, 0}, {0, 0, 1}, {0.8, 0.8, 0.8, 1}, {1, 1}},
    {{-0.5f,  0.5f, 0}, {0, 0, 1}, {0.8, 0.8, 0.8, 1}, {0, 0}},
    {{ 0.5f,  0.5f, 0}, {0, 0, 1}, {0.8, 0.8, 0.8, 1}, {1, 0}}
};


#include "darray.cpp"
#include "input.cpp"
#include "win32.cpp"
#include "line.cpp"
#include "view.cpp"

#include "mainView.cpp"
#include "frontView.cpp"
#include "topView.cpp"
#include "sideView.cpp"

#include "viewManager.cpp"

#include "gui.cpp"

void ProcessWindowResize(ViewManager *vm, CBuffer *cbuffer, f32 &clientWidth, Rect &clientRect, Rect &uiRect)
{
    if(gResizeWidth != 0 && gResizeHeight != 0)
    {
        ResizeD3D11();
        cbuffer->proj = Mat4Ortho(0.0f, (f32)gResizeWidth, 0.0f, (f32)gResizeHeight, 0.0f, 100.0f);
        UpdateConstBuffer(&gConstBuffer, (void *)cbuffer);
       
        clientWidth = (gResizeWidth - gFixWidth);
        if(clientWidth > 0)
        {
            uiRect = {0, 0, gFixWidth, (f32)gResizeHeight};
            clientRect = {gFixWidth, 0, clientWidth, (f32)gResizeHeight};
            RecalculateViewDimensions(vm, clientRect);
        }

        gCurrentWindowWidth = gResizeWidth;
        gCurrentWindowHeight = gResizeHeight;
        gResizeWidth = gResizeHeight = 0;
    } 
}

int main()
{
    HINSTANCE instace = GetModuleHandle(0);
    HWND window = InitWindow(instace);
    InitD3D11(window);
    LineRendererInitialize(200);
    InitImGui(window);

    // Load Shader and Create Input Layout
    gColShader = LoadShader("../src/shaders/colVert.hlsl", "../src/shaders/colFrag.hlsl");
    gTexShader = LoadShader("../src/shaders/texVert.hlsl", "../src/shaders/texFrag.hlsl");
    
    ID3D11InputLayout *layout = 0;
    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
         0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,
         0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
         0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
         0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
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
    f32 clientWidth = (WINDOW_WIDTH - gFixWidth);
    Rect uiRect = {0, 0, gFixWidth, WINDOW_HEIGHT};
    Rect clientRect = {gFixWidth, 0, clientWidth, WINDOW_HEIGHT};
    
    ViewManager vm; // NOT initialize to zero (default values are used)
    ViewManagerSetup(&vm, clientRect);

    // create the const buffer data to be pass to the gpu
    Vec3 up  = {0, 1,  0};
    Vec3 pos = {0, 0, -50};
    Vec3 tar = {0, 0, 0};
    CBuffer cbuffer{};
    cbuffer.view = Mat4LookAt(pos, tar, up);
    cbuffer.world = Mat4Identity();
    cbuffer.proj = Mat4Ortho(0.0f, (f32)WINDOW_WIDTH, 0.0f, (f32)WINDOW_HEIGHT, 0.0f, 100.0f);
    cbuffer.viewPos = {};
    gConstBuffer = LoadConstBuffer((void *)&cbuffer, sizeof(CBuffer), 0);

    ShowWindow(window, SW_MAXIMIZE);

    gRunning = true;
    while(gRunning)
    {
        // Handle Input and Events
        InputPrepareForFrame();
        FlushEvents(window);
        ViewManagerHandleInput(&vm, clientRect);
        ProcessWindowResize(&vm, &cbuffer, clientWidth, clientRect, uiRect);

        // Render To Views
        ViewManagerProcessAndRenderToViews(&vm);
        // Render View To Main Back Buffer
        ViewManagerRenderViews(&vm, &cbuffer, clientRect);

        // Render GUI
        RenderImGui();
        PresentImGui();
        
        // Swap main Buffer
        swapChain->Present(1, 0);

        gLastInput = gInput;
    }

    ShutDownImGui();

    UnloadVertexBuffer(&gVertexBuffer);
    UnloadDynamicVertexBuffer(&gDynamicVertexBuffer);
    UnloadShader(&gColShader);
    UnloadShader(&gTexShader);
    UnloadConstBuffer(&gConstBuffer);
    
    ViewManagerShutDown(&vm);
 
    if(layout) layout->Release();

    LineRendererShutdown();
    ShutdownD3D11();

    return 0;
}

