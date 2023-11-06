static Plane GetPlaneFromThreePoints(Vec3 a, Vec3 b, Vec3 c)
{
    Plane p;
    p.n = Vec3Normalized(Vec3Cross(b - a, c - a));
    p.d = Vec3Dot(p.n, a);
    return p;
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

i32 MouseRelToClientX()
{
    i32 mouseRelToClientX = MouseX() - gFixWidth;
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

Entity *MousePicking2D(View *view)
{
    ViewOrthoState *state = &view->orthoState;

    i32 mouseRelX = MouseRelX(view);
    i32 mouseRelY = MouseRelY(view);

    f32 mouseWorldX, mouseWorldY;
    ScreenToWorld(mouseRelX, mouseRelY, mouseWorldX, mouseWorldY,
                  state->offsetX, state->offsetY, state->zoom);

    Entity *entity = gEntityList;
    while(entity)
    {
        
        Brush2D *brush = &entity->brushes2D[view->id];
        // loop over every polygon in the list
        for(i32 i = 0; i < DarraySize(brush->polygons); ++i)
        {
            Poly2D *poly = brush->polygons + i;
            if(PointHitPoly2D(poly, mouseWorldX, mouseWorldY, state->zoom))
            {
                return entity;
            }
        }

        entity = entity->next;
    }

    return nullptr;
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
    for(i32 i = 0; i < DarraySize(brushPlane->planes); ++i)
    {
        Plane p = brushPlane->planes[i].plane;
        f32 denom = Vec3Dot(p.n, d);
        //f32 dist = (p.d / g3DScale) - Vec3Dot(p.n, a);
        f32 dist = Vec3Dot(p.n, a) - (p.d / g3DScale);
        // test if segment runs parallel to tha plane
        if(denom == 0.0f)
        {
            // If so, return "no intersection" if segemnt lies outside the plane
            if(dist > 0.0f) return 0;
        }
        else
        {
            f32 t = -(dist / denom);
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

Entity *MousePicking3D(View *view)
{
    // get the mouse relative to the view but from the bottom up
    i32 mouseRelX = MouseRelX(view) + view->w * 0.5f;
    i32 mouseRelY = view->h - (MouseRelY(view) + view->h * 0.5f);

    Ray ray = GetMouseRay(view, mouseRelX, mouseRelY); 

    // loop over every polygon in the list and save the hitPoint with
    // the smallest t value
    f32 tMin = FLT_MAX;
    Entity *hitEntity = nullptr;

    Entity *entity = gEntityList;
    while(entity)
    {
        BrushPlane *brushPlane = &entity->brushPlane;
        f32 t = -1.0f;
        if(RayHitBrushPlane(ray, brushPlane, &t))
        {
            if(t < tMin)
            {
                tMin = t;
                hitEntity = entity;
            }
        }
        entity = entity->next;
    }

    return hitEntity;
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
