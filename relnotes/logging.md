Priority-based log filtering is now supported at a core Skia level. This update includes moving
LoggingPriority out of Graphite and into core Skia. SKGPU_GRAPHITE_LOWEST_ACTIVE_LOG_PRIORITY is
still supported for backwards compatibility, but new users are encouraged to use
SKIA_LOWEST_ACTIVE_LOG_PRIORITY in their config file.