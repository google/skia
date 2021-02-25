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
OpName %_0_blend_lighten "_0_blend_lighten"
OpName %_1_blend_src_over "_1_blend_src_over"
OpName %_2_result "_2_result"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %20 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
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
%_0_blend_lighten = OpVariable %_ptr_Function_v4float Function
%_1_blend_src_over = OpVariable %_ptr_Function_v4float Function
%_2_result = OpVariable %_ptr_Function_v4float Function
%20 = OpLoad %v4float %src
%22 = OpLoad %v4float %src
%23 = OpCompositeExtract %float %22 3
%24 = OpFSub %float %float_1 %23
%25 = OpLoad %v4float %dst
%26 = OpVectorTimesScalar %v4float %25 %24
%27 = OpFAdd %v4float %20 %26
OpStore %_2_result %27
%29 = OpLoad %v4float %_2_result
%30 = OpVectorShuffle %v3float %29 %29 0 1 2
%32 = OpLoad %v4float %dst
%33 = OpCompositeExtract %float %32 3
%34 = OpFSub %float %float_1 %33
%35 = OpLoad %v4float %src
%36 = OpVectorShuffle %v3float %35 %35 0 1 2
%37 = OpVectorTimesScalar %v3float %36 %34
%38 = OpLoad %v4float %dst
%39 = OpVectorShuffle %v3float %38 %38 0 1 2
%40 = OpFAdd %v3float %37 %39
%28 = OpExtInst %v3float %1 FMax %30 %40
%41 = OpLoad %v4float %_2_result
%42 = OpVectorShuffle %v4float %41 %28 4 5 6 3
OpStore %_2_result %42
%43 = OpLoad %v4float %_2_result
OpStore %sk_FragColor %43
OpReturn
OpFunctionEnd
