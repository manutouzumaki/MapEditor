struct Entity 
{
    BrushPlane brushPlane;
    BrushVertex brushVert;
    Brush2D brushes2D[3];

    u32 key;
    Entity *next;
    Entity *prev;

    static u32 entityCount;
};
