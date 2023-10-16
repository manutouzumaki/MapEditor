struct View;

typedef void (*SetupFNP) (View *view);
typedef void (*ProcessFNP) (View *view);
typedef void (*RenderFNP) (View *view);

typedef void (*AddOtherViewsBrushFNP) (Vec2 start, Vec2 end, u32 color);
typedef void (*UpdateOtherViewsBrushFNP) (RectMinMax rect, i32 quadIndex, u32 color);
typedef Plane (*CreateViewClipPlaneFNP) (Vec2 a, Vec2 b);

typedef i32 (*MousePickingFNP) (View *view);


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

struct Camera
{
    Vec3 pos;
    Vec3 rot;

    Vec3 dir;
    Vec3 right;
    Vec3 up;
};

struct ViewPerspState
{
    bool leftButtonDown;
    bool rightButtonDown;  
    
    Camera camera;
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

    i32 controlPointDown;
    bool planeCreated;

    Vec2 startP;
    Vec2 endP;
    i32 quadIndex;
    RectMinMax rect;
    AddOtherViewsBrushFNP addOtherViewsBrush;
    UpdateOtherViewsBrushFNP updateOtherViewsBrush;
    CreateViewClipPlaneFNP createViewClipPlane;

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

    MousePickingFNP mousePicking;
};

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
    for(f32 y = -gGridSize; y < gGridSize; ++y)
    {
        f32 ax = -gGridSize * gUnitSize;
        f32 ay = y * gUnitSize;
        f32 bx = gGridSize * gUnitSize;
        f32 by = y * gUnitSize;
        
        f32 sax, say, sbx, sby;

        WorldToScreen(ax, ay, sax, say, offsetX, offsetY, zoom); 
        WorldToScreen(bx, by, sbx, sby, offsetX, offsetY, zoom); 

        DrawLine(sax,  say, 0, sbx,  sby, 0, 0xFF333333);
    }

    for(f32 x = -gGridSize; x < gGridSize; ++x)
    {

        f32 ax = x * gUnitSize;
        f32 ay = -gGridSize * gUnitSize;
        f32 bx = x * gUnitSize;
        f32 by = gUnitSize*gGridSize;
        
        f32 sax, say, sbx, sby;

        WorldToScreen(ax, ay, sax, say, offsetX, offsetY, zoom); 
        WorldToScreen(bx, by, sbx, sby, offsetX, offsetY, zoom); 

        DrawLine(sax,  say, 0, sbx,  sby, 0, 0xFF333333);
    }

    f32 ax = 0;
    f32 ay = -gGridSize * gUnitSize;
    f32 bx = 0;
    f32 by = gUnitSize*gGridSize;
    f32 sax, say, sbx, sby;
    WorldToScreen(ax, ay, sax, say, offsetX, offsetY, zoom); 
    WorldToScreen(bx, by, sbx, sby, offsetX, offsetY, zoom); 
    DrawLine(sax,  say, 0, sbx,  sby, -1, 0xFFAAFFAA);

    ax = -gGridSize * gUnitSize;
    ay = 0;
    bx =  gGridSize * gUnitSize;
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

Brush2DStorage *ViewGetBrush2DStorage(View *view)
{
    Brush2DStorage *brush2dStorage = gSharedMemory.brush2dStorage + view->id;
    return brush2dStorage;
}


static bool GetIntersection(Vec3 n1, Vec3 n2, Vec3 n3, f32 d1, f32 d2, f32 d3, Vertex *vertex)
{
    Vec3 u = Vec3Cross(n2, n3);
    f32 denom = Vec3Dot(n1, u);
    if(fabsf(denom) < FLT_EPSILON) return false;
    Vec3 pos = (d1 * u + Vec3Cross(n1, d3 * n2 - d2 * n3)) / denom;
    Vec4 col = {0.9, 0.7, 1, 1.0f};
    *vertex = {pos, {}, col, {}};
    return true;
}

static Plane GetPlaneFromThreePoints(Vec3 a, Vec3 b, Vec3 c)
{

    Plane p;
    p.n = Vec3Normalized(Vec3Cross(b - a, c - a));
    p.d = Vec3Dot(p.n, a);
    return p;

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


void RemoveVertexAtIndex(Poly3D *poly, i32 index)
{
    for(i32 i = index; i < (poly->verticesCount - 1); ++i)
    {
        poly->vertices[i] = poly->vertices[i + 1];
    }
    poly->verticesCount--;
}

BrushVertex CreateBrushVertexFromBrushPlane(BrushPlane *brushPlane)
{
    BrushVertex brushVertex{};
    brushVertex.polygonsCount = brushPlane->planesCount;

    for(i32 i = 0; i < brushPlane->planesCount - 2; ++i) {
    for(i32 j = i; j < brushPlane->planesCount - 1; ++j) {
    for(i32 k = j; k < brushPlane->planesCount - 0; ++k) {

        if(i != j && i != k && j != k)
        {
            Plane a = brushPlane->planes[i];
            Plane b = brushPlane->planes[j];
            Plane c = brushPlane->planes[k];

            Vertex vertex = {};
            if(GetIntersection(a.n, b.n, c.n, a.d, b.d, c.d, &vertex))
            {
                bool illegal = false;
                for(i32 m = 0; m < brushPlane->planesCount; ++m)
                {
                    Plane plane = brushPlane->planes[m];
                    f32 dot = Vec3Dot(plane.n, vertex.position);
                    f32 d = plane.d;
                    f32 dist = dot - d;
                    if(dist > EPSILON)
                    {
                        illegal = true;
                    }
                }
                if(illegal == false)
                {
                    Vertex iVert = vertex; iVert.normal = brushPlane->planes[i].n;
                    Vertex jVert = vertex; jVert.normal = brushPlane->planes[j].n;
                    Vertex kVert = vertex; kVert.normal = brushPlane->planes[k].n;
                    brushVertex.polygons[i].vertices[brushVertex.polygons[i].verticesCount++] = iVert;
                    brushVertex.polygons[j].vertices[brushVertex.polygons[j].verticesCount++] = jVert;
                    brushVertex.polygons[k].vertices[brushVertex.polygons[k].verticesCount++] = kVert;
                }
            }
        }

    }}}

    for(i32 i = 0; i < brushPlane->planesCount; ++i)
    {
        // for now all our polys have for vertices
        // so we cant hard code the valus TODO: change this for a more general code
        TextureAxisNormal texAxis = brushPlane->axisNormals[i];
        u32 texture = brushPlane->textures[i];

        Poly3D *polyD = &brushVertex.polygons[i];
        Vec3 center = GetCenterOfPolygon(polyD);
        for(i32 j = 0; j < polyD->verticesCount; ++j)
        {
            polyD->vertices[j].texture = texture;

            f32 u, v;
            u = Vec3Dot(texAxis.u, polyD->vertices[j].position);
            u = u / gUnitSize;

            v = Vec3Dot(texAxis.v, polyD->vertices[j].position);
            v = v / gUnitSize;

            polyD->vertices[j].uv = { u, v };
        }
    }

    // order the vertices in the polygons
    for(i32 p = 0; p < brushPlane->planesCount; ++p)
    {
        Plane polygonPlane = brushPlane->planes[p]; 
        Poly3D *polygon = brushVertex.polygons + p;

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
                if((Vec3Dot(p.n, vertex.position) - p.d) > EPSILON)
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

    // remove repeted vertex
    for(i32 j = 0; j < brushVertex.polygonsCount; ++j)
    {
        Poly3D *poly = brushVertex.polygons + j;
        for(i32 i = 0; i < poly->verticesCount; ++i)
        {
            Vertex a = poly->vertices[i];
            for(i32 k = 0; k < poly->verticesCount; ++k)
            {
                Vertex b = poly->vertices[k];
                if(i != k)
                {
                    // remove the vertex
                    if(a.position == b.position)
                    {
                        RemoveVertexAtIndex(poly, k);
                        k--;
                    }
                }
            }
        }
    }

    return brushVertex;
}

void PushBrushVertexToVertexBuffer(BrushVertex *brushVertex)
{
    Vertex *vertices = 0;
    
    for(i32 i = 0; i < brushVertex->polygonsCount; ++i)
    {
        Poly3D *polyD = &brushVertex->polygons[i];
        for(i32 j = 0; j < polyD->verticesCount - 2; ++j)
        {
            Vertex a = polyD->vertices[0];
            Vertex b = polyD->vertices[j + 1];
            Vertex c = polyD->vertices[j + 2];

            // scale down for rendering
            Mat4 scaleMat = Mat4Scale(1.0f/g3DScale, 1.0f/g3DScale, 1.0f/g3DScale);
            a.position = Mat4TransformPoint(scaleMat, a.position);
            b.position = Mat4TransformPoint(scaleMat, b.position);
            c.position = Mat4TransformPoint(scaleMat, c.position);
            
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
}

// TODO: remplace this function for a generic polygon function
// not just cubes
BrushPlane CreateBrushPlane(Vec2 xMinMax, Vec2 yMinMax, Vec2 zMinMax)
{
    BrushPlane brushPlane;

    brushPlane.planes[0] = { { 1,  0,  0},  xMinMax.y };
    brushPlane.planes[1] = { {-1,  0,  0}, -xMinMax.x };
    brushPlane.planes[2] = { { 0,  1,  0},  yMinMax.y };
    brushPlane.planes[3] = { { 0, -1,  0}, -yMinMax.x };
    brushPlane.planes[4] = { { 0,  0,  1},  zMinMax.y };
    brushPlane.planes[5] = { { 0,  0, -1}, -zMinMax.x };

    brushPlane.axisNormals[0] = { { 0,  0, 1}, {0, -1, 0} };
    brushPlane.axisNormals[1] = { { 0,  0, 1}, {0, -1, 0} };
    brushPlane.axisNormals[2] = { { 1,  0, 0}, {0, 0, -1} };
    brushPlane.axisNormals[3] = { { 1,  0, 0}, {0, 0, -1} };
    brushPlane.axisNormals[4] = { { 1,  0, 0}, {0, -1, 0} };
    brushPlane.axisNormals[5] = { { 1,  0, 0}, {0, -1, 0} };

    brushPlane.planesCount = 6;
    return brushPlane;
}

PointToPlane ClassifyPointToPlane(Vec3 p, Plane plane)
{
    f32 dist = Vec3Dot(plane.n, p) - plane.d;
    if(dist > EPSILON)
        return POINT_IN_FRONT_OF_PLANE;
    if(dist< -EPSILON)
        return POINT_BEHIND_PLANE;
    return POINT_ON_PLANE;
}

PolyToPlane ClassifyPolygonToPlane(Poly3D *poly, Plane plane)
{
    i32 numInFront = 0, numBehind = 0;
    i32 numVerts = poly->verticesCount;
    for(i32 i = 0; i < numVerts; ++i)
    {
        Vec3 p = poly->vertices[i].position;
        switch(ClassifyPointToPlane(p, plane))
        {
            case POINT_IN_FRONT_OF_PLANE:
            {
                numInFront++;
                break;
            }
            case POINT_BEHIND_PLANE:
            {
                numBehind++;
                break;
            }
        }
    }

    if(numBehind != 0 && numInFront != 0)
        return POLYGON_STRADDLING_PLANE;
    if(numInFront != 0)
        return POLYGON_IN_FRONT_OF_PLANE;
    if(numBehind != 0)
        return POLYGON_BEHIND_PLANE;
    return POLYGON_COPLANAR_WITH_PLANE;
}

void ViewClipBrush(i32 index, Plane clipPlane, ViewId id)
{
    // Clip the polyVert with the clip plane
    BrushStorage *brushStorage = &gSharedMemory.brushStorage;
    BrushVertex *brushVert = brushStorage->brushVerts + index;
    BrushPlane *brushPlane = brushStorage->brushPlanes + index;

    // Remove invalid planes
    i32 *planesToRemove = 0;
    for(i32 i = 0; i < brushVert->polygonsCount; ++i)
    {
        Poly3D *poly = brushVert->polygons + i;

        if(ClassifyPolygonToPlane(poly, clipPlane) == POLYGON_IN_FRONT_OF_PLANE)
        {
            DarrayPush(planesToRemove, 1, i32);
        }
        else
        {   
            DarrayPush(planesToRemove, 0, i32);
        }
    }

    BrushPlane newBrushPlane = {};

    for(i32 i = 0; i < brushPlane->planesCount; ++i)
    {
        if(planesToRemove[i] == 0)
        {
            newBrushPlane.planes[newBrushPlane.planesCount] = brushPlane->planes[i];
            newBrushPlane.axisNormals[newBrushPlane.planesCount] = brushPlane->axisNormals[i];
            newBrushPlane.textures[newBrushPlane.planesCount] = brushPlane->textures[i];
            newBrushPlane.planesCount++;
        }
    }

    DarrayDestroy(planesToRemove);

    // Add the new clipPlane
    newBrushPlane.planes[newBrushPlane.planesCount] = clipPlane;
    switch(id)
    {
        case VIEW_TOP: 
        {
            Vec3 tangent = {0, -1, 0};
            newBrushPlane.axisNormals[newBrushPlane.planesCount] =  {Vec3Normalized(Vec3Cross(clipPlane.n, tangent)), tangent};
        } break;
        case VIEW_FRONT:
        {
            Vec3 tangent = {0, 0, 1};
            newBrushPlane.axisNormals[newBrushPlane.planesCount] =  {Vec3Normalized(Vec3Cross(clipPlane.n, tangent)), tangent};
        } break;
        case VIEW_SIDE:
        {
            Vec3 tangent = {1, 0, 0};
            newBrushPlane.axisNormals[newBrushPlane.planesCount] =  {Vec3Normalized(Vec3Cross(clipPlane.n, tangent)), tangent};
        } break;
    }
    newBrushPlane.textures[newBrushPlane.planesCount] = newBrushPlane.textures[0];
    newBrushPlane.planesCount++;

    // TODO: we probably will have to change this
    *brushPlane = newBrushPlane;
    *brushVert = CreateBrushVertexFromBrushPlane(brushPlane);

    // reload the dynamic vertex buffer
    gDynamicVertexBuffer.used = 0;
    gDynamicVertexBuffer.verticesCount = 0;
    for(i32 i = 0; i < brushStorage->brushesCount; ++i)
    {
        PushBrushVertexToVertexBuffer(brushStorage->brushVerts + i);
    }
}

Plane Poly3DCalculatePlane(Poly3D *poly)
{
    Vec3 centerOfMass;
    f32	magnitude;
    i32 i, j;

    if ( poly->verticesCount < 3 )
	{
		printf("Polygon has less than 3 vertices!\n");
        ASSERT(!"INVALID_CODE_PATH");
	}

    Plane plane = {};
    centerOfMass.x	= 0.0f; 
    centerOfMass.y	= 0.0f; 
    centerOfMass.z	= 0.0f;

    for ( i = 0; i < poly->verticesCount; i++ )
    {
        j = i + 1;

        if ( j >= poly->verticesCount )
		{
			j = 0;
		}

        Vertex *verts = poly->vertices;

        plane.n.x += ( verts[ i ].position.y - verts[ j ].position.y ) * ( verts[ i ].position.z + verts[ j ].position.z );
        plane.n.y += ( verts[ i ].position.z - verts[ j ].position.z ) * ( verts[ i ].position.x + verts[ j ].position.x );
        plane.n.z += ( verts[ i ].position.x - verts[ j ].position.x ) * ( verts[ i ].position.y + verts[ j ].position.y );

        centerOfMass.x += verts[ i ].position.x;
        centerOfMass.y += verts[ i ].position.y;
        centerOfMass.z += verts[ i ].position.z;
    }

    if ( ( fabs ( plane.n.x ) < FLT_EPSILON ) &&
         ( fabs ( plane.n.y ) < FLT_EPSILON ) &&
		 ( fabs ( plane.n.z ) < FLT_EPSILON ) )
    {
         ASSERT(!"INVALID_CODE_PATH");
    }

    magnitude = sqrt ( plane.n.x * plane.n.x + plane.n.y * plane.n.y + plane.n.z * plane.n.z );

    if ( magnitude < FLT_EPSILON )
	{
        ASSERT(!"INVALID_CODE_PATH");
	}

    plane.n.x /= magnitude;
    plane.n.y /= magnitude;
    plane.n.z /= magnitude;

    plane.n = Vec3Normalized(plane.n);

    centerOfMass.x /= (f32)poly->verticesCount;
    centerOfMass.y /= (f32)poly->verticesCount;
    centerOfMass.z /= (f32)poly->verticesCount;

    plane.d = Vec3Dot(centerOfMass, plane.n);

    return plane;

}

void ViewUpdateBrushPlaneFromBrushVertex(i32 index)
{
    BrushStorage *brushStorage = &gSharedMemory.brushStorage;
    BrushVertex *brushVert = brushStorage->brushVerts + index;
    BrushPlane *brushPlane = brushStorage->brushPlanes + index;

    for(i32 i = 0; i < brushVert->polygonsCount; ++i)
    {
        Poly3D *poly = brushVert->polygons + i;

        // Update the plane
        Plane newPlane = Poly3DCalculatePlane(poly);
        brushPlane->planes[i] = newPlane;

        // Update the UVs
        TextureAxisNormal texAxis = brushPlane->axisNormals[i];
        Vec3 center = GetCenterOfPolygon(poly);
        for(i32 j = 0; j < poly->verticesCount; ++j)
        {
            f32 u, v;
            u = Vec3Dot(texAxis.u, poly->vertices[j].position);
            u = u / gUnitSize;

            v = Vec3Dot(texAxis.v, poly->vertices[j].position);
            v = v / gUnitSize;

            poly->vertices[j].uv = { u, v };
        }
    }

    // reload the dynamic vertex buffer
    gDynamicVertexBuffer.used = 0;
    gDynamicVertexBuffer.verticesCount = 0;
    for(i32 i = 0; i < brushStorage->brushesCount; ++i)
    {
        PushBrushVertexToVertexBuffer(brushStorage->brushVerts + i);
    } 
}

void UpdateBrush2D(BrushVertex *brushVert, BrushPlane *brushPlane, Brush2D *brush2D, Vec3 normal)
{
    // we need to add to the vertexPoly2D all the face that are not perpendicular to (0, 1, 0)
    // remove all the old information
    for(i32 i = 0; i < DarraySize(brush2D->polygons); ++i)
    {
        Poly2D *poly = brush2D->polygons + i;
        DarrayDestroy(poly->vertices);
    }
    DarrayDestroy(brush2D->polygons);
    brush2D->polygons = 0;
    
    i32 *facesToAdd = 0;
    for(i32 i = 0; i < brushPlane->planesCount; ++i)
    {
        Plane p = brushPlane->planes[i];
        f32 dot = Vec3Dot(p.n, normal);
        if(dot >= FLT_EPSILON || dot <= -FLT_EPSILON)
        {
            DarrayPush(facesToAdd, i, i32);
        }
    }

    for(i32 i = 0; i < DarraySize(facesToAdd); ++i)
    {
        i32 faceIndex = facesToAdd[i];
        Poly3D *poly3D = brushVert->polygons + faceIndex;
        Poly2D poly2D = {};
        poly2D.color = 0xFFFFFFAA;
        for(i32 j = 0; j < poly3D->verticesCount; ++j)
        {
            Vertex vertex = poly3D->vertices[j];
            Vec2 vertice = {};

            if(normal.x != 0.0f)
                vertice = {vertex.position.z, vertex.position.y};

            if(normal.y != 0.0f)
                vertice = {vertex.position.x, vertex.position.z};

            if(normal.z != 0.0f)
                vertice = {vertex.position.x, vertex.position.y};  

            DarrayPush(poly2D.vertices, vertice, Vec2);
        }
        DarrayPush(brush2D->polygons, poly2D, Poly2D);
    }

    DarrayDestroy(facesToAdd);
}

void ViewUpdateBrush2DFromBrushPlane(i32 index)
{
    BrushStorage *brushStorage = &gSharedMemory.brushStorage;
    Brush2DStorage *topStorage = gSharedMemory.brush2dStorage + VIEW_TOP;
    Brush2DStorage *frontStorage = gSharedMemory.brush2dStorage + VIEW_FRONT;
    Brush2DStorage *sideStorage = gSharedMemory.brush2dStorage + VIEW_SIDE;

    // src (where we get the data from)
    BrushVertex   *brushVert    = brushStorage->brushVerts  + index;
    BrushPlane    *brushPlane   = brushStorage->brushPlanes + index;
    // dst (what we need to modify with the data we get)
    Brush2D *topBrush2D   = topStorage->brushes   + index;
    Brush2D *frontBrush2D = frontStorage->brushes + index;
    Brush2D *sideBrush2D  = sideStorage->brushes  + index;

    UpdateBrush2D(brushVert, brushPlane, topBrush2D,   {0, 1, 0});
    UpdateBrush2D(brushVert, brushPlane, frontBrush2D, {0, 0, 1});
    UpdateBrush2D(brushVert, brushPlane, sideBrush2D,  {1, 0, 0});
}

i32 ViewAddBrushPlane()
{
    BrushStorage *brushStorage = &gSharedMemory.brushStorage;
    Brush2DStorage *frontStorage = gSharedMemory.brush2dStorage + VIEW_FRONT;
    Brush2DStorage *sideStorage = gSharedMemory.brush2dStorage + VIEW_SIDE;

    ASSERT((brushStorage->brushesCount + 1) < ARRAY_LENGTH(brushStorage->brushPlanes));
    i32 index = brushStorage->brushesCount++;

    Brush2D *frontBrush = frontStorage->brushes + index;
    Brush2D *sideBrush  = sideStorage->brushes  + index;

    Poly2D *front = &frontBrush->polygons[0]; 
    Poly2D *side = &sideBrush->polygons[0]; 

    Vec2 xDim = {FLT_MAX, -FLT_MAX}; 
    Vec2 yDim = {FLT_MAX, -FLT_MAX}; 
    Vec2 zDim = {FLT_MAX, -FLT_MAX};

    for(i32 i = 0; i < DarraySize(front->vertices); ++i)
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

    for(i32 i = 0; i < DarraySize(side->vertices); ++i)
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

    BrushPlane brushPlane = CreateBrushPlane(xDim, yDim, zDim);
    brushPlane.textures[0] = gCurrentTexture;
    brushPlane.textures[1] = gCurrentTexture;
    brushPlane.textures[2] = gCurrentTexture;
    brushPlane.textures[3] = gCurrentTexture;
    brushPlane.textures[4] = gCurrentTexture;
    brushPlane.textures[5] = gCurrentTexture;
    brushStorage->brushPlanes[index] = brushPlane;
    brushStorage->brushVerts[index] = CreateBrushVertexFromBrushPlane(&brushPlane);
    PushBrushVertexToVertexBuffer(brushStorage->brushVerts + index);

    return index;
}

// TODO: IMPORTANT dont forget to free this
i32 ViewAddBrush2D(View *view, Vec2 start, Vec2 end)
{
    Brush2DStorage *brush2dStorage = ViewGetBrush2DStorage(view);

    Poly2D poly = {};
    
    Vec2 a = {start.x, start.y};
    Vec2 b = {end.x,   start.y};
    Vec2 c = {end.x,   end.y}; 
    Vec2 d = {start.x, end.y};

    DarrayPush(poly.vertices, a, Vec2);
    DarrayPush(poly.vertices, b, Vec2);
    DarrayPush(poly.vertices, c, Vec2);
    DarrayPush(poly.vertices, d, Vec2);

    poly.color = 0xFFFFFFAA;

    Brush2D brush = {};
    DarrayPush(brush.polygons, poly, Poly2D);
    DarrayPush(brush2dStorage->brushes, brush, Brush2D);

    i32 index = DarraySize(brush2dStorage->brushes) - 1;

    return index;
}

// TODO: mabye change the name of this function (this is only use when we add more brushes)
void ViewUpdateBrush2D(View *view, Vec2 start, Vec2 end, i32 index)
{
    Brush2DStorage *brush2dStorage = ViewGetBrush2DStorage(view);

    Brush2D *brush = brush2dStorage->brushes + index;

    Poly2D *poly = &brush->polygons[0];

    // TODO: fix this
    ASSERT(DarraySize(poly->vertices) >= 4);

    poly->vertices[0] = {start.x, start.y};
    poly->vertices[1] = {end.x, start.y};
    poly->vertices[2] = {end.x, end.y};
    poly->vertices[3] = {start.x, end.y};
    poly->color = 0xFFFFFFAA;
}

void RenderBrush2D(View *view, Brush2D *brush)
{
    ViewOrthoState *state = &view->orthoState;

    for(i32 j = 0; j < DarraySize(brush->polygons); ++j)
    {
        Poly2D *poly = brush->polygons + j;
        u32 color = poly->color;
        for(i32 i = 0; i < DarraySize(poly->vertices) - 1; ++i)
        {
            Vec2 a = poly->vertices[i + 0];
            Vec2 b = poly->vertices[i + 1];

            f32 sax, say, sbx, sby;
            WorldToScreen(a.x, a.y, sax, say, state->offsetX, state->offsetY, state->zoom); 
            WorldToScreen(b.x, b.y, sbx, sby, state->offsetX, state->offsetY, state->zoom); 

            DrawLine(sax,  say, -2, sbx,  sby, -2, color);
        }

        Vec2 a = poly->vertices[DarraySize(poly->vertices) - 1];
        Vec2 b = poly->vertices[0];
        f32 sax, say, sbx, sby;
        WorldToScreen(a.x, a.y, sax, say, state->offsetX, state->offsetY, state->zoom); 
        WorldToScreen(b.x, b.y, sbx, sby, state->offsetX, state->offsetY, state->zoom); 
        DrawLine(sax,  say, -2, sbx,  sby, -2, color);
    }
}

void RenderBrush2DColor(View *view, Brush2D *brush, u32 color)
{
    ViewOrthoState *state = &view->orthoState;

    for(i32 j = 0; j < DarraySize(brush->polygons); ++j)
    {
        Poly2D *poly = brush->polygons + j;
        for(i32 i = 0; i < DarraySize(poly->vertices) - 1; ++i)
        {
            Vec2 a = poly->vertices[i + 0];
            Vec2 b = poly->vertices[i + 1];

            f32 sax, say, sbx, sby;
            WorldToScreen(a.x, a.y, sax, say, state->offsetX, state->offsetY, state->zoom); 
            WorldToScreen(b.x, b.y, sbx, sby, state->offsetX, state->offsetY, state->zoom); 

            DrawLine(sax,  say, -2, sbx,  sby, -2, color);
        }

        Vec2 a = poly->vertices[DarraySize(poly->vertices) - 1];
        Vec2 b = poly->vertices[0];
        f32 sax, say, sbx, sby;
        WorldToScreen(a.x, a.y, sax, say, state->offsetX, state->offsetY, state->zoom); 
        WorldToScreen(b.x, b.y, sbx, sby, state->offsetX, state->offsetY, state->zoom); 
        DrawLine(sax,  say, -2, sbx,  sby, -2, color);
    }
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
