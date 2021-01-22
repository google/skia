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
OpDecorate %17 RelaxedPrecision
OpDecorate %19 RelaxedPrecision
OpDecorate %20 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%main = OpFunction %void None %14
%15 = OpLabel
%17 = OpLoad %v4float %src
%18 = OpCompositeExtract %float %17 3
%19 = OpFSub %float %float_1 %18
%20 = OpLoad %v4float %dst
%21 = OpVectorShuffle %v3float %20 %20 0 1 2
%23 = OpVectorTimesScalar %v3float %21 %19
%24 = OpLoad %v4float %dst
%25 = OpCompositeExtract %float %24 3
%26 = OpFSub %float %float_1 %25
%27 = OpLoad %v4float %src
%28 = OpVectorShuffle %v3float %27 %27 0 1 2
%29 = OpVectorTimesScalar %v3float %28 %26
%30 = OpFAdd %v3float %23 %29
%31 = OpLoad %v4float %src
%32 = OpVectorShuffle %v3float %31 %31 0 1 2
%33 = OpLoad %v4float %dst
%34 = OpVectorShuffle %v3float %33 %33 0 1 2
%35 = OpFMul %v3float %32 %34
%36 = OpFAdd %v3float %30 %35
%37 = OpCompositeExtract %float %36 0
%38 = OpCompositeExtract %float %36 1
%39 = OpCompositeExtract %float %36 2
%40 = OpLoad %v4float %src
%41 = OpCompositeExtract %float %40 3
%42 = OpLoad %v4float %src
%43 = OpCompositeExtract %float %42 3
%44 = OpFSub %float %float_1 %43
%45 = OpLoad %v4float %dst
%46 = OpCompositeExtract %float %45 3
%47 = OpFMul %float %44 %46
%48 = OpFAdd %float %41 %47
%49 = OpCompositeConstruct %v4float %37 %38 %39 %48
OpStore %sk_FragColor %49
OpReturn
OpFunctionEnd
