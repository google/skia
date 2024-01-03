`SkCanvas::onDrawImage` is no longer called by SkCanvas. The public `drawImage` call now directs to
`drawImageRect` with the source rect equal to the full image. `SkCanvas::onDrawImage` should no
longer be overridden and it will be deleted at a future date.
