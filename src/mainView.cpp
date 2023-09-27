void SetupMainView(View *view)
{
    view->id = VIEW_MAIN;
}

void ProcessMainView(View *view)
{

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
    static f32 angle = 0; angle += 0.01f;
    view->cbuffer.world = Mat4RotateY(angle);// * Mat4RotateX(angle);
    UpdateConstBuffer(&gConstBuffer, (void *)&view->cbuffer);

    // Render the vertices
    deviceContext->Draw(gDynamicVertexBuffer.verticesCount, 0);
}
