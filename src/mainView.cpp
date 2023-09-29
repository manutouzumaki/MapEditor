void SetupMainView(View *view)
{
    view->id = VIEW_MAIN;

    ViewPerspState *state = &view->perspState;
    state->camera.dir = {0, 0, 1};
    state->camera.right = {1, 0, 0};
    state->camera.up = {0, 1, 0};
    state->camera.pos = {0, 2.5, -5};
    state->camera.rot = {0, 0, 0};

    view->mousePicking = MousePicking3D;
}

void ProcessMainView(View *view)
{
    EditorModeMove3DCamera(view);
    EditorModeSelectPoly(view);
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
