Added the optional `SK_ASSUME_SINGLE_GANESH_CONTEXT` build define. Clients that guarantee at
most one `GrDirectContext` is in use per process may define it to skip the spinlock that protects
Ganesh's global `GrProcessor` memory pool. This replaces the previous behavior where the lock
was implicitly disabled by `SK_BUILD_FOR_ANDROID_FRAMEWORK`.
