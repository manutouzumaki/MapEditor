
// Brush2D ////////////////////////////
struct Brush2D
{
    Poly2D *polygons;
};


// BrushPlane /////////////////////////
struct PolyPlane
{
    Plane plane;
    TextureAxisNormal axisNormal;
    u32 texture;
};

struct BrushPlane
{
    PolyPlane *planes;
};


// BrushVertex ////////////////////////
struct Poly3D
{
    Vertex vertices[256];
    i32 verticesCount;
};

struct BrushVertex
{
    Poly3D *polygons;
};
