OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %rgb "rgb"
OpName %a "a"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %rgb RelaxedPrecision
OpDecorate %_arr_float_int_3 ArrayStride 16
OpDecorate %a RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%12 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%20 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%int_3 = OpConstant %int 3
%_arr_float_int_3 = OpTypeArray %float %int_3
%_ptr_Function__arr_float_int_3 = OpTypePointer Function %_arr_float_int_3
%_ptr_Function_float = OpTypePointer Function %float
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %12
%13 = OpLabel
%17 = OpVariable %_ptr_Function_v2float Function
OpStore %17 %16
%19 = OpFunctionCall %v4float %main %17
OpStore %sk_FragColor %19
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %20
%21 = OpFunctionParameter %_ptr_Function_v2float
%22 = OpLabel
%rgb = OpVariable %_ptr_Function__arr_float_int_3 Function
%a = OpVariable %_ptr_Function_float Function
%31 = OpAccessChain %_ptr_Function_float %rgb %int_0
OpStore %31 %float_0
%34 = OpAccessChain %_ptr_Function_float %rgb %int_1
OpStore %34 %float_1
%36 = OpAccessChain %_ptr_Function_float %rgb %int_2
OpStore %36 %float_0
OpStore %a %float_1
%37 = OpAccessChain %_ptr_Function_float %rgb %int_0
%38 = OpLoad %float %37
%39 = OpAccessChain %_ptr_Function_float %rgb %int_1
%40 = OpLoad %float %39
%41 = OpAccessChain %_ptr_Function_float %rgb %int_2
%42 = OpLoad %float %41
%43 = OpLoad %float %a
%44 = OpCompositeConstruct %v4float %38 %40 %42 %43
OpReturnValue %44
OpFunctionEnd
