void EditorModeMove3DCamera(View *view)
{
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
        }

        if(state->leftButtonDown)
        {
            //  update the end position every frame to get real time
            //  response to moving the mouse
            f32 endX = floorf(mouseWorldX / gUnitSize) * gUnitSize;
            f32 endY = floorf(mouseWorldY / gUnitSize) * gUnitSize;
            state->endP = {endX, endY};

            RectMinMax rect = {};
            rect.min.x = Min(state->startP.x, state->endP.x);
            rect.min.y = Min(state->startP.y, state->endP.y);
            rect.max.x = Max(state->startP.x, state->endP.x) + gUnitSize;
            rect.max.y = Max(state->startP.y, state->endP.y) + gUnitSize;

            Vec2 botL = {rect.min.x, rect.min.y};
            Vec2 botR = {rect.max.x, rect.min.y};
            Vec2 topL = {rect.min.x, rect.max.y};
            Vec2 topR = {rect.max.x, rect.max.y};
            //Draw2DQuad(state, botL, botR, topL, topR, 0xFFFFFFAA, -3);
            Draw2DQuad(state, botL, botR, topL, topR, 0xFF00FF00, -3);
        }
        
        if(MouseJustUp(MOUSE_BUTTON_LEFT) && state->leftButtonDown)
        {
            state->leftButtonDown = false;
            f32 endX = floorf(mouseWorldX / gUnitSize) * gUnitSize;
            f32 endY = floorf(mouseWorldY / gUnitSize) * gUnitSize;
            state->endP = {endX, endY};

            RectMinMax rect = {};
            rect.min.x = Min(state->startP.x, state->endP.x);
            rect.min.y = Min(state->startP.y, state->endP.y);
            rect.max.x = Max(state->startP.x, state->endP.x) + gUnitSize;
            rect.max.y = Max(state->startP.y, state->endP.y) + gUnitSize;

            gEntityList =  EntityAdd(gEntityList, rect.min, rect.max, view->id);

            gSelectedEntity = gEntityList;
            gCurrentEditorMode = EDITOR_MODE_MODIFY_POLY;
            gDirtyFlag = true;
        }
    }
}

void EditorModeClipping(View *view)
{
    if(gCurrentEditorMode != EDITOR_MODE_CLIPPING)
        return;

    ViewOrthoState *state = &view->orthoState;

    ASSERT(gSelectedEntity != nullptr);

    Brush2D *brush = &gSelectedEntity->brushes2D[view->id];

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
            u32 color = 0xFF22FFFF;
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

    if(MouseJustDown(MOUSE_BUTTON_RIGHT) && state->planeCreated == true)
    {
        // a function that return the correct plane depending on the view you use to
        // create the clipping plane
        Plane clipPlane = state->createViewClipPlane(state->startP, state->endP);
        EntityClip(gSelectedEntity, view->id, clipPlane);
        state->planeCreated = false;
        gDirtyFlag = true;
    }
}

void EditorModeModifyPoly(View *view)
{
    if(gCurrentEditorMode != EDITOR_MODE_MODIFY_POLY)
        return;

    if(gSelectedEntity == nullptr) 
        return;

    ASSERT(gSelectedEntity != nullptr);

    ViewOrthoState *state = &view->orthoState;
    // Get Min and Max
    Vec2 xDim = {FLT_MAX, -FLT_MAX}; 
    Vec2 yDim = {FLT_MAX, -FLT_MAX}; 

    Brush2D *brush = &gSelectedEntity->brushes2D[view->id];

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
        EntityUpdate(gSelectedEntity, view->id, oBotL, oBotR, oTopL, oTopR, botL,  botR,  topL,  topR);
        gDirtyFlag = true;
    }

    if(KeyJustDown(VK_BACK) || KeyJustDown(VK_DELETE))
    {
        EntityRemove(gSelectedEntity);
        gSelectedEntity = nullptr;
        gDirtyFlag = true;
        gCurrentEditorMode = EDITOR_MODE_SELECT_POLY;
    }
}

void EditorModeSelectPoly(View *view)
{
    if(gCurrentEditorMode != EDITOR_MODE_SELECT_POLY)
        return;

    if(MouseIsHot(view) && MouseJustDown(MOUSE_BUTTON_LEFT))
    {
        gSelectedEntity = view->mousePicking(view);
        if(gSelectedEntity != nullptr)
        {
            gCurrentEditorMode = EDITOR_MODE_MODIFY_POLY;
        }
    }
}

void EditorModeSetTexture(View *view)
{
    if(gCurrentEditorMode != EDITOR_MODE_SET_TEXTURE)
        return;

    if(MouseIsHot(view) && MouseJustDown(MOUSE_BUTTON_LEFT))
    {
        Entity *clickEntity = view->mousePicking(view);
        if(clickEntity == nullptr) return;

        // TODO: create 
        // - BrushPlaneSetTexture
        // - BrushVertexSetTexture

        BrushPlane *brushPlane = &clickEntity->brushPlane;
        BrushVertex *brushVert = &clickEntity->brushVert;
        for(i32 i = 0; i < DarraySize(brushPlane->planes); ++i)
        {
            brushPlane->planes[i].texture = gCurrentTexture;

            Poly3D *poly = &brushVert->polygons[i];
            for(i32 j = 0; j < poly->verticesCount; ++j)
            {
                poly->vertices[j].texture =gCurrentTexture;
            }
        }

        gDirtyFlag = true;
    }
}
