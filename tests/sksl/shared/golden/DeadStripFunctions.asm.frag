OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %blend_dst "blend_dst"
OpName %live_fn "live_fn"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %17 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%12 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%25 = OpTypeFunction %void
%float_3 = OpConstant %float 3
%27 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
%float_n3 = OpConstant %float -3
%30 = OpConstantComposite %v4float %float_n3 %float_n3 %float_n3 %float_n3
%float_7 = OpConstant %float 7
%34 = OpConstantComposite %v4float %float_7 %float_7 %float_7 %float_7
%float_n7 = OpConstant %float -7
%37 = OpConstantComposite %v4float %float_n7 %float_n7 %float_n7 %float_n7
%blend_dst = OpFunction %v4float None %12
%14 = OpFunctionParameter %_ptr_Function_v4float
%15 = OpFunctionParameter %_ptr_Function_v4float
%16 = OpLabel
%17 = OpLoad %v4float %15
OpReturnValue %17
OpFunctionEnd
%live_fn = OpFunction %v4float None %12
%18 = OpFunctionParameter %_ptr_Function_v4float
%19 = OpFunctionParameter %_ptr_Function_v4float
%20 = OpLabel
%21 = OpLoad %v4float %18
%22 = OpLoad %v4float %19
%23 = OpFAdd %v4float %21 %22
OpReturnValue %23
OpFunctionEnd
%main = OpFunction %void None %25
%26 = OpLabel
%29 = OpVariable %_ptr_Function_v4float Function
%32 = OpVariable %_ptr_Function_v4float Function
%36 = OpVariable %_ptr_Function_v4float Function
%39 = OpVariable %_ptr_Function_v4float Function
OpStore %29 %27
OpStore %32 %30
%33 = OpFunctionCall %v4float %live_fn %29 %32
OpStore %sk_FragColor %33
OpStore %36 %34
OpStore %39 %37
%40 = OpFunctionCall %v4float %blend_dst %36 %39
OpStore %sk_FragColor %40
OpReturn
OpFunctionEnd
