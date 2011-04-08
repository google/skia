#ifndef SkLayerDrawLooper_DEFINED
#define SkLayerDrawLooper_DEFINED

#include "SkDrawLooper.h"

struct SkPoint;

class SkLayerDrawLooper : public SkDrawLooper {
public:
            SkLayerDrawLooper();
    virtual ~SkLayerDrawLooper();

    enum Bits {
        kAlpha_Bit      = 1 << 0,   //!< use this layer's alpha
        kColor_Bit      = 1 << 1,   //!< use this layer's color
        kStyle_Bit      = 1 << 2,   //!< use this layer's Style/stroke settings
        kTextSkewX_Bit  = 1 << 3,   //!< use this layer's textskewx
        kPathEffect_Bit = 1 << 4,   //!< use this layer's patheffect
        kMaskFilter_Bit = 1 << 5,   //!< use this layer's maskfilter
        kShader_Bit     = 1 << 6,   //!< use this layer's shader
        kColorFilter_Bit = 1 << 7,  //!< use this layer's colorfilter
        kXfermode_Bit   = 1 << 8,   //!< use this layer's xfermode
        
        kEntirePaint_Bits = -1,      //!< use this layer's paint entirely
    };
    typedef int32_t BitFlags;
    
    /**
     *  Call for each layer you want to add (from top to bottom).
     *  This returns a paint you can modify, but that ptr is only valid until
     *  the next call made to this object.
     *
     *  The optional bits parameter specifies which aspects of this paint
     *  should replace the paint that is passed to the draw call. If 0 is
     *  specified, then this layer's paint will be ignored.
     */
    SkPaint* addLayer(SkScalar dx, SkScalar dy, BitFlags = kEntirePaint_Bits);
    
    /**
     *  Helper for addLayer() which passes (0, 0) for the offset parameters
     */
    SkPaint* addLayer(BitFlags bits = kEntirePaint_Bits) {
        return this->addLayer(0, 0, bits);
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
        uint32_t fBits;
        
        static Rec* Reverse(Rec*);
    };
    Rec*    fRecs;
    int     fCount;

    // state-machine during the init/next cycle
    Rec* fCurrRec;

    static void ApplyBits(SkPaint* dst, const SkPaint& src, BitFlags bits);

    class MyRegistrar : public SkFlattenable::Registrar {
    public:
        MyRegistrar();
    };
    
    typedef SkDrawLooper INHERITED;
};


#endif
