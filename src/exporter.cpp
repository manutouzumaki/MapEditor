struct HMapHeader
{
    u64 entityOffset;
    u64 textureOffset;
};

struct EntityHeader
{
    u32 faceCount;
    u64 faceSize;
};

struct EntityFace
{
    Plane plane;
    TextureAxisNormal axisNormal;
    u32 texture;
};

struct TextureHeader
{
    u32 textureWidth;
    u32 textureHeight;
};

struct BinaryFile
{
    u8 *data;
    size_t size;
    size_t used;
};



void CreateBinaryFile_HMAP()
{
    size_t fileSize = sizeof(HMapHeader);
    
    // find the amount of memory to alloc
    if(gEntityList == nullptr) return;
    
    Entity *entity = gEntityList;
    while(entity)
    {

        u32 faceCount = DarraySize(entity->brushPlane.planes);
        fileSize += sizeof(EntityHeader);
        fileSize += faceCount * sizeof(EntityFace);
               
        entity = entity->next;
    }


    for(i32 i = 0; i < DarraySize(gTextureArray.cpuTextureArray); ++i)
    {
        Texture *texture = &gTextureArray.cpuTextureArray[i];
        fileSize += sizeof(TextureHeader);
        fileSize += sizeof(u32) * texture->w * texture->h; 
    }



    // alloc memory and init the file object
    BinaryFile file = {};
    file.data = (u8 *)malloc(fileSize);
    file.size = fileSize;
    memset(file.data, 0, fileSize);

    // fill the header
    HMapHeader *header = (HMapHeader *)(file.data + file.used);
    file.used += sizeof(HMapHeader);

    header->entityOffset = file.used;

    entity = gEntityList;
    while(entity)
    {
        EntityHeader *entityHeader = (EntityHeader *)(file.data + file.used);
        entityHeader->faceSize = sizeof(EntityFace);
        file.used += sizeof(EntityHeader);

        BrushPlane *brushPlane = &entity->brushPlane;
        for(i32 i = 0; i < DarraySize(brushPlane->planes); ++i)
        {
            // fill the face data
            EntityFace *entityFace = (EntityFace *)(file.data + file.used);

            PolyPlane *polyPlane = &brushPlane->planes[i];

            entityFace->plane = polyPlane->plane;
            entityFace->axisNormal = polyPlane->axisNormal;
            entityFace->texture = polyPlane->texture;

            file.used += sizeof(EntityFace);
            entityHeader->faceCount++; 
        }

               
        entity = entity->next;
    }

    header->textureOffset = file.used;

    for(i32 i = 0; i < DarraySize(gTextureArray.cpuTextureArray); ++i)
    {
        Texture *texture = &gTextureArray.cpuTextureArray[i];
        TextureHeader *textureHeader = (TextureHeader *)(file.data + file.used);
        textureHeader->textureWidth = texture->w;
        textureHeader->textureHeight = texture->h;
        file.used += sizeof(TextureHeader);
        u32 *texturePixels = (u32 *)(file.data + file.used);
        memcpy(texturePixels, texture->pixels, sizeof(u32) * texture->w * texture->h);
        file.used += sizeof(u32) * texture->w * texture->h;
    }

    ASSERT(file.used <= file.size);

    // TODO: write file ...
    WriteBinaryFile("../maps/test2.map", (void *)file.data, file.size);
}

void LoadBinaryFile_HMAP(char *filepath)
{
    if(gEntityList != nullptr)
    {
        Entity *entity = gEntityList;
        while(entity)
        {
            Entity *next = entity->next;
            EntityDestroy(entity);
            entity = next;
        }
        gEntityList = nullptr;
        gSelectedEntity = nullptr;
    }

    File file = ReadFile(filepath);
    u8 *data = (u8 *)file.data;

    HMapHeader *hMapheader = (HMapHeader *)data;
    u8 *entityChunk = data + hMapheader->entityOffset;
    u8 *textureChunk = data + hMapheader->textureOffset;

    u8 *currentEntity = entityChunk;
    while(currentEntity < textureChunk)
    {
        BrushPlane brushPlane = {};

        EntityHeader *entityHeader = (EntityHeader *)currentEntity;
        u8 *faces = currentEntity + sizeof(EntityHeader);
        for(i32 i = 0; i < entityHeader->faceCount; ++i)
        {
            EntityFace *entityFace = (EntityFace *)(faces + (entityHeader->faceSize * i));
            PolyPlane polyPlane = {entityFace->plane, entityFace->axisNormal, entityFace->texture};
            DarrayPush(brushPlane.planes, polyPlane, PolyPlane); 
        }

        gEntityList =  EntityAdd(gEntityList, EntityCreateFromBrushPlane(&brushPlane));

        currentEntity += (entityHeader->faceCount * entityHeader->faceSize) + sizeof(EntityHeader);
    }


    gDirtyFlag = true;

    

    // TODO: for testing we just need the entities ...
    /*
    u8 *currentTexture = textureChunk;
    while(currentTexture != '\0')
    {

    }
    */


    free(file.data); 
}
