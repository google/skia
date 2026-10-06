
Skia no longer provides default SkExecutor implementations and the corresponding factories have been removed from SkExecutor.h.

Clients wishing to accelerate their applications with threading should implement their own SkExecutor-derived classes.
