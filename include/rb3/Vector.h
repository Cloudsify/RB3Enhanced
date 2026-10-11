#ifndef _VECTOR_H
#define _VECTOR_H

// basic vector type
// used constantly across the code
// in C++ it would be used like vector<GameGem> or etc.
typedef struct _vector
{
    // structure of a vector differs on 360 vs Wii
#ifdef RB3E_XBOX
    void *begin;
    void *end;
    void *capacity;
#else
    void *data;
    short count;
    short capacity;
#endif
} vector;

// quick macro to get the first element and the number of elements of a vector without needing platform-specific ifdefs
#ifdef RB3E_XBOX
#define VECTOR_BEGIN(v) ((v).begin)
#define VECTOR_SIZE(v, elementSize) ((int)(((char *)(v).end - (char *)(v).begin) / (elementSize)))
#else
#define VECTOR_BEGIN(v) ((v).data)
#define VECTOR_SIZE(v, elementSize) ((int)(v).count)
#endif

void vector_push_back(vector *v, void *item);

#endif // _VECTOR_H