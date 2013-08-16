#include "SkFontMgr.h"

extern SkFontMgr* SkFontMgr_New_DirectWrite();

SkFontMgr* SkFontMgr::Factory() {
    return SkFontMgr_New_DirectWrite();
}
