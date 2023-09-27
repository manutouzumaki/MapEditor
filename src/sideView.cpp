static void AddFrontAndTopViewsPolys(Vec2 startP, Vec2 endP)
{
    Poly2D poly;

    Poly2DStorage *frontStorage = gSharedMemory.poly2dStorage + VIEW_FRONT;
    poly.vertices[0] = {0, endP.y};
    poly.vertices[1] = {gUnitSize, endP.y};
    poly.vertices[2] = {gUnitSize, startP.y};
    poly.vertices[3] = {0, startP.y};
    poly.verticesCount = 4;
    ASSERT(frontStorage->polygonsCount < ARRAY_LENGTH(frontStorage->polygons));
    frontStorage->polygons[frontStorage->polygonsCount++] = poly;
    
    Poly2DStorage *topStorage = gSharedMemory.poly2dStorage + VIEW_TOP;
    poly.vertices[0] = {0, endP.x};
    poly.vertices[1] = {gUnitSize, endP.x};
    poly.vertices[2] = {gUnitSize, startP.x};
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
    poly.vertices[1] = {gUnitSize, rect.max.y};
    poly.vertices[2] = {gUnitSize, rect.min.y};
    poly.vertices[3] = {0, rect.min.y};
    poly.verticesCount = 4;
    frontStorage->polygons[quadIndex] = poly;
    
    Poly2DStorage *topStorage = gSharedMemory.poly2dStorage + VIEW_TOP;
    poly.vertices[0] = {0, rect.max.x};
    poly.vertices[1] = {gUnitSize, rect.max.x};
    poly.vertices[2] = {gUnitSize, rect.min.x};
    poly.vertices[3] = {0, rect.min.x};
    poly.verticesCount = 4;
    topStorage->polygons[quadIndex] = poly;
}


void SetupSideView(View *view)
{
    view->id = VIEW_SIDE;
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseSetup(view);
    state->addOtherViewsPolys = AddFrontAndTopViewsPolys;
    state->updateOtherViewsPolys = UpdateFrontAndTopViewsPolys;
}

void ProcessSideView(View *view)
{
    ViewOrthoBasePannelAndZoom(view);
    ViewOrthoBaseDraw(view);
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
