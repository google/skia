OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %S "S"
OpMemberName %S 0 "rgb"
OpMemberName %S 1 "a"
OpName %s "s"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %_arr_float_int_3 ArrayStride 16
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 0 RelaxedPrecision
OpMemberDecorate %S 1 Offset 48
OpMemberDecorate %S 1 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
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
%S = OpTypeStruct %_arr_float_int_3 %float
%_ptr_Function_S = OpTypePointer Function %S
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
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
%s = OpVariable %_ptr_Function_S Function
%30 = OpAccessChain %_ptr_Function_float %s %int_0 %int_0
OpStore %30 %float_0
%34 = OpAccessChain %_ptr_Function_float %s %int_0 %int_1
OpStore %34 %float_1
%36 = OpAccessChain %_ptr_Function_float %s %int_0 %int_2
OpStore %36 %float_0
%37 = OpAccessChain %_ptr_Function_float %s %int_1
OpStore %37 %float_1
%38 = OpAccessChain %_ptr_Function_float %s %int_0 %int_0
%39 = OpLoad %float %38
%40 = OpAccessChain %_ptr_Function_float %s %int_0 %int_1
%41 = OpLoad %float %40
%42 = OpAccessChain %_ptr_Function_float %s %int_0 %int_2
%43 = OpLoad %float %42
%44 = OpAccessChain %_ptr_Function_float %s %int_1
%45 = OpLoad %float %44
%46 = OpCompositeConstruct %v4float %39 %41 %43 %45
OpReturnValue %46
OpFunctionEnd
