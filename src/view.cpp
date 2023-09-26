struct View;

typedef void (*SetupFNP) (View *view);
typedef void (*ProcessFNP) (View *view);
typedef void (*RenderFNP) (View *view);

enum ProjType
{
    PROJ_TYPE_PERSP,
    PROJ_TYPE_ORTHO
};

enum ViewId
{
    VIEW_TOP,
    VIEW_FRONT,
    VIEW_SIDE,
    VIEW_MAIN,

    VIEW_COUNT
};

struct ViewPerspState
{

};

struct ViewOrthoState
{
    f32 offsetX;
    f32 offsetY;

    f32 lastClickX;
    f32 lastClickY;

    f32 wheelOffset;
    f32 zoom;

    bool leftButtonDown;
    bool middleButtonDown;
    bool rightButtonDown;
};

struct View
{
    ViewId id;
    f32 x, y, w, h;
    CBuffer cbuffer;
    FrameBuffer fb;
    ProjType projType;
    union
    {
        ViewPerspState perspState;
        ViewOrthoState orthoState;
    };

    SetupFNP setup;
    ProcessFNP process;
    RenderFNP render;
};


// TODO: use a link list base storage system
struct Poly2DStorage
{
    Poly2D polygons[255];
    i32 polygonsCount;
};

struct PolyPlaneStorage
{
    PolyPlane polygons[255];
    i32 polygonsCount;
};

struct SharedMemory
{
    Poly2DStorage poly2dStorage[3];
    PolyPlaneStorage polyPlaneStorage;
};

static SharedMemory gSharedMemory;

View ViewCreate(f32 x, f32 y, f32 w, f32 h, ProjType projType,
                SetupFNP setup,
                ProcessFNP process,
                RenderFNP render)
{
    View view = {};

    view.x = x;
    view.y = y;
    view.w = w;
    view.h = h;

    switch(projType)
    {
        case PROJ_TYPE_PERSP:
        {
            view.cbuffer.proj = Mat4Perspective(60, w/h, 0.01f, 100.0f);
        } break;
        case PROJ_TYPE_ORTHO:
        {
            view.cbuffer.proj = Mat4Ortho(w*-0.5f, w*0.5f, h*-0.5f, h*0.5f, 0.01f, 100.0f);
        } break;
    }
    view.cbuffer.view = Mat4LookAt({0, 0, -2}, {0, 0, 0}, {0, 1,  0});
    view.cbuffer.world = Mat4Identity();

    view.fb = LoadFrameBuffer(x, y, w, h, DXGI_FORMAT_R8G8B8A8_UNORM);

    view.setup = setup;
    view.process = process;
    view.render = render;

    view.projType = projType;

    return view;
}

void ViewDestroy(View *view)
{
    UnloadFrameBuffer(&view->fb);
}

void ViewResize(View *view, f32 x, f32 y, f32 w, f32 h)
{
    view->x = x;
    view->y = y;
    view->w = w;
    view->h = h;
    switch(view->projType)
    {
        case PROJ_TYPE_PERSP:
        {
            view->cbuffer.proj = Mat4Perspective(60, w/h, 0.01f, 100.0f);
        } break;
        case PROJ_TYPE_ORTHO:
        {
            view->cbuffer.proj = Mat4Ortho(w*-0.5f, w*0.5f, h*-0.5f, h*0.5f, 0.01f, 100.0f);
        } break;
    }
    ResizeFrameBuffer(&view->fb, x, y, w, h);
}

void ViewSetup(View *view)
{
    if(view->setup) view->setup(view);
}

void ViewProcess(View *view)
{
    if(view->process) view->process(view);
}

void ViewRender(View *view)
{
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = view->w;
    viewport.Height = view->h;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);

    // Clear the screen
    f32 clearColor[] = { 0.05, 0.05, 0.05, 1 };
    deviceContext->ClearRenderTargetView(view->fb.renderTargetView, clearColor);
    deviceContext->OMSetRenderTargets(1, &view->fb.renderTargetView, 0);

    if(view->render) view->render(view);

    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = gCurrentWindowWidth;
    viewport.Height = gCurrentWindowHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);
}

void WorldToScreen(f32 wx, f32 wy, f32 &sx, f32 &sy, f32 offsetX, f32 offsetY, f32 zoom)
{
    sx = (wx - offsetX) * zoom;
    sy = (wy - offsetY) * zoom;
}

void ScreenToWorld(f32 sx, f32 sy, f32 &wx, f32 &wy, f32 offsetX, f32 offsetY, f32 zoom)
{
    wx = (sx / zoom) + offsetX;
    wy = (sy / zoom) + offsetY;
}

i32 MouseRelToClientX()
{
    i32 mouseRelToClientX = MouseX() - 200; // TODO: use a global for this value
    return mouseRelToClientX;
}

i32 MouseRelToClientY()
{
    i32 mouseRelToClientY = gCurrentWindowHeight - MouseY();
    return mouseRelToClientY;
}

i32 MouseRelX(View *view)
{
    i32 mouseRelToClientX = MouseRelToClientX();
    i32 mouseRelX = mouseRelToClientX - view->x;
    return mouseRelX;
}

i32 MouseRelY(View *view)
{
    i32 mouseRelToClientY = MouseRelToClientY();
    i32 mouseRelY = mouseRelToClientY - view->y;
    return mouseRelY;
}

bool MouseIsHot(View *view)
{
    i32 mouseRelX = MouseRelX(view);
    i32 mouseRelY = MouseRelY(view);
    if(mouseRelX >= view->w*-0.5f && mouseRelX <= view->w*0.5f &&
       mouseRelY >= view->h*-0.5f && mouseRelY <= view->h*0.5f)
    {
        return true;
    }
    return false;
}

void RenderGrid(f32 offsetX, f32 offsetY, f32 zoom)
{
    for(f32 y = -100.0f; y < 100.0f; ++y)
    {
        f32 ax = -100.0f * 64;
        f32 ay = y * 64;
        f32 bx = 100.0f * 64;
        f32 by = y * 64;
        
        f32 sax, say, sbx, sby;

        WorldToScreen(ax, ay, sax, say, offsetX, offsetY, zoom); 
        WorldToScreen(bx, by, sbx, sby, offsetX, offsetY, zoom); 

        DrawLine(sax,  say, 0, sbx,  sby, 0, 0xFF333333);
    }

    for(f32 x = -100.0f; x < 100.0f; ++x)
    {

        f32 ax = x * 64;
        f32 ay = -100.0f * 64.0f;
        f32 bx = x * 64;
        f32 by = 64.f*100.0f;
        
        f32 sax, say, sbx, sby;

        WorldToScreen(ax, ay, sax, say, offsetX, offsetY, zoom); 
        WorldToScreen(bx, by, sbx, sby, offsetX, offsetY, zoom); 

        DrawLine(sax,  say, 0, sbx,  sby, 0, 0xFF333333);
    }

    f32 ax = 0;
    f32 ay = -100.0f * 64.0f;
    f32 bx = 0;
    f32 by = 64.f*100.0f;
    f32 sax, say, sbx, sby;
    WorldToScreen(ax, ay, sax, say, offsetX, offsetY, zoom); 
    WorldToScreen(bx, by, sbx, sby, offsetX, offsetY, zoom); 
    DrawLine(sax,  say, 0, sbx,  sby, 0, 0xFFAAFFAA);

    ax = -100.0f * 64.0f;
    ay = 0;
    bx =  100.0f * 64.0f;
    by = 0;
    WorldToScreen(ax, ay, sax, say, offsetX, offsetY, zoom); 
    WorldToScreen(bx, by, sbx, sby, offsetX, offsetY, zoom); 
    DrawLine(sax,  say, 0, sbx,  sby, 0, 0xFFFFAAAA);

}

void ViewOrthoBaseSetup(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    state->wheelOffset = 5.0f;
    state->zoom = Remap(0.0f, 50.0f, 0.05f, 10.0f, state->wheelOffset);
}

void ViewOrthoBaseProcess(View *view)
{
    ViewOrthoState *state = &view->orthoState;

    i32 mouseRelX = MouseRelX(view);
    i32 mouseRelY = MouseRelY(view);

    if(MouseIsHot(view))
    {
        if(MouseJustDown(MOUSE_BUTTON_MIDDLE))
        {
            state->middleButtonDown = true;
            state->lastClickX = mouseRelX;
            state->lastClickY = mouseRelY;
        }

        f32 mouseWorldPreZoomX, mouseWorldPreZoomY;
        ScreenToWorld(mouseRelX, mouseRelY, mouseWorldPreZoomX, mouseWorldPreZoomY,
                      state->offsetX, state->offsetY, state->zoom);

        i32 mouseWheelDelta = MouseWheelDelta();
        if(mouseWheelDelta != 0)
        {
            state->wheelOffset += (f32)mouseWheelDelta;
            state->wheelOffset = Clamp(state->wheelOffset, 0.0f, 50.0f);
            state->zoom = Remap(0.0f, 50.0f, 0.2f, 10.0f, state->wheelOffset);
        }

        f32 mouseWorldPostZoomX, mouseWorldPostZoomY;
        ScreenToWorld(mouseRelX, mouseRelY, mouseWorldPostZoomX, mouseWorldPostZoomY,
                      state->offsetX, state->offsetY, state->zoom);

        state->offsetX += (mouseWorldPreZoomX - mouseWorldPostZoomX);
        state->offsetY += (mouseWorldPreZoomY - mouseWorldPostZoomY);
    }
    
    if(MouseJustUp(MOUSE_BUTTON_MIDDLE) && state->middleButtonDown)
    {
        state->middleButtonDown = false;
    }

    if(state->middleButtonDown)
    {
        state->offsetX += (state->lastClickX - mouseRelX) / state->zoom;
        state->offsetY += (state->lastClickY - mouseRelY) / state->zoom;
        state->lastClickX = mouseRelX;
        state->lastClickY = mouseRelY;
    }
}

void ViewOrthoBaseRender(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    // set the shader
    deviceContext->VSSetShader(gColShader.vertex, 0, 0);
    deviceContext->PSSetShader(gColShader.fragment, 0, 0);

    // Update the constBuffer
    view->cbuffer.world = Mat4Identity();
    view->cbuffer.view = Mat4LookAt({0, 0, -50}, {0, 0, 0}, {0, 1, 0});
    UpdateConstBuffer(&gConstBuffer, (void *)&view->cbuffer);
        
    RenderGrid(state->offsetX, state->offsetY, state->zoom);
    LineRendererDraw();
}

Poly2DStorage *ViewGetPoly2DStorage(View *view)
{
    Poly2DStorage *poly2dStorage = gSharedMemory.poly2dStorage + view->id;
    return poly2dStorage;
}

i32 ViewAddQuad(View *view, Vec2 start, Vec2 end)
{
    Poly2DStorage *poly2dStorage = ViewGetPoly2DStorage(view);

    Poly2D poly;
    poly.vertices[0] = {start.x, start.y};
    poly.vertices[1] = {end.x, start.y};
    poly.vertices[2] = {end.x, end.y};
    poly.vertices[3] = {start.x, end.y};
    poly.verticesCount = 4;

    ASSERT(poly2dStorage->polygonsCount < ARRAY_LENGTH(poly2dStorage->polygons));
    i32 index = poly2dStorage->polygonsCount++;
    poly2dStorage->polygons[index] = poly;

    return index;
}

void ViewUpdateQuad(View *view, Vec2 start, Vec2 end, i32 index)
{
    Poly2DStorage *poly2dStorage = ViewGetPoly2DStorage(view);

    Poly2D poly;
    poly.vertices[0] = {start.x, start.y};
    poly.vertices[1] = {end.x, start.y};
    poly.vertices[2] = {end.x, end.y};
    poly.vertices[3] = {start.x, end.y};
    poly.verticesCount = 4;

    poly2dStorage->polygons[index] = poly;
}

void RenderPoly2D(View *view, Poly2D *poly, u32 color)
{
    ViewOrthoState *state = &view->orthoState;
    for(i32 i = 0; i < poly->verticesCount - 1; ++i)
    {
        Vec2 a = poly->vertices[i + 0];
        Vec2 b = poly->vertices[i + 1];

        f32 sax, say, sbx, sby;
        WorldToScreen(a.x, a.y, sax, say, state->offsetX, state->offsetY, state->zoom); 
        WorldToScreen(b.x, b.y, sbx, sby, state->offsetX, state->offsetY, state->zoom); 

        DrawLine(sax,  say, 0, sbx,  sby, 0, color);
    }

    Vec2 a = poly->vertices[poly->verticesCount - 1];
    Vec2 b = poly->vertices[0];
    f32 sax, say, sbx, sby;
    WorldToScreen(a.x, a.y, sax, say, state->offsetX, state->offsetY, state->zoom); 
    WorldToScreen(b.x, b.y, sbx, sby, state->offsetX, state->offsetY, state->zoom); 
    DrawLine(sax,  say, 0, sbx,  sby, 0, color);
}
