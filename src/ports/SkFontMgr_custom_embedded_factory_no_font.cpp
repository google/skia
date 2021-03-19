#include "include/core/SkFontMgr.h"

struct SkEmbeddedResource { const uint8_t* data; size_t size; };
struct SkEmbeddedResourceHeader { const SkEmbeddedResource* entries; int count; };
sk_sp<SkFontMgr> SkFontMgr_New_Custom_Embedded(const SkEmbeddedResourceHeader* header);

sk_sp<SkFontMgr> SkFontMgr::Factory() {
    return SkFontMgr_New_Custom_Embedded(nullptr);
}
