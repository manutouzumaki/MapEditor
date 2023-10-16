void EditorModeMove3DCamera(View *view)
{
    // TODO: see how we want to handle this ...
    //if(gCurrentEditorMode != EDITOR_MODE_MOVE_3D_CAMERA)
    if(gCurrentEditorMode == EDITOR_MODE_SELECT_POLY ||
       gCurrentEditorMode == EDITOR_MODE_SET_TEXTURE)
        return;

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

        if (state->camera.rot.x >  (89.0f/180.0f) * PI)
            state->camera.rot.x =  (89.0f/180.0f) * PI;
        if (state->camera.rot.x < -(89.0f/180.0f) * PI)
            state->camera.rot.x = -(89.0f/180.0f) * PI;
    }

    if(state->rightButtonDown)
    {
        f32 deltaX = (f32)(MouseX() - MouseLastX()) * 0.015f;
        f32 deltaY = (f32)(MouseY() - MouseLastY()) * 0.015f;

        state->camera.pos = state->camera.pos + state->camera.right *  deltaX;
        state->camera.pos = state->camera.pos + state->camera.up    * -deltaY;
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

    state->camera.right = Vec3Normalized(Vec3Cross({0, 1, 0}, state->camera.dir));
    state->camera.up =  Vec3Normalized(Vec3Cross(state->camera.dir, state->camera.right));

    view->cbuffer.view = Mat4LookAt(state->camera.pos, state->camera.pos + state->camera.dir, {0, 1, 0});
    view->cbuffer.viewDir = state->camera.pos;
}

void EditorModeAddPoly(View *view)
{
    if(gCurrentEditorMode != EDITOR_MODE_ADD_POLY)
        return;

    ViewOrthoState *state = &view->orthoState;
    i32 mouseRelX = MouseRelX(view);
    i32 mouseRelY = MouseRelY(view);

    if(MouseIsHot(view))
    {
        f32 mouseWorldX, mouseWorldY;
        ScreenToWorld(mouseRelX, mouseRelY, mouseWorldX, mouseWorldY,
                      state->offsetX, state->offsetY, state->zoom);

        if(MouseJustDown(MOUSE_BUTTON_LEFT))
        {
            state->leftButtonDown = true;
            f32 startX = floorf(mouseWorldX / gUnitSize) * gUnitSize;
            f32 startY = floorf(mouseWorldY / gUnitSize) * gUnitSize;
            state->startP = {startX, startY};
            state->quadIndex = ViewAddBrush2D(view, state->startP, {state->startP.x + gUnitSize, state->startP.y + gUnitSize});
            state->addOtherViewsBrush(state->startP, {state->startP.x + gUnitSize, state->startP.y + gUnitSize}, 0xFFFFFFAA);
        }
        
        if(MouseJustUp(MOUSE_BUTTON_LEFT) && state->leftButtonDown)
        {
            state->leftButtonDown = false;
            f32 endX = floorf(mouseWorldX / gUnitSize) * gUnitSize;
            f32 endY = floorf(mouseWorldY / gUnitSize) * gUnitSize;
            state->endP = {endX, endY};

            state->rect.min.x = Min(state->startP.x, state->endP.x);
            state->rect.min.y = Min(state->startP.y, state->endP.y);
            state->rect.max.x = Max(state->startP.x, state->endP.x) + gUnitSize;
            state->rect.max.y = Max(state->startP.y, state->endP.y) + gUnitSize;
            ViewUpdateBrush2D(view, state->rect.min, state->rect.max, state->quadIndex);
            state->updateOtherViewsBrush(state->rect, state->quadIndex, 0xFFFFFFAA);
            ViewAddBrushPlane();
            gSharedMemory.selectedPolygon = state->quadIndex;
            gCurrentEditorMode = EDITOR_MODE_MODIFY_POLY;
        }
        
        if(state->leftButtonDown)
        {
            //  update the end position every frame to get real time
            //  response to moving the mouse
            f32 endX = floorf(mouseWorldX / gUnitSize) * gUnitSize;
            f32 endY = floorf(mouseWorldY / gUnitSize) * gUnitSize;
            state->endP = {endX, endY};

            state->rect.min.x = Min(state->startP.x, state->endP.x);
            state->rect.min.y = Min(state->startP.y, state->endP.y);
            state->rect.max.x = Max(state->startP.x, state->endP.x) + gUnitSize;
            state->rect.max.y = Max(state->startP.y, state->endP.y) + gUnitSize;
            ViewUpdateBrush2D(view, state->rect.min, state->rect.max, state->quadIndex);
            state->updateOtherViewsBrush(state->rect, state->quadIndex, 0xFFFFFFAA);
        }
    }
}

void Draw2DQuad(ViewOrthoState *state, Vec2 botL, Vec2 botR, Vec2 topL, Vec2 topR, u32 color, i32 zIndex)
{
        Vec2 a = botL;
        Vec2 b = topL;
        f32 sax, say, sbx, sby;
        WorldToScreen(a.x, a.y, sax, say, state->offsetX, state->offsetY, state->zoom); 
        WorldToScreen(b.x, b.y, sbx, sby, state->offsetX, state->offsetY, state->zoom); 
        DrawLine(sax,  say, zIndex, sbx,  sby, zIndex, color);

        a = botL;
        b = botR;
        WorldToScreen(a.x, a.y, sax, say, state->offsetX, state->offsetY, state->zoom); 
        WorldToScreen(b.x, b.y, sbx, sby, state->offsetX, state->offsetY, state->zoom); 
        DrawLine(sax,  say, zIndex, sbx,  sby, zIndex, color);

        a = topL;
        b = topR;
        WorldToScreen(a.x, a.y, sax, say, state->offsetX, state->offsetY, state->zoom); 
        WorldToScreen(b.x, b.y, sbx, sby, state->offsetX, state->offsetY, state->zoom); 
        DrawLine(sax,  say, zIndex, sbx,  sby, zIndex, color);


        a = botR;
        b = topR;
        WorldToScreen(a.x, a.y, sax, say, state->offsetX, state->offsetY, state->zoom); 
        WorldToScreen(b.x, b.y, sbx, sby, state->offsetX, state->offsetY, state->zoom); 
        DrawLine(sax,  say, zIndex, sbx,  sby, zIndex, color);
}

bool MouseInControlPoint(View *view, Vec2 rect)
{
    ViewOrthoState *state = &view->orthoState;
    f32 mouseRelX = MouseRelX(view);
    f32 mouseRelY = MouseRelY(view);
    f32 mouseScrX, mouseScrY;
    ScreenToWorld(mouseRelX, mouseRelY, mouseScrX, mouseScrY, state->offsetX, state->offsetY, state->zoom); 

    f32 w = 4*(1.0f/state->zoom);

    if(mouseScrX >= rect.x - w &&
       mouseScrX <= rect.x + w &&
       mouseScrY >= rect.y - w &&
       mouseScrY <= rect.y + w)
    {
        return true;
    }
    return false;
}


void EditorModeClipping(View *view)
{
    if(gCurrentEditorMode != EDITOR_MODE_CLIPPING)
        return;

    ViewOrthoState *state = &view->orthoState;

    i32 selectedPoly = gSharedMemory.selectedPolygon;
    Brush2DStorage *viewStorage = ViewGetBrush2DStorage(view);

    if(selectedPoly < 0 || selectedPoly >= DarraySize(viewStorage->brushes))
        return;

    Brush2D *brush = viewStorage->brushes + selectedPoly;
    RenderBrush2DColor(view , brush, 0xFF0000FF);

    if(!MouseIsHot(view))
        return;

    i32 mouseRelX = MouseRelX(view);
    i32 mouseRelY = MouseRelY(view);

    f32 mouseWorldX, mouseWorldY;
    ScreenToWorld(mouseRelX, mouseRelY, mouseWorldX, mouseWorldY,
                  state->offsetX, state->offsetY, state->zoom);

    f32 currentX = floorf((mouseWorldX + gUnitSize/2) / gUnitSize) * gUnitSize;
    f32 currentY = floorf((mouseWorldY + gUnitSize/2) / gUnitSize) * gUnitSize;

    if(MouseJustDown(MOUSE_BUTTON_LEFT) && state->planeCreated == false)
    {
        state->leftButtonDown = true;
        state->startP = {currentX, currentY};
    }

    if(state->leftButtonDown || state->planeCreated)
    {
        if(state->planeCreated == false) state->endP = {currentX, currentY};

        Vec2 a = state->startP;
        Vec2 b = state->endP;
        f32 sax, say, sbx, sby;
        WorldToScreen(a.x, a.y, sax, say, state->offsetX, state->offsetY, state->zoom); 
        WorldToScreen(b.x, b.y, sbx, sby, state->offsetX, state->offsetY, state->zoom); 
        DrawLine(sax,  say, -3, sbx,  sby, -3, 0xFF00FF00);

        Vec2 ab = b - a;
        Vec2 n = Vec2Normalized({ab.y, -ab.x});

        a = a + ab * 0.5f;;
        b = a + n * gUnitSize;
        WorldToScreen(a.x, a.y, sax, say, state->offsetX, state->offsetY, state->zoom); 
        WorldToScreen(b.x, b.y, sbx, sby, state->offsetX, state->offsetY, state->zoom); 
        DrawLine(sax,  say, -3, sbx,  sby, -3, 0xFFFF0000);
    }

    if(MouseJustUp(MOUSE_BUTTON_LEFT) && state->leftButtonDown && state->planeCreated == false)
    {
        state->leftButtonDown = false;
        state->planeCreated = true;
        state->endP = {currentX, currentY};
    }

    if(state->planeCreated == true)
    {
        Vec2 a = state->startP;
        Vec2 b = state->endP;

        // Draw control points
        Vec2 u = {0,4*(1.0f/state->zoom)};
        Vec2 r = {4*(1.0f/state->zoom),0};

        RectVert quads[2];
        quads[0] = { a-r-u, a+r-u, a-r+u, a+r+u };
        quads[1] = { b-r-u, b+r-u, b-r+u, b+r+u };

        Vec2 controlP[2] {
            state->startP, state->endP
        };
        
        for(i32 i = 0; i < 2; ++i)
        {
            u32 color = 0xFF2222FF;
            if(MouseInControlPoint(view, controlP[i]))
            {
                color = 0xFFFFFF00;
                if(MouseJustDown(MOUSE_BUTTON_LEFT))
                {
                    state->leftButtonDown = true;
                    state->controlPointDown = i;
                }
            }
            Draw2DQuad(state, quads[i].a, quads[i].b, quads[i].c, quads[i].d, color, -4.0f);
        }

        if(state->leftButtonDown)
        {
            if(state->controlPointDown == 0)
            {
                state->startP = {currentX, currentY};
            }
            else
            {
                state->endP = {currentX, currentY};
            }
        }

        if(MouseJustUp(MOUSE_BUTTON_LEFT) && state->leftButtonDown)
        {
            state->leftButtonDown = false;
            state->controlPointDown = -1;
        }
    }

    // TODO: change this to use the Enter key
    if(MouseJustDown(MOUSE_BUTTON_RIGHT) && state->planeCreated == true)
    {
        // a function that return the correct plane depending on the view you use to
        // create the clipping plane
        Plane clipPlane = state->createViewClipPlane(state->startP, state->endP);
        ViewClipBrush(selectedPoly, clipPlane, view->id);
        ViewUpdateBrush2DFromBrushPlane(selectedPoly);
    }
}

void EditorModeModifyPoly(View *view)
{
    if(gCurrentEditorMode != EDITOR_MODE_MODIFY_POLY)
        return;

    ViewOrthoState *state = &view->orthoState;

    i32 selectedPoly = gSharedMemory.selectedPolygon;

    BrushStorage *brushStorage = &gSharedMemory.brushStorage;
    Brush2DStorage *viewStorage = ViewGetBrush2DStorage(view);


    if(selectedPoly < 0 || selectedPoly >= DarraySize(viewStorage->brushes))
        return;

    // Get Min and Max
    Vec2 xDim = {FLT_MAX, -FLT_MAX}; 
    Vec2 yDim = {FLT_MAX, -FLT_MAX}; 

    Brush2D *brush = viewStorage->brushes + selectedPoly;
    for(i32 j = 0; j < DarraySize(brush->polygons); ++j)
    {
        Poly2D *poly = brush->polygons + j;

        for(i32 i = 0; i < DarraySize(poly->vertices); ++i)
        {
            Vec2 vert = poly->vertices[i];
            if(vert.x <= xDim.x)
                xDim.x = vert.x;
            if(vert.x >= xDim.y)
                xDim.y = vert.x; 
            if(vert.y <= yDim.x)
                yDim.x = vert.y;
            if(vert.y >= yDim.y)
                yDim.y = vert.y;
        }

    }


    Vec2 botL = {xDim.x, yDim.x};
    Vec2 botR = {xDim.y, yDim.x};
    Vec2 topL = {xDim.x, yDim.y};
    Vec2 topR = {xDim.y, yDim.y};
    Vec2 midL = {botL + (topL - botL) * 0.5f};
    Vec2 midR = {botR + (topR - botR) * 0.5f};
    Vec2 midT = {topL + (topR - topL) * 0.5f};
    Vec2 midB = {botL + (botR - botL) * 0.5f};

    Vec2 controlP[CONTROL_POINT_COUNT] {
        botL, botR, topL, topR,
        midL, midR, midT, midB
    };

    i32 mouseRelX = MouseRelX(view);
    i32 mouseRelY = MouseRelY(view);

    f32 mouseWorldX, mouseWorldY;
    ScreenToWorld(mouseRelX, mouseRelY, mouseWorldX, mouseWorldY,
                  state->offsetX, state->offsetY, state->zoom);

    f32 currentX = floorf(mouseWorldX / gUnitSize) * gUnitSize;
    f32 currentY = floorf(mouseWorldY / gUnitSize) * gUnitSize;

    for(i32 i = 0; i < CONTROL_POINT_COUNT; ++i)
    {
        if(MouseInControlPoint(view, controlP[i]))
        {
            if(MouseJustDown(MOUSE_BUTTON_LEFT))
            {
                state->leftButtonDown = true;
                state->controlPointDown = i;
            }
        }
    }

    if(state->leftButtonDown)
    {
        switch(state->controlPointDown)
        {
            // TODO: we have to move tree vertices
            case CONTROL_POINT_BOTL:
            {
                controlP[CONTROL_POINT_BOTL].x = currentX;
                controlP[CONTROL_POINT_BOTL].y = currentY;
                controlP[CONTROL_POINT_TOPL].x = currentX;
                controlP[CONTROL_POINT_BOTR].y = currentY;
            } break;
            case CONTROL_POINT_BOTR:
            {
                controlP[CONTROL_POINT_BOTR].x = currentX;
                controlP[CONTROL_POINT_BOTR].y = currentY;
                controlP[CONTROL_POINT_TOPR].x = currentX;
                controlP[CONTROL_POINT_BOTL].y = currentY;
            } break;
            case CONTROL_POINT_TOPL:
            { 
                controlP[CONTROL_POINT_TOPL].x = currentX;
                controlP[CONTROL_POINT_TOPL].y = currentY;
                controlP[CONTROL_POINT_BOTL].x = currentX;
                controlP[CONTROL_POINT_TOPR].y = currentY;
            } break;
            case CONTROL_POINT_TOPR:
            {
                controlP[CONTROL_POINT_TOPR].x = currentX;
                controlP[CONTROL_POINT_TOPR].y = currentY;
                controlP[CONTROL_POINT_BOTR].x = currentX;
                controlP[CONTROL_POINT_TOPL].y = currentY;
            } break;
            case CONTROL_POINT_MIDL:
            {
                controlP[CONTROL_POINT_BOTL].x = currentX;
                controlP[CONTROL_POINT_TOPL].x = currentX;
            } break;
            case CONTROL_POINT_MIDR:
            {
                controlP[CONTROL_POINT_BOTR].x = currentX;
                controlP[CONTROL_POINT_TOPR].x = currentX;
            } break;
            case CONTROL_POINT_MIDT:
            {
                controlP[CONTROL_POINT_TOPL].y = currentY;
                controlP[CONTROL_POINT_TOPR].y = currentY;
            } break;
            case CONTROL_POINT_MIDB:
            {
                controlP[CONTROL_POINT_BOTL].y = currentY;
                controlP[CONTROL_POINT_BOTR].y = currentY;
            } break;
        }
    }

    botL = controlP[CONTROL_POINT_BOTL];
    botR = controlP[CONTROL_POINT_BOTR];
    topL = controlP[CONTROL_POINT_TOPL];
    topR = controlP[CONTROL_POINT_TOPR];
    midL = {botL + (topL - botL) * 0.5f};
    midR = {botR + (topR - botR) * 0.5f};
    midT = {topL + (topR - topL) * 0.5f};
    midB = {botL + (botR - botL) * 0.5f};

    Draw2DQuad(state, botL, botR, topL, topR, 0xFF0000FF, -3.0f);
    
    Vec2 u = {0,4*(1.0f/state->zoom)};
    Vec2 r = {4*(1.0f/state->zoom),0};

    RectVert quads[CONTROL_POINT_COUNT];
    quads[CONTROL_POINT_BOTL] = { botL-r-u, botL+r-u, botL-r+u, botL+r+u };
    quads[CONTROL_POINT_BOTR] = { botR-r-u, botR+r-u, botR-r+u, botR+r+u };
    quads[CONTROL_POINT_TOPL] = { topL-r-u, topL+r-u, topL-r+u, topL+r+u };
    quads[CONTROL_POINT_TOPR] = { topR-r-u, topR+r-u, topR-r+u, topR+r+u };
    quads[CONTROL_POINT_MIDL] = { midL-r-u, midL+r-u, midL-r+u, midL+r+u };
    quads[CONTROL_POINT_MIDR] = { midR-r-u, midR+r-u, midR-r+u, midR+r+u };
    quads[CONTROL_POINT_MIDT] = { midT-r-u, midT+r-u, midT-r+u, midT+r+u };
    quads[CONTROL_POINT_MIDB] = { midB-r-u, midB+r-u, midB-r+u, midB+r+u };
    
    for(i32 i = 0; i < CONTROL_POINT_COUNT; ++i)
    {
        u32 color = 0xFFFF2222;
        if(MouseInControlPoint(view, controlP[i]))
        {
            color = 0xFFFFFF00;
        }
        Draw2DQuad(state, quads[i].a, quads[i].b, quads[i].c, quads[i].d, color, -4.0f);
    }


    Vec2 oBotL = {xDim.x, yDim.x};
    Vec2 oBotR = {xDim.y, yDim.x};
    Vec2 oTopL = {xDim.x, yDim.y};
    Vec2 oTopR = {xDim.y, yDim.y};


    if(MouseJustUp(MOUSE_BUTTON_LEFT) && state->leftButtonDown)
    {
        state->leftButtonDown = false;
        state->controlPointDown = -1;
        state->rect.min.x = botL.x;
        state->rect.min.y = botL.y;
        state->rect.max.x = topR.x;
        state->rect.max.y = topR.y;

        f32 xNewDim = topR.x - botL.x; 
        f32 yNewDim = topR.y - botL.y;

        BrushVertex *brushVert = brushStorage->brushVerts + selectedPoly;
        for(i32 j = 0; j < brushVert->polygonsCount; ++j)
        {
            Poly3D *poly = brushVert->polygons + j;
            for(i32 i = 0; i < poly->verticesCount; ++i)
            {
                Vertex *v = poly->vertices + i;
                Vec3 *p = &v->position;
                switch(view->id)
                {
                    case VIEW_TOP:
                    {
                        // modify x and z coord
                        f32 xRatio = (p->x - oBotL.x) / (oBotR.x - oBotL.x);
                        f32 yRatio = (p->z - oBotL.y) / (oTopL.y - oBotL.y);
                        p->x = botL.x + (xRatio * xNewDim);
                        p->z = botL.y + (yRatio * yNewDim);
                    } break;
                    case VIEW_FRONT:
                    {
                        // modify x and y coord
                        f32 xRatio = (p->x - oBotL.x) / (oBotR.x - oBotL.x);
                        f32 yRatio = (p->y - oBotL.y) / (oTopL.y - oBotL.y);
                        p->x = botL.x + (xRatio * xNewDim);
                        p->y = botL.y + (yRatio * yNewDim);
                    } break;
                    case VIEW_SIDE:
                    {
                        // modify z and y coord
                        f32 xRatio = (p->z - oBotL.x) / (oBotR.x - oBotL.x);
                        f32 yRatio = (p->y - oBotL.y) / (oTopL.y - oBotL.y);
                        p->z = botL.x + (xRatio * xNewDim);
                        p->y = botL.y + (yRatio * yNewDim);
                    } break;
                }
            }
        }


        ViewUpdateBrushPlaneFromBrushVertex(selectedPoly);
        ViewUpdateBrush2DFromBrushPlane(selectedPoly);
    }
}

bool PointHitPoly2D(Poly2D *poly, f32 x, f32 y, f32 zoom)
{
    Vec2 mouseP = {x, y};
    for(i32 i = 0; i < DarraySize(poly->vertices); ++i)
    {
        if(i == 1)
            i32 stophere = 1;

        Vec2 a = poly->vertices[(i + 0) % DarraySize(poly->vertices)];
        Vec2 b = poly->vertices[(i + 1) % DarraySize(poly->vertices)];
        Vec2 ab = b - a;

        // create the rect orthonormal basis
        Vec2 o = a + ab * 0.5f;
        Vec2 r = Vec2Normalized(ab);
        Vec2 u = Vec2Normalized({-r.y ,r.x});
        

        // transform the mouse world coord the the rect basis
        Vec2 testP = mouseP - o;
        f32 localX = Vec2Dot(r, testP);
        f32 localY = Vec2Dot(u, testP);

        // AABB point intersection check
        f32 hW = Vec2Len(ab) * 0.5f;
        f32 hH = 2*(1.0f/zoom);
        if(localX >= -hW && localX <= hW &&
           localY >= -hH && localY <= hH)
        {
            return true;
        }

    }
    return false;
}

i32 MousePicking2D(View *view)
{

    ViewOrthoState *state = &view->orthoState;

    i32 mouseRelX = MouseRelX(view);
    i32 mouseRelY = MouseRelY(view);

    f32 mouseWorldX, mouseWorldY;
    ScreenToWorld(mouseRelX, mouseRelY, mouseWorldX, mouseWorldY,
                  state->offsetX, state->offsetY, state->zoom);

    Brush2DStorage *viewStorage = ViewGetBrush2DStorage(view);
    for(i32 j = 0; j < DarraySize(viewStorage->brushes); ++j)
    {
        Brush2D *brush = viewStorage->brushes + j;
        // loop over every polygon in the list
        for(i32 i = 0; i < DarraySize(brush->polygons); ++i)
        {
            Poly2D *poly = brush->polygons + i;
            if(PointHitPoly2D(poly, mouseWorldX, mouseWorldY, state->zoom))
            {
                return j;
            }
        }
    }
    return -1;
}

static Ray GetMouseRay(View *view, f32 x, f32 y) 
{
    ViewPerspState *state = &view->perspState;

    Mat4 invView = Mat4Inverse(view->cbuffer.view);
    Mat4 invProj = Mat4Inverse(view->cbuffer.proj);

    Vec4 rayClip;
    rayClip.x = 2.0f * x / view->w - 1.0f;
    rayClip.y = 1.0f - (2.0f * y) / view->h;
    rayClip.z = 1.0f;
    rayClip.w = 1.0f;
    Vec4 rayEye = invProj * rayClip;
    rayEye.z =  1.0f;
    rayEye.w =  0.0f;
    Vec4 rayWorld = invView * rayEye;
    rayWorld = Vec4Normalized(rayWorld);

    Ray ray;
    ray.o = state->camera.pos;
    ray.d = {rayWorld.x, rayWorld.y, rayWorld.z};
    Vec3Normalize(&ray.d);
    return ray;
}

static bool RayHitBrushPlane(Ray ray, BrushPlane *brushPlane, f32 *tOut)
{
    Vec3 a = ray.o;
    Vec3 d = ray.d;
    // Set initial interval to being the whole segment. For a ray, tlast should be
    // sety to FLT_MAX. For a line tfirst should be set to - FLT_MAX
    f32 tFirst = 0;
    f32 tLast = FLT_MAX;
    // intersect segment agains each plane
    for(i32 i = 0; i < brushPlane->planesCount; ++i)
    {
        Plane p = brushPlane->planes[i];
        f32 denom = Vec3Dot(p.n, d);
        f32 dist = (p.d / g3DScale) - Vec3Dot(p.n, a);
        // test if segment runs parallel to tha plane
        if(denom == 0.0f)
        {
            // If so, return "no intersection" if segemnt lies outside the plane
            if(dist > 0.0f) return 0;
        }
        else
        {
            f32 t = dist / denom;
            if(denom < 0.0f)
            {
                // when entering halfspace, update tfirst if t is larger
                if(t > tFirst) tFirst = t;
            }
            else
            {
                // when exiting halfspace, update tLast if t is smaller
                if(t < tLast) tLast = t;
            }

            if(tFirst > tLast) return 0;
        }
    }
    *tOut = tFirst;
    return 1;
}

i32 MousePicking3D(View *view)
{
    BrushStorage *viewStorage = &gSharedMemory.brushStorage; 

    // get the mouse relative to the view but from the bottom up
    i32 mouseRelX = MouseRelX(view) + view->w * 0.5f;
    i32 mouseRelY = view->h - (MouseRelY(view) + view->h * 0.5f);

    Ray ray = GetMouseRay(view, mouseRelX, mouseRelY); 

    // loop over every polygon in the list and save the hitPoint with
    // the smallest t value
    f32 tMin = FLT_MAX;
    i32 hitIndex = -1;
    for(i32 i = 0; i < viewStorage->brushesCount; ++i)
    {
        BrushPlane *brushPlane = viewStorage->brushPlanes + i;
        f32 t = -1.0f;
        if(RayHitBrushPlane(ray, brushPlane, &t))
        {
            if(t < tMin)
            {
                tMin = t;
                hitIndex = i;
            }
        }
    }


    return hitIndex;
}

void EditorModeSelectPoly(View *view)
{
    if(gCurrentEditorMode != EDITOR_MODE_SELECT_POLY)
        return;

    if(MouseIsHot(view) && MouseJustDown(MOUSE_BUTTON_LEFT))
    {
        gSharedMemory.selectedPolygon = view->mousePicking(view);
        if(gSharedMemory.selectedPolygon >= 0)
        {
            gCurrentEditorMode = EDITOR_MODE_MODIFY_POLY;
        }
    }
}

// TODO: ...
void EditorModeSetTexture(View *view)
{
    if(gCurrentEditorMode != EDITOR_MODE_SET_TEXTURE)
        return;

    if(MouseIsHot(view) && MouseJustDown(MOUSE_BUTTON_LEFT))
    {
        i32 clickPolygon = view->mousePicking(view);
        printf("change texture of the polygon: %d\n", clickPolygon);
        BrushStorage *viewStorage = &gSharedMemory.brushStorage; 
    }
}
