### Compilation failed:

error: SPIR-V validation error: Operand 3 of TypeImage requires one of these capabilities: Sampled1D Image1D 
  %13 = OpTypeImage %float 1D 0 0 0 1 Unknown

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %tex "tex"
OpName %main "main"
OpName %a "a"
OpName %b "b"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %tex RelaxedPrecision
OpDecorate %tex Binding 0
OpDecorate %tex DescriptorSet 0
OpDecorate %20 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%13 = OpTypeImage %float 1D 0 0 0 1 Unknown
%12 = OpTypeSampledImage %13
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
%tex = OpVariable %_ptr_UniformConstant_12 UniformConstant
%void = OpTypeVoid
%15 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%26 = OpConstantComposite %v2float %float_0 %float_0
%main = OpFunction %void None %15
%16 = OpLabel
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%20 = OpLoad %12 %tex
%19 = OpImageSampleImplicitLod %v4float %20 %float_0
OpStore %a %19
%24 = OpLoad %12 %tex
%23 = OpImageSampleProjImplicitLod %v4float %24 %26
OpStore %b %23
%27 = OpLoad %v4float %a
%28 = OpVectorShuffle %v2float %27 %27 0 1
%29 = OpCompositeExtract %float %28 0
%30 = OpCompositeExtract %float %28 1
%31 = OpLoad %v4float %b
%32 = OpVectorShuffle %v2float %31 %31 2 3
%33 = OpCompositeExtract %float %32 0
%34 = OpCompositeExtract %float %32 1
%35 = OpCompositeConstruct %v4float %29 %30 %33 %34
OpStore %sk_FragColor %35
OpReturn
OpFunctionEnd

1 error
