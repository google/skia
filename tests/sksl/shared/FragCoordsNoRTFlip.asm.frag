OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_FragCoord %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %sksl_synthetic_uniforms "sksl_synthetic_uniforms"
OpMemberName %sksl_synthetic_uniforms 0 "u_skRTFlip"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %sksl_synthetic_uniforms 0 Offset 16384
OpDecorate %sksl_synthetic_uniforms Block
OpDecorate %15 Binding 0
OpDecorate %15 DescriptorSet 0
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%13 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%sksl_synthetic_uniforms = OpTypeStruct %v2float
%_ptr_Uniform_sksl_synthetic_uniforms = OpTypePointer Uniform %sksl_synthetic_uniforms
%15 = OpVariable %_ptr_Uniform_sksl_synthetic_uniforms Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%main = OpFunction %void None %13
%14 = OpLabel
%19 = OpLoad %v4float %sk_FragCoord
%20 = OpCompositeExtract %float %19 0
%23 = OpAccessChain %_ptr_Uniform_v2float %15 %int_0
%25 = OpLoad %v2float %23
%26 = OpCompositeExtract %float %25 0
%27 = OpAccessChain %_ptr_Uniform_v2float %15 %int_0
%28 = OpLoad %v2float %27
%29 = OpCompositeExtract %float %28 1
%30 = OpLoad %v4float %sk_FragCoord
%31 = OpCompositeExtract %float %30 1
%32 = OpFMul %float %29 %31
%33 = OpFAdd %float %26 %32
%34 = OpLoad %v4float %sk_FragCoord
%35 = OpCompositeExtract %float %34 2
%36 = OpLoad %v4float %sk_FragCoord
%37 = OpCompositeExtract %float %36 3
%38 = OpCompositeConstruct %v4float %20 %33 %35 %37
%39 = OpVectorShuffle %v2float %38 %38 0 1
%40 = OpLoad %v4float %sk_FragColor
%41 = OpVectorShuffle %v4float %40 %39 4 5 2 3
OpStore %sk_FragColor %41
OpReturn
OpFunctionEnd
