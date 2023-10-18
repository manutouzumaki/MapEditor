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
            view.cbuffer.proj = Mat4Perspective(60, w/h, 0.01f, 1000.0f);
        } break;
        case PROJ_TYPE_ORTHO:
        {
            view.cbuffer.proj = Mat4Ortho(w*-0.5f, w*0.5f, h*-0.5f, h*0.5f, 0.01f, 100.0f);
        } break;
    }
    view.cbuffer.view = Mat4LookAt({0, 2.5f, -5}, {0, 0, 0}, {0, 1,  0});
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
            view->cbuffer.proj = Mat4Perspective(60, w/h, 0.01f, 1000.0f);
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
    deviceContext->OMSetRenderTargets(1, &view->fb.renderTargetView, view->fb.depthStencilView);
    deviceContext->ClearRenderTargetView(view->fb.renderTargetView, clearColor);
    deviceContext->ClearDepthStencilView(view->fb.depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

    if(view->render) view->render(view);

    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = gCurrentWindowWidth;
    viewport.Height = gCurrentWindowHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);
}

void ViewOrthoBaseSetup(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    state->wheelOffset = 0.5f;
    state->zoom = state->wheelOffset * state->wheelOffset;
}

void ViewOrthoBasePannelAndZoom(View *view)
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
            if(mouseWheelDelta > 0)
            {
                state->wheelOffset *= 1.1f;
            }
            else
            {
                state->wheelOffset *= 0.9f;
            }
            state->wheelOffset = Max(0.1f, state->wheelOffset);
            state->zoom = state->wheelOffset * state->wheelOffset;
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


void RenderBrush2D(View *view, Brush2D *brush)
{
    ViewOrthoState *state = &view->orthoState;

    for(i32 j = 0; j < DarraySize(brush->polygons); ++j)
    {
        Poly2D *poly = brush->polygons + j;
        u32 color = poly->color;
        for(i32 i = 0; i < DarraySize(poly->vertices) - 1; ++i)
        {
            Vec2 a = poly->vertices[i + 0];
            Vec2 b = poly->vertices[i + 1];

            f32 sax, say, sbx, sby;
            WorldToScreen(a.x, a.y, sax, say, state->offsetX, state->offsetY, state->zoom); 
            WorldToScreen(b.x, b.y, sbx, sby, state->offsetX, state->offsetY, state->zoom); 

            DrawLine(sax,  say, -2, sbx,  sby, -2, color);
        }

        Vec2 a = poly->vertices[DarraySize(poly->vertices) - 1];
        Vec2 b = poly->vertices[0];
        f32 sax, say, sbx, sby;
        WorldToScreen(a.x, a.y, sax, say, state->offsetX, state->offsetY, state->zoom); 
        WorldToScreen(b.x, b.y, sbx, sby, state->offsetX, state->offsetY, state->zoom); 
        DrawLine(sax,  say, -2, sbx,  sby, -2, color);
    }
}

void RenderBrush2DColor(View *view, Brush2D *brush, u32 color)
{
    ViewOrthoState *state = &view->orthoState;

    for(i32 j = 0; j < DarraySize(brush->polygons); ++j)
    {
        Poly2D *poly = brush->polygons + j;
        for(i32 i = 0; i < DarraySize(poly->vertices) - 1; ++i)
        {
            Vec2 a = poly->vertices[i + 0];
            Vec2 b = poly->vertices[i + 1];

            f32 sax, say, sbx, sby;
            WorldToScreen(a.x, a.y, sax, say, state->offsetX, state->offsetY, state->zoom); 
            WorldToScreen(b.x, b.y, sbx, sby, state->offsetX, state->offsetY, state->zoom); 

            DrawLine(sax,  say, -2, sbx,  sby, -2, color);
        }

        Vec2 a = poly->vertices[DarraySize(poly->vertices) - 1];
        Vec2 b = poly->vertices[0];
        f32 sax, say, sbx, sby;
        WorldToScreen(a.x, a.y, sax, say, state->offsetX, state->offsetY, state->zoom); 
        WorldToScreen(b.x, b.y, sbx, sby, state->offsetX, state->offsetY, state->zoom); 
        DrawLine(sax,  say, -2, sbx,  sby, -2, color);
    }
}
