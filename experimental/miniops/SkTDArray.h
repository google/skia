#ifndef SkTDArray_DEFINED
#define SkTDArray_DEFINED

#include "SkMalloc.h"

template <typename T> class SkTDArray {
public:
    int count() const;
    T*  begin();
    T*  end();
    T* append();
    T* append(int count, const T* src = nullptr);
    void     pop();
    void     pop(T* elem);
    void     push(const T& elem);
    T&       top();
    void reset();
    T&  operator[](int index);
};

#endif
