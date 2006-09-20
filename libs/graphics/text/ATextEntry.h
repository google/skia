#ifndef ATextEntry_DEFINED
#define ATextEntry_DEFINED

#include "SkTypes.h"

class SkCanavs;

class ATextEntry {
public:
    ATextEntry();
    ~ATextEntry();
    
    void    setUtf16(const U16 text[], size_t count);
    void    setSize(SkScalar width, SkScalar height);
    void    setSelection(int start, int stop);
    void    draw(SkCanvas*);
    void    handleKey(int key);
};

#endif
