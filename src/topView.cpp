static f32 gTopViewOffsetX;
static f32 gTopViewOffsetY;

static f32 gTopViewLastClickX;
static f32 gTopViewLastClickY;

static f32 gTopViewWheelOffset;
static f32 gTopViewZoom;

static bool gTopViewHot;

void SetupTopView(View *view)
{
    gTopViewWheelOffset = 25.0f;
    gTopViewZoom = Remap(0.0f, 50.0f, 0.05f, 1.0f, gTopViewWheelOffset);
}

void ProcessTopView(View *view)
{
    // TODO: get the mouse relative to the view
    i32 mouseRelToClientX = mouseX - 200; // TODO: use a global for this value
    i32 mouseRelToClientY = currentWindowHeight - mouseY;

    i32 mouseRelX = mouseRelToClientX - view->x;
    i32 mouseRelY = mouseRelToClientY - view->y;

    if(mouseRelX >= view->w*-0.5f && mouseRelX <= view->w*0.5f &&
       mouseRelY >= view->h*-0.5f && mouseRelY <= view->h*0.5f)
    {
        gTopViewWheelOffset += (f32)mouseWheelDelta;
        gTopViewWheelOffset = Clamp(gTopViewWheelOffset, 0.0f, 50.0f);
        gTopViewZoom = Remap(0.0f, 50.0f, 0.05f, 1.0f, gTopViewWheelOffset);

        if(MouseJustDown())
        {
            gTopViewHot = true;
            gTopViewLastClickX = mouseRelX;
            gTopViewLastClickY = mouseRelY;
        }
    }
    
    if(MouseJustUp() && gTopViewHot)
    {
        gTopViewHot = false;
    }

    if(gTopViewHot)
    {
        gTopViewOffsetX += mouseRelX - gTopViewLastClickX;
        gTopViewOffsetY += mouseRelY - gTopViewLastClickY;
        gTopViewLastClickX = mouseRelX;
        gTopViewLastClickY = mouseRelY;
    }
}

void RenderTopView(View *view)
{
    // set the shader
    deviceContext->VSSetShader(colShader.vertex, 0, 0);
    deviceContext->PSSetShader(colShader.fragment, 0, 0);

    f32 viewUnit = gViewMaxZoom * gTopViewZoom;
    
    // Update the constBuffer
    view->cbuffer.view = Mat4LookAt({0, 0, -50}, {0, 0, 0}, {0, 1, 0});
    UpdateConstBuffer(&constBuffer, (void *)&view->cbuffer);
    Draw2dGrid(gTopViewOffsetX, gTopViewOffsetY, view->w, view->h, viewUnit);

    // Update the view matrix
    view->cbuffer.view = Mat4LookAt({-gTopViewOffsetX, -gTopViewOffsetY, -50},
                                    {-gTopViewOffsetX, -gTopViewOffsetY, 0},
                                    {0, 1, 0});
    UpdateConstBuffer(&constBuffer, (void *)&view->cbuffer);
    DrawLine(0, 0, 0, viewUnit, 0, 0, 0xFFFF0000);
    DrawLine(0, 0, 0, 0, viewUnit, 0, 0xFF00FF00);
    LineRendererDraw();
}
