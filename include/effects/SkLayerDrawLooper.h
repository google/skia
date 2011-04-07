#ifndef SkLayerDrawLooper_DEFINED
#define SkLayerDrawLooper_DEFINED

#include "SkDrawLooper.h"

struct SkPoint;

class SkLayerDrawLooper : public SkDrawLooper {
public:
            SkLayerDrawLooper();
    virtual ~SkLayerDrawLooper();
    
    /** Call for each layer you want to add (from top to bottom).
        This returns a paint you can modify, but that ptr is only valid until
        the next call made to this object.
     */
    SkPaint* addLayer(SkScalar dx, SkScalar dy);
    
    /** Helper for addLayer() which passes (0, 0) for the offset parameters
     */
    SkPaint* addLayer() {
        return this->addLayer(0, 0);
    }
    
    // overrides from SkDrawLooper
    virtual void init(SkCanvas*);
    virtual bool next(SkCanvas*, SkPaint* paint);

    // must be public for Registrar :(
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkLayerDrawLooper, (buffer));
    }
    
protected:
    SkLayerDrawLooper(SkFlattenableReadBuffer&);

    // overrides from SkFlattenable
    virtual void flatten(SkFlattenableWriteBuffer& );
    virtual Factory getFactory() { return CreateProc; }
    
private:
    struct Rec {
        Rec*    fNext;
        SkPaint fPaint;
        SkPoint fOffset;
        
        static Rec* Reverse(Rec*);
    };
    Rec*    fRecs;
    int     fCount;

    // state-machine during the init/next cycle
    Rec* fCurrRec;
    
    class MyRegistrar : public SkFlattenable::Registrar {
    public:
        MyRegistrar();
    };
    
    typedef SkDrawLooper INHERITED;
};


#endif
