static void AddSideAndTopViewsPolys(Vec2 startP, Vec2 endP, u32 color)
{
    Poly2D poly;

    Poly2DStorage *sideStorage = gSharedMemory.poly2dStorage + VIEW_SIDE;
    poly.vertices[0] = {0, endP.y};
    poly.vertices[1] = {gUnitSize, endP.y};
    poly.vertices[2] = {gUnitSize, startP.y};
    poly.vertices[3] = {0, startP.y};
    poly.verticesCount = 4;
    poly.color = color;
    ASSERT(sideStorage->polygonsCount < ARRAY_LENGTH(sideStorage->polygons));
    sideStorage->polygons[sideStorage->polygonsCount++] = poly;
    
    Poly2DStorage *topStorage = gSharedMemory.poly2dStorage + VIEW_TOP;
    poly.vertices[0] = {startP.x, gUnitSize};
    poly.vertices[1] = {endP.x, gUnitSize};
    poly.vertices[2] = {endP.x, 0};
    poly.vertices[3] = {startP.x, 0};
    poly.verticesCount = 4;
    poly.color = color;
    ASSERT(topStorage->polygonsCount < ARRAY_LENGTH(topStorage->polygons));
    topStorage->polygons[topStorage->polygonsCount++] = poly;
}

static void UpdateSideAndTopViewsPolys(RectMinMax rect, i32 quadIndex, u32 color)
{
    Poly2D poly;

    Poly2DStorage *sideStorage = gSharedMemory.poly2dStorage + VIEW_SIDE; 
    poly.vertices[0] = {0, rect.max.y};
    poly.vertices[1] = {gUnitSize, rect.max.y};
    poly.vertices[2] = {gUnitSize, rect.min.y};
    poly.vertices[3] = {0, rect.min.y};
    poly.verticesCount = 4;
    poly.color = color;
    sideStorage->polygons[quadIndex] = poly;
    
    Poly2DStorage *topStorage = gSharedMemory.poly2dStorage + VIEW_TOP;
    poly.vertices[0] = {rect.min.x, gUnitSize};
    poly.vertices[1] = {rect.max.x, gUnitSize};
    poly.vertices[2] = {rect.max.x, 0};
    poly.vertices[3] = {rect.min.x, 0};
    poly.verticesCount = 4;
    poly.color = color;
    topStorage->polygons[quadIndex] = poly;
}

void SetupFrontView(View *view)
{
    view->id = VIEW_FRONT;
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseSetup(view);
    state->addOtherViewsPolys = AddSideAndTopViewsPolys;
    state->updateOtherViewsPolys = UpdateSideAndTopViewsPolys;
}

void ProcessFrontView(View *view)
{
    ViewOrthoBasePannelAndZoom(view);

    EditorModeAddPoly(view);
    EditorModeModifyPoly(view);
}

void RenderFrontView(View *view)
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
