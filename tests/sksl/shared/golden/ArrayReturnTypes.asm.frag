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
OpDecorate %_arr_float_int_2 ArrayStride 16
OpDecorate %_arr__arr_float_int_2_int_2 ArrayStride 32
OpDecorate %29 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_arr__arr_float_int_2_int_2 = OpTypeArray %_arr_float_int_2 %int_2
%_ptr_Function__arr__arr_float_int_2_int_2 = OpTypePointer Function %_arr__arr_float_int_2_int_2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%int_1 = OpConstant %int 1
%main = OpFunction %void None %11
%12 = OpLabel
%13 = OpVariable %_ptr_Function__arr__arr_float_int_2_int_2 Function
%30 = OpVariable %_ptr_Function__arr__arr_float_int_2_int_2 Function
%37 = OpVariable %_ptr_Function__arr__arr_float_int_2_int_2 Function
%43 = OpVariable %_ptr_Function__arr__arr_float_int_2_int_2 Function
%21 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
%24 = OpCompositeConstruct %_arr_float_int_2 %float_3 %float_4
%25 = OpCompositeConstruct %_arr__arr_float_int_2_int_2 %21 %24
OpStore %13 %25
%27 = OpAccessChain %_ptr_Function_float %13 %int_0 %int_0
%29 = OpLoad %float %27
%31 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
%32 = OpCompositeConstruct %_arr_float_int_2 %float_3 %float_4
%33 = OpCompositeConstruct %_arr__arr_float_int_2_int_2 %31 %32
OpStore %30 %33
%35 = OpAccessChain %_ptr_Function_float %30 %int_0 %int_1
%36 = OpLoad %float %35
%38 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
%39 = OpCompositeConstruct %_arr_float_int_2 %float_3 %float_4
%40 = OpCompositeConstruct %_arr__arr_float_int_2_int_2 %38 %39
OpStore %37 %40
%41 = OpAccessChain %_ptr_Function_float %37 %int_1 %int_0
%42 = OpLoad %float %41
%44 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
%45 = OpCompositeConstruct %_arr_float_int_2 %float_3 %float_4
%46 = OpCompositeConstruct %_arr__arr_float_int_2_int_2 %44 %45
OpStore %43 %46
%47 = OpAccessChain %_ptr_Function_float %43 %int_1 %int_1
%48 = OpLoad %float %47
%49 = OpCompositeConstruct %v4float %29 %36 %42 %48
OpStore %sk_FragColor %49
OpReturn
OpFunctionEnd
