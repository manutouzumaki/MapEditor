static void AddSideAndTopViewsPolys(Vec2 startP, Vec2 endP, u32 color)
{
    Poly2DStorage *sideStorage = gSharedMemory.poly2dStorage + VIEW_SIDE;

    Poly2D poly = {};
    Vec2 a = {0, endP.y};
    Vec2 b = {gUnitSize, endP.y};
    Vec2 c = {gUnitSize, startP.y};
    Vec2 d = {0, startP.y};
    DarrayPush(poly.vertices, a, Vec2);
    DarrayPush(poly.vertices, b, Vec2);
    DarrayPush(poly.vertices, c, Vec2);
    DarrayPush(poly.vertices, d, Vec2);
    poly.color = color;

    PolyVertex2D polyVert = {};
    DarrayPush(polyVert.polygons, poly, Poly2D);
    DarrayPush(sideStorage->polyVerts, polyVert, PolyVertex2D);
     
    Poly2DStorage *topStorage = gSharedMemory.poly2dStorage + VIEW_TOP;

    poly = {};
    a = {startP.x, gUnitSize};
    b = {endP.x, gUnitSize};
    c = {endP.x, 0};
    d = {startP.x, 0};
    DarrayPush(poly.vertices, a, Vec2);
    DarrayPush(poly.vertices, b, Vec2);
    DarrayPush(poly.vertices, c, Vec2);
    DarrayPush(poly.vertices, d, Vec2);
    poly.color = color;

    polyVert = {};
    DarrayPush(polyVert.polygons, poly, Poly2D);
    DarrayPush(topStorage->polyVerts, polyVert, PolyVertex2D);
}

static void UpdateSideAndTopViewsPolys(RectMinMax rect, i32 quadIndex, u32 color)
{
    Poly2DStorage *sideStorage = gSharedMemory.poly2dStorage + VIEW_SIDE; 
    PolyVertex2D *polyVert = sideStorage->polyVerts + quadIndex;
    Poly2D *poly = &polyVert->polygons[0];
    poly->vertices[0] = {poly->vertices[0].x, rect.max.y};
    poly->vertices[1] = {poly->vertices[1].x, rect.max.y};
    poly->vertices[2] = {poly->vertices[2].x, rect.min.y};
    poly->vertices[3] = {poly->vertices[3].x, rect.min.y};
    
    Poly2DStorage *topStorage = gSharedMemory.poly2dStorage + VIEW_TOP;
    polyVert = topStorage->polyVerts + quadIndex;
    poly = &polyVert->polygons[0];
    poly->vertices[0] = {rect.min.x, poly->vertices[0].y};
    poly->vertices[1] = {rect.max.x, poly->vertices[1].y};
    poly->vertices[2] = {rect.max.x, poly->vertices[2].y};
    poly->vertices[3] = {rect.min.x, poly->vertices[3].y};
}

Plane CreateFrontClipPlane(Vec2 a, Vec2 b)
{
    Vec3 v0 = {a.x, a.y,  0.0f};
    Vec3 v1 = {a.x, a.y, -64.0f};
    Vec3 v2 = {b.x, b.y,  0.0f};
    Plane plane = GetPlaneFromThreePoints(v0, v1, v2);
    return plane;
}

void SetupFrontView(View *view)
{
    view->id = VIEW_FRONT;
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseSetup(view);
    state->addOtherViewsPolys = AddSideAndTopViewsPolys;
    state->updateOtherViewsPolys = UpdateSideAndTopViewsPolys;
    state->createViewClipPlane = CreateFrontClipPlane;
    state->controlPointDown = -1;
    state->planeCreated = false;
    view->mousePicking = MousePicking2D;
}

void ProcessFrontView(View *view)
{
    ViewOrthoBasePannelAndZoom(view);

    EditorModeAddPoly(view);
    EditorModeModifyPoly(view);
    EditorModeSelectPoly(view);
    EditorModeClipping(view);
}

void RenderFrontView(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseRender(view);

    Poly2DStorage *poly2dStorage = ViewGetPoly2DStorage(view);
    for(i32 i = 0; i  < DarraySize(poly2dStorage->polyVerts); ++i)
    {
        PolyVertex2D *polyVert = poly2dStorage->polyVerts + i;
        RenderPolyVertex2D(view ,polyVert);
    }
    LineRendererDraw();
}
