struct HMapHeader
{
    u32 entityCount;
    u64 entitySize;
    u64 entityOffset;

    u32 textureCount;
    u32 textureWidth;
    u32 textureHeight;
    u64 textureOffset;
};

struct EntityHeader
{
    u32 faceCount;
    u64 faceSize;
};


void CreateBinaryFile_HMAP()
{

    HMapHeader header = {};

    Entity *entity = gEntityList;
    while(entity)
    {




        entity = entity->next;
    }

}

