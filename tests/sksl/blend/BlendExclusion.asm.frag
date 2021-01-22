OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %16 RelaxedPrecision
OpDecorate %19 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%src = OpVariable %_ptr_Input_v4float Input
%dst = OpVariable %_ptr_Input_v4float Input
%void = OpTypeVoid
%14 = OpTypeFunction %void
%v3float = OpTypeVector %float 3
%float_2 = OpConstant %float 2
%float_1 = OpConstant %float 1
%main = OpFunction %void None %14
%15 = OpLabel
%16 = OpLoad %v4float %dst
%17 = OpVectorShuffle %v3float %16 %16 0 1 2
%19 = OpLoad %v4float %src
%20 = OpVectorShuffle %v3float %19 %19 0 1 2
%21 = OpFAdd %v3float %17 %20
%23 = OpLoad %v4float %dst
%24 = OpVectorShuffle %v3float %23 %23 0 1 2
%25 = OpVectorTimesScalar %v3float %24 %float_2
%26 = OpLoad %v4float %src
%27 = OpVectorShuffle %v3float %26 %26 0 1 2
%28 = OpFMul %v3float %25 %27
%29 = OpFSub %v3float %21 %28
%30 = OpCompositeExtract %float %29 0
%31 = OpCompositeExtract %float %29 1
%32 = OpCompositeExtract %float %29 2
%33 = OpLoad %v4float %src
%34 = OpCompositeExtract %float %33 3
%36 = OpLoad %v4float %src
%37 = OpCompositeExtract %float %36 3
%38 = OpFSub %float %float_1 %37
%39 = OpLoad %v4float %dst
%40 = OpCompositeExtract %float %39 3
%41 = OpFMul %float %38 %40
%42 = OpFAdd %float %34 %41
%43 = OpCompositeConstruct %v4float %30 %31 %32 %42
OpStore %sk_FragColor %43
OpReturn
OpFunctionEnd
