The Vulkan backend for both Ganesh and Graphite will now invoke an optional client-provided callback
function when a `VK_ERROR_DEVICE_LOST` error code is returned from the Vulkan driver. Additional
debugging information will be passed from the driver to this callback if the `VK_EXT_device_fault`
extension is supported and enabled.

This optional callback can be be provided via the `fDeviceLostContext` and `fDeviceLostProc` fields
on `GrVkBackendContext` (Ganesh) and `VulkanBackendContext` (Graphite).
