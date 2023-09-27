static void AddFrontAndTopViewsPolys(Vec2 startP, Vec2 endP)
{
    Poly2D poly;

    Poly2DStorage *frontStorage = gSharedMemory.poly2dStorage + VIEW_FRONT;
    poly.vertices[0] = {0, endP.y};
    poly.vertices[1] = {64.0f, endP.y};
    poly.vertices[2] = {64.0f, startP.y};
    poly.vertices[3] = {0, startP.y};
    poly.verticesCount = 4;
    ASSERT(frontStorage->polygonsCount < ARRAY_LENGTH(frontStorage->polygons));
    frontStorage->polygons[frontStorage->polygonsCount++] = poly;
    
    Poly2DStorage *topStorage = gSharedMemory.poly2dStorage + VIEW_TOP;
    poly.vertices[0] = {0, endP.x};
    poly.vertices[1] = {64.0f, endP.x};
    poly.vertices[2] = {64.0f, startP.x};
    poly.vertices[3] = {0, startP.x};
    poly.verticesCount = 4;
    ASSERT(topStorage->polygonsCount < ARRAY_LENGTH(topStorage->polygons));
    topStorage->polygons[topStorage->polygonsCount++] = poly;
}

static void UpdateFrontAndTopViewsPolys(RectMinMax rect, i32 quadIndex)
{
    Poly2D poly;

    Poly2DStorage *frontStorage = gSharedMemory.poly2dStorage + VIEW_FRONT; 
    poly.vertices[0] = {0, rect.max.y};
    poly.vertices[1] = {64.0f, rect.max.y};
    poly.vertices[2] = {64.0f, rect.min.y};
    poly.vertices[3] = {0, rect.min.y};
    poly.verticesCount = 4;
    frontStorage->polygons[quadIndex] = poly;
    
    Poly2DStorage *topStorage = gSharedMemory.poly2dStorage + VIEW_TOP;
    poly.vertices[0] = {0, rect.max.x};
    poly.vertices[1] = {64.0f, rect.max.x};
    poly.vertices[2] = {64.0f, rect.min.x};
    poly.vertices[3] = {0, rect.min.x};
    poly.verticesCount = 4;
    topStorage->polygons[quadIndex] = poly;
}


void SetupSideView(View *view)
{
    view->id = VIEW_SIDE;
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseSetup(view);
}

void ProcessSideView(View *view)
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
            AddFrontAndTopViewsPolys(startP,  {startP.x + 64.0f, startP.y + 64.0f});
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
            UpdateFrontAndTopViewsPolys(rect, quadIndex);
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
            UpdateFrontAndTopViewsPolys(rect, quadIndex);
        }
    }

}

void RenderSideView(View *view)
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
