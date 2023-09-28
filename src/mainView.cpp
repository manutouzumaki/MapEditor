void SetupMainView(View *view)
{
    view->id = VIEW_MAIN;

    ViewPerspState *state = &view->perspState;
    state->camera.dir = {0, 0, 1};
    state->camera.right = {1, 0, 0};
    state->camera.up = {0, 1, 0};
    state->camera.pos = {0, 2.5, -5};
    state->camera.rot = {0, 0, 0};
}

void ProcessMainView(View *view)
{

    ViewPerspState *state = &view->perspState;

    if(MouseIsHot(view))
    {
        if(MouseJustDown(MOUSE_BUTTON_LEFT))
        {
            state->leftButtonDown = true;
            ShowCursor(false);
        }
        if(MouseJustDown(MOUSE_BUTTON_RIGHT))
        {
            state->rightButtonDown = true;
            ShowCursor(false);
        }

        i32 mouseWheelDelta = MouseWheelDelta();
        if(mouseWheelDelta != 0)
        {
            state->camera.pos = state->camera.pos + state->camera.dir * ((f32)mouseWheelDelta) * 0.5f;
        }

    }

    if(MouseJustUp(MOUSE_BUTTON_LEFT) && state->leftButtonDown)
    {
        state->leftButtonDown = false;
        ShowCursor(true);
    } 
    if(MouseJustUp(MOUSE_BUTTON_RIGHT) && state->rightButtonDown)
    {
        state->rightButtonDown = false;
        ShowCursor(true);
    }
    
    if(state->leftButtonDown)
    {
        f32 deltaX = (f32)(MouseX() - MouseLastX()) * 0.0015f;
        f32 deltaY = (f32)(MouseY() - MouseLastY()) * 0.0015f;

        state->camera.rot.y -= deltaX;
        state->camera.rot.x -= deltaY;

        if (state->camera.rot.x > (89.0f/180.0f) * PI)
            state->camera.rot.x = (89.0f / 180.0f) * PI;
        if (state->camera.rot.x < -(89.0f / 180.0f) * PI)
            state->camera.rot.x = -(89.0f/180.0f) * PI;
    }

    if(state->rightButtonDown)
    {
        f32 deltaX = (f32)(MouseX() - MouseLastX()) * 0.015f;
        f32 deltaY = (f32)(MouseY() - MouseLastY()) * 0.015f;

        state->camera.pos = state->camera.pos + state->camera.right * -deltaX;
        state->camera.pos = state->camera.pos + state->camera.up * -deltaY;
    }

    if(state->leftButtonDown || state->rightButtonDown)
    {
        // convert mouse relative coords to screen coords to set the mouse at
        // the center of the view
        f32 mouseRelX = 0; 
        f32 mouseRelY = 0; 
        f32 mouseRelClientX = mouseRelX + view->x;
        f32 mouseRelClientY = mouseRelY + view->y;
        f32 mouseScrX = gFixWidth +  mouseRelClientX;
        f32 mouseScrY = gCurrentWindowHeight - mouseRelClientY;
        
        SetCursorPos(gWindowX + mouseScrX, gWindowY + mouseScrY);
        gInput.x = mouseScrX;
        gInput.y = mouseScrY;
        gLastInput.x = mouseScrX;
        gLastInput.y = mouseScrY;
    }

    Vec3 dir = {0, 0, 1};
    state->camera.dir = Mat4TransformVector(Mat4RotateX(state->camera.rot.x), dir);
    state->camera.dir = Mat4TransformVector(Mat4RotateY(state->camera.rot.y), state->camera.dir);
    state->camera.dir = Mat4TransformVector(Mat4RotateZ(state->camera.rot.z), state->camera.dir);
    state->camera.dir = Vec3Normalized(state->camera.dir);

    state->camera.right = Vec3Normalized(Vec3Cross(state->camera.dir, {0, 1, 0}));
    state->camera.up =  Vec3Normalized(Vec3Cross(state->camera.right, state->camera.dir));

    view->cbuffer.view = Mat4LookAt(state->camera.pos, state->camera.pos + state->camera.dir, {0, 1, 0});
    view->cbuffer.viewPos = state->camera.pos;
}

void RenderMainView(View *view)
{
    deviceContext->VSSetShader(gColShader.vertex, 0, 0);
    deviceContext->PSSetShader(gColShader.fragment, 0, 0);

    u32 stride = sizeof(Vertex);
    u32 offset = 0;
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(gDynamicVertexBuffer.layout);
    deviceContext->IASetVertexBuffers(0, 1, &gDynamicVertexBuffer.GPUBuffer, &stride, &offset);

    // Update the constBuffer
    view->cbuffer.world = Mat4Identity();
    UpdateConstBuffer(&gConstBuffer, (void *)&view->cbuffer);

    // Render the vertices
    deviceContext->Draw(gDynamicVertexBuffer.verticesCount, 0);
}
