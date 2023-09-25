static f32 gFrontViewOffsetX;
static f32 gFrontViewOffsetY;

static f32 gFrontViewLastClickX;
static f32 gFrontViewLastClickY;

static f32 gFrontViewWheelOffset;
static f32 gFrontViewZoom;

static bool gFrontViewHot;

void SetupFrontView(View *view)
{
    gFrontViewWheelOffset = 25.0f;
    gFrontViewZoom = Remap(0.0f, 50.0f, 0.05f, 1.0f, gFrontViewWheelOffset);
}

void ProcessFrontView(View *view)
{
    // TODO: get the mouse relative to the view
    i32 mouseRelToClientX = MouseX() - 200; // TODO: use a global for this value
    i32 mouseRelToClientY = gCurrentWindowHeight - MouseY();

    i32 mouseRelX = mouseRelToClientX - view->x;
    i32 mouseRelY = mouseRelToClientY - view->y;

    if(mouseRelX >= view->w*-0.5f && mouseRelX <= view->w*0.5f &&
       mouseRelY >= view->h*-0.5f && mouseRelY <= view->h*0.5f)
    {
        gFrontViewWheelOffset += (f32)MouseWheelDelta();
        gFrontViewWheelOffset = Clamp(gFrontViewWheelOffset, 0.0f, 50.0f);
        gFrontViewZoom = Remap(0.0f, 50.0f, 0.05f, 1.0f, gFrontViewWheelOffset);

        if(MouseJustDown(MOUSE_BUTTON_RIGHT))
        {
            gFrontViewHot = true;
            gFrontViewLastClickX = mouseRelX;
            gFrontViewLastClickY = mouseRelY;
        }
    }

    if(MouseJustUp(MOUSE_BUTTON_RIGHT) && gFrontViewHot)
    {
        gFrontViewHot = false;
    }

    if(gFrontViewHot)
    {
        gFrontViewOffsetX += mouseRelX - gFrontViewLastClickX;
        gFrontViewOffsetY += mouseRelY - gFrontViewLastClickY;
        gFrontViewLastClickX = mouseRelX;
        gFrontViewLastClickY = mouseRelY;
    }
}

void RenderFrontView(View *view)
{
    // set the shader
    deviceContext->VSSetShader(gColShader.vertex, 0, 0);
    deviceContext->PSSetShader(gColShader.fragment, 0, 0);

    f32 viewUnit = gViewMaxZoom * gFrontViewZoom;
    
    // Update the constBuffer
    view->cbuffer.view = Mat4LookAt({0, 0, -50}, {0, 0, 0}, {0, 1, 0});
    UpdateConstBuffer(&gConstBuffer, (void *)&view->cbuffer);
    Draw2dGrid(gFrontViewOffsetX, gFrontViewOffsetY, view->w, view->h, viewUnit);

    // Update the view matrix
    view->cbuffer.view = Mat4LookAt({-gFrontViewOffsetX, -gFrontViewOffsetY, -50},
                                    {-gFrontViewOffsetX, -gFrontViewOffsetY, 0},
                                    {0, 1, 0});
    UpdateConstBuffer(&gConstBuffer, (void *)&view->cbuffer);
    DrawLine(0, 0, 0, viewUnit, 0, 0, 0xFFFF0000);
    DrawLine(0, 0, 0, 0, viewUnit, 0, 0xFF00FF00);
    LineRendererDraw();
}
