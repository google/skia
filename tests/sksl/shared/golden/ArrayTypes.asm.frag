OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %_arr_v2float_int_2 ArrayStride 16
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_v2float_int_2 = OpTypeArray %v2float %int_2
%_ptr_Function__arr_v2float_int_2 = OpTypePointer Function %_arr_v2float_int_2
%float_1 = OpConstant %float 1
%19 = OpConstantComposite %v2float %float_1 %float_1
%float_2 = OpConstant %float 2
%21 = OpConstantComposite %v2float %float_2 %float_2
%int_0 = OpConstant %int 0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%float_3 = OpConstant %float 3
%31 = OpConstantComposite %v2float %float_3 %float_3
%float_4 = OpConstant %float 4
%33 = OpConstantComposite %v2float %float_4 %float_4
%int_1 = OpConstant %int 1
%main = OpFunction %void None %11
%12 = OpLabel
%13 = OpVariable %_ptr_Function__arr_v2float_int_2 Function
%30 = OpVariable %_ptr_Function__arr_v2float_int_2 Function
%23 = OpCompositeConstruct %_arr_v2float_int_2 %19 %21
OpStore %13 %23
%25 = OpAccessChain %_ptr_Function_v2float %13 %int_0
%27 = OpLoad %v2float %25
%28 = OpCompositeExtract %float %27 0
%29 = OpCompositeExtract %float %27 1
%35 = OpCompositeConstruct %_arr_v2float_int_2 %31 %33
OpStore %30 %35
%37 = OpAccessChain %_ptr_Function_v2float %30 %int_1
%38 = OpLoad %v2float %37
%39 = OpCompositeExtract %float %38 0
%40 = OpCompositeExtract %float %38 1
%41 = OpCompositeConstruct %v4float %28 %29 %39 %40
OpStore %sk_FragColor %41
OpReturn
OpFunctionEnd
