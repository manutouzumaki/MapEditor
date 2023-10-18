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

    Entity *entity = gEntityList;
    while(entity)
    {
        Brush2D *brush = &entity->brushes2D[view->id];
        RenderBrush2D(view , brush);
        entity = entity->next;
    }

    LineRendererDraw();
}
