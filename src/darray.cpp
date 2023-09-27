

#define DARRAY_RAW_DATA(array) ((u32 *)(array) - 2)
#define DARRAY_GET_CAPACITY(array) (DARRAY_RAW_DATA(array)[0])
#define DARRAY_GET_SIZE(array) (DARRAY_RAW_DATA(array)[1])


void *DarrayCreate_(void *array, u32 elementSize, u32 elementCount) {
    if(array == NULL) {
        u32 rawSize = (sizeof(u32) * 2) + (elementSize * elementCount);
        u32 *base = (u32 *)malloc(rawSize);
        base[0] = elementCount;
        base[1] = elementCount;
        return (void *)(base + 2);

    }
    else if((DARRAY_GET_SIZE(array) + elementCount) <= DARRAY_GET_CAPACITY(array)) {
        DARRAY_GET_SIZE(array) += elementCount;
        return array;
    }
    else {
        u32 neededSize = DARRAY_GET_SIZE(array) + elementCount;
        u32 newCapacity = DARRAY_GET_CAPACITY(array) * 2;
        u32 capacity = neededSize > newCapacity ? neededSize : newCapacity;
        u32 size = neededSize;
        u32 rawSize = (sizeof(u32) * 2) + (elementSize * capacity);
        u32 *base = (u32 *)realloc(DARRAY_RAW_DATA(array), rawSize);
        base[0] = capacity;
        base[1] = size;
        return (void *)(base + 2);

    }
}

void DarrayModifySize(void *array, u32 size) {
    u32 *base = DARRAY_RAW_DATA(array);
    base[1] = size;
}

void DarrayDestroy_(void *array) {
    ASSERT(array != NULL);
    void *rawData = DARRAY_RAW_DATA(array);
    free(rawData);
}

u32 DarraySize_(void *array) {
    ASSERT(array != NULL);
    return DARRAY_GET_SIZE(array);
    
}

u32 DarrayCapacity_(void *array) {
    ASSERT(array != NULL);
    return DARRAY_GET_CAPACITY(array);
}
