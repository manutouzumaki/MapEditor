static f32 gSideViewOffsetX;
static f32 gSideViewOffsetY;

static f32 gSideViewLastClickX;
static f32 gSideViewLastClickY;

static bool gSideViewHot;

void SetupSideView(View *view)
{

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
        gSideViewOffsetY += -(mouseRelY - gSideViewLastClickY);
        gSideViewLastClickX = mouseRelX;
        gSideViewLastClickY = mouseRelY;
    }
}

void RenderSideView(View *view)
{
    // Update the constBuffer
    UpdateConstBuffer(&constBuffer, (void *)&view->cbuffer);

    Draw2dGrid(gSideViewOffsetX, gSideViewOffsetY, view->w, view->h, 10.0f);

    u32 stride = sizeof(Vertex);
    u32 offset = 0;
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(vertexBuffer.layout);
    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer.GPUBuffer, &stride, &offset);
    
    deviceContext->VSSetShader(colShader.vertex, 0, 0);
    deviceContext->PSSetShader(colShader.fragment, 0, 0);

    static f32 angle = 0; angle += 0.01f;
    view->cbuffer.world = Mat4Translate(-sinf(angle) * 250, cosf(angle) * 100, 0) *
                          Mat4Scale(100, 100, 1) * Mat4RotateZ(angle);
    UpdateConstBuffer(&constBuffer, (void *)&view->cbuffer);
    
    // Render the vertices
    deviceContext->Draw(vertexBuffer.verticesCount, 0);
}
