`SkStream::getData()` has been added as a virtual. Subclasses can implement this if it is efficient
to turn the underlying contents into an SkData (e.g. SkStreamMemory). `SkStreamMemory::asData()`
has been renamed to `getData()` as a result of this change and will be removed in a future release.