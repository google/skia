static const spv_operand_desc_t pygen_variable_ImageOperandsEntries[] = {
  {"None", 0x0000, {}, {}, {}},
  {"Bias", 0x0001, {SpvCapabilityShader}, {}, {SPV_OPERAND_TYPE_ID}},
  {"Lod", 0x0002, {}, {}, {SPV_OPERAND_TYPE_ID}},
  {"Grad", 0x0004, {}, {}, {SPV_OPERAND_TYPE_ID, SPV_OPERAND_TYPE_ID}},
  {"ConstOffset", 0x0008, {}, {}, {SPV_OPERAND_TYPE_ID}},
  {"Offset", 0x0010, {SpvCapabilityImageGatherExtended}, {}, {SPV_OPERAND_TYPE_ID}},
  {"ConstOffsets", 0x0020, {}, {}, {SPV_OPERAND_TYPE_ID}},
  {"Sample", 0x0040, {}, {}, {SPV_OPERAND_TYPE_ID}},
  {"MinLod", 0x0080, {SpvCapabilityMinLod}, {}, {SPV_OPERAND_TYPE_ID}}
};

static const spv_operand_desc_t pygen_variable_FPFastMathModeEntries[] = {
  {"None", 0x0000, {}, {}, {}},
  {"NotNaN", 0x0001, {SpvCapabilityKernel}, {}, {}},
  {"NotInf", 0x0002, {SpvCapabilityKernel}, {}, {}},
  {"NSZ", 0x0004, {SpvCapabilityKernel}, {}, {}},
  {"AllowRecip", 0x0008, {SpvCapabilityKernel}, {}, {}},
  {"Fast", 0x0010, {SpvCapabilityKernel}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_SelectionControlEntries[] = {
  {"None", 0x0000, {}, {}, {}},
  {"Flatten", 0x0001, {}, {}, {}},
  {"DontFlatten", 0x0002, {}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_LoopControlEntries[] = {
  {"None", 0x0000, {}, {}, {}},
  {"Unroll", 0x0001, {}, {}, {}},
  {"DontUnroll", 0x0002, {}, {}, {}},
  {"DependencyInfinite", 0x0004, {}, {}, {}},
  {"DependencyLength", 0x0008, {}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}}
};

static const spv_operand_desc_t pygen_variable_FunctionControlEntries[] = {
  {"None", 0x0000, {}, {}, {}},
  {"Inline", 0x0001, {}, {}, {}},
  {"DontInline", 0x0002, {}, {}, {}},
  {"Pure", 0x0004, {}, {}, {}},
  {"Const", 0x0008, {}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_MemorySemanticsEntries[] = {
  {"Relaxed", 0x0000, {}, {}, {}},
  {"None", 0x0000, {}, {}, {}},
  {"Acquire", 0x0002, {}, {}, {}},
  {"Release", 0x0004, {}, {}, {}},
  {"AcquireRelease", 0x0008, {}, {}, {}},
  {"SequentiallyConsistent", 0x0010, {}, {}, {}},
  {"UniformMemory", 0x0040, {SpvCapabilityShader}, {}, {}},
  {"SubgroupMemory", 0x0080, {}, {}, {}},
  {"WorkgroupMemory", 0x0100, {}, {}, {}},
  {"CrossWorkgroupMemory", 0x0200, {}, {}, {}},
  {"AtomicCounterMemory", 0x0400, {SpvCapabilityAtomicStorage}, {}, {}},
  {"ImageMemory", 0x0800, {}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_MemoryAccessEntries[] = {
  {"None", 0x0000, {}, {}, {}},
  {"Volatile", 0x0001, {}, {}, {}},
  {"Aligned", 0x0002, {}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"Nontemporal", 0x0004, {}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_KernelProfilingInfoEntries[] = {
  {"None", 0x0000, {}, {}, {}},
  {"CmdExecTime", 0x0001, {SpvCapabilityKernel}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_SourceLanguageEntries[] = {
  {"Unknown", 0, {}, {}, {}},
  {"ESSL", 1, {}, {}, {}},
  {"GLSL", 2, {}, {}, {}},
  {"OpenCL_C", 3, {}, {}, {}},
  {"OpenCL_CPP", 4, {}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_ExecutionModelEntries[] = {
  {"Vertex", 0, {SpvCapabilityShader}, {}, {}},
  {"TessellationControl", 1, {SpvCapabilityTessellation}, {}, {}},
  {"TessellationEvaluation", 2, {SpvCapabilityTessellation}, {}, {}},
  {"Geometry", 3, {SpvCapabilityGeometry}, {}, {}},
  {"Fragment", 4, {SpvCapabilityShader}, {}, {}},
  {"GLCompute", 5, {SpvCapabilityShader}, {}, {}},
  {"Kernel", 6, {SpvCapabilityKernel}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_AddressingModelEntries[] = {
  {"Logical", 0, {}, {}, {}},
  {"Physical32", 1, {SpvCapabilityAddresses}, {}, {}},
  {"Physical64", 2, {SpvCapabilityAddresses}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_MemoryModelEntries[] = {
  {"Simple", 0, {SpvCapabilityShader}, {}, {}},
  {"GLSL450", 1, {SpvCapabilityShader}, {}, {}},
  {"OpenCL", 2, {SpvCapabilityKernel}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_ExecutionModeEntries[] = {
  {"Invocations", 0, {SpvCapabilityGeometry}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"SpacingEqual", 1, {SpvCapabilityTessellation}, {}, {}},
  {"SpacingFractionalEven", 2, {SpvCapabilityTessellation}, {}, {}},
  {"SpacingFractionalOdd", 3, {SpvCapabilityTessellation}, {}, {}},
  {"VertexOrderCw", 4, {SpvCapabilityTessellation}, {}, {}},
  {"VertexOrderCcw", 5, {SpvCapabilityTessellation}, {}, {}},
  {"PixelCenterInteger", 6, {SpvCapabilityShader}, {}, {}},
  {"OriginUpperLeft", 7, {SpvCapabilityShader}, {}, {}},
  {"OriginLowerLeft", 8, {SpvCapabilityShader}, {}, {}},
  {"EarlyFragmentTests", 9, {SpvCapabilityShader}, {}, {}},
  {"PointMode", 10, {SpvCapabilityTessellation}, {}, {}},
  {"Xfb", 11, {SpvCapabilityTransformFeedback}, {}, {}},
  {"DepthReplacing", 12, {SpvCapabilityShader}, {}, {}},
  {"DepthGreater", 14, {SpvCapabilityShader}, {}, {}},
  {"DepthLess", 15, {SpvCapabilityShader}, {}, {}},
  {"DepthUnchanged", 16, {SpvCapabilityShader}, {}, {}},
  {"LocalSize", 17, {}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER, SPV_OPERAND_TYPE_LITERAL_INTEGER, SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"LocalSizeHint", 18, {SpvCapabilityKernel}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER, SPV_OPERAND_TYPE_LITERAL_INTEGER, SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"InputPoints", 19, {SpvCapabilityGeometry}, {}, {}},
  {"InputLines", 20, {SpvCapabilityGeometry}, {}, {}},
  {"InputLinesAdjacency", 21, {SpvCapabilityGeometry}, {}, {}},
  {"Triangles", 22, {SpvCapabilityGeometry, SpvCapabilityTessellation}, {}, {}},
  {"InputTrianglesAdjacency", 23, {SpvCapabilityGeometry}, {}, {}},
  {"Quads", 24, {SpvCapabilityTessellation}, {}, {}},
  {"Isolines", 25, {SpvCapabilityTessellation}, {}, {}},
  {"OutputVertices", 26, {SpvCapabilityGeometry, SpvCapabilityTessellation}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"OutputPoints", 27, {SpvCapabilityGeometry}, {}, {}},
  {"OutputLineStrip", 28, {SpvCapabilityGeometry}, {}, {}},
  {"OutputTriangleStrip", 29, {SpvCapabilityGeometry}, {}, {}},
  {"VecTypeHint", 30, {SpvCapabilityKernel}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"ContractionOff", 31, {SpvCapabilityKernel}, {}, {}},
  {"Initializer", 33, {SpvCapabilityKernel}, {}, {}},
  {"Finalizer", 34, {SpvCapabilityKernel}, {}, {}},
  {"SubgroupSize", 35, {SpvCapabilitySubgroupDispatch}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"SubgroupsPerWorkgroup", 36, {SpvCapabilitySubgroupDispatch}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}}
};

static const spv_operand_desc_t pygen_variable_StorageClassEntries[] = {
  {"UniformConstant", 0, {}, {}, {}},
  {"Input", 1, {}, {}, {}},
  {"Uniform", 2, {SpvCapabilityShader}, {}, {}},
  {"Output", 3, {SpvCapabilityShader}, {}, {}},
  {"Workgroup", 4, {}, {}, {}},
  {"CrossWorkgroup", 5, {}, {}, {}},
  {"Private", 6, {SpvCapabilityShader}, {}, {}},
  {"Function", 7, {}, {}, {}},
  {"Generic", 8, {SpvCapabilityGenericPointer}, {}, {}},
  {"PushConstant", 9, {SpvCapabilityShader}, {}, {}},
  {"AtomicCounter", 10, {SpvCapabilityAtomicStorage}, {}, {}},
  {"Image", 11, {}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_DimEntries[] = {
  {"1D", 0, {SpvCapabilitySampled1D}, {}, {}},
  {"2D", 1, {}, {}, {}},
  {"3D", 2, {}, {}, {}},
  {"Cube", 3, {SpvCapabilityShader}, {}, {}},
  {"Rect", 4, {SpvCapabilitySampledRect}, {}, {}},
  {"Buffer", 5, {SpvCapabilitySampledBuffer}, {}, {}},
  {"SubpassData", 6, {SpvCapabilityInputAttachment}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_SamplerAddressingModeEntries[] = {
  {"None", 0, {SpvCapabilityKernel}, {}, {}},
  {"ClampToEdge", 1, {SpvCapabilityKernel}, {}, {}},
  {"Clamp", 2, {SpvCapabilityKernel}, {}, {}},
  {"Repeat", 3, {SpvCapabilityKernel}, {}, {}},
  {"RepeatMirrored", 4, {SpvCapabilityKernel}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_SamplerFilterModeEntries[] = {
  {"Nearest", 0, {SpvCapabilityKernel}, {}, {}},
  {"Linear", 1, {SpvCapabilityKernel}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_ImageFormatEntries[] = {
  {"Unknown", 0, {}, {}, {}},
  {"Rgba32f", 1, {SpvCapabilityShader}, {}, {}},
  {"Rgba16f", 2, {SpvCapabilityShader}, {}, {}},
  {"R32f", 3, {SpvCapabilityShader}, {}, {}},
  {"Rgba8", 4, {SpvCapabilityShader}, {}, {}},
  {"Rgba8Snorm", 5, {SpvCapabilityShader}, {}, {}},
  {"Rg32f", 6, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"Rg16f", 7, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"R11fG11fB10f", 8, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"R16f", 9, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"Rgba16", 10, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"Rgb10A2", 11, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"Rg16", 12, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"Rg8", 13, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"R16", 14, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"R8", 15, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"Rgba16Snorm", 16, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"Rg16Snorm", 17, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"Rg8Snorm", 18, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"R16Snorm", 19, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"R8Snorm", 20, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"Rgba32i", 21, {SpvCapabilityShader}, {}, {}},
  {"Rgba16i", 22, {SpvCapabilityShader}, {}, {}},
  {"Rgba8i", 23, {SpvCapabilityShader}, {}, {}},
  {"R32i", 24, {SpvCapabilityShader}, {}, {}},
  {"Rg32i", 25, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"Rg16i", 26, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"Rg8i", 27, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"R16i", 28, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"R8i", 29, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"Rgba32ui", 30, {SpvCapabilityShader}, {}, {}},
  {"Rgba16ui", 31, {SpvCapabilityShader}, {}, {}},
  {"Rgba8ui", 32, {SpvCapabilityShader}, {}, {}},
  {"R32ui", 33, {SpvCapabilityShader}, {}, {}},
  {"Rgb10a2ui", 34, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"Rg32ui", 35, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"Rg16ui", 36, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"Rg8ui", 37, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"R16ui", 38, {SpvCapabilityStorageImageExtendedFormats}, {}, {}},
  {"R8ui", 39, {SpvCapabilityStorageImageExtendedFormats}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_ImageChannelOrderEntries[] = {
  {"R", 0, {SpvCapabilityKernel}, {}, {}},
  {"A", 1, {SpvCapabilityKernel}, {}, {}},
  {"RG", 2, {SpvCapabilityKernel}, {}, {}},
  {"RA", 3, {SpvCapabilityKernel}, {}, {}},
  {"RGB", 4, {SpvCapabilityKernel}, {}, {}},
  {"RGBA", 5, {SpvCapabilityKernel}, {}, {}},
  {"BGRA", 6, {SpvCapabilityKernel}, {}, {}},
  {"ARGB", 7, {SpvCapabilityKernel}, {}, {}},
  {"Intensity", 8, {SpvCapabilityKernel}, {}, {}},
  {"Luminance", 9, {SpvCapabilityKernel}, {}, {}},
  {"Rx", 10, {SpvCapabilityKernel}, {}, {}},
  {"RGx", 11, {SpvCapabilityKernel}, {}, {}},
  {"RGBx", 12, {SpvCapabilityKernel}, {}, {}},
  {"Depth", 13, {SpvCapabilityKernel}, {}, {}},
  {"DepthStencil", 14, {SpvCapabilityKernel}, {}, {}},
  {"sRGB", 15, {SpvCapabilityKernel}, {}, {}},
  {"sRGBx", 16, {SpvCapabilityKernel}, {}, {}},
  {"sRGBA", 17, {SpvCapabilityKernel}, {}, {}},
  {"sBGRA", 18, {SpvCapabilityKernel}, {}, {}},
  {"ABGR", 19, {SpvCapabilityKernel}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_ImageChannelDataTypeEntries[] = {
  {"SnormInt8", 0, {SpvCapabilityKernel}, {}, {}},
  {"SnormInt16", 1, {SpvCapabilityKernel}, {}, {}},
  {"UnormInt8", 2, {SpvCapabilityKernel}, {}, {}},
  {"UnormInt16", 3, {SpvCapabilityKernel}, {}, {}},
  {"UnormShort565", 4, {SpvCapabilityKernel}, {}, {}},
  {"UnormShort555", 5, {SpvCapabilityKernel}, {}, {}},
  {"UnormInt101010", 6, {SpvCapabilityKernel}, {}, {}},
  {"SignedInt8", 7, {SpvCapabilityKernel}, {}, {}},
  {"SignedInt16", 8, {SpvCapabilityKernel}, {}, {}},
  {"SignedInt32", 9, {SpvCapabilityKernel}, {}, {}},
  {"UnsignedInt8", 10, {SpvCapabilityKernel}, {}, {}},
  {"UnsignedInt16", 11, {SpvCapabilityKernel}, {}, {}},
  {"UnsignedInt32", 12, {SpvCapabilityKernel}, {}, {}},
  {"HalfFloat", 13, {SpvCapabilityKernel}, {}, {}},
  {"Float", 14, {SpvCapabilityKernel}, {}, {}},
  {"UnormInt24", 15, {SpvCapabilityKernel}, {}, {}},
  {"UnormInt101010_2", 16, {SpvCapabilityKernel}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_FPRoundingModeEntries[] = {
  {"RTE", 0, {SpvCapabilityKernel}, {}, {}},
  {"RTZ", 1, {SpvCapabilityKernel}, {}, {}},
  {"RTP", 2, {SpvCapabilityKernel}, {}, {}},
  {"RTN", 3, {SpvCapabilityKernel}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_LinkageTypeEntries[] = {
  {"Export", 0, {SpvCapabilityLinkage}, {}, {}},
  {"Import", 1, {SpvCapabilityLinkage}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_AccessQualifierEntries[] = {
  {"ReadOnly", 0, {SpvCapabilityKernel}, {}, {}},
  {"WriteOnly", 1, {SpvCapabilityKernel}, {}, {}},
  {"ReadWrite", 2, {SpvCapabilityKernel}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_FunctionParameterAttributeEntries[] = {
  {"Zext", 0, {SpvCapabilityKernel}, {}, {}},
  {"Sext", 1, {SpvCapabilityKernel}, {}, {}},
  {"ByVal", 2, {SpvCapabilityKernel}, {}, {}},
  {"Sret", 3, {SpvCapabilityKernel}, {}, {}},
  {"NoAlias", 4, {SpvCapabilityKernel}, {}, {}},
  {"NoCapture", 5, {SpvCapabilityKernel}, {}, {}},
  {"NoWrite", 6, {SpvCapabilityKernel}, {}, {}},
  {"NoReadWrite", 7, {SpvCapabilityKernel}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_DecorationEntries[] = {
  {"RelaxedPrecision", 0, {SpvCapabilityShader}, {}, {}},
  {"SpecId", 1, {SpvCapabilityShader, SpvCapabilityKernel}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"Block", 2, {SpvCapabilityShader}, {}, {}},
  {"BufferBlock", 3, {SpvCapabilityShader}, {}, {}},
  {"RowMajor", 4, {SpvCapabilityMatrix}, {}, {}},
  {"ColMajor", 5, {SpvCapabilityMatrix}, {}, {}},
  {"ArrayStride", 6, {SpvCapabilityShader}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"MatrixStride", 7, {SpvCapabilityMatrix}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"GLSLShared", 8, {SpvCapabilityShader}, {}, {}},
  {"GLSLPacked", 9, {SpvCapabilityShader}, {}, {}},
  {"CPacked", 10, {SpvCapabilityKernel}, {}, {}},
  {"BuiltIn", 11, {}, {}, {SPV_OPERAND_TYPE_BUILT_IN}},
  {"NoPerspective", 13, {SpvCapabilityShader}, {}, {}},
  {"Flat", 14, {SpvCapabilityShader}, {}, {}},
  {"Patch", 15, {SpvCapabilityTessellation}, {}, {}},
  {"Centroid", 16, {SpvCapabilityShader}, {}, {}},
  {"Sample", 17, {SpvCapabilitySampleRateShading}, {}, {}},
  {"Invariant", 18, {SpvCapabilityShader}, {}, {}},
  {"Restrict", 19, {}, {}, {}},
  {"Aliased", 20, {}, {}, {}},
  {"Volatile", 21, {}, {}, {}},
  {"Constant", 22, {SpvCapabilityKernel}, {}, {}},
  {"Coherent", 23, {}, {}, {}},
  {"NonWritable", 24, {}, {}, {}},
  {"NonReadable", 25, {}, {}, {}},
  {"Uniform", 26, {SpvCapabilityShader}, {}, {}},
  {"SaturatedConversion", 28, {SpvCapabilityKernel}, {}, {}},
  {"Stream", 29, {SpvCapabilityGeometryStreams}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"Location", 30, {SpvCapabilityShader}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"Component", 31, {SpvCapabilityShader}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"Index", 32, {SpvCapabilityShader}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"Binding", 33, {SpvCapabilityShader}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"DescriptorSet", 34, {SpvCapabilityShader}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"Offset", 35, {SpvCapabilityShader}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"XfbBuffer", 36, {SpvCapabilityTransformFeedback}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"XfbStride", 37, {SpvCapabilityTransformFeedback}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"FuncParamAttr", 38, {SpvCapabilityKernel}, {}, {SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE}},
  {"FPRoundingMode", 39, {SpvCapabilityKernel}, {}, {SPV_OPERAND_TYPE_FP_ROUNDING_MODE}},
  {"FPFastMathMode", 40, {SpvCapabilityKernel}, {}, {SPV_OPERAND_TYPE_FP_FAST_MATH_MODE}},
  {"LinkageAttributes", 41, {SpvCapabilityLinkage}, {}, {SPV_OPERAND_TYPE_LITERAL_STRING, SPV_OPERAND_TYPE_LINKAGE_TYPE}},
  {"NoContraction", 42, {SpvCapabilityShader}, {}, {}},
  {"InputAttachmentIndex", 43, {SpvCapabilityInputAttachment}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"Alignment", 44, {SpvCapabilityKernel}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"MaxByteOffset", 45, {SpvCapabilityAddresses}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}},
  {"OverrideCoverageNV", 5248, {SpvCapabilitySampleMaskOverrideCoverageNV}, {}, {}},
  {"PassthroughNV", 5250, {SpvCapabilityGeometryShaderPassthroughNV}, {}, {}},
  {"ViewportRelativeNV", 5252, {SpvCapabilityShaderViewportMaskNV}, {}, {}},
  {"SecondaryViewportRelativeNV", 5256, {SpvCapabilityShaderStereoViewNV}, {}, {SPV_OPERAND_TYPE_LITERAL_INTEGER}}
};

static const spv_operand_desc_t pygen_variable_BuiltInEntries[] = {
  {"Position", 0, {SpvCapabilityShader}, {}, {}},
  {"PointSize", 1, {SpvCapabilityShader}, {}, {}},
  {"ClipDistance", 3, {SpvCapabilityClipDistance}, {}, {}},
  {"CullDistance", 4, {SpvCapabilityCullDistance}, {}, {}},
  {"VertexId", 5, {SpvCapabilityShader}, {}, {}},
  {"InstanceId", 6, {SpvCapabilityShader}, {}, {}},
  {"PrimitiveId", 7, {SpvCapabilityGeometry, SpvCapabilityTessellation}, {}, {}},
  {"InvocationId", 8, {SpvCapabilityGeometry, SpvCapabilityTessellation}, {}, {}},
  {"Layer", 9, {SpvCapabilityGeometry}, {}, {}},
  {"ViewportIndex", 10, {SpvCapabilityMultiViewport}, {}, {}},
  {"TessLevelOuter", 11, {SpvCapabilityTessellation}, {}, {}},
  {"TessLevelInner", 12, {SpvCapabilityTessellation}, {}, {}},
  {"TessCoord", 13, {SpvCapabilityTessellation}, {}, {}},
  {"PatchVertices", 14, {SpvCapabilityTessellation}, {}, {}},
  {"FragCoord", 15, {SpvCapabilityShader}, {}, {}},
  {"PointCoord", 16, {SpvCapabilityShader}, {}, {}},
  {"FrontFacing", 17, {SpvCapabilityShader}, {}, {}},
  {"SampleId", 18, {SpvCapabilitySampleRateShading}, {}, {}},
  {"SamplePosition", 19, {SpvCapabilitySampleRateShading}, {}, {}},
  {"SampleMask", 20, {SpvCapabilitySampleRateShading}, {}, {}},
  {"FragDepth", 22, {SpvCapabilityShader}, {}, {}},
  {"HelperInvocation", 23, {SpvCapabilityShader}, {}, {}},
  {"NumWorkgroups", 24, {}, {}, {}},
  {"WorkgroupSize", 25, {}, {}, {}},
  {"WorkgroupId", 26, {}, {}, {}},
  {"LocalInvocationId", 27, {}, {}, {}},
  {"GlobalInvocationId", 28, {}, {}, {}},
  {"LocalInvocationIndex", 29, {}, {}, {}},
  {"WorkDim", 30, {SpvCapabilityKernel}, {}, {}},
  {"GlobalSize", 31, {SpvCapabilityKernel}, {}, {}},
  {"EnqueuedWorkgroupSize", 32, {SpvCapabilityKernel}, {}, {}},
  {"GlobalOffset", 33, {SpvCapabilityKernel}, {}, {}},
  {"GlobalLinearId", 34, {SpvCapabilityKernel}, {}, {}},
  {"SubgroupSize", 36, {SpvCapabilityKernel}, {}, {}},
  {"SubgroupMaxSize", 37, {SpvCapabilityKernel}, {}, {}},
  {"NumSubgroups", 38, {SpvCapabilityKernel}, {}, {}},
  {"NumEnqueuedSubgroups", 39, {SpvCapabilityKernel}, {}, {}},
  {"SubgroupId", 40, {SpvCapabilityKernel}, {}, {}},
  {"SubgroupLocalInvocationId", 41, {SpvCapabilityKernel}, {}, {}},
  {"VertexIndex", 42, {SpvCapabilityShader}, {}, {}},
  {"InstanceIndex", 43, {SpvCapabilityShader}, {}, {}},
  {"SubgroupEqMaskKHR", 4416, {SpvCapabilitySubgroupBallotKHR}, {}, {}},
  {"SubgroupGeMaskKHR", 4417, {SpvCapabilitySubgroupBallotKHR}, {}, {}},
  {"SubgroupGtMaskKHR", 4418, {SpvCapabilitySubgroupBallotKHR}, {}, {}},
  {"SubgroupLeMaskKHR", 4419, {SpvCapabilitySubgroupBallotKHR}, {}, {}},
  {"SubgroupLtMaskKHR", 4420, {SpvCapabilitySubgroupBallotKHR}, {}, {}},
  {"BaseVertex", 4424, {SpvCapabilityDrawParameters}, {}, {}},
  {"BaseInstance", 4425, {SpvCapabilityDrawParameters}, {}, {}},
  {"DrawIndex", 4426, {SpvCapabilityDrawParameters}, {}, {}},
  {"DeviceIndex", 4438, {SpvCapabilityDeviceGroup}, {}, {}},
  {"ViewIndex", 4440, {SpvCapabilityMultiView}, {}, {}},
  {"ViewportMaskNV", 5253, {SpvCapabilityShaderViewportMaskNV}, {}, {}},
  {"SecondaryPositionNV", 5257, {SpvCapabilityShaderStereoViewNV}, {}, {}},
  {"SecondaryViewportMaskNV", 5258, {SpvCapabilityShaderStereoViewNV}, {}, {}},
  {"PositionPerViewNV", 5261, {SpvCapabilityPerViewAttributesNV}, {}, {}},
  {"ViewportMaskPerViewNV", 5262, {SpvCapabilityPerViewAttributesNV}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_ScopeEntries[] = {
  {"CrossDevice", 0, {}, {}, {}},
  {"Device", 1, {}, {}, {}},
  {"Workgroup", 2, {}, {}, {}},
  {"Subgroup", 3, {}, {}, {}},
  {"Invocation", 4, {}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_GroupOperationEntries[] = {
  {"Reduce", 0, {SpvCapabilityKernel}, {}, {}},
  {"InclusiveScan", 1, {SpvCapabilityKernel}, {}, {}},
  {"ExclusiveScan", 2, {SpvCapabilityKernel}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_KernelEnqueueFlagsEntries[] = {
  {"NoWait", 0, {SpvCapabilityKernel}, {}, {}},
  {"WaitKernel", 1, {SpvCapabilityKernel}, {}, {}},
  {"WaitWorkGroup", 2, {SpvCapabilityKernel}, {}, {}}
};

static const spv_operand_desc_t pygen_variable_CapabilityEntries[] = {
  {"Matrix", 0, {}, {}, {}},
  {"Shader", 1, {SpvCapabilityMatrix}, {}, {}},
  {"Geometry", 2, {SpvCapabilityShader}, {}, {}},
  {"Tessellation", 3, {SpvCapabilityShader}, {}, {}},
  {"Addresses", 4, {}, {}, {}},
  {"Linkage", 5, {}, {}, {}},
  {"Kernel", 6, {}, {}, {}},
  {"Vector16", 7, {SpvCapabilityKernel}, {}, {}},
  {"Float16Buffer", 8, {SpvCapabilityKernel}, {}, {}},
  {"Float16", 9, {}, {}, {}},
  {"Float64", 10, {}, {}, {}},
  {"Int64", 11, {}, {}, {}},
  {"Int64Atomics", 12, {SpvCapabilityInt64}, {}, {}},
  {"ImageBasic", 13, {SpvCapabilityKernel}, {}, {}},
  {"ImageReadWrite", 14, {SpvCapabilityImageBasic}, {}, {}},
  {"ImageMipmap", 15, {SpvCapabilityImageBasic}, {}, {}},
  {"Pipes", 17, {SpvCapabilityKernel}, {}, {}},
  {"Groups", 18, {}, {}, {}},
  {"DeviceEnqueue", 19, {SpvCapabilityKernel}, {}, {}},
  {"LiteralSampler", 20, {SpvCapabilityKernel}, {}, {}},
  {"AtomicStorage", 21, {SpvCapabilityShader}, {}, {}},
  {"Int16", 22, {}, {}, {}},
  {"TessellationPointSize", 23, {SpvCapabilityTessellation}, {}, {}},
  {"GeometryPointSize", 24, {SpvCapabilityGeometry}, {}, {}},
  {"ImageGatherExtended", 25, {SpvCapabilityShader}, {}, {}},
  {"StorageImageMultisample", 27, {SpvCapabilityShader}, {}, {}},
  {"UniformBufferArrayDynamicIndexing", 28, {SpvCapabilityShader}, {}, {}},
  {"SampledImageArrayDynamicIndexing", 29, {SpvCapabilityShader}, {}, {}},
  {"StorageBufferArrayDynamicIndexing", 30, {SpvCapabilityShader}, {}, {}},
  {"StorageImageArrayDynamicIndexing", 31, {SpvCapabilityShader}, {}, {}},
  {"ClipDistance", 32, {SpvCapabilityShader}, {}, {}},
  {"CullDistance", 33, {SpvCapabilityShader}, {}, {}},
  {"ImageCubeArray", 34, {SpvCapabilitySampledCubeArray}, {}, {}},
  {"SampleRateShading", 35, {SpvCapabilityShader}, {}, {}},
  {"ImageRect", 36, {SpvCapabilitySampledRect}, {}, {}},
  {"SampledRect", 37, {SpvCapabilityShader}, {}, {}},
  {"GenericPointer", 38, {SpvCapabilityAddresses}, {}, {}},
  {"Int8", 39, {SpvCapabilityKernel}, {}, {}},
  {"InputAttachment", 40, {SpvCapabilityShader}, {}, {}},
  {"SparseResidency", 41, {SpvCapabilityShader}, {}, {}},
  {"MinLod", 42, {SpvCapabilityShader}, {}, {}},
  {"Sampled1D", 43, {}, {}, {}},
  {"Image1D", 44, {SpvCapabilitySampled1D}, {}, {}},
  {"SampledCubeArray", 45, {SpvCapabilityShader}, {}, {}},
  {"SampledBuffer", 46, {}, {}, {}},
  {"ImageBuffer", 47, {SpvCapabilitySampledBuffer}, {}, {}},
  {"ImageMSArray", 48, {SpvCapabilityShader}, {}, {}},
  {"StorageImageExtendedFormats", 49, {SpvCapabilityShader}, {}, {}},
  {"ImageQuery", 50, {SpvCapabilityShader}, {}, {}},
  {"DerivativeControl", 51, {SpvCapabilityShader}, {}, {}},
  {"InterpolationFunction", 52, {SpvCapabilityShader}, {}, {}},
  {"TransformFeedback", 53, {SpvCapabilityShader}, {}, {}},
  {"GeometryStreams", 54, {SpvCapabilityGeometry}, {}, {}},
  {"StorageImageReadWithoutFormat", 55, {SpvCapabilityShader}, {}, {}},
  {"StorageImageWriteWithoutFormat", 56, {SpvCapabilityShader}, {}, {}},
  {"MultiViewport", 57, {SpvCapabilityGeometry}, {}, {}},
  {"SubgroupDispatch", 58, {SpvCapabilityDeviceEnqueue}, {}, {}},
  {"NamedBarrier", 59, {SpvCapabilityKernel}, {}, {}},
  {"PipeStorage", 60, {SpvCapabilityPipes}, {}, {}},
  {"SubgroupBallotKHR", 4423, {}, {libspirv::Extension::kSPV_KHR_shader_ballot}, {}},
  {"DrawParameters", 4427, {}, {libspirv::Extension::kSPV_KHR_shader_draw_parameters}, {}},
  {"SubgroupVoteKHR", 4431, {}, {libspirv::Extension::kSPV_KHR_subgroup_vote}, {}},
  {"StorageUniformBufferBlock16", 4433, {}, {libspirv::Extension::kSPV_KHR_16bit_storage}, {}},
  {"StorageUniform16", 4434, {}, {libspirv::Extension::kSPV_KHR_16bit_storage}, {}},
  {"StoragePushConstant16", 4435, {}, {libspirv::Extension::kSPV_KHR_16bit_storage}, {}},
  {"StorageInputOutput16", 4436, {}, {libspirv::Extension::kSPV_KHR_16bit_storage}, {}},
  {"DeviceGroup", 4437, {}, {libspirv::Extension::kSPV_KHR_device_group}, {}},
  {"MultiView", 4439, {SpvCapabilityShader}, {libspirv::Extension::kSPV_KHR_multiview}, {}},
  {"SampleMaskOverrideCoverageNV", 5249, {SpvCapabilitySampleRateShading}, {libspirv::Extension::kSPV_NV_sample_mask_override_coverage}, {}},
  {"GeometryShaderPassthroughNV", 5251, {SpvCapabilityGeometry}, {libspirv::Extension::kSPV_NV_geometry_shader_passthrough}, {}},
  {"ShaderViewportIndexLayerNV", 5254, {SpvCapabilityMultiViewport}, {libspirv::Extension::kSPV_NV_viewport_array2}, {}},
  {"ShaderViewportMaskNV", 5255, {SpvCapabilityShaderViewportIndexLayerNV}, {libspirv::Extension::kSPV_NV_viewport_array2}, {}},
  {"ShaderStereoViewNV", 5259, {SpvCapabilityShaderViewportMaskNV}, {libspirv::Extension::kSPV_NV_stereo_view_rendering}, {}},
  {"PerViewAttributesNV", 5260, {SpvCapabilityMultiView}, {libspirv::Extension::kSPV_NVX_multiview_per_view_attributes}, {}}
};

static const spv_operand_desc_group_t pygen_variable_OperandInfoTable[] = {
  {SPV_OPERAND_TYPE_IMAGE, ARRAY_SIZE(pygen_variable_ImageOperandsEntries), pygen_variable_ImageOperandsEntries},
  {SPV_OPERAND_TYPE_FP_FAST_MATH_MODE, ARRAY_SIZE(pygen_variable_FPFastMathModeEntries), pygen_variable_FPFastMathModeEntries},
  {SPV_OPERAND_TYPE_SELECTION_CONTROL, ARRAY_SIZE(pygen_variable_SelectionControlEntries), pygen_variable_SelectionControlEntries},
  {SPV_OPERAND_TYPE_LOOP_CONTROL, ARRAY_SIZE(pygen_variable_LoopControlEntries), pygen_variable_LoopControlEntries},
  {SPV_OPERAND_TYPE_FUNCTION_CONTROL, ARRAY_SIZE(pygen_variable_FunctionControlEntries), pygen_variable_FunctionControlEntries},
  {SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID, ARRAY_SIZE(pygen_variable_MemorySemanticsEntries), pygen_variable_MemorySemanticsEntries},
  {SPV_OPERAND_TYPE_MEMORY_ACCESS, ARRAY_SIZE(pygen_variable_MemoryAccessEntries), pygen_variable_MemoryAccessEntries},
  {SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO, ARRAY_SIZE(pygen_variable_KernelProfilingInfoEntries), pygen_variable_KernelProfilingInfoEntries},
  {SPV_OPERAND_TYPE_SOURCE_LANGUAGE, ARRAY_SIZE(pygen_variable_SourceLanguageEntries), pygen_variable_SourceLanguageEntries},
  {SPV_OPERAND_TYPE_EXECUTION_MODEL, ARRAY_SIZE(pygen_variable_ExecutionModelEntries), pygen_variable_ExecutionModelEntries},
  {SPV_OPERAND_TYPE_ADDRESSING_MODEL, ARRAY_SIZE(pygen_variable_AddressingModelEntries), pygen_variable_AddressingModelEntries},
  {SPV_OPERAND_TYPE_MEMORY_MODEL, ARRAY_SIZE(pygen_variable_MemoryModelEntries), pygen_variable_MemoryModelEntries},
  {SPV_OPERAND_TYPE_EXECUTION_MODE, ARRAY_SIZE(pygen_variable_ExecutionModeEntries), pygen_variable_ExecutionModeEntries},
  {SPV_OPERAND_TYPE_STORAGE_CLASS, ARRAY_SIZE(pygen_variable_StorageClassEntries), pygen_variable_StorageClassEntries},
  {SPV_OPERAND_TYPE_DIMENSIONALITY, ARRAY_SIZE(pygen_variable_DimEntries), pygen_variable_DimEntries},
  {SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE, ARRAY_SIZE(pygen_variable_SamplerAddressingModeEntries), pygen_variable_SamplerAddressingModeEntries},
  {SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE, ARRAY_SIZE(pygen_variable_SamplerFilterModeEntries), pygen_variable_SamplerFilterModeEntries},
  {SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT, ARRAY_SIZE(pygen_variable_ImageFormatEntries), pygen_variable_ImageFormatEntries},
  {SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER, ARRAY_SIZE(pygen_variable_ImageChannelOrderEntries), pygen_variable_ImageChannelOrderEntries},
  {SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE, ARRAY_SIZE(pygen_variable_ImageChannelDataTypeEntries), pygen_variable_ImageChannelDataTypeEntries},
  {SPV_OPERAND_TYPE_FP_ROUNDING_MODE, ARRAY_SIZE(pygen_variable_FPRoundingModeEntries), pygen_variable_FPRoundingModeEntries},
  {SPV_OPERAND_TYPE_LINKAGE_TYPE, ARRAY_SIZE(pygen_variable_LinkageTypeEntries), pygen_variable_LinkageTypeEntries},
  {SPV_OPERAND_TYPE_ACCESS_QUALIFIER, ARRAY_SIZE(pygen_variable_AccessQualifierEntries), pygen_variable_AccessQualifierEntries},
  {SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE, ARRAY_SIZE(pygen_variable_FunctionParameterAttributeEntries), pygen_variable_FunctionParameterAttributeEntries},
  {SPV_OPERAND_TYPE_DECORATION, ARRAY_SIZE(pygen_variable_DecorationEntries), pygen_variable_DecorationEntries},
  {SPV_OPERAND_TYPE_BUILT_IN, ARRAY_SIZE(pygen_variable_BuiltInEntries), pygen_variable_BuiltInEntries},
  {SPV_OPERAND_TYPE_SCOPE_ID, ARRAY_SIZE(pygen_variable_ScopeEntries), pygen_variable_ScopeEntries},
  {SPV_OPERAND_TYPE_GROUP_OPERATION, ARRAY_SIZE(pygen_variable_GroupOperationEntries), pygen_variable_GroupOperationEntries},
  {SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS, ARRAY_SIZE(pygen_variable_KernelEnqueueFlagsEntries), pygen_variable_KernelEnqueueFlagsEntries},
  {SPV_OPERAND_TYPE_CAPABILITY, ARRAY_SIZE(pygen_variable_CapabilityEntries), pygen_variable_CapabilityEntries},
  {SPV_OPERAND_TYPE_OPTIONAL_IMAGE, ARRAY_SIZE(pygen_variable_ImageOperandsEntries), pygen_variable_ImageOperandsEntries},
  {SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS, ARRAY_SIZE(pygen_variable_MemoryAccessEntries), pygen_variable_MemoryAccessEntries},
  {SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER, ARRAY_SIZE(pygen_variable_AccessQualifierEntries), pygen_variable_AccessQualifierEntries}
};
