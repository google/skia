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
OpDecorate %24 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
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
%16 = OpLoad %v4float %src
%17 = OpVectorShuffle %v3float %16 %16 0 1 2
%19 = OpLoad %v4float %dst
%20 = OpVectorShuffle %v3float %19 %19 0 1 2
%21 = OpFAdd %v3float %17 %20
%24 = OpLoad %v4float %src
%25 = OpVectorShuffle %v3float %24 %24 0 1 2
%26 = OpLoad %v4float %dst
%27 = OpCompositeExtract %float %26 3
%28 = OpVectorTimesScalar %v3float %25 %27
%29 = OpLoad %v4float %dst
%30 = OpVectorShuffle %v3float %29 %29 0 1 2
%31 = OpLoad %v4float %src
%32 = OpCompositeExtract %float %31 3
%33 = OpVectorTimesScalar %v3float %30 %32
%23 = OpExtInst %v3float %1 FMin %28 %33
%34 = OpVectorTimesScalar %v3float %23 %float_2
%35 = OpFSub %v3float %21 %34
%36 = OpCompositeExtract %float %35 0
%37 = OpCompositeExtract %float %35 1
%38 = OpCompositeExtract %float %35 2
%39 = OpLoad %v4float %src
%40 = OpCompositeExtract %float %39 3
%42 = OpLoad %v4float %src
%43 = OpCompositeExtract %float %42 3
%44 = OpFSub %float %float_1 %43
%45 = OpLoad %v4float %dst
%46 = OpCompositeExtract %float %45 3
%47 = OpFMul %float %44 %46
%48 = OpFAdd %float %40 %47
%49 = OpCompositeConstruct %v4float %36 %37 %38 %48
OpStore %sk_FragColor %49
OpReturn
OpFunctionEnd
