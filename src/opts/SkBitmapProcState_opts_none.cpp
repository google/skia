#include "SkBitmapProcState.h"

/*  A platform may optionally overwrite any of these with accelerated
    versions. On input, these will already have valid function pointers,
    so a platform need only overwrite the ones it chooses, based on the
    current state (e.g. fBitmap, fInvMatrix, etc.)

    fShaderProc32
    fShaderProc16
    fMatrixProc
    fSampleProc32
    fSampleProc32
 */

// empty implementation just uses default supplied function pointers
void SkBitmapProcState::platformProcs() {}


