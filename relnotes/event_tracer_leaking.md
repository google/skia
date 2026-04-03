* The `leakTracer` argument to `SkEventTracer::SetInstance` is removed and now behaves as if
  `leakTracer=true`. Previously, with `leakTracer=false` the `SkEventTracer` would be deleted in an
  `atexit` handler, but this could lead to race conditions in a multithreaded application during
  process exiting. Leaking the object at exit is effectively the same behavior but avoids the races.
