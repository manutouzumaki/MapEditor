void SetupFrontView(View *view)
{
    view->id = VIEW_FRONT;
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseSetup(view);
}

void ProcessFrontView(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseProcess(view);

    i32 mouseRelX = MouseRelX(view);
    i32 mouseRelY = MouseRelY(view);

    if(MouseIsHot(view))
    {
        f32 mouseWorldX, mouseWorldY;
        ScreenToWorld(mouseRelX, mouseRelY, mouseWorldX, mouseWorldY,
                      state->offsetX, state->offsetY, state->zoom);

        static Vec2 startP;
        static Vec2 endP;
        static i32 quadIndex;
        static RectMinMax rect;

        if(MouseJustDown(MOUSE_BUTTON_LEFT))
        {
            state->leftButtonDown = true;
            f32 startX = floorf(mouseWorldX / 64.0f) * 64.0f;
            f32 startY = floorf(mouseWorldY / 64.0f) * 64.0f;
            startP = {startX, startY};
            quadIndex = ViewAddQuad(view, startP, {startP.x + 64.0f, startP.y + 64.0f});
        }
        
        if(MouseJustUp(MOUSE_BUTTON_LEFT) && state->leftButtonDown)
        {
            state->leftButtonDown = false;
            f32 endX = floorf(mouseWorldX / 64.0f) * 64.0f;
            f32 endY = floorf(mouseWorldY / 64.0f) * 64.0f;
            endP = {endX, endY};

            rect.min.x = Min(startP.x, endP.x);
            rect.min.y = Min(startP.y, endP.y);
            rect.max.x = Max(startP.x, endP.x) + 64.0f;
            rect.max.y = Max(startP.y, endP.y) + 64.0f;
            ViewUpdateQuad(view, rect.min, rect.max, quadIndex);
        }
        
        if(state->leftButtonDown)
        {
            //  update the end position every frame to get real time
            //  response to moving the mouse
            f32 endX = floorf(mouseWorldX / 64.0f) * 64.0f;
            f32 endY = floorf(mouseWorldY / 64.0f) * 64.0f;
            endP = {endX, endY};

            rect.min.x = Min(startP.x, endP.x);
            rect.min.y = Min(startP.y, endP.y);
            rect.max.x = Max(startP.x, endP.x) + 64.0f;
            rect.max.y = Max(startP.y, endP.y) + 64.0f;
            ViewUpdateQuad(view, rect.min, rect.max, quadIndex);
        }
    }
}

void RenderFrontView(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseRender(view);

    Poly2DStorage *poly2dStorage = ViewGetPoly2DStorage(view);
    for(i32 i = 0; i  < poly2dStorage->polygonsCount; ++i)
    {
        Poly2D *poly = poly2dStorage->polygons + i;
        RenderPoly2D(view ,poly, 0xFFFFFFAA);
    }
    LineRendererDraw();
}
