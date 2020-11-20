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
OpName %_0_blend_exclusion "_0_blend_exclusion"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %18 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%v3float = OpTypeVector %float 3
%float_2 = OpConstant %float 2
%float_1 = OpConstant %float 1
%main = OpFunction %void None %14
%15 = OpLabel
%_0_blend_exclusion = OpVariable %_ptr_Function_v4float Function
%18 = OpLoad %v4float %dst
%19 = OpVectorShuffle %v3float %18 %18 0 1 2
%21 = OpLoad %v4float %src
%22 = OpVectorShuffle %v3float %21 %21 0 1 2
%23 = OpFAdd %v3float %19 %22
%25 = OpLoad %v4float %dst
%26 = OpVectorShuffle %v3float %25 %25 0 1 2
%27 = OpVectorTimesScalar %v3float %26 %float_2
%28 = OpLoad %v4float %src
%29 = OpVectorShuffle %v3float %28 %28 0 1 2
%30 = OpFMul %v3float %27 %29
%31 = OpFSub %v3float %23 %30
%32 = OpCompositeExtract %float %31 0
%33 = OpCompositeExtract %float %31 1
%34 = OpCompositeExtract %float %31 2
%35 = OpLoad %v4float %src
%36 = OpCompositeExtract %float %35 3
%38 = OpLoad %v4float %src
%39 = OpCompositeExtract %float %38 3
%40 = OpFSub %float %float_1 %39
%41 = OpLoad %v4float %dst
%42 = OpCompositeExtract %float %41 3
%43 = OpFMul %float %40 %42
%44 = OpFAdd %float %36 %43
%45 = OpCompositeConstruct %v4float %32 %33 %34 %44
OpStore %_0_blend_exclusion %45
%46 = OpLoad %v4float %_0_blend_exclusion
OpStore %sk_FragColor %46
OpReturn
OpFunctionEnd
