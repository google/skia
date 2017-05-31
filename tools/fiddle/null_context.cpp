#include "fiddle_main.h"

// create_grcontext for when neither Mesa nor EGL are available.
sk_sp<GrContext> create_grcontext() {
    return nullptr;
}
