Graphite's logging priority can now be adjusted by defining
`SKGPU_GRAPHITE_LOWEST_ACTIVE_LOG_PRIORITY` in `SkUserConfig.h` to a value specified by the
`skgpu::graphite::LogPriority` enum.

For example:
```
#define SKGPU_GRAPHITE_LOWEST_ACTIVE_LOG_PRIORITY skgpu::graphite::LogPriority::kWarning
```

Would cause Graphite to log warnings, non-fatal errors, and fatal errors. However, debug logs would
be omitted.

`SKGPU_GRAPHITE_LOWEST_ACTIVE_LOG_PRIORITY` will default to `kWarning` in debug builds, and `kError`
in release builds.
