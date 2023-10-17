struct View;
struct Entity;

typedef void (*SetupFNP) (View *view);
typedef void (*ProcessFNP) (View *view);
typedef void (*RenderFNP) (View *view);

typedef void (*AddOtherViewsBrushFNP) (Vec2 start, Vec2 end, u32 color);
typedef void (*UpdateOtherViewsBrushFNP) (RectMinMax rect, i32 quadIndex, u32 color);
typedef Plane (*CreateViewClipPlaneFNP) (Vec2 a, Vec2 b);

//typedef i32 (*MousePickingFNP) (View *view);
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
    i32 quadIndex;
    RectMinMax rect;

    // TODO: remove this callbacks
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
