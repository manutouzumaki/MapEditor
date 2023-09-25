static f32 gTopViewOffsetX;
static f32 gTopViewOffsetY;

static f32 gTopViewLastClickX;
static f32 gTopViewLastClickY;

static f32 gTopViewWheelOffset;
static f32 gTopViewZoom;

static bool gTopViewHot;

static f32 gTileX;
static f32 gTileY;
static f32 gTileW;
static f32 gTileH;

void WorldToScreen(f32 wx, f32 wy, f32 &sx, f32 &sy)
{
    sx = (wx - gTopViewOffsetX) * gTopViewZoom;
    sy = (wy - gTopViewOffsetY) * gTopViewZoom;
}

void ScreenToWorld(f32 sx, f32 sy, f32 &wx, f32 &wy)
{
    wx = (sx / gTopViewZoom) + gTopViewOffsetX;
    wy = (sy / gTopViewZoom) + gTopViewOffsetY;
}

void SetupTopView(View *view)
{
    gTopViewWheelOffset = 25.0f;
    gTopViewZoom = Remap(0.0f, 50.0f, 0.05f, 10.0f, gTopViewWheelOffset);
}

void ProcessTopView(View *view)
{
    // TODO: get the mouse relative to the view
    i32 mouseRelToClientX = MouseX() - 200; // TODO: use a global for this value
    i32 mouseRelToClientY = gCurrentWindowHeight - MouseY();

    i32 mouseRelX = mouseRelToClientX - view->x;
    i32 mouseRelY = mouseRelToClientY - view->y;

    if(mouseRelX >= view->w*-0.5f && mouseRelX <= view->w*0.5f &&
       mouseRelY >= view->h*-0.5f && mouseRelY <= view->h*0.5f)
    {
        if(MouseJustDown(MOUSE_BUTTON_RIGHT))
        {
            gTopViewHot = true;
            gTopViewLastClickX = mouseRelX;
            gTopViewLastClickY = mouseRelY;
        }

        if(MouseJustDown(MOUSE_BUTTON_LEFT))
        {
            f32 mouseWorldX, mouseWorldY;
            ScreenToWorld(mouseRelX, mouseRelY, mouseWorldX, mouseWorldY);
    
            gTileX = floorf(mouseWorldX / 64.0f) * 64.0f;
            gTileY = floorf(mouseWorldY / 64.0f) * 64.0f; 
            gTileW = 64;
            gTileH = 64;
        }


        f32 mouseWorldPreZoomX, mouseWorldPreZoomY;
        ScreenToWorld(mouseRelX, mouseRelY, mouseWorldPreZoomX, mouseWorldPreZoomY);

        i32 mouseWheelDelta = MouseWheelDelta();
        if(mouseWheelDelta != 0)
        {
            gTopViewWheelOffset += (f32)mouseWheelDelta;
            gTopViewWheelOffset = Clamp(gTopViewWheelOffset, 0.0f, 50.0f);
            gTopViewZoom = Remap(0.0f, 50.0f, 0.2f, 10.0f, gTopViewWheelOffset);
        }

        f32 mouseWorldPostZoomX, mouseWorldPostZoomY;
        ScreenToWorld(mouseRelX, mouseRelY, mouseWorldPostZoomX, mouseWorldPostZoomY);

        gTopViewOffsetX += (mouseWorldPreZoomX - mouseWorldPostZoomX);
        gTopViewOffsetY += (mouseWorldPreZoomY - mouseWorldPostZoomY);
    }
    
    if(MouseJustUp(MOUSE_BUTTON_RIGHT) && gTopViewHot)
    {
        gTopViewHot = false;
    }

    if(gTopViewHot)
    {
        gTopViewOffsetX += (gTopViewLastClickX - mouseRelX) / gTopViewZoom;
        gTopViewOffsetY += (gTopViewLastClickY - mouseRelY) / gTopViewZoom;
        gTopViewLastClickX = mouseRelX;
        gTopViewLastClickY = mouseRelY;
    }

}

void RenderGrid()
{
    for(f32 y = -100.0f; y < 100.0f; ++y)
    {
        f32 ax = -100.0f * 64;
        f32 ay = y * 64;
        f32 bx = 100.0f * 64;
        f32 by = y * 64;
        
        f32 sax, say, sbx, sby;

        WorldToScreen(ax, ay, sax, say); 
        WorldToScreen(bx, by, sbx, sby); 

        DrawLine(sax,  say, 0, sbx,  sby, 0, 0xFF333333);
    }

    for(f32 x = -100.0f; x < 100.0f; ++x)
    {

        f32 ax = x * 64;
        f32 ay = -100.0f * 64.0f;
        f32 bx = x * 64;
        f32 by = 64.f*100.0f;
        
        f32 sax, say, sbx, sby;

        WorldToScreen(ax, ay, sax, say); 
        WorldToScreen(bx, by, sbx, sby); 

        DrawLine(sax,  say, 0, sbx,  sby, 0, 0xFF333333);
    }
}

void RenderQuad()
{
    f32 ax = gTileX;
    f32 ay = gTileY;
    f32 bx = gTileX + gTileW;
    f32 by = gTileY;
    
    f32 sax, say, sbx, sby;

    WorldToScreen(ax, ay, sax, say); 
    WorldToScreen(bx, by, sbx, sby); 

    DrawLine(sax,  say, 0, sbx,  sby, 0, 0xFFFF3333);

    ax = gTileX;
    ay = gTileY + gTileH;
    bx = gTileX + gTileW;
    by = gTileY + gTileH;
    
    WorldToScreen(ax, ay, sax, say); 
    WorldToScreen(bx, by, sbx, sby); 

    DrawLine(sax,  say, 0, sbx,  sby, 0, 0xFFFF3333);

    ax = gTileX;
    ay = gTileY;
    bx = gTileX;
    by = gTileY + gTileH;
    
    WorldToScreen(ax, ay, sax, say); 
    WorldToScreen(bx, by, sbx, sby); 

    DrawLine(sax,  say, 0, sbx,  sby, 0, 0xFFFF3333);

    ax = gTileX + gTileW;
    ay = gTileY;
    bx = gTileX + gTileW;
    by = gTileY + gTileH;
    
    WorldToScreen(ax, ay, sax, say); 
    WorldToScreen(bx, by, sbx, sby); 

    DrawLine(sax,  say, 0, sbx,  sby, 0, 0xFFFF3333);
}

void RenderTopView(View *view)
{
    // set the shader
    deviceContext->VSSetShader(gColShader.vertex, 0, 0);
    deviceContext->PSSetShader(gColShader.fragment, 0, 0);

    // Update the constBuffer
    view->cbuffer.view = Mat4LookAt({0, 0, -50}, {0, 0, 0}, {0, 1, 0});
    UpdateConstBuffer(&gConstBuffer, (void *)&view->cbuffer);
        
    RenderGrid();
    LineRendererDraw();

    RenderQuad();
    LineRendererDraw();
}
