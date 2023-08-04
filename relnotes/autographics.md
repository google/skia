`SkAutoGraphics` was removed. This was a helper struct that simply called `SkGraphics::Init`.
Any instance of `SkAutoGraphics` can be replaced with a call to `SkGraphics::Init`.
