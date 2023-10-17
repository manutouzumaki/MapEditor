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

static Plane GetPlaneFromThreePoints(Vec3 a, Vec3 b, Vec3 c)
{
    Plane p;
    p.n = Vec3Normalized(Vec3Cross(b - a, c - a));
    p.d = Vec3Dot(p.n, a);
    return p;
}

static void RemoveVertexAtIndex(Poly3D *poly, i32 index)
{
    for(i32 i = index; i < (poly->verticesCount - 1); ++i)
    {
        poly->vertices[i] = poly->vertices[i + 1];
    }
    poly->verticesCount--;
}

static Plane Poly3DCalculatePlane(Poly3D *poly)
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

static PointToPlane ClassifyPointToPlane(Vec3 p, Plane plane)
{
    f32 dist = Vec3Dot(plane.n, p) - plane.d;
    if(dist > EPSILON)
        return POINT_IN_FRONT_OF_PLANE;
    if(dist< -EPSILON)
        return POINT_BEHIND_PLANE;
    return POINT_ON_PLANE;
}

static PolyToPlane ClassifyPolygonToPlane(Poly3D *poly, Plane plane)
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

static void WorldToScreen(f32 wx, f32 wy, f32 &sx, f32 &sy, f32 offsetX, f32 offsetY, f32 zoom)
{
    sx = (wx - offsetX) * zoom;
    sy = (wy - offsetY) * zoom;
}

static void ScreenToWorld(f32 sx, f32 sy, f32 &wx, f32 &wy, f32 offsetX, f32 offsetY, f32 zoom)
{
    wx = (sx / zoom) + offsetX;
    wy = (sy / zoom) + offsetY;
}

static void Draw2DQuad(ViewOrthoState *state, Vec2 botL, Vec2 botR, Vec2 topL, Vec2 topR, u32 color, i32 zIndex)
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

static void RenderGrid(f32 offsetX, f32 offsetY, f32 zoom)
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
