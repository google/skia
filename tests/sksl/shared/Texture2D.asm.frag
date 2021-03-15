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
OpDecorate %26 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%13 = OpTypeImage %float 2D 0 0 0 1 Unknown
%12 = OpTypeSampledImage %13
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
%tex = OpVariable %_ptr_UniformConstant_12 UniformConstant
%void = OpTypeVoid
%15 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%23 = OpConstantComposite %v2float %float_0 %float_0
%v3float = OpTypeVector %float 3
%28 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%main = OpFunction %void None %15
%16 = OpLabel
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%20 = OpLoad %12 %tex
%19 = OpImageSampleImplicitLod %v4float %20 %23
OpStore %a %19
%26 = OpLoad %12 %tex
%25 = OpImageSampleProjImplicitLod %v4float %26 %28
OpStore %b %25
%29 = OpLoad %v4float %a
%30 = OpVectorShuffle %v2float %29 %29 0 1
%31 = OpCompositeExtract %float %30 0
%32 = OpCompositeExtract %float %30 1
%33 = OpLoad %v4float %b
%34 = OpVectorShuffle %v2float %33 %33 2 3
%35 = OpCompositeExtract %float %34 0
%36 = OpCompositeExtract %float %34 1
%37 = OpCompositeConstruct %v4float %31 %32 %35 %36
OpStore %sk_FragColor %37
OpReturn
OpFunctionEnd
