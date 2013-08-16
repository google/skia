#include "SkFontMgr.h"

extern SkFontMgr* SkFontMgr_New_GDI();

SkFontMgr* SkFontMgr::Factory() {
    return SkFontMgr_New_GDI();
}
