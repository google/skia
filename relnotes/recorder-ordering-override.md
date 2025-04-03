`RecorderOptions.fRequireOrderedRecordings` can now be used to specify a per-`Recorder` ordering
policy for how its `Recordings` must be inserted into a `Context`. If not provided, the `Recorder`
will default to the value in `ContextOptions`.
