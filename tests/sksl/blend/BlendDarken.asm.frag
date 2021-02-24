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
OpName %_0_blend_darken "_0_blend_darken"
OpName %_1_result "_1_result"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %19 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%main = OpFunction %void None %14
%15 = OpLabel
%_0_blend_darken = OpVariable %_ptr_Function_v4float Function
%_1_result = OpVariable %_ptr_Function_v4float Function
%19 = OpLoad %v4float %src
%21 = OpLoad %v4float %src
%22 = OpCompositeExtract %float %21 3
%23 = OpFSub %float %float_1 %22
%24 = OpLoad %v4float %dst
%25 = OpVectorTimesScalar %v4float %24 %23
%26 = OpFAdd %v4float %19 %25
OpStore %_1_result %26
%28 = OpLoad %v4float %_1_result
%29 = OpVectorShuffle %v3float %28 %28 0 1 2
%31 = OpLoad %v4float %dst
%32 = OpCompositeExtract %float %31 3
%33 = OpFSub %float %float_1 %32
%34 = OpLoad %v4float %src
%35 = OpVectorShuffle %v3float %34 %34 0 1 2
%36 = OpVectorTimesScalar %v3float %35 %33
%37 = OpLoad %v4float %dst
%38 = OpVectorShuffle %v3float %37 %37 0 1 2
%39 = OpFAdd %v3float %36 %38
%27 = OpExtInst %v3float %1 FMin %29 %39
%40 = OpLoad %v4float %_1_result
%41 = OpVectorShuffle %v4float %40 %27 4 5 6 3
OpStore %_1_result %41
%42 = OpLoad %v4float %_1_result
OpStore %sk_FragColor %42
OpReturn
OpFunctionEnd
