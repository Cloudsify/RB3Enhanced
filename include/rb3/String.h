#ifndef _STRING_H
#define _STRING_H

typedef struct _String String;

// deleteFlag 1 also frees the String itself, 0 only frees the text buffer it owns
typedef void (*StringDestructor_t)(String *thisString, int deleteFlag);
typedef void (*StringPrint_t)(String *thisString, const char *text);

typedef struct _String_vtable
{
    // Wii-specific RTTI stuff
#ifdef RB3E_WII
    int *pointer;
    int unk;
#endif

    StringDestructor_t destructor;
    StringPrint_t print;
} String_vtable;

struct _String
{
    String_vtable *vtable;
    int length;
    char *buf;
};

int StringConstructor(int *thisFactory, String *str);

#endif // _STRING_H
