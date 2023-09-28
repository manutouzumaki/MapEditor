static void AddFrontAndSideViewsPolys(Vec2 startP, Vec2 endP, u32 color)
{
    Poly2D poly;
    Poly2DStorage *frontStorage = gSharedMemory.poly2dStorage + VIEW_FRONT;
    poly.vertices[0] = {startP.x, gUnitSize};
    poly.vertices[1] = {endP.x, gUnitSize};
    poly.vertices[2] = {endP.x, 0};
    poly.vertices[3] = {startP.x, 0};
    poly.verticesCount = 4;
    poly.color = color;
    ASSERT(frontStorage->polygonsCount < ARRAY_LENGTH(frontStorage->polygons));
    frontStorage->polygons[frontStorage->polygonsCount++] = poly;

    Poly2DStorage *sideStorage = gSharedMemory.poly2dStorage + VIEW_SIDE;
    poly.vertices[0] = {startP.y, gUnitSize};
    poly.vertices[1] = {endP.y, gUnitSize};
    poly.vertices[2] = {endP.y, 0};
    poly.vertices[3] = {startP.y, 0};
    poly.verticesCount = 4;
    poly.color = color;
    ASSERT(sideStorage->polygonsCount < ARRAY_LENGTH(sideStorage->polygons));
    sideStorage->polygons[sideStorage->polygonsCount++] = poly;
}

static void UpdateFrontAndSideViewsPolys(RectMinMax rect, i32 quadIndex, u32 color)
{
    Poly2D poly;
    Poly2DStorage *frontStorage = gSharedMemory.poly2dStorage + VIEW_FRONT; 
    poly.vertices[0] = {rect.min.x, gUnitSize};
    poly.vertices[1] = {rect.max.x, gUnitSize};
    poly.vertices[2] = {rect.max.x, 0};
    poly.vertices[3] = {rect.min.x, 0};
    poly.verticesCount = 4;
    poly.color = color;
    frontStorage->polygons[quadIndex] = poly;

    Poly2DStorage *sideStorage = gSharedMemory.poly2dStorage + VIEW_SIDE;
    poly.vertices[0] = {rect.min.y, gUnitSize};
    poly.vertices[1] = {rect.max.y, gUnitSize};
    poly.vertices[2] = {rect.max.y, 0};
    poly.vertices[3] = {rect.min.y, 0};
    poly.verticesCount = 4;
    poly.color = color;
    sideStorage->polygons[quadIndex] = poly;
}

void SetupTopView(View *view)
{
    view->id = VIEW_TOP;
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseSetup(view);
    state->addOtherViewsPolys = AddFrontAndSideViewsPolys;
    state->updateOtherViewsPolys = UpdateFrontAndSideViewsPolys;
}

void ProcessTopView(View *view)
{
    ViewOrthoBasePannelAndZoom(view);

    EditorModeAddPoly(view);
    EditorModeModifyPoly(view);
}

void RenderTopView(View *view)
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
