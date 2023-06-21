`SkCanvas::flush()` has been removed. It can be replaced with:
```
    if (auto dContext = GrAsDirectContext(canvas->recordingContext())) {
        dContext->flushAndSubmit();
    }
```

`SkCanvas::recordingContext()` and `SkCanvas::recorder()` are now const. They were implicitly const
but are now declared to be such.