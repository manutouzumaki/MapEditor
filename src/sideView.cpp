void SetupSideView(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseSetup(view);
}

void ProcessSideView(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseProcess(view);
}

void RenderSideView(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseRender(view);
}
