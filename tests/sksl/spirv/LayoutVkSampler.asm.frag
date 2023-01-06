### Compilation failed:

error: SPIR-V validation error: [VUID-StandaloneSpirv-UniformConstant-06677] UniformConstant id '14' is missing Binding decoration.
From Vulkan spec:
These variables must have DescriptorSet and Binding decorations specified
  %vkSampler = OpVariable %_ptr_UniformConstant_12 UniformConstant

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %yuvSampler "yuvSampler"
OpName %vkSampler "vkSampler"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %yuvSampler RelaxedPrecision
OpDecorate %yuvSampler Binding 5
OpDecorate %yuvSampler DescriptorSet 3
OpDecorate %vkSampler RelaxedPrecision
OpDecorate %vkSampler DescriptorSet 7
OpDecorate %19 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%11 = OpTypeImage %float 2D 0 0 0 1 Unknown
%12 = OpTypeSampledImage %11
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
%yuvSampler = OpVariable %_ptr_UniformConstant_12 UniformConstant
%vkSampler = OpVariable %_ptr_UniformConstant_12 UniformConstant
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%22 = OpConstantComposite %v2float %float_0 %float_0
%main = OpFunction %void None %16
%17 = OpLabel
%19 = OpLoad %12 %yuvSampler
%18 = OpImageSampleImplicitLod %v4float %19 %22
%24 = OpLoad %12 %vkSampler
%23 = OpImageSampleImplicitLod %v4float %24 %22
%25 = OpFAdd %v4float %18 %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd

1 error
