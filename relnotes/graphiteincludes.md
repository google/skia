Graphite's backend specific headers are being renamed to be more consistent between backends:
   * DawnTypes.h -> DawnGraphiteTypes.h
   * DawnUtils.h's content moved to DawnBackendContext.h
   * MtlGraphiteTypesUtils.h -> DwnGraphiteTypes_cpp.h (the non-Obj-C portion of
     MtlGraphiteTypes.h).
   * MtlGraphiteUtils.h's content moved to MtlBackendContext.h
   * VulkanGraphiteUtils.h -> VulkanGraphiteContext.h (there is a shared
     VulkanBackendContext.h header for both Ganesh and Graphite already).

The deprecated headers now just forward to the new header names and will be removed in a future
release.
