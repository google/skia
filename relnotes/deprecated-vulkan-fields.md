The following deprecated fields have been removed from `GrVkBackendContext`:
 - `fMinAPIVersion`. Use `fMaxAPIVersion` instead.
 - `fInstanceVersion`. Use `fMaxAPIVersion` instead.
 - `fFeatures`. Use `fDeviceFeatures` or `fDeviceFeatures2` instead.
 - `fOwnsInstanceAndDevice`. No replacement, as it had no effect.

`GrVkBackendContext` is now an alias for `skgpu::VulkanBackendContext`. Clients should use the latter, as the former will be eventually removed.
