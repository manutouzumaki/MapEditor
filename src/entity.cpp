u32 Entity::entityCount;

Entity *EntityCreate(Vec2 start, Vec2 end, ViewId viewId)
{
    Entity *entity = (Entity *)malloc(sizeof(Entity));
    memset(entity, 0, sizeof(Entity));
    
    // here the order of creation matters
    switch(viewId)
    {
        case VIEW_TOP:
        {
            entity->brushes2D[VIEW_TOP]   = Brush2DCreate(start, end);
            entity->brushes2D[VIEW_FRONT] = Brush2DCreate({start.x, gUnitSize}, {end.x, 0});
            entity->brushes2D[VIEW_SIDE]  = Brush2DCreate({start.y, gUnitSize}, {end.y, 0});
        } break;
        case VIEW_FRONT:
        {
            entity->brushes2D[VIEW_FRONT] = Brush2DCreate(start, end);
            entity->brushes2D[VIEW_SIDE]  = Brush2DCreate({0, end.y}, {gUnitSize, start.y});
            entity->brushes2D[VIEW_TOP]   = Brush2DCreate({start.x, gUnitSize}, {end.x, 0});
        } break;
        case VIEW_SIDE:
        {
            entity->brushes2D[VIEW_SIDE]  = Brush2DCreate(start, end);
            entity->brushes2D[VIEW_FRONT] = Brush2DCreate({0, end.y}, {gUnitSize, start.y});
            entity->brushes2D[VIEW_TOP]   = Brush2DCreate({0, end.x}, {gUnitSize, start.x});
        } break;
    }

    Vec2 xDim, yDim, zDim;
    CalculateDimensionsFromBrushes2D(&entity->brushes2D[VIEW_FRONT], &entity->brushes2D[VIEW_SIDE], xDim, yDim, zDim);

    entity->brushPlane = BrushPlaneCreate(xDim, yDim, zDim);
    entity->brushVert = BrushVertexCreate(&entity->brushPlane);

    entity->key = Entity::entityCount++;

    return entity;
}

Entity *EntityCreateFromBrushPlane(BrushPlane *brushPlane)
{
    Entity *entity = (Entity *)malloc(sizeof(Entity));
    memset(entity, 0, sizeof(Entity)); 

    entity->brushPlane = *brushPlane;
    entity->brushVert = BrushVertexCreate(&entity->brushPlane);
    
    // TODO: temporal hack FIX this
    entity->brushes2D[VIEW_SIDE]  = Brush2DCreate({}, {});
    entity->brushes2D[VIEW_FRONT] = Brush2DCreate({}, {});
    entity->brushes2D[VIEW_TOP]   = Brush2DCreate({}, {});
    Brush2DUpdate(&entity->brushes2D[VIEW_TOP],   &entity->brushVert, &entity->brushPlane, {0, 1, 0});
    Brush2DUpdate(&entity->brushes2D[VIEW_FRONT], &entity->brushVert, &entity->brushPlane, {0, 0, 1});
    Brush2DUpdate(&entity->brushes2D[VIEW_SIDE],  &entity->brushVert, &entity->brushPlane, {1, 0, 0});

    return entity;
}

void EntityUpdate(Entity *entity, ViewId viewId,
                  Vec2 oBotL, Vec2 oBotR, Vec2 oTopL, Vec2 oTopR,
                  Vec2 botL,  Vec2 botR,  Vec2 topL,  Vec2 topR)
{
    BrushVertexUpdate(&entity->brushVert, viewId, oBotL, oBotR, oTopL, oTopR, botL,  botR,  topL,  topR);
    BrushPlaneUpdate(&entity->brushPlane, &entity->brushVert);
    Brush2DUpdate(&entity->brushes2D[VIEW_TOP],   &entity->brushVert, &entity->brushPlane, {0, 1, 0});
    Brush2DUpdate(&entity->brushes2D[VIEW_FRONT], &entity->brushVert, &entity->brushPlane, {0, 0, 1});
    Brush2DUpdate(&entity->brushes2D[VIEW_SIDE],  &entity->brushVert, &entity->brushPlane, {1, 0, 0});
}

void EntityClip(Entity *entity, ViewId viewId, Plane clipPlane)
{
    BrushPlaneClip(&entity->brushPlane, &entity->brushVert, clipPlane, viewId);
    BrushVertexDestroy(&entity->brushVert);
    entity->brushVert = BrushVertexCreate(&entity->brushPlane);
    Brush2DUpdate(&entity->brushes2D[VIEW_TOP],   &entity->brushVert, &entity->brushPlane, {0, 1, 0});
    Brush2DUpdate(&entity->brushes2D[VIEW_FRONT], &entity->brushVert, &entity->brushPlane, {0, 0, 1});
    Brush2DUpdate(&entity->brushes2D[VIEW_SIDE],  &entity->brushVert, &entity->brushPlane, {1, 0, 0});
}

void EntityDestroy(Entity *entity)
{
    BrushVertexDestroy(&entity->brushVert);
    BrushPlaneDestroy(&entity->brushPlane);
    Brush2DDestroy(&entity->brushes2D[VIEW_TOP]);
    Brush2DDestroy(&entity->brushes2D[VIEW_FRONT]);
    Brush2DDestroy(&entity->brushes2D[VIEW_SIDE]);
    free(entity);
}

Entity *EntityAdd(Entity *front, Vec2 start, Vec2 end, ViewId viewId)
{
    if(front != nullptr)
    {
        Entity *newEntity = EntityCreate(start, end, viewId);
        newEntity->prev = nullptr;
        newEntity->next = front;
        front->prev = newEntity;
        return newEntity;
    }
    else
    {
        Entity *newEntity = EntityCreate(start, end, viewId);
        newEntity->prev = nullptr;
        newEntity->next = nullptr;
        return newEntity;
    }
}

Entity *EntityAdd(Entity *front, Entity *newEntity)
{
    if(front != nullptr)
    {
        newEntity->prev = nullptr;
        newEntity->next = front;
        front->prev = newEntity;
        return newEntity;
    }
    else
    {
        newEntity->prev = nullptr;
        newEntity->next = nullptr;
        return newEntity;
    }
}

void EntityRemove(Entity *entity)
{
    if(gEntityList == entity)
    {
        gEntityList = entity->next;
    }

    if(entity->prev != nullptr)
        entity->prev->next = entity->next;
    if(entity->next != nullptr)
        entity->next->prev = entity->prev;
    EntityDestroy(entity);
}
