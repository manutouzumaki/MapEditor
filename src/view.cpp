struct View;

typedef void (*SetupFNP) (View *view);
typedef void (*ProcessFNP) (View *view);
typedef void (*RenderFNP) (View *view);

enum ProjType
{
    PROJ_TYPE_PERSP,
    PROJ_TYPE_ORTHO
};

struct View
{
    f32 x, y, w, h;
    CBuffer cbuffer;
    FrameBuffer fb;
    ProjType projType;

    SetupFNP setup;
    ProcessFNP process;
    RenderFNP render;
};

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
    viewport.Width = currentWindowWidth;
    viewport.Height = currentWindowHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);
}
