Plane CreateSideClipPlane(Vec2 a, Vec2 b)
{
    Vec3 v0 = { 0.0f,  a.y, a.x};
    Vec3 v1 = { 0.0f,  b.y, b.x};
    Vec3 v2 = {-64.0f, a.y, a.x};
    Plane plane = GetPlaneFromThreePoints(v0, v1, v2);
    return plane;
}

void SetupSideView(View *view)
{
    view->id = VIEW_SIDE;
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseSetup(view);
    state->createViewClipPlane = CreateSideClipPlane;
    state->controlPointDown = -1;
    state->planeCreated = false;
    view->mousePicking = MousePicking2D;
}

void ProcessSideView(View *view)
{
    ViewOrthoBasePannelAndZoom(view);

    EditorModeAddPoly(view);
    EditorModeModifyPoly(view);
    EditorModeSelectPoly(view);
    EditorModeClipping(view);
}

void RenderSideView(View *view)
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
