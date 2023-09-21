void SetupFrontView(View *view)
{

}

void ProcessFrontView(View *view)
{

}

void RenderFrontView(View *view)
{
    // Update the constBuffer
    static f32 angle = 0; angle += 0.01f;
    view->cbuffer.world = Mat4Translate(-sinf(angle) * 250, cosf(angle) * 100, 0) *
                          Mat4Scale(300, 300, 1) * Mat4RotateZ(angle);
    UpdateConstBuffer(&constBuffer, (void *)&view->cbuffer);
    
    // Render the vertices
    deviceContext->Draw(vertexBuffer.verticesCount, 0);
}
