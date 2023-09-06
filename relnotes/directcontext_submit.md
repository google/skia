`GrDirectContext::submit` and `GrDirectContext::flushAndSubmit` calls now take a GrSyncCpu enum
instead of a error-prone boolean.

Similarly, calls to `GrDirectContext::performDeferredCleanup` and
`GrDirectContext::purgeUnlockedResources` take a GrPurgeResourceOptions enum.