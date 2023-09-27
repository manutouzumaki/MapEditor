#ifndef _DARRAY_H__
#define _DARRAY_H__

#define DarrayPush(array, value, type)                             \
    do {                                                             \
        (array) = (type *)DarrayCreate_((array), sizeof(type), 1); \
        (array)[DarraySize(array) - 1] = (value);                  \
    }while(0);

#define DarrayDestroy(array) DarrayDestroy_((void *)(array))
#define DarraySize(array) DarraySize_((void *)(array))
#define DarrayCapacity(array) DarrayCapacity_((void *)(array))

void *DarrayCreate_(void *array, u32 elementSize, u32 elementCount);
void DarrayDestroy_(void *array);
u32 DarraySize_(void *array);
u32 DarrayCapacity_(void *array);


void DarrayModifySize(void *array, u32 size);

#endif
