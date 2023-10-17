Brush2D Brush2DCreate(Vec2 start, Vec2 end) 
{
    Brush2D brush = {};
    
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

    DarrayPush(brush.polygons, poly, Poly2D);
    return brush;
}

void Brush2DDestroy(Brush2D *brush)
{
    for(i32 i = 0; i < DarraySize(brush->polygons); ++i)
    {
        Poly2D *poly = brush->polygons + i;
        if(poly->vertices) DarrayDestroy(poly->vertices);
    }
    if(brush->polygons) DarrayDestroy(brush->polygons);
}


void Brush2DUpdate(Brush2D *brush2D, BrushVertex *brushVert, BrushPlane *brushPlane, Vec3 normal)
{
    // remove all the old information
    Brush2DDestroy(brush2D);
    brush2D->polygons = 0;
    
    i32 *facesToAdd = 0;
    for(i32 i = 0; i < DarraySize(brushPlane->planes); ++i)
    {
        Plane p = brushPlane->planes[i].plane;
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

void CalculateDimensionsFromBrushes2D(Brush2D *frontBrush, Brush2D *sideBrush,
                                       Vec2 &xDim, Vec2 &yDim, Vec2 &zDim)
{
    Poly2D *front = &frontBrush->polygons[0]; 
    Poly2D *side = &sideBrush->polygons[0]; 

    xDim = {FLT_MAX, -FLT_MAX}; 
    yDim = {FLT_MAX, -FLT_MAX}; 
    zDim = {FLT_MAX, -FLT_MAX};

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
}
