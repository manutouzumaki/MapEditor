BrushVertex BrushVertexCreate(BrushPlane *brushPlane)
{
    i32  planesCount = DarraySize(brushPlane->planes);
    BrushVertex brushVertex = {};
    brushVertex.polygons = (Poly3D *)DarrayCreate_(brushVertex.polygons, sizeof(Poly3D), planesCount);
    memset(brushVertex.polygons, 0, sizeof(Poly3D)*planesCount);

    for(i32 i = 0; i < planesCount - 2; ++i) {
    for(i32 j = i; j < planesCount - 1; ++j) {
    for(i32 k = j; k < planesCount - 0; ++k) {

        if(i != j && i != k && j != k)
        {
            Plane a = brushPlane->planes[i].plane;
            Plane b = brushPlane->planes[j].plane;
            Plane c = brushPlane->planes[k].plane;

            Vertex vertex = {};
            if(GetIntersection(a.n, b.n, c.n, a.d, b.d, c.d, &vertex))
            {
                bool illegal = false;
                for(i32 m = 0; m < planesCount; ++m)
                {
                    Plane plane = brushPlane->planes[m].plane;
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
                    Vertex iVert = vertex; iVert.normal = brushPlane->planes[i].plane.n;
                    Vertex jVert = vertex; jVert.normal = brushPlane->planes[j].plane.n;
                    Vertex kVert = vertex; kVert.normal = brushPlane->planes[k].plane.n;
                    brushVertex.polygons[i].vertices[brushVertex.polygons[i].verticesCount++] = iVert;
                    brushVertex.polygons[j].vertices[brushVertex.polygons[j].verticesCount++] = jVert;
                    brushVertex.polygons[k].vertices[brushVertex.polygons[k].verticesCount++] = kVert;
                }
            }
        }

    }}}

    for(i32 i = 0; i < planesCount; ++i)
    {
        TextureAxisNormal texAxis = brushPlane->planes[i].axisNormal;
        u32 texture = brushPlane->planes[i].texture;

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
    for(i32 p = 0; p < planesCount; ++p)
    {
        Plane polygonPlane = brushPlane->planes[p].plane; 
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
    for(i32 j = 0; j < DarraySize(brushVertex.polygons); ++j)
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


void BrushVertexUpdate(BrushVertex *brushVert, ViewId viewId,
                       Vec2 oBotL, Vec2 oBotR, Vec2 oTopL, Vec2 oTopR,
                       Vec2 botL,  Vec2 botR,  Vec2 topL,  Vec2 topR)
{
    f32 xNewDim = topR.x - botL.x; 
    f32 yNewDim = topR.y - botL.y;

    for(i32 j = 0; j < DarraySize(brushVert->polygons); ++j)
    {
        Poly3D *poly = brushVert->polygons + j;
        for(i32 i = 0; i < poly->verticesCount; ++i)
        {
            Vertex *v = poly->vertices + i;
            Vec3 *p = &v->position;
            switch(viewId)
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
}

void BrushVertexPushToVertexBuffer(BrushVertex *brushVertex)
{
    Vertex *vertices = 0;
    
    for(i32 i = 0; i < DarraySize(brushVertex->polygons); ++i)
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

void BrushVertexDestroy(BrushVertex *brushVertex)
{
    if(brushVertex->polygons) DarrayDestroy(brushVertex->polygons);
}
