`GrDirectContext::MakeMetal` has been moved to `GrDirectContexts::MakeMetal`, located in
`include/gpu/ganesh/mtl/GrMtlDirectContext.h`. The APIs that passed in void* have been removed
in that change, so clients who use those need to create a `GrMtlBackendContext` themselves.

`include/gpu/mtl/GrMtlTypes.h` and `include/gpu/mtl/GrMtlBackendContext.h` have been relocated to
`include/gpu/ganesh/mtl/GrMtlTypes.h` and `include/gpu/ganesh/mtl/GrMtlBackendContext.h`
respectively.