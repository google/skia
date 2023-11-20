`GrBackendSemaphore::initVk` and `GrBackendSemaphore::vkSemaphore` have been replaced with
`GrBackendSemaphores::MakeVk` and `GrBackendSemaphores::GetVkSemaphore`, defined in
`include/gpu/ganesh/vk/GrVkBackendSemaphore.h`