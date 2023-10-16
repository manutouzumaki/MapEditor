static void AddFrontAndSideViewsBrush(Vec2 startP, Vec2 endP, u32 color)
{
    Brush2DStorage *frontStorage = gSharedMemory.brush2dStorage + VIEW_FRONT;

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

    Brush2D brush = {};
    DarrayPush(brush.polygons, poly, Poly2D);
    DarrayPush(frontStorage->brushes, brush, Brush2D);

    Brush2DStorage *sideStorage = gSharedMemory.brush2dStorage + VIEW_SIDE;

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

    brush = {};
    DarrayPush(brush.polygons, poly, Poly2D);
    DarrayPush(sideStorage->brushes, brush, Brush2D);
}

static void UpdateFrontAndSideViewsBrush(RectMinMax rect, i32 quadIndex, u32 color)
{
    Brush2DStorage *frontStorage = gSharedMemory.brush2dStorage + VIEW_FRONT; 
    Brush2D *brush = frontStorage->brushes + quadIndex;
    Poly2D *poly = &brush->polygons[0];
    poly->vertices[0] = {rect.min.x, poly->vertices[0].y};
    poly->vertices[1] = {rect.max.x, poly->vertices[1].y};
    poly->vertices[2] = {rect.max.x, poly->vertices[2].y};
    poly->vertices[3] = {rect.min.x, poly->vertices[3].y};

    Brush2DStorage *sideStorage = gSharedMemory.brush2dStorage + VIEW_SIDE;
    brush = sideStorage->brushes + quadIndex;
    poly = &brush->polygons[0];
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
    state->addOtherViewsBrush = AddFrontAndSideViewsBrush;
    state->updateOtherViewsBrush = UpdateFrontAndSideViewsBrush;
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

    Brush2DStorage *brush2dStorage = ViewGetBrush2DStorage(view);
    for(i32 i = 0; i  < DarraySize(brush2dStorage->brushes); ++i)
    {
        Brush2D *brush = brush2dStorage->brushes + i;
        RenderBrush2D(view , brush);
    }
    LineRendererDraw();
}
