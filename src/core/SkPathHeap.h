#ifndef SkPathHeap_DEFINED
#define SkPathHeap_DEFINED

#include "SkRefCnt.h"
#include "SkChunkAlloc.h"
#include "SkTDArray.h"

class SkPath;
class SkFlattenableReadBuffer;
class SkFlattenableWriteBuffer;

class SkPathHeap : public SkRefCnt {
public:
            SkPathHeap();
            SkPathHeap(SkFlattenableReadBuffer&);
    virtual ~SkPathHeap();

    // called during picture-record
    int append(const SkPath&);
    
    // called during picture-playback
    int count() const { return fPaths.count(); }
    const SkPath& operator[](int index) const {
        return *fPaths[index];
    }
    
    void flatten(SkFlattenableWriteBuffer&) const;
        
private:
    // we store the paths in the heap (placement new)
    SkChunkAlloc        fHeap;
    // we just store ptrs into fHeap here
    SkTDArray<SkPath*>  fPaths;
};

#endif

