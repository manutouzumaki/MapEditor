void SetupFrontView(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseSetup(view);
}

void ProcessFrontView(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseProcess(view);
}

void RenderFrontView(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseRender(view);
}
