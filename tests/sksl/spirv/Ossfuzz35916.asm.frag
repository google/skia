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
OpDecorate %19 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpMemberDecorate %sksl_synthetic_uniforms 0 Offset 16384
OpDecorate %sksl_synthetic_uniforms Block
OpDecorate %22 Binding 0
OpDecorate %22 DescriptorSet 0
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
%_ptr_Output_float = OpTypePointer Output %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%v2float = OpTypeVector %float 2
%sksl_synthetic_uniforms = OpTypeStruct %v2float
%_ptr_Uniform_sksl_synthetic_uniforms = OpTypePointer Uniform %sksl_synthetic_uniforms
%22 = OpVariable %_ptr_Uniform_sksl_synthetic_uniforms Uniform
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%main = OpFunction %void None %13
%14 = OpLabel
%15 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
%19 = OpLoad %float %15
%21 = OpFSub %float %19 %float_1
OpStore %15 %21
%26 = OpLoad %v4float %sk_FragCoord
%27 = OpCompositeExtract %float %26 0
%28 = OpAccessChain %_ptr_Uniform_v2float %22 %int_0
%30 = OpLoad %v2float %28
%31 = OpCompositeExtract %float %30 0
%32 = OpAccessChain %_ptr_Uniform_v2float %22 %int_0
%33 = OpLoad %v2float %32
%34 = OpCompositeExtract %float %33 1
%35 = OpLoad %v4float %sk_FragCoord
%36 = OpCompositeExtract %float %35 1
%37 = OpFMul %float %34 %36
%38 = OpFAdd %float %31 %37
%39 = OpLoad %v4float %sk_FragCoord
%40 = OpCompositeExtract %float %39 2
%41 = OpLoad %v4float %sk_FragCoord
%42 = OpCompositeExtract %float %41 3
%43 = OpCompositeConstruct %v4float %27 %38 %40 %42
%44 = OpCompositeConstruct %v4float %19 %19 %19 %19
%45 = OpFAdd %v4float %44 %43
OpReturn
OpFunctionEnd
