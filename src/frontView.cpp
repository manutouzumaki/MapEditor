static f32 gFrontViewOffsetX;
static f32 gFrontViewOffsetY;

static f32 gFrontViewLastClickX;
static f32 gFrontViewLastClickY;

static bool gFrontViewHot;

void SetupFrontView(View *view)
{

}

void ProcessFrontView(View *view)
{
    // TODO: get the mouse relative to the view
    i32 mouseRelToClientX = mouseX - 200; // TODO: use a global for this value
    i32 mouseRelToClientY = currentWindowHeight - mouseY;

    i32 mouseRelX = mouseRelToClientX - view->x;
    i32 mouseRelY = mouseRelToClientY - view->y;

    if(mouseRelX >= view->w*-0.5f && mouseRelX <= view->w*0.5f &&
       mouseRelY >= view->h*-0.5f && mouseRelY <= view->h*0.5f)
    {
        if(MouseJustDown())
        {
            gFrontViewHot = true;
            gFrontViewLastClickX = mouseRelX;
            gFrontViewLastClickY = mouseRelY;
        }
    }

    if(MouseJustUp() && gFrontViewHot)
    {
        gFrontViewHot = false;
    }

    if(gFrontViewHot)
    {
        gFrontViewOffsetX += mouseRelX - gFrontViewLastClickX;
        gFrontViewOffsetY += -(mouseRelY - gFrontViewLastClickY);
        gFrontViewLastClickX = mouseRelX;
        gFrontViewLastClickY = mouseRelY;
    }
}

void RenderFrontView(View *view)
{
    // Update the constBuffer
    UpdateConstBuffer(&constBuffer, (void *)&view->cbuffer);
    


    Draw2dGrid(gFrontViewOffsetX, gFrontViewOffsetY, view->w, view->h, 30.0f);

    u32 stride = sizeof(Vertex);
    u32 offset = 0;
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(vertexBuffer.layout);
    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer.GPUBuffer, &stride, &offset);
    
    deviceContext->VSSetShader(colShader.vertex, 0, 0);
    deviceContext->PSSetShader(colShader.fragment, 0, 0);


    // Update the constBuffer
    static f32 angle = 0; angle += 0.01f;
    view->cbuffer.world = Mat4Translate(-sinf(angle) * 250, cosf(angle) * 100, 0) *
                          Mat4Scale(300, 300, 1) * Mat4RotateZ(angle);
    UpdateConstBuffer(&constBuffer, (void *)&view->cbuffer);
    
    // Render the vertices
    deviceContext->Draw(vertexBuffer.verticesCount, 0);
}
