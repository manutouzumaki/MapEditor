#ifndef _TYPES_H_
#define _TYPES_H_

struct Vertex
{
    Vec3 position;
    Vec3 normal;
    Vec4 color;
    Vec2 uv;
    u32 texture; 
};

struct Shader
{
    ID3D11VertexShader *vertex;
    ID3D11PixelShader *fragment;
    ID3DBlob *vertexShaderCompiled;
    ID3DBlob *fragmentShaderCompiled;
};

struct ConstBuffer
{
    ID3D11Buffer *buffer;
    u32 index;
};

struct VertexBuffer
{
    ID3D11Buffer *GPUBuffer;
    u32 verticesCount;

    ID3D11InputLayout *layout;
};

struct DynamicVertexBuffer
{
    ID3D11Buffer *GPUBuffer;
    Vertex *CPUBuffer;
    ID3D11InputLayout *layout;

    size_t size;
    size_t used;
    u32 verticesCount;
};

struct Texture
{
    u32 *pixels;
    i32 w, h;
};

// we are not going to use a texture atlas any more\
// we are going to use a texture Array
struct TextureArray
{
    ID3D11ShaderResourceView *srv;
    ID3D11Texture2D *gpuTextureArray;
    Texture         *cpuTextureArray;
    ID3D11Texture2D **guiTextures;
    ID3D11ShaderResourceView **guiSrv;
    u32 size;
};

struct FrameBuffer
{
    // TODO: MOVE the x ,y,w and h to view structure
    f32 x;
    f32 y;
    f32 width;
    f32 height;
    DXGI_FORMAT format;
     
    ID3D11Texture2D *texture;
    ID3D11RenderTargetView *renderTargetView;
    ID3D11ShaderResourceView *shaderResourceView;
    ID3D11DepthStencilView* depthStencilView;
};

struct File
{
    void *data;
    size_t size;
};

struct CBuffer
{
    Mat4 proj;
    Mat4 view;
    Mat4 world;
    Vec3 viewDir;
    float pad;
};

struct Rect
{
    f32 x, y;
    f32 w, h;
};

// TODO: fix this 
struct RectMinMax
{
    union
    {
        struct
        {
            f32 minX, minY;
            f32 maxX, maxY;
        };
        struct
        {
            Vec2 min;
            Vec2 max;
        };
    };
};

struct RectVert
{
    Vec2 a, b, c, d;
};

struct Plane
{
    Vec3 n;
    f32 d;
};

struct Ray
{
    Vec3 o;
    Vec3 d;
};

struct TextureAxisNormal
{
    Vec3 u, v;
};

struct Poly2D
{
    Vec2 *vertices;
    u32 color;
};

enum PointToPlane
{
    POINT_IN_FRONT_OF_PLANE,
    POINT_BEHIND_PLANE,
    POINT_ON_PLANE
};

enum PolyToPlane
{
    POLYGON_IN_FRONT_OF_PLANE,
    POLYGON_BEHIND_PLANE,
    POLYGON_STRADDLING_PLANE,
    POLYGON_COPLANAR_WITH_PLANE
};

#endif
