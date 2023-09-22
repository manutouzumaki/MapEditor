static f32 gSideViewOffsetX;
static f32 gSideViewOffsetY;

static f32 gSideViewLastClickX;
static f32 gSideViewLastClickY;

static f32 gSideViewWheelOffset;
static f32 gSideViewZoom;

static bool gSideViewHot;

void SetupSideView(View *view)
{
    gSideViewWheelOffset = 25.0f;
    gSideViewZoom = Remap(0.0f, 50.0f, 0.05f, 1.0f, gSideViewWheelOffset);
}

void ProcessSideView(View *view)
{
    // TODO: get the mouse relative to the view
    i32 mouseRelToClientX = mouseX - 200; // TODO: use a global for this value
    i32 mouseRelToClientY = currentWindowHeight - mouseY;

    i32 mouseRelX = mouseRelToClientX - view->x;
    i32 mouseRelY = mouseRelToClientY - view->y;

    if(mouseRelX >= view->w*-0.5f && mouseRelX <= view->w*0.5f &&
       mouseRelY >= view->h*-0.5f && mouseRelY <= view->h*0.5f)
    {
        gSideViewWheelOffset += (f32)mouseWheelDelta;
        gSideViewWheelOffset = Clamp(gSideViewWheelOffset, 0.0f, 50.0f);
        gSideViewZoom = Remap(0.0f, 50.0f, 0.05f, 1.0f, gSideViewWheelOffset);

        if(MouseJustDown())
        {
            gSideViewHot = true;
            gSideViewLastClickX = mouseRelX;
            gSideViewLastClickY = mouseRelY;
        }
    }

    if(MouseJustUp() && gSideViewHot)
    {
        gSideViewHot = false;
    }

    if(gSideViewHot)
    {
        gSideViewOffsetX += mouseRelX - gSideViewLastClickX;
        gSideViewOffsetY += mouseRelY - gSideViewLastClickY;
        gSideViewLastClickX = mouseRelX;
        gSideViewLastClickY = mouseRelY;
    }
}

void RenderSideView(View *view)
{
    // set the shader
    deviceContext->VSSetShader(colShader.vertex, 0, 0);
    deviceContext->PSSetShader(colShader.fragment, 0, 0);

    f32 viewUnit = gViewMaxZoom * gSideViewZoom;
    
    // Update the constBuffer
    view->cbuffer.view = Mat4LookAt({0, 0, -50}, {0, 0, 0}, {0, 1, 0});
    UpdateConstBuffer(&constBuffer, (void *)&view->cbuffer);
    Draw2dGrid(gSideViewOffsetX, gSideViewOffsetY, view->w, view->h, viewUnit);

    // Update the view matrix
    view->cbuffer.view = Mat4LookAt({-gSideViewOffsetX, -gSideViewOffsetY, -50},
                                    {-gSideViewOffsetX, -gSideViewOffsetY, 0},
                                    {0, 1, 0});
    UpdateConstBuffer(&constBuffer, (void *)&view->cbuffer);
    DrawLine(0, 0, 0, viewUnit, 0, 0, 0xFFFF0000);
    DrawLine(0, 0, 0, 0, viewUnit, 0, 0xFF00FF00);
    LineRendererDraw();
}
