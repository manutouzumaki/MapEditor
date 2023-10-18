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

    Entity *entity = gEntityList;
    while(entity)
    {
        Brush2D *brush = &entity->brushes2D[view->id];
        RenderBrush2D(view , brush);
        entity = entity->next;
    }

    LineRendererDraw();
}
