static void AddFrontAndSideViewsPolys(Vec2 startP, Vec2 endP)
{
    Poly2D poly;
    Poly2DStorage *frontStorage = gSharedMemory.poly2dStorage + VIEW_FRONT;
    poly.vertices[0] = {startP.x, 64.0f};
    poly.vertices[1] = {endP.x, 64.0f};
    poly.vertices[2] = {endP.x, 0};
    poly.vertices[3] = {startP.x, 0};
    poly.verticesCount = 4;
    ASSERT(frontStorage->polygonsCount < ARRAY_LENGTH(frontStorage->polygons));
    frontStorage->polygons[frontStorage->polygonsCount++] = poly;

    Poly2DStorage *sideStorage = gSharedMemory.poly2dStorage + VIEW_SIDE;
    poly.vertices[0] = {startP.y, 64.0f};
    poly.vertices[1] = {endP.y, 64.0f};
    poly.vertices[2] = {endP.y, 0};
    poly.vertices[3] = {startP.y, 0};
    poly.verticesCount = 4;
    ASSERT(sideStorage->polygonsCount < ARRAY_LENGTH(sideStorage->polygons));
    sideStorage->polygons[sideStorage->polygonsCount++] = poly;
}

static void UpdateFrontAndSideViewsPolys(RectMinMax rect, i32 quadIndex)
{
    Poly2D poly;
    Poly2DStorage *frontStorage = gSharedMemory.poly2dStorage + VIEW_FRONT; 
    poly.vertices[0] = {rect.min.x, 64.0f};
    poly.vertices[1] = {rect.max.x, 64.0f};
    poly.vertices[2] = {rect.max.x, 0};
    poly.vertices[3] = {rect.min.x, 0};
    poly.verticesCount = 4;
    frontStorage->polygons[quadIndex] = poly;

    Poly2DStorage *sideStorage = gSharedMemory.poly2dStorage + VIEW_SIDE;
    poly.vertices[0] = {rect.min.y, 64.0f};
    poly.vertices[1] = {rect.max.y, 64.0f};
    poly.vertices[2] = {rect.max.y, 0};
    poly.vertices[3] = {rect.min.y, 0};
    poly.verticesCount = 4;
    sideStorage->polygons[quadIndex] = poly;
}

void SetupTopView(View *view)
{
    view->id = VIEW_TOP;
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseSetup(view);
}

void ProcessTopView(View *view)
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
            AddFrontAndSideViewsPolys(startP, {startP.x + 64.0f, startP.y + 64.0f});
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
            UpdateFrontAndSideViewsPolys(rect, quadIndex);
            ViewAddPolyPlane();
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
            UpdateFrontAndSideViewsPolys(rect, quadIndex);
        }
    }
}

void RenderTopView(View *view)
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
