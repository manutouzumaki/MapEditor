struct ViewManager
{
    f32 divisorX = 0.5f;
    f32 divisorY = 0.5f;
    f32 newDivisorX = 0.5f;
    f32 newDivisorY = 0.5f;
    f32 separation = 6;
    bool modifyViewsX = false;
    bool modifyViewsY = false;

    View views[4];
};

bool MouseInDivisorX(ViewManager *vm, Rect clientRect, f32 fixWidth)
{
    // handle the vertical line
    f32 xpos = clientRect.w * vm->divisorX;
    RectMinMax vRect = {xpos-vm->separation*0.5f, 0, xpos+vm->separation*0.5f, clientRect.h};

    i32 mouseRelX = MouseX() - fixWidth;
    i32 mouseRelY = MouseY();
    
    if(mouseRelX >= vRect.minX && mouseRelX <= vRect.maxX &&
       mouseRelY >= vRect.minY && mouseRelY <= vRect.maxY)
    {
        return true;
    }
    return false;
}

bool MouseInDivisorY(ViewManager *vm, Rect clientRect, f32 fixWidth)
{
    // handle the horizontal line
    f32 ypos = clientRect.h * vm->divisorY;
    RectMinMax vRect = {0, ypos-vm->separation*0.5f, clientRect.w, ypos+vm->separation*0.5f};

    i32 mouseRelX = MouseX() - fixWidth;
    i32 mouseRelY = MouseY();
    
    if(mouseRelX >= vRect.minX && mouseRelX <= vRect.maxX &&
       mouseRelY >= vRect.minY && mouseRelY <= vRect.maxY)
    {
        return true;
    }
    return false;
}

void RecalculateViewDimensions(ViewManager *vm, Rect clientRect)
{
    f32 viewWidth  = clientRect.w * vm->divisorX;
    f32 viewHeight = clientRect.h * vm->divisorY;
    View *views = vm->views;
    if(viewWidth-vm->separation > 1 && viewHeight-vm->separation > 1)
    {
        ViewResize(views + 0,
                   viewWidth*0.5f,
                   (clientRect.h*(1.0f-vm->divisorY))+viewHeight*0.5f,
                   viewWidth-vm->separation, viewHeight-vm->separation);
    }

    viewWidth  = clientRect.w * (1.0f-vm->divisorX);
    viewHeight = clientRect.h * vm->divisorY;
    if(viewWidth-vm->separation > 1 && viewHeight-vm->separation > 1)
    {
        ViewResize(views + 1,
                   (clientRect.w*vm->divisorX)+viewWidth*0.5f,
                   (clientRect.h*(1.0f-vm->divisorY))+viewHeight*0.5f,
                   viewWidth-vm->separation, viewHeight-vm->separation);
    }

    viewWidth  = clientRect.w * vm->divisorX;
    viewHeight = clientRect.h * (1.0f-vm->divisorY);
    if(viewWidth-vm->separation > 1 && viewHeight-vm->separation > 1)
    {
        ViewResize(views + 2,
                   viewWidth*0.5f,
                   viewHeight*0.5f,
                   viewWidth-vm->separation, viewHeight-vm->separation);
    }

    viewWidth  = clientRect.w * (1.0f-vm->divisorX);
    viewHeight = clientRect.h * (1.0f-vm->divisorY);
    if(viewWidth-vm->separation > 1 && viewHeight-vm->separation > 1)
    {
        ViewResize(views + 3,
                   (clientRect.w*vm->divisorX)+viewWidth*0.5f,
                   viewHeight*0.5f,
                   viewWidth-vm->separation, viewHeight-vm->separation);
    }
}

void ViewManagerSetup(ViewManager *vm, Rect clientRect)
{
    f32 viewWidth  = clientRect.w * vm->divisorX;
    f32 viewHeight = clientRect.h * vm->divisorY;
    View *views = vm->views;
    views[0] = ViewCreate(viewWidth*0.5f,
                          (clientRect.h*(1.0f-vm->divisorY))+viewHeight*0.5f,
                          viewWidth-vm->separation, viewHeight-vm->separation,
                          PROJ_TYPE_PERSP,
                          SetupMainView, ProcessMainView, RenderMainView);

    viewWidth  = clientRect.w * (1.0f-vm->divisorX);
    viewHeight = clientRect.h * vm->divisorY;
    views[1] = ViewCreate((clientRect.w*vm->divisorX)+viewWidth*0.5f,
                          (clientRect.h*(1.0f-vm->divisorY))+viewHeight*0.5f,
                          viewWidth-vm->separation, viewHeight-vm->separation,
                          PROJ_TYPE_ORTHO,
                          SetupTopView, ProcessTopView, RenderTopView);

    viewWidth  = clientRect.w * vm->divisorX;
    viewHeight = clientRect.h * (1.0f-vm->divisorY);
    views[2] = ViewCreate(viewWidth*0.5f,
                          viewHeight*0.5f,
                          viewWidth-vm->separation, viewHeight-vm->separation,
                          PROJ_TYPE_ORTHO,
                          SetupFrontView, ProcessFrontView, RenderFrontView);

    viewWidth  = clientRect.w * (1.0f-vm->divisorX);
    viewHeight = clientRect.h * (1.0f-vm->divisorY);
    views[3] = ViewCreate((clientRect.w*vm->divisorX)+viewWidth*0.5f,
                          viewHeight*0.5f,
                          viewWidth-vm->separation, viewHeight-vm->separation,
                          PROJ_TYPE_ORTHO,
                          SetupSideView, ProcessSideView, RenderSideView);

    for(i32 i = 0; i < 4; ++i)
    {
        ViewSetup(views + i);
    }
}


void ViewManagerHandleInput(ViewManager *vm, Rect clientRect)
{
    if(MouseJustDown(MOUSE_BUTTON_LEFT))
    {
        if(MouseInDivisorX(vm, clientRect, gFixWidth)) vm->modifyViewsX = true;
        if(MouseInDivisorY(vm, clientRect, gFixWidth)) vm->modifyViewsY = true;
    }

    if(MouseJustUp(MOUSE_BUTTON_LEFT))
    {
        if(vm->modifyViewsX) { vm->divisorX = vm->newDivisorX; vm->modifyViewsX = false; }
        if(vm->modifyViewsY) { vm->divisorY = vm->newDivisorY; vm->modifyViewsY = false; }
        RecalculateViewDimensions(vm, clientRect);
    }

    if(vm->modifyViewsX)
    {
        i32 mouseRelX = MouseX() - gFixWidth;
        vm->newDivisorX = Clamp(mouseRelX / clientRect.w, 0.1f, 0.9f);
    }

    if(vm->modifyViewsY)
    {
        i32 mouseRelY = MouseY();
        vm->newDivisorY = Clamp(mouseRelY / clientRect.h, 0.1f, 0.9f);
    }
}



void ViewManagerProcessAndRenderToViews(ViewManager *vm)
{
    View *views = vm->views;
    for(i32 i = 0; i < 4; ++i)
    {
        ViewProcess(views + i);

        // reload the dynamic vertex buffer
        if(gDirtyFlag == true)
        {
            gDirtyFlag = false;
            gDynamicVertexBuffer.used = 0;
            gDynamicVertexBuffer.verticesCount = 0;
            Entity *entity = gEntityList;
            while(entity)
            {
                BrushVertexPushToVertexBuffer(&entity->brushVert);
                entity = entity->next;
            }
        }


        ViewRender(views + i);
    }
}

void ViewManagerRenderViews(ViewManager *vm, CBuffer *cbuffer, Rect clientRect)
{
    View *views = vm->views;
    //  render to main render target output
    u32 stride = sizeof(Vertex);
    u32 offset = 0;
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(gVertexBuffer.layout);
    deviceContext->IASetVertexBuffers(0, 1, &gVertexBuffer.GPUBuffer, &stride, &offset);
    deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
    
    ImVec4 clear_color = ImVec4(116.0f/255.0f, 116.0f/255.0f, 245.0f/255.0f, 1.00f);
    float clearColor[] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
    deviceContext->ClearRenderTargetView(renderTargetView, clearColor);
    deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

    deviceContext->VSSetShader(gTex2DShader.vertex, 0, 0);
    deviceContext->PSSetShader(gTex2DShader.fragment, 0, 0);

    // Render the for Views
    for(i32 i = 0; i < 4; ++i)
    {
        deviceContext->PSSetShaderResources(0, 1, &views[i].fb.shaderResourceView);
        cbuffer->world = Mat4Translate(clientRect.x + views[i].x,
                                      clientRect.y + views[i].y, 1) * Mat4Scale(views[i].w, views[i].h, 1);
        UpdateConstBuffer(&gConstBuffer, (void *)cbuffer);
        deviceContext->Draw(gVertexBuffer.verticesCount, 0);
    }

    if(vm->modifyViewsX)
    {
        // draw the mouse position 
        deviceContext->VSSetShader(gCol2DShader.vertex, 0, 0);
        deviceContext->PSSetShader(gCol2DShader.fragment, 0, 0);

        // Horizontal line
        cbuffer->world = Mat4Translate(clientRect.x + clientRect.w * vm->newDivisorX,
                                      clientRect.y + clientRect.h*0.5f, 0.0f) * Mat4Scale(3, clientRect.h, 1);
        UpdateConstBuffer(&gConstBuffer, (void *)cbuffer);
        deviceContext->Draw(gVertexBuffer.verticesCount, 0);
    }
    if(vm->modifyViewsY)
    {
        // draw the mouse position 
        deviceContext->VSSetShader(gCol2DShader.vertex, 0, 0);
        deviceContext->PSSetShader(gCol2DShader.fragment, 0, 0);
        // Vertical line
        cbuffer->world = Mat4Translate(clientRect.x + clientRect.w*0.5f,
                                      clientRect.y + clientRect.h * (1.0f-vm->newDivisorY), 0.0f) * Mat4Scale(clientRect.w, 3, 1);
        UpdateConstBuffer(&gConstBuffer, (void *)cbuffer);
        deviceContext->Draw(gVertexBuffer.verticesCount, 0);
    }
}

void ViewManagerShutDown(ViewManager *vm)
{
    Entity *entity = gEntityList;
    while(entity)
    {
        Entity *next = entity->next;
        EntityDestroy(entity);
        entity = next;
    }

    View *views = vm->views;
    for(i32 i = 0; i < 4; ++i)
    {
        ViewDestroy(views + i);
    }
}
