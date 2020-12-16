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
OpDecorate %19 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
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
%19 = OpLoad %v4float %src
%21 = OpLoad %v4float %src
%22 = OpCompositeExtract %float %21 3
%23 = OpFSub %float %float_1 %22
%24 = OpLoad %v4float %dst
%25 = OpVectorTimesScalar %v4float %24 %23
%26 = OpFAdd %v4float %19 %25
OpStore %_1_blend_src_over %26
%28 = OpLoad %v4float %_1_blend_src_over
OpStore %_2_result %28
%30 = OpLoad %v4float %_2_result
%31 = OpVectorShuffle %v3float %30 %30 0 1 2
%33 = OpLoad %v4float %dst
%34 = OpCompositeExtract %float %33 3
%35 = OpFSub %float %float_1 %34
%36 = OpLoad %v4float %src
%37 = OpVectorShuffle %v3float %36 %36 0 1 2
%38 = OpVectorTimesScalar %v3float %37 %35
%39 = OpLoad %v4float %dst
%40 = OpVectorShuffle %v3float %39 %39 0 1 2
%41 = OpFAdd %v3float %38 %40
%29 = OpExtInst %v3float %1 FMax %31 %41
%42 = OpLoad %v4float %_2_result
%43 = OpVectorShuffle %v4float %42 %29 4 5 6 3
OpStore %_2_result %43
%44 = OpLoad %v4float %_2_result
OpStore %_0_blend_lighten %44
%45 = OpLoad %v4float %_0_blend_lighten
OpStore %sk_FragColor %45
OpReturn
OpFunctionEnd
