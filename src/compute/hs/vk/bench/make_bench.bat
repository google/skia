@echo off

::
::
::

set SRC_C=^
../../../common/util.c ^
../../../common/vk/assert_vk.c ^
../../../common/vk/host_alloc.c ^
../../../common/vk/cache_vk.c ^
../hs_vk.c ^
../intel/gen8/u32/hs_intel_gen8_u32.c ^
../intel/gen8/u64/hs_intel_gen8_u64.c ^
../nvidia/sm_35/u32/hs_nvidia_sm35_u32.c ^
../nvidia/sm_35/u64/hs_nvidia_sm35_u64.c ^
../amd/gcn/u32/hs_amd_gcn_u32.c ^
../amd/gcn/u64/hs_amd_gcn_u64.c ^
main.c

set SRC_CPP=sort.cpp

::
:: /DHS_VK_VERBOSE_DISASSEMBLY_AMD ^
::

:: SET AMD_OPTS= /DHS_VK_VERBOSE_STATISTICS_AMD /DHS_VK_VERBOSE_DISASSEMBLY_AMD
SET AMD_OPTS= /DHS_VK_VERBOSE_STATISTICS_AMD 

cl ^
/Fe:hs_bench_vk ^
/O2 ^
/DNDEBUG ^
%AMD_OPTS% ^
/std:c++latest /Zc:__cplusplus /EHs ^
/I../../.. /I.. /I%VULKAN_SDK%/include ^
%VULKAN_SDK%/lib/vulkan-1.lib ^
%SRC_C% %SRC_CPP%

::
::
::

del *.obj

::
::
::
