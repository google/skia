The following symbols (and their files) have been deleted in favor of their
GPU-backend-agnostic form:
 - `GrVkBackendContext` -> `skgpu::VulkanBackendContext`
 - `GrVkExtensions` -> `skgpu::VulkanExtensions`
 - `GrVkMemoryAllocator` = `skgpu::VulkanMemoryAllocator`
 - `GrVkBackendMemory` = `skgpu::VulkanBackendMemory`
 - `GrVkAlloc` = `skgpu::VulkanAlloc`
 - `GrVkYcbcrConversionInfo` = `skgpu::VulkanYcbcrConversionInfo`
 - `GrVkGetProc` = `skgpu::VulkanGetProc`