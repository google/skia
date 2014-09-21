#ifndef SkDeviceProperties_DEFINED
#define SkDeviceProperties_DEFINED

//TODO: get everyone to stop using SkFontLCDConfig::SetSubpixel* and remove this import.
#include "SkFontLCDConfig.h"

struct SkDeviceProperties {
    struct Geometry {
        /** The orientation of the pixel specifies the interpretation of the
        *  layout. If the orientation is horizontal, the layout is interpreted as
        *  left to right. It the orientation is vertical, the layout is
        *  interpreted top to bottom (rotated 90deg cw from horizontal).
        */
        enum Orientation {
            kUnknown_Orientation      = 0x0,
            kKnown_Orientation        = 0x2,

            kHorizontal_Orientation   = 0x2,  //!< this is the default
            kVertical_Orientation     = 0x3,

            kOrientationMask          = 0x3,
        };

        /** The layout of the pixel specifies its subpixel geometry.
        *
        *  kUnknown_Layout means that the subpixel elements are not spatially
        *  separated in any known or usable fashion.
        */
        enum Layout {
            kUnknown_Layout   = 0x0,
            kKnown_Layout     = 0x8,

            kRGB_Layout       = 0x8,  //!< this is the default
            kBGR_Layout       = 0xC,

            kLayoutMask       = 0xC,
        };

        Orientation getOrientation() {
            return static_cast<Orientation>(fGeometry & kOrientationMask);
        }
        Layout getLayout() {
            return static_cast<Layout>(fGeometry & kLayoutMask);
        }

        bool isOrientationKnown() {
            return SkToBool(fGeometry & kKnown_Orientation);
        }
        bool isLayoutKnown() {
            return SkToBool(fGeometry & kKnown_Layout);
        }

    private:
        //TODO: get everyone to stop using SkFontLCDConfig::SetSubpixel* and replace these calls with constants.
        static Orientation fromOldOrientation(SkFontLCDConfig::LCDOrientation orientation) {
            switch (orientation) {
            case SkFontLCDConfig::kHorizontal_LCDOrientation: return kHorizontal_Orientation;
            case SkFontLCDConfig::kVertical_LCDOrientation: return kVertical_Orientation;
            default: return kUnknown_Orientation;
            }
        }
        static Layout fromOldLayout(SkFontLCDConfig::LCDOrder order) {
            switch (order) {
            case SkFontLCDConfig::kRGB_LCDOrder: return kRGB_Layout;
            case SkFontLCDConfig::kBGR_LCDOrder: return kBGR_Layout;
            default: return kUnknown_Layout;
            }
        }
    public:
        static Geometry MakeDefault() {
            Orientation orientation = fromOldOrientation(SkFontLCDConfig::GetSubpixelOrientation()); //kHorizontal_Orientation
            Layout layout = fromOldLayout(SkFontLCDConfig::GetSubpixelOrder()); //kRGB_Layout
            Geometry ret = { SkToU8(orientation | layout) };
            return ret;
        }

        static Geometry Make(Orientation orientation, Layout layout) {
            Geometry ret = { SkToU8(orientation | layout) };
            return ret;
        }

        uint8_t fGeometry;
    };

    static SkDeviceProperties MakeDefault() {
        SkDeviceProperties ret = { Geometry::MakeDefault(), SK_GAMMA_EXPONENT };
        return ret;
    }

    static SkDeviceProperties Make(Geometry geometry, SkScalar gamma) {
        SkDeviceProperties ret = { geometry, gamma };
        return ret;
    }

    /** Each pixel of an image will have some number of channels.
     *  Can the layout of those channels be exploited? */
    Geometry fGeometry;

    /** Represents the color space of the image. This is a woefully inadequate beginning. */
    SkScalar fGamma;
};

#endif
