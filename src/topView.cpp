static void AddFrontAndSideViewsPolys(Vec2 startP, Vec2 endP, u32 color)
{
    Poly2DStorage *frontStorage = gSharedMemory.poly2dStorage + VIEW_FRONT;

    Poly2D poly = {};
    Vec2 a = {startP.x, gUnitSize};
    Vec2 b = {endP.x, gUnitSize};
    Vec2 c = {endP.x, 0};
    Vec2 d = {startP.x, 0};
    DarrayPush(poly.vertices, a, Vec2);
    DarrayPush(poly.vertices, b, Vec2);
    DarrayPush(poly.vertices, c, Vec2);
    DarrayPush(poly.vertices, d, Vec2);
    poly.color = color;

    PolyVertex2D polyVert = {};
    DarrayPush(polyVert.polygons, poly, Poly2D);
    DarrayPush(frontStorage->polyVerts, polyVert, PolyVertex2D);

    Poly2DStorage *sideStorage = gSharedMemory.poly2dStorage + VIEW_SIDE;

    poly = {};
    a = {startP.y, gUnitSize};
    b = {endP.y, gUnitSize};
    c = {endP.y, 0};
    d = {startP.y, 0};
    DarrayPush(poly.vertices, a, Vec2);
    DarrayPush(poly.vertices, b, Vec2);
    DarrayPush(poly.vertices, c, Vec2);
    DarrayPush(poly.vertices, d, Vec2);
    poly.color = color;

    polyVert = {};
    DarrayPush(polyVert.polygons, poly, Poly2D);
    DarrayPush(sideStorage->polyVerts, polyVert, PolyVertex2D);
}

static void UpdateFrontAndSideViewsPolys(RectMinMax rect, i32 quadIndex, u32 color)
{
    Poly2DStorage *frontStorage = gSharedMemory.poly2dStorage + VIEW_FRONT; 
    PolyVertex2D *polyVert = frontStorage->polyVerts + quadIndex;
    Poly2D *poly = &polyVert->polygons[0];
    poly->vertices[0] = {rect.min.x, poly->vertices[0].y};
    poly->vertices[1] = {rect.max.x, poly->vertices[1].y};
    poly->vertices[2] = {rect.max.x, poly->vertices[2].y};
    poly->vertices[3] = {rect.min.x, poly->vertices[3].y};

    Poly2DStorage *sideStorage = gSharedMemory.poly2dStorage + VIEW_SIDE;
    polyVert = sideStorage->polyVerts + quadIndex;
    poly = &polyVert->polygons[0];
    poly->vertices[0] = {rect.min.y, poly->vertices[0].y};
    poly->vertices[1] = {rect.max.y, poly->vertices[1].y};
    poly->vertices[2] = {rect.max.y, poly->vertices[2].y};
    poly->vertices[3] = {rect.min.y, poly->vertices[3].y};
}

Plane CreateTopClipPlane(Vec2 a, Vec2 b)
{
    Vec3 v0 = {a.x,  0.0f,  a.y};
    Vec3 v1 = {b.x,  0.0f,  b.y};
    Vec3 v2 = {a.x, -64.0f, a.y};
    Plane plane = GetPlaneFromThreePoints(v0, v1, v2);
    return plane;
}

void SetupTopView(View *view)
{
    view->id = VIEW_TOP;
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseSetup(view);
    state->addOtherViewsPolys = AddFrontAndSideViewsPolys;
    state->updateOtherViewsPolys = UpdateFrontAndSideViewsPolys;
    state->createViewClipPlane = CreateTopClipPlane;
    state->controlPointDown = -1;
    state->planeCreated = false;
    view->mousePicking = MousePicking2D;
}

void ProcessTopView(View *view)
{
    ViewOrthoBasePannelAndZoom(view);

    EditorModeAddPoly(view);
    EditorModeModifyPoly(view);
    EditorModeSelectPoly(view);
    EditorModeClipping(view);
}

void RenderTopView(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseRender(view);

    Poly2DStorage *poly2dStorage = ViewGetPoly2DStorage(view);
    for(i32 i = 0; i  < DarraySize(poly2dStorage->polyVerts); ++i)
    {
        PolyVertex2D *polyVert = poly2dStorage->polyVerts + i;
        RenderPolyVertex2D(view , polyVert);
    }
    LineRendererDraw();
}
