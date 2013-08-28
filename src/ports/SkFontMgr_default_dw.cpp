#include "SkFontMgr.h"
#include "SkTypeface_win.h"

SkFontMgr* SkFontMgr::Factory() {
    return SkFontMgr_New_DirectWrite();
}
