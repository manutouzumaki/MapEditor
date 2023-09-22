void SetupMainView(View *view)
{

}

void ProcessMainView(View *view)
{

}

void RenderMainView(View *view)
{
    deviceContext->VSSetShader(colShader.vertex, 0, 0);
    deviceContext->PSSetShader(colShader.fragment, 0, 0);

    // Update the constBuffer
    static f32 angle = 0; angle += 0.01f;
    view->cbuffer.world = Mat4RotateY(angle) * Mat4RotateX(angle);
    UpdateConstBuffer(&constBuffer, (void *)&view->cbuffer);

    // Render the vertices
    deviceContext->Draw(vertexBuffer.verticesCount, 0);
}
