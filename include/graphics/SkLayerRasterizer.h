#ifndef SkLayerRasterizer_DEFINED
#define SkLayerRasterizer_DEFINED

#include "SkRasterizer.h"
#include "SkDeque.h"
#include "SkScalar.h"

class SkPaint;

class SkLayerRasterizer : public SkRasterizer {
public:
            SkLayerRasterizer();
    virtual ~SkLayerRasterizer();
    
    void addLayer(const SkPaint& paint)
    {
        this->addLayer(paint, 0, 0);
    }
    void addLayer(const SkPaint& paint, SkScalar dx, SkScalar dy);

    // overrides from SkFlattenable
	virtual Factory	getFactory();
	virtual void	flatten(SkWBuffer&);

protected:
    SkLayerRasterizer(SkRBuffer&);

    // override from SkRasterizer
    virtual bool onRasterize(const SkPath& path, const SkMatrix& matrix,
                             const SkRect16* clipBounds,
                             SkMask* mask, SkMask::CreateMode mode);

private:
    SkDeque fLayers;
    
    static SkFlattenable* CreateProc(SkRBuffer&);

    typedef SkRasterizer INHERITED;
};

#endif
