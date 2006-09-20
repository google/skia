#ifndef SkRasterizer_DEFINED
#define SkRasterizer_DEFINED

#include "SkFlattenable.h"
#include "SkMask.h"

class SkMaskFilter;
class SkMatrix;
class SkPath;
struct SkRect16;

class SkRasterizer : public SkFlattenable {
public:
    SkRasterizer() {}

    /** Turn the path into a mask, respecting the specified local->device matrix.
    */
    bool rasterize(const SkPath& path, const SkMatrix& matrix,
                   const SkRect16* clipBounds, SkMaskFilter* filter,
                   SkMask* mask, SkMask::CreateMode mode);

protected:
    SkRasterizer(SkRBuffer&);

    virtual bool onRasterize(const SkPath& path, const SkMatrix& matrix,
                             const SkRect16* clipBounds,
                             SkMask* mask, SkMask::CreateMode mode);

private:
    typedef SkFlattenable INHERITED;
};

#endif
