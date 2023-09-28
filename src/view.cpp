struct View;

typedef void (*SetupFNP) (View *view);
typedef void (*ProcessFNP) (View *view);
typedef void (*RenderFNP) (View *view);

typedef void (*AddOtherViewsPolysFNP) (Vec2 start, Vec2 end);
typedef void (*UpdateOtherViewsPolysFNP) (RectMinMax rect, i32 quadIndex);


enum ProjType
{
    PROJ_TYPE_PERSP,
    PROJ_TYPE_ORTHO
};

enum ViewId
{
    VIEW_TOP,
    VIEW_FRONT,
    VIEW_SIDE,
    VIEW_MAIN,

    VIEW_COUNT
};

struct ViewPerspState
{

};

struct ViewOrthoState
{
    f32 offsetX;
    f32 offsetY;

    f32 lastClickX;
    f32 lastClickY;

    f32 wheelOffset;
    f32 zoom;

    bool leftButtonDown;
    bool middleButtonDown;
    bool rightButtonDown;

    Vec2 startP;
    Vec2 endP;
    i32 quadIndex;
    RectMinMax rect;
    AddOtherViewsPolysFNP addOtherViewsPolys;
    UpdateOtherViewsPolysFNP updateOtherViewsPolys;

};

struct View
{
    ViewId id;
    f32 x, y, w, h;
    CBuffer cbuffer;
    FrameBuffer fb;
    ProjType projType;
    union
    {
        ViewPerspState perspState;
        ViewOrthoState orthoState;
    };

    SetupFNP setup;
    ProcessFNP process;
    RenderFNP render;
};


// TODO: use a link list base storage system
struct Poly2DStorage
{
    Poly2D polygons[255];
    i32 polygonsCount;
};

struct PolyPlaneStorage
{
    PolyPlane polygons[255];
    i32 polygonsCount;
};

struct SharedMemory
{
    Poly2DStorage poly2dStorage[3];
    PolyPlaneStorage polyPlaneStorage;

    i32 selectedPolygon = -1;
};

static SharedMemory gSharedMemory;

View ViewCreate(f32 x, f32 y, f32 w, f32 h, ProjType projType,
                SetupFNP setup,
                ProcessFNP process,
                RenderFNP render)
{
    View view = {};

    view.x = x;
    view.y = y;
    view.w = w;
    view.h = h;

    switch(projType)
    {
        case PROJ_TYPE_PERSP:
        {
            view.cbuffer.proj = Mat4Perspective(60, w/h, 0.01f, 1000.0f);
        } break;
        case PROJ_TYPE_ORTHO:
        {
            view.cbuffer.proj = Mat4Ortho(w*-0.5f, w*0.5f, h*-0.5f, h*0.5f, 0.01f, 100.0f);
        } break;
    }
    view.cbuffer.view = Mat4LookAt({0, 2.5f, -5}, {0, 0, 0}, {0, 1,  0});
    view.cbuffer.world = Mat4Identity();

    view.fb = LoadFrameBuffer(x, y, w, h, DXGI_FORMAT_R8G8B8A8_UNORM);

    view.setup = setup;
    view.process = process;
    view.render = render;

    view.projType = projType;

    return view;
}

void ViewDestroy(View *view)
{
    UnloadFrameBuffer(&view->fb);
}

void ViewResize(View *view, f32 x, f32 y, f32 w, f32 h)
{
    view->x = x;
    view->y = y;
    view->w = w;
    view->h = h;
    switch(view->projType)
    {
        case PROJ_TYPE_PERSP:
        {
            view->cbuffer.proj = Mat4Perspective(60, w/h, 0.01f, 100.0f);
        } break;
        case PROJ_TYPE_ORTHO:
        {
            view->cbuffer.proj = Mat4Ortho(w*-0.5f, w*0.5f, h*-0.5f, h*0.5f, 0.01f, 100.0f);
        } break;
    }
    ResizeFrameBuffer(&view->fb, x, y, w, h);
}

void ViewSetup(View *view)
{
    if(view->setup) view->setup(view);
}

void ViewProcess(View *view)
{
    if(view->process) view->process(view);
}

void ViewRender(View *view)
{
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = view->w;
    viewport.Height = view->h;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);

    // Clear the screen
    f32 clearColor[] = { 0.05, 0.05, 0.05, 1 };
    deviceContext->OMSetRenderTargets(1, &view->fb.renderTargetView, view->fb.depthStencilView);
    deviceContext->ClearRenderTargetView(view->fb.renderTargetView, clearColor);
    deviceContext->ClearDepthStencilView(view->fb.depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

    if(view->render) view->render(view);

    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = gCurrentWindowWidth;
    viewport.Height = gCurrentWindowHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);
}

void WorldToScreen(f32 wx, f32 wy, f32 &sx, f32 &sy, f32 offsetX, f32 offsetY, f32 zoom)
{
    sx = (wx - offsetX) * zoom;
    sy = (wy - offsetY) * zoom;
}

void ScreenToWorld(f32 sx, f32 sy, f32 &wx, f32 &wy, f32 offsetX, f32 offsetY, f32 zoom)
{
    wx = (sx / zoom) + offsetX;
    wy = (sy / zoom) + offsetY;
}

i32 MouseRelToClientX()
{
    i32 mouseRelToClientX = MouseX() - gFixWidth; // TODO: use a global for this value
    return mouseRelToClientX;
}

i32 MouseRelToClientY()
{
    i32 mouseRelToClientY = gCurrentWindowHeight - MouseY();
    return mouseRelToClientY;
}

i32 MouseRelX(View *view)
{
    i32 mouseRelToClientX = MouseRelToClientX();
    i32 mouseRelX = mouseRelToClientX - view->x;
    return mouseRelX;
}

i32 MouseRelY(View *view)
{
    i32 mouseRelToClientY = MouseRelToClientY();
    i32 mouseRelY = mouseRelToClientY - view->y;
    return mouseRelY;
}

bool MouseIsHot(View *view)
{
    i32 mouseRelX = MouseRelX(view);
    i32 mouseRelY = MouseRelY(view);
    if(mouseRelX >= view->w*-0.5f && mouseRelX <= view->w*0.5f &&
       mouseRelY >= view->h*-0.5f && mouseRelY <= view->h*0.5f)
    {
        return true;
    }
    return false;
}

void RenderGrid(f32 offsetX, f32 offsetY, f32 zoom)
{
    for(f32 y = -100.0f; y < 100.0f; ++y)
    {
        f32 ax = -100.0f * gUnitSize;
        f32 ay = y * gUnitSize;
        f32 bx = 100.0f * gUnitSize;
        f32 by = y * gUnitSize;
        
        f32 sax, say, sbx, sby;

        WorldToScreen(ax, ay, sax, say, offsetX, offsetY, zoom); 
        WorldToScreen(bx, by, sbx, sby, offsetX, offsetY, zoom); 

        DrawLine(sax,  say, 0, sbx,  sby, 0, 0xFF333333);
    }

    for(f32 x = -100.0f; x < 100.0f; ++x)
    {

        f32 ax = x * gUnitSize;
        f32 ay = -100.0f * gUnitSize;
        f32 bx = x * gUnitSize;
        f32 by = gUnitSize*100.0f;
        
        f32 sax, say, sbx, sby;

        WorldToScreen(ax, ay, sax, say, offsetX, offsetY, zoom); 
        WorldToScreen(bx, by, sbx, sby, offsetX, offsetY, zoom); 

        DrawLine(sax,  say, 0, sbx,  sby, 0, 0xFF333333);
    }

    f32 ax = 0;
    f32 ay = -100.0f * gUnitSize;
    f32 bx = 0;
    f32 by = gUnitSize*100.0f;
    f32 sax, say, sbx, sby;
    WorldToScreen(ax, ay, sax, say, offsetX, offsetY, zoom); 
    WorldToScreen(bx, by, sbx, sby, offsetX, offsetY, zoom); 
    DrawLine(sax,  say, 0, sbx,  sby, -1, 0xFFAAFFAA);

    ax = -100.0f * gUnitSize;
    ay = 0;
    bx =  100.0f * gUnitSize;
    by = 0;
    WorldToScreen(ax, ay, sax, say, offsetX, offsetY, zoom); 
    WorldToScreen(bx, by, sbx, sby, offsetX, offsetY, zoom); 
    DrawLine(sax,  say, 0, sbx,  sby, -1, 0xFFFFAAAA);

}

void ViewOrthoBaseRender(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    // set the shader
    deviceContext->VSSetShader(gColShader.vertex, 0, 0);
    deviceContext->PSSetShader(gColShader.fragment, 0, 0);

    // Update the constBuffer
    view->cbuffer.world = Mat4Identity();
    view->cbuffer.view = Mat4LookAt({0, 0, -50}, {0, 0, 0}, {0, 1, 0});
    UpdateConstBuffer(&gConstBuffer, (void *)&view->cbuffer);
        
    RenderGrid(state->offsetX, state->offsetY, state->zoom);
    LineRendererDraw();
}

Poly2DStorage *ViewGetPoly2DStorage(View *view)
{
    Poly2DStorage *poly2dStorage = gSharedMemory.poly2dStorage + view->id;
    return poly2dStorage;
}


static bool GetIntersection(Vec3 n1, Vec3 n2, Vec3 n3, f32 d1, f32 d2, f32 d3, Vertex *vertex)
{
    f32 denom = Vec3Dot(n1, Vec3Cross(n2, n3));
    if(denom <= FLT_EPSILON && denom >= -FLT_EPSILON)
    {
        return false;
    }
    Vec3 pos = (-d1 * Vec3Cross(n2, n3) -d2 * Vec3Cross(n3, n1) -d3 * Vec3Cross(n1, n2)) / denom;
    f32 r = InvLerp(0, 20, rand()%20);
    f32 g = InvLerp(0, 20, rand()%20);
    f32 b = InvLerp(0, 20, rand()%20);
    Vec4 col = {r, g, b, 1.0f};
    *vertex = {pos, col};
    return true;
}

static Plane GetPlaneFromThreePoints(Vec3 a, Vec3 b, Vec3 c)
{
    Plane plane;
    Vec3 ab = b - a;
    Vec3 ac = c - a;
    Vec3 n = Vec3Normalized(Vec3Cross(ab, ac));
    f32 d = -n.x*a.x -n.y*a.y -n.z*a.z;
    return {n, d};
}

static Vec3 GetCenterOfPolygon(Poly3D *polygon)
{
    Vec3 center = {};
    for(i32 i = 0; i < polygon->verticesCount; ++i)
    {
        center = center + polygon->vertices[i].position;
    }
    center = center / polygon->verticesCount;
    return center;
}


void PushPolyPlaneToVertexBuffer(PolyPlane *poly)
{
    Poly3D *polygons = (Poly3D *)malloc(poly->planesCount * sizeof(Poly3D));
    memset(polygons, 0, poly->planesCount * sizeof(Poly3D));
    for(i32 i = 0; i < poly->planesCount - 2; ++i) {
    for(i32 j = i; j < poly->planesCount - 1; ++j) {
    for(i32 k = j; k < poly->planesCount - 0; ++k) {

        if(i != j && i != k && j != k)
        {
            Plane a = poly->planes[i];
            Plane b = poly->planes[j];
            Plane c = poly->planes[k];

            Vertex vertex = {};
            if(GetIntersection(a.n, b.n, c.n, a.d, b.d, c.d, &vertex))
            {
                bool illegal = false;
                for(i32 m = 0; m < poly->planesCount; ++m)
                {
                    Plane plane = poly->planes[m];
                    f32 dot = Vec3Dot(plane.n, vertex.position);
                    f32 d = plane.d;
                    f32 test = dot + d;
                    if(test > EPSILON)
                    {
                        illegal = true;
                    }
                }
                if(illegal == false)
                {
                    // TODO: add the vertex
                    Mat4 scaleMat = Mat4Scale(1.0f/128.0f, 1.0f/128.0f, 1.0f/128.0f);
                    vertex.position = Mat4TransformPoint(scaleMat, vertex.position);
                    polygons[i].vertices[polygons[i].verticesCount++] = vertex;
                    polygons[j].vertices[polygons[j].verticesCount++] = vertex;
                    polygons[k].vertices[polygons[k].verticesCount++] = vertex;
                }
            }
        }

    }}}

    // order the vertices in the polygons
    for(i32 p = 0; p < poly->planesCount; ++p)
    {
        Plane polygonPlane = poly->planes[p]; 
        Poly3D *polygon = polygons + p;

        ASSERT(polygon->verticesCount >= 3);

        Vec3 center = GetCenterOfPolygon(polygon);

        
        for(i32 n = 0; n <= polygon->verticesCount - 3; ++n)
        {
            Vec3 a = Vec3Normalized(polygon->vertices[n].position - center);
            Plane p = GetPlaneFromThreePoints(polygon->vertices[n].position,
                                              center, center + polygonPlane.n);

            f32 smallestAngle = -1;
            i32 smallest = -1;

            for(i32 m = n + 1; m <= polygon->verticesCount - 1; ++m)
            {
                Vertex vertex = polygon->vertices[m];
                if((Vec3Dot(p.n, vertex.position) + p.d) > 0.0f)
                {
                    Vec3 b = Vec3Normalized(vertex.position - center);
                    f32 angle = Vec3Dot(a, b);
                    if(angle > smallestAngle)
                    {
                        smallestAngle = angle;
                        smallest = m;
                    }
                }
            }

            if(smallest >= 0)
            {
                Vertex tmp = polygon->vertices[n + 1];
                polygon->vertices[n + 1] = polygon->vertices[smallest];
                polygon->vertices[smallest] = tmp;
            }
        } 
    }

    Vertex *vertices = 0;
    
    for(i32 i = 0; i < poly->planesCount; ++i)
    {
        Poly3D *polyD = &polygons[i];
        for(i32 j = 0; j < polyD->verticesCount - 2; ++j)
        {
            Vertex a = polyD->vertices[0];
            Vertex b = polyD->vertices[j + 1];
            Vertex c = polyD->vertices[j + 2];
            DarrayPush(vertices, a, Vertex);
            DarrayPush(vertices, b, Vertex);
            DarrayPush(vertices, c, Vertex);
        }
    }

    ASSERT((gDynamicVertexBuffer.used + sizeof(Vertex)) < gDynamicVertexBuffer.size);
    memcpy((char *)gDynamicVertexBuffer.CPUBuffer + gDynamicVertexBuffer.used, vertices, DarraySize(vertices) * sizeof(Vertex));
    gDynamicVertexBuffer.used += DarraySize(vertices) * sizeof(Vertex);
    gDynamicVertexBuffer.verticesCount += DarraySize(vertices);

    PushToGPUDynamicVertexBuffer(&gDynamicVertexBuffer);

    DarrayDestroy(vertices);
    free(polygons);
}

// TODO: remplace this function for a generic polygon function
// not just cubes
i32 ViewAddPolyPlane()
{
    PolyPlaneStorage *polyPlaneStorage = &gSharedMemory.polyPlaneStorage;
    Poly2DStorage *frontStorage = gSharedMemory.poly2dStorage + VIEW_FRONT;
    Poly2DStorage *sideStorage = gSharedMemory.poly2dStorage + VIEW_SIDE;

    ASSERT(polyPlaneStorage->polygonsCount < ARRAY_LENGTH(polyPlaneStorage->polygons));
    i32 index = polyPlaneStorage->polygonsCount++;

    Poly2D *front = frontStorage->polygons + index;
    Poly2D *side  = sideStorage->polygons  + index;

    Vec2 xDim = {FLT_MAX, -FLT_MAX}; 
    Vec2 yDim = {FLT_MAX, -FLT_MAX}; 
    Vec2 zDim = {FLT_MAX, -FLT_MAX};

    for(i32 i = 0; i < front->verticesCount; ++i)
    {
        Vec2 vertice = front->vertices[i];
        if(vertice.x <= xDim.x)
        {
            xDim.x = vertice.x;
        }
        if(vertice.x >= xDim.y)
        {
            xDim.y = vertice.x;
        }

        if(vertice.y <= yDim.x)
        {
            yDim.x = vertice.y;
        }
        if(vertice.y >= yDim.y)
        {
            yDim.y = vertice.y;
        }
    }

    for(i32 i = 0; i < side->verticesCount; ++i)
    {
        Vec2 vertice = side->vertices[i];
        if(vertice.x <= zDim.x)
        {
            zDim.x = vertice.x;
        }
        if(vertice.x >= zDim.y)
        {
            zDim.y = vertice.x;
        }
    }

    PolyPlane poly;
    poly.planes[0] = { { 1,  0,  0}, -xDim.y };
    poly.planes[1] = { {-1,  0,  0},  xDim.x };
    poly.planes[2] = { { 0,  1,  0}, -yDim.y };
    poly.planes[3] = { { 0, -1,  0},  yDim.x };
    poly.planes[4] = { { 0,  0,  1}, -zDim.y };
    poly.planes[5] = { { 0,  0, -1},  zDim.x };
    poly.planesCount = 6;
    polyPlaneStorage->polygons[index] = poly;

    PushPolyPlaneToVertexBuffer(&poly);

    return index;
}

i32 ViewAddQuad(View *view, Vec2 start, Vec2 end)
{
    Poly2DStorage *poly2dStorage = ViewGetPoly2DStorage(view);

    Poly2D poly;
    poly.vertices[0] = {start.x, start.y};
    poly.vertices[1] = {end.x, start.y};
    poly.vertices[2] = {end.x, end.y};
    poly.vertices[3] = {start.x, end.y};
    poly.verticesCount = 4;

    ASSERT(poly2dStorage->polygonsCount < ARRAY_LENGTH(poly2dStorage->polygons));
    i32 index = poly2dStorage->polygonsCount++;
    poly2dStorage->polygons[index] = poly;

    return index;
}

void ViewUpdateQuad(View *view, Vec2 start, Vec2 end, i32 index)
{
    Poly2DStorage *poly2dStorage = ViewGetPoly2DStorage(view);

    Poly2D poly;
    poly.vertices[0] = {start.x, start.y};
    poly.vertices[1] = {end.x, start.y};
    poly.vertices[2] = {end.x, end.y};
    poly.vertices[3] = {start.x, end.y};
    poly.verticesCount = 4;

    poly2dStorage->polygons[index] = poly;
}

void RenderPoly2D(View *view, Poly2D *poly, u32 color)
{
    ViewOrthoState *state = &view->orthoState;
    for(i32 i = 0; i < poly->verticesCount - 1; ++i)
    {
        Vec2 a = poly->vertices[i + 0];
        Vec2 b = poly->vertices[i + 1];

        f32 sax, say, sbx, sby;
        WorldToScreen(a.x, a.y, sax, say, state->offsetX, state->offsetY, state->zoom); 
        WorldToScreen(b.x, b.y, sbx, sby, state->offsetX, state->offsetY, state->zoom); 

        DrawLine(sax,  say, -2, sbx,  sby, -2, color);
    }

    Vec2 a = poly->vertices[poly->verticesCount - 1];
    Vec2 b = poly->vertices[0];
    f32 sax, say, sbx, sby;
    WorldToScreen(a.x, a.y, sax, say, state->offsetX, state->offsetY, state->zoom); 
    WorldToScreen(b.x, b.y, sbx, sby, state->offsetX, state->offsetY, state->zoom); 
    DrawLine(sax,  say, -2, sbx,  sby, -2, color);
}


void ViewOrthoBaseSetup(View *view)
{
    ViewOrthoState *state = &view->orthoState;
    state->wheelOffset = 0.5f;
    state->zoom = state->wheelOffset * state->wheelOffset;
}

void ViewOrthoBasePannelAndZoom(View *view)
{
    ViewOrthoState *state = &view->orthoState;

    i32 mouseRelX = MouseRelX(view);
    i32 mouseRelY = MouseRelY(view);

    if(MouseIsHot(view))
    {
        if(MouseJustDown(MOUSE_BUTTON_MIDDLE))
        {
            state->middleButtonDown = true;
            state->lastClickX = mouseRelX;
            state->lastClickY = mouseRelY;
        }

        f32 mouseWorldPreZoomX, mouseWorldPreZoomY;
        ScreenToWorld(mouseRelX, mouseRelY, mouseWorldPreZoomX, mouseWorldPreZoomY,
                      state->offsetX, state->offsetY, state->zoom);

        i32 mouseWheelDelta = MouseWheelDelta();
        if(mouseWheelDelta != 0)
        {
            if(mouseWheelDelta > 0)
            {
                state->wheelOffset *= 1.1f;
            }
            else
            {
                state->wheelOffset *= 0.9f;
            }
            state->wheelOffset = Max(0.1f, state->wheelOffset);
            state->zoom = state->wheelOffset * state->wheelOffset;
        }

        f32 mouseWorldPostZoomX, mouseWorldPostZoomY;
        ScreenToWorld(mouseRelX, mouseRelY, mouseWorldPostZoomX, mouseWorldPostZoomY,
                      state->offsetX, state->offsetY, state->zoom);

        state->offsetX += (mouseWorldPreZoomX - mouseWorldPostZoomX);
        state->offsetY += (mouseWorldPreZoomY - mouseWorldPostZoomY);
    }
    
    if(MouseJustUp(MOUSE_BUTTON_MIDDLE) && state->middleButtonDown)
    {
        state->middleButtonDown = false;
    }

    if(state->middleButtonDown)
    {
        state->offsetX += (state->lastClickX - mouseRelX) / state->zoom;
        state->offsetY += (state->lastClickY - mouseRelY) / state->zoom;
        state->lastClickX = mouseRelX;
        state->lastClickY = mouseRelY;
    }
}

void ViewOrthoBaseDraw(View *view)
{
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
            state->quadIndex = ViewAddQuad(view, state->startP, {state->startP.x + gUnitSize, state->startP.y + gUnitSize});
            state->addOtherViewsPolys(state->startP, {state->startP.x + gUnitSize, state->startP.y + gUnitSize});
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
            ViewUpdateQuad(view, state->rect.min, state->rect.max, state->quadIndex);
            state->updateOtherViewsPolys(state->rect, state->quadIndex);
            ViewAddPolyPlane();
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
            ViewUpdateQuad(view, state->rect.min, state->rect.max, state->quadIndex);
            state->updateOtherViewsPolys(state->rect, state->quadIndex);
        }
    }
}
