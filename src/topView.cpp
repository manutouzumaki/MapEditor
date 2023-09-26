static f32 gTileX;
static f32 gTileY;
static f32 gTileW;
static f32 gTileH;

void RenderQuad(f32 offsetX, f32 offsetY, f32 zoom)
{
    f32 ax = gTileX;
    f32 ay = gTileY;
    f32 bx = gTileX + gTileW;
    f32 by = gTileY;
    
    f32 sax, say, sbx, sby;

    WorldToScreen(ax, ay, sax, say, offsetX, offsetY, zoom); 
    WorldToScreen(bx, by, sbx, sby, offsetX, offsetY, zoom); 

    DrawLine(sax,  say, 0, sbx,  sby, 0, 0xFFFF3333);

    ax = gTileX;
    ay = gTileY + gTileH;
    bx = gTileX + gTileW;
    by = gTileY + gTileH;
    
    WorldToScreen(ax, ay, sax, say, offsetX, offsetY, zoom); 
    WorldToScreen(bx, by, sbx, sby, offsetX, offsetY, zoom); 

    DrawLine(sax,  say, 0, sbx,  sby, 0, 0xFFFF3333);

    ax = gTileX;
    ay = gTileY;
    bx = gTileX;
    by = gTileY + gTileH;
    
    WorldToScreen(ax, ay, sax, say, offsetX, offsetY, zoom); 
    WorldToScreen(bx, by, sbx, sby, offsetX, offsetY, zoom); 

    DrawLine(sax,  say, 0, sbx,  sby, 0, 0xFFFF3333);

    ax = gTileX + gTileW;
    ay = gTileY;
    bx = gTileX + gTileW;
    by = gTileY + gTileH;
    
    WorldToScreen(ax, ay, sax, say, offsetX, offsetY, zoom); 
    WorldToScreen(bx, by, sbx, sby, offsetX, offsetY, zoom); 

    DrawLine(sax,  say, 0, sbx,  sby, 0, 0xFFFF3333);
}

void SetupTopView(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseSetup(view);
    
}

void ProcessTopView(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseProcess(view);
    
    // TODO: get the mouse relative to the view
    i32 mouseRelX = MouseRelX(view);
    i32 mouseRelY = MouseRelY(view);

    if(MouseIsHot(view))
    {
        if(MouseJustDown(MOUSE_BUTTON_LEFT))
        {
            f32 mouseWorldX, mouseWorldY;
            ScreenToWorld(mouseRelX, mouseRelY, mouseWorldX, mouseWorldY,
                          state->offsetX, state->offsetY, state->zoom);
    
            gTileX = floorf(mouseWorldX / 64.0f) * 64.0f;
            gTileY = floorf(mouseWorldY / 64.0f) * 64.0f; 
            gTileW = 64;
            gTileH = 64;
        }
    }
}

void RenderTopView(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    ViewOrthoBaseRender(view);

    RenderQuad(state->offsetX, state->offsetY, state->zoom);
    LineRendererDraw();
}
