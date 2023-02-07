OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor %sk_FragCoord
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_FragCoord "sk_FragCoord"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %sksl_synthetic_uniforms "sksl_synthetic_uniforms"
OpMemberName %sksl_synthetic_uniforms 0 "u_skRTFlip"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpMemberDecorate %sksl_synthetic_uniforms 0 Offset 16384
OpDecorate %sksl_synthetic_uniforms Block
OpDecorate %25 Binding 0
OpDecorate %25 DescriptorSet 0
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%void = OpTypeVoid
%14 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%18 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%22 = OpTypeFunction %v4float %_ptr_Function_v2float
%sksl_synthetic_uniforms = OpTypeStruct %v2float
%_ptr_Uniform_sksl_synthetic_uniforms = OpTypePointer Uniform %sksl_synthetic_uniforms
%25 = OpVariable %_ptr_Uniform_sksl_synthetic_uniforms Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%float_1 = OpConstant %float 1
%_entrypoint_v = OpFunction %void None %14
%15 = OpLabel
%19 = OpVariable %_ptr_Function_v2float Function
OpStore %19 %18
%21 = OpFunctionCall %v4float %main %19
OpStore %sk_FragColor %21
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %22
%23 = OpFunctionParameter %_ptr_Function_v2float
%24 = OpLabel
%28 = OpLoad %v4float %sk_FragCoord
%29 = OpCompositeExtract %float %28 0
%32 = OpAccessChain %_ptr_Uniform_v2float %25 %int_0
%34 = OpLoad %v2float %32
%35 = OpCompositeExtract %float %34 0
%36 = OpAccessChain %_ptr_Uniform_v2float %25 %int_0
%37 = OpLoad %v2float %36
%38 = OpCompositeExtract %float %37 1
%39 = OpLoad %v4float %sk_FragCoord
%40 = OpCompositeExtract %float %39 1
%41 = OpFMul %float %38 %40
%42 = OpFAdd %float %35 %41
%43 = OpLoad %v4float %sk_FragCoord
%44 = OpCompositeExtract %float %43 2
%45 = OpLoad %v4float %sk_FragCoord
%46 = OpCompositeExtract %float %45 3
%47 = OpCompositeConstruct %v4float %29 %42 %44 %46
%48 = OpVectorShuffle %v2float %47 %47 1 0
%49 = OpCompositeExtract %float %48 0
%50 = OpCompositeExtract %float %48 1
%52 = OpCompositeConstruct %v4float %49 %50 %float_1 %float_1
OpReturnValue %52
OpFunctionEnd
