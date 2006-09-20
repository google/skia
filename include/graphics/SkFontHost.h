#ifndef SkFontHost_DEFINED
#define SkFontHost_DEFINED

#include "SkScalerContext.h"
#include "SkTypeface.h"

class SkDescriptor;

/** \class SkFontHost

	This class is ported to each environment. It is responsible for bridging the gap
    between SkTypeface and the resulting platform-specific instance of SkScalerContext.
*/
class SkFontHost {
public:
	/**	Return a subclass of SkTypeface, one that can be used by your scalaracontext
		(returned by SkFontHost::CreateScalarContext).
        1) If family is nil, use name.
        2) If name is nil, use family.
        3) If both are nil, use default family.
	*/
	static SkTypeface* CreateTypeface(const SkTypeface* family, const char name[], SkTypeface::Style);
	/** Given a typeface (or nil), return the number of bytes needed to flatten it
        into a buffer, for the purpose of communicating information to the
        scalercontext. If buffer is nil, then ignore it but still return the number
        of bytes that would be written.
    */
	static uint32_t FlattenTypeface(const SkTypeface* face, void* buffer);
	/**	Return a subclass of SkScalarContext
	*/
	static SkScalerContext* CreateScalerContext(const SkDescriptor* desc);
    
    enum ScalerContextID {
        kMissing_ScalerContextID = SK_UnknownAuxScalerContextID,
        kMax_ScalerContextID = SK_MaxAuxScalerContextID
    };
    static ScalerContextID FindScalerContextIDForUnichar(int32_t unichar);

    static SkScalerContext* CreateScalerContextFromID(ScalerContextID, const SkScalerContext::Rec&);
};

#endif

