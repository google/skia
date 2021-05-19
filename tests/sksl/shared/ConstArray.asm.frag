OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %test "test"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %test RelaxedPrecision
OpDecorate %_arr_float_int_4 ArrayStride 16
OpDecorate %17 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4
%_ptr_Private__arr_float_int_4 = OpTypePointer Private %_arr_float_int_4
%test = OpVariable %_ptr_Private__arr_float_int_4 Private
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%20 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%27 = OpTypeFunction %v4float %_ptr_Function_v2float
%int_0 = OpConstant %int 0
%_ptr_Private_float = OpTypePointer Private %float
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%_entrypoint_v = OpFunction %void None %20
%21 = OpLabel
%24 = OpVariable %_ptr_Function_v2float Function
OpStore %24 %23
%26 = OpFunctionCall %v4float %main %24
OpStore %sk_FragColor %26
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %27
%28 = OpFunctionParameter %_ptr_Function_v2float
%29 = OpLabel
%17 = OpCompositeConstruct %_arr_float_int_4 %float_0 %float_1 %float_0 %float_1
OpStore %test %17
%31 = OpAccessChain %_ptr_Private_float %test %int_0
%33 = OpLoad %float %31
%35 = OpAccessChain %_ptr_Private_float %test %int_1
%36 = OpLoad %float %35
%38 = OpAccessChain %_ptr_Private_float %test %int_2
%39 = OpLoad %float %38
%41 = OpAccessChain %_ptr_Private_float %test %int_3
%42 = OpLoad %float %41
%43 = OpCompositeConstruct %v4float %33 %36 %39 %42
OpReturnValue %43
OpFunctionEnd
