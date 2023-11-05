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

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "defines.h"
#include "darray.h"
#include "math.h"
#include "types.h"
#include "view.h"
#include "brush.h"
#include "entity.h"
#include "editorModes.h"

// gobals to handle general app and window state
static bool gRunning;
static i32 gWindowX;
static i32 gWindowY;
static u32 gResizeWidth;
static u32 gResizeHeight;
static f32 gCurrentWindowWidth = WINDOW_WIDTH;
static f32 gCurrentWindowHeight = WINDOW_HEIGHT;

// TODO: this cant be change by the user
static f32 gFixWidth = 200.0f;
static f32 gUnitSize = 64.0f;
static f32 gGridSize = 200.0f;
static f32 g3DScale = 64.0f;

// global variables for use during rendering
static Shader gColShader;
static Shader gCol2DShader;
static Shader gTexShader;
static Shader gTex2DShader;

static bool gDirtyFlag;
static ConstBuffer gConstBuffer;
static VertexBuffer gVertexBuffer;
static DynamicVertexBuffer gDynamicVertexBuffer;
static TextureArray gTextureArray;
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

// TODO: see what to do with this ones
static Entity *gSelectedEntity;
static Entity *gEntityList;
static EditorMode gCurrentEditorMode;
static u32 gCurrentTexture;

#include "darray.cpp"
#include "input.cpp"
#include "win32.cpp"
#include "line.cpp"
#include "utils.cpp"
#include "brush2d.cpp"
#include "brushPlane.cpp"
#include "brushVertex.cpp"
#include "entity.cpp"
#include "view.cpp"
#include "editorModes.cpp"
#include "mainView.cpp"
#include "frontView.cpp"
#include "topView.cpp"
#include "sideView.cpp"
#include "viewManager.cpp"
#include "exporter.cpp"
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


//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
int main()
{
    HINSTANCE instace = GetModuleHandle(0);
    HWND window = InitWindow(instace);
    InitD3D11(window);
    LineRendererInitialize(200);
    InitImGui(window);

    // Load Shader and Create Input Layout
    gColShader = LoadShader("../src/shaders/colVert.hlsl", "../src/shaders/colFrag.hlsl");
    gCol2DShader = LoadShader("../src/shaders/col2dVert.hlsl", "../src/shaders/col2dFrag.hlsl");
    gTexShader = LoadShader("../src/shaders/texVert.hlsl", "../src/shaders/texFrag.hlsl");
    gTex2DShader = LoadShader("../src/shaders/tex2dVert.hlsl", "../src/shaders/tex2dFrag.hlsl");
    
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
        {"TEXCOORD", 1, DXGI_FORMAT_R32_UINT,
         0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    i32 totalLayoutElements = ARRAY_LENGTH(inputLayoutDesc);
    HRESULT layoutResult = device->CreateInputLayout(inputLayoutDesc,
        totalLayoutElements,
        gTexShader.vertexShaderCompiled->GetBufferPointer(),
        gTexShader.vertexShaderCompiled->GetBufferSize(),
        &layout);

    if(FAILED(layoutResult))
    {
        OutputDebugString("ERROR Creating Layout Input\n");
        ASSERT(!"ERROR");
    }

    // Load Vertex Buffer
    gVertexBuffer = LoadVertexBuffer(gQuad, ARRAY_LENGTH(gQuad), layout);
    gDynamicVertexBuffer = LoadDynamicVertexBuffer(2*13770*sizeof(Vertex), layout); // TODO: set the size better

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
    cbuffer.viewDir = {};
    gConstBuffer = LoadConstBuffer((void *)&cbuffer, sizeof(CBuffer), 0);

#if 0
    LoadTextureToTextureArray(&gTextureArray, "../assets/wood.png");
    LoadTextureToTextureArray(&gTextureArray, "../assets/white.png");
    LoadTextureToTextureArray(&gTextureArray, "../assets/brick.png");
    LoadTextureToTextureArray(&gTextureArray, "../assets/grass.png");
    LoadTextureToTextureArray(&gTextureArray, "../assets/cool.png");
    LoadTextureToTextureArray(&gTextureArray, "../assets/noTexture.png");
#endif
    LoadTextureToTextureArray(&gTextureArray, "../assets/wood.png");
    LoadTextureToTextureArray(&gTextureArray, "../assets/lava.png");
    LoadTextureToTextureArray(&gTextureArray, "../assets/water.png");
    LoadTextureToTextureArray(&gTextureArray, "../assets/brick1.png");
    LoadTextureToTextureArray(&gTextureArray, "../assets/brick2.png");
    LoadTextureToTextureArray(&gTextureArray, "../assets/brick3.png");
    LoadTextureToTextureArray(&gTextureArray, "../assets/brick4.png");
    LoadTextureToTextureArray(&gTextureArray, "../assets/crate.png");
    LoadTextureToTextureArray(&gTextureArray, "../assets/grass1.png");
    LoadTextureToTextureArray(&gTextureArray, "../assets/grass2.png");
    LoadTextureToTextureArray(&gTextureArray, "../assets/grass3.png");
    LoadTextureToTextureArray(&gTextureArray, "../assets/grass4.png");

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

    UnloadTextureArray(&gTextureArray);

    ShutDownImGui();

    UnloadVertexBuffer(&gVertexBuffer);
    UnloadDynamicVertexBuffer(&gDynamicVertexBuffer);
    UnloadShader(&gColShader);
    UnloadShader(&gCol2DShader);
    UnloadShader(&gTexShader);
    UnloadShader(&gTex2DShader);
    UnloadConstBuffer(&gConstBuffer);
    
    ViewManagerShutDown(&vm);
 
    if(layout) layout->Release();

    LineRendererShutdown();
    ShutdownD3D11();

    return 0;
}

