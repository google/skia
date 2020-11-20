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
OpName %_0_blend_dst_over "_0_blend_dst_over"
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
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
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
%main = OpFunction %void None %14
%15 = OpLabel
%_0_blend_dst_over = OpVariable %_ptr_Function_v4float Function
%19 = OpLoad %v4float %dst
%20 = OpCompositeExtract %float %19 3
%21 = OpFSub %float %float_1 %20
%22 = OpLoad %v4float %src
%23 = OpVectorTimesScalar %v4float %22 %21
%24 = OpLoad %v4float %dst
%25 = OpFAdd %v4float %23 %24
OpStore %_0_blend_dst_over %25
%26 = OpLoad %v4float %_0_blend_dst_over
OpStore %sk_FragColor %26
OpReturn
OpFunctionEnd
