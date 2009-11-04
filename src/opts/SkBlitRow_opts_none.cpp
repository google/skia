#include "SkBlitRow.h"

// Platform impl of Platform_procs with no overrides

const SkBlitRow::Proc SkBlitRow::PlatformProcs4444(unsigned flags) {
    return NULL;
}

const SkBlitRow::Proc SkBlitRow::PlatformProcs565(unsigned flags) {
    return NULL;
}

const SkBlitRow::Proc32 SkBlitRow::PlatformProcs32(unsigned flags) {
    return NULL;
}
