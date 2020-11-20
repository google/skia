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
OpName %_0_blend_difference "_0_blend_difference"
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
OpDecorate %26 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
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
%_0_blend_difference = OpVariable %_ptr_Function_v4float Function
%18 = OpLoad %v4float %src
%19 = OpVectorShuffle %v3float %18 %18 0 1 2
%21 = OpLoad %v4float %dst
%22 = OpVectorShuffle %v3float %21 %21 0 1 2
%23 = OpFAdd %v3float %19 %22
%26 = OpLoad %v4float %src
%27 = OpVectorShuffle %v3float %26 %26 0 1 2
%28 = OpLoad %v4float %dst
%29 = OpCompositeExtract %float %28 3
%30 = OpVectorTimesScalar %v3float %27 %29
%31 = OpLoad %v4float %dst
%32 = OpVectorShuffle %v3float %31 %31 0 1 2
%33 = OpLoad %v4float %src
%34 = OpCompositeExtract %float %33 3
%35 = OpVectorTimesScalar %v3float %32 %34
%25 = OpExtInst %v3float %1 FMin %30 %35
%36 = OpVectorTimesScalar %v3float %25 %float_2
%37 = OpFSub %v3float %23 %36
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeExtract %float %37 1
%40 = OpCompositeExtract %float %37 2
%41 = OpLoad %v4float %src
%42 = OpCompositeExtract %float %41 3
%44 = OpLoad %v4float %src
%45 = OpCompositeExtract %float %44 3
%46 = OpFSub %float %float_1 %45
%47 = OpLoad %v4float %dst
%48 = OpCompositeExtract %float %47 3
%49 = OpFMul %float %46 %48
%50 = OpFAdd %float %42 %49
%51 = OpCompositeConstruct %v4float %38 %39 %40 %50
OpStore %_0_blend_difference %51
%52 = OpLoad %v4float %_0_blend_difference
OpStore %sk_FragColor %52
OpReturn
OpFunctionEnd
