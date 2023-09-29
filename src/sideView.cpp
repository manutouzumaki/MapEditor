static void AddFrontAndTopViewsPolys(Vec2 startP, Vec2 endP, u32 color)
{
    Poly2D poly;

    Poly2DStorage *frontStorage = gSharedMemory.poly2dStorage + VIEW_FRONT;
    poly.vertices[0] = {0, endP.y};
    poly.vertices[1] = {gUnitSize, endP.y};
    poly.vertices[2] = {gUnitSize, startP.y};
    poly.vertices[3] = {0, startP.y};
    poly.verticesCount = 4;
    poly.color = color;
    ASSERT(frontStorage->polygonsCount < ARRAY_LENGTH(frontStorage->polygons));
    frontStorage->polygons[frontStorage->polygonsCount++] = poly;
    
    Poly2DStorage *topStorage = gSharedMemory.poly2dStorage + VIEW_TOP;
    poly.vertices[0] = {0, endP.x};
    poly.vertices[1] = {gUnitSize, endP.x};
    poly.vertices[2] = {gUnitSize, startP.x};
    poly.vertices[3] = {0, startP.x};
    poly.verticesCount = 4;
    poly.color = color;
    ASSERT(topStorage->polygonsCount < ARRAY_LENGTH(topStorage->polygons));
    topStorage->polygons[topStorage->polygonsCount++] = poly;
}

static void UpdateFrontAndTopViewsPolys(RectMinMax rect, i32 quadIndex, u32 color)
{

    Poly2DStorage *frontStorage = gSharedMemory.poly2dStorage + VIEW_FRONT; 
    Poly2D poly = frontStorage->polygons[quadIndex];
    poly.vertices[0] = {poly.vertices[0].x, rect.max.y};
    poly.vertices[1] = {poly.vertices[1].x, rect.max.y};
    poly.vertices[2] = {poly.vertices[2].x, rect.min.y};
    poly.vertices[3] = {poly.vertices[3].x, rect.min.y};
    poly.verticesCount = 4;
    poly.color = color;
    frontStorage->polygons[quadIndex] = poly;
    

    Poly2DStorage *topStorage = gSharedMemory.poly2dStorage + VIEW_TOP;
    poly = topStorage->polygons[quadIndex];
    poly.vertices[0] = {poly.vertices[0].x, rect.max.x};
    poly.vertices[1] = {poly.vertices[1].x, rect.max.x};
    poly.vertices[2] = {poly.vertices[2].x, rect.min.x};
    poly.vertices[3] = {poly.vertices[3].x, rect.min.x};
    poly.verticesCount = 4;
    poly.color = color;
    topStorage->polygons[quadIndex] = poly;
}


void SetupSideView(View *view)
{
    view->id = VIEW_SIDE;
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseSetup(view);
    state->addOtherViewsPolys = AddFrontAndTopViewsPolys;
    state->updateOtherViewsPolys = UpdateFrontAndTopViewsPolys;
    state->controlPointDown = -1;
}

void ProcessSideView(View *view)
{
    ViewOrthoBasePannelAndZoom(view);

    EditorModeAddPoly(view);
    EditorModeModifyPoly(view);
}

void RenderSideView(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseRender(view);

    Poly2DStorage *poly2dStorage = ViewGetPoly2DStorage(view);
    for(i32 i = 0; i  < poly2dStorage->polygonsCount; ++i)
    {
        Poly2D *poly = poly2dStorage->polygons + i;
        RenderPoly2D(view ,poly, poly->color);
    }
    LineRendererDraw();
}
