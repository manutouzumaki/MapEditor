BrushPlane BrushPlaneCreate(Vec2 xMinMax, Vec2 yMinMax, Vec2 zMinMax)
{
    BrushPlane brushPlane = {};

    PolyPlane polyPlane0 = { { { 1,  0,  0},  xMinMax.y }, { { 0,  0, 1}, {0, -1, 0} }, gCurrentTexture};
    PolyPlane polyPlane1 = { { {-1,  0,  0}, -xMinMax.x }, { { 0,  0, 1}, {0, -1, 0} }, gCurrentTexture};
    PolyPlane polyPlane2 = { { { 0,  1,  0},  yMinMax.y }, { { 1,  0, 0}, {0, 0, -1} }, gCurrentTexture};
    PolyPlane polyPlane3 = { { { 0, -1,  0}, -yMinMax.x }, { { 1,  0, 0}, {0, 0, -1} }, gCurrentTexture};
    PolyPlane polyPlane4 = { { { 0,  0,  1},  zMinMax.y }, { { 1,  0, 0}, {0, -1, 0} }, gCurrentTexture};
    PolyPlane polyPlane5 = { { { 0,  0, -1}, -zMinMax.x }, { { 1,  0, 0}, {0, -1, 0} }, gCurrentTexture};

    DarrayPush(brushPlane.planes, polyPlane0, PolyPlane); 
    DarrayPush(brushPlane.planes, polyPlane1, PolyPlane); 
    DarrayPush(brushPlane.planes, polyPlane2, PolyPlane); 
    DarrayPush(brushPlane.planes, polyPlane3, PolyPlane); 
    DarrayPush(brushPlane.planes, polyPlane4, PolyPlane); 
    DarrayPush(brushPlane.planes, polyPlane5, PolyPlane);

    return brushPlane;
}


void BrushPlaneUpdate(BrushPlane *brushPlane, BrushVertex *brushVert)
{
    for(i32 i = 0; i < DarraySize(brushVert->polygons); ++i)
    {
        Poly3D *poly = brushVert->polygons + i;
        // Update the plane
        brushPlane->planes[i].plane = Poly3DCalculatePlane(poly);

        // Update the UVs TODO: find a better place to update this uvs
        // hopefully in the brushVertex.cpp
        TextureAxisNormal texAxis = brushPlane->planes[i].axisNormal;
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
        ///////////////////////////////////////////////////////////////
    }
}

void BrushPlaneDestroy(BrushPlane *brushPlane)
{
    if(brushPlane->planes) DarrayDestroy(brushPlane->planes);
}

void BrushPlaneClip(BrushPlane *brushPlane, BrushVertex *brushVert, Plane clipPlane, ViewId viewId)
{
    // Remove invalid planes
    i32 *planesToRemove = 0;
    for(i32 i = 0; i < DarraySize(brushVert->polygons); ++i)
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

    for(i32 i = 0; i < DarraySize(brushPlane->planes); ++i)
    {
        if(planesToRemove[i] == 0)
        {
            PolyPlane polyPlane;
            polyPlane.plane = brushPlane->planes[i].plane;
            polyPlane.axisNormal = brushPlane->planes[i].axisNormal;
            polyPlane.texture = brushPlane->planes[i].texture;
            DarrayPush(newBrushPlane.planes, polyPlane, PolyPlane);
        }
    }

    DarrayDestroy(planesToRemove);

    // Add the new clipPlane
    Vec3 tangent;
    switch(viewId)
    {
        case VIEW_TOP: 
        {
            tangent = {0, -1, 0};
        } break;
        case VIEW_FRONT:
        {
            tangent = {0, 0, 1};
        } break;
        case VIEW_SIDE:
        {
            tangent = {1, 0, 0};
        } break;
    }

    PolyPlane polyPlane;
    polyPlane.plane = clipPlane;
    polyPlane.axisNormal =  {Vec3Normalized(Vec3Cross(clipPlane.n, tangent)), tangent};
    polyPlane.texture = newBrushPlane.planes[0].texture;
    DarrayPush(newBrushPlane.planes, polyPlane, PolyPlane);

    // delete the old array
    BrushPlaneDestroy(brushPlane);
    *brushPlane = newBrushPlane;
}
