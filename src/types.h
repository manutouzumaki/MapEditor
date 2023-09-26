#ifndef _TYPES_H_
#define _TYPES_H_

struct Vertex
{
    Vec3 position;
    Vec4 color;
    Vec2 uv;
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

struct Plane
{
    Vec3 n;
    f32 d;
};

struct PolyPlane
{
    Plane planes[255];
    i32 planesCount;
};

struct Poly2D
{
    Vec2 vertices[255];
    i32 verticesCount;
};

#endif
