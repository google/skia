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
OpName %_0_blend_multiply "_0_blend_multiply"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %19 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%main = OpFunction %void None %14
%15 = OpLabel
%_0_blend_multiply = OpVariable %_ptr_Function_v4float Function
%19 = OpLoad %v4float %src
%20 = OpCompositeExtract %float %19 3
%21 = OpFSub %float %float_1 %20
%22 = OpLoad %v4float %dst
%23 = OpVectorShuffle %v3float %22 %22 0 1 2
%25 = OpVectorTimesScalar %v3float %23 %21
%26 = OpLoad %v4float %dst
%27 = OpCompositeExtract %float %26 3
%28 = OpFSub %float %float_1 %27
%29 = OpLoad %v4float %src
%30 = OpVectorShuffle %v3float %29 %29 0 1 2
%31 = OpVectorTimesScalar %v3float %30 %28
%32 = OpFAdd %v3float %25 %31
%33 = OpLoad %v4float %src
%34 = OpVectorShuffle %v3float %33 %33 0 1 2
%35 = OpLoad %v4float %dst
%36 = OpVectorShuffle %v3float %35 %35 0 1 2
%37 = OpFMul %v3float %34 %36
%38 = OpFAdd %v3float %32 %37
%39 = OpCompositeExtract %float %38 0
%40 = OpCompositeExtract %float %38 1
%41 = OpCompositeExtract %float %38 2
%42 = OpLoad %v4float %src
%43 = OpCompositeExtract %float %42 3
%44 = OpLoad %v4float %src
%45 = OpCompositeExtract %float %44 3
%46 = OpFSub %float %float_1 %45
%47 = OpLoad %v4float %dst
%48 = OpCompositeExtract %float %47 3
%49 = OpFMul %float %46 %48
%50 = OpFAdd %float %43 %49
%51 = OpCompositeConstruct %v4float %39 %40 %41 %50
OpStore %_0_blend_multiply %51
%52 = OpLoad %v4float %_0_blend_multiply
OpStore %sk_FragColor %52
OpReturn
OpFunctionEnd
