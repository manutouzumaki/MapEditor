struct View;
struct Entity;

typedef void (*SetupFNP) (View *view);
typedef void (*ProcessFNP) (View *view);
typedef void (*RenderFNP) (View *view);
typedef Plane (*CreateViewClipPlaneFNP) (Vec2 a, Vec2 b);
typedef Entity *(*MousePickingFNP) (View *view);


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
