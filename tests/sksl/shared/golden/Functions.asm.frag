OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %x "x"
OpName %_2_y "_2_y"
OpName %_3_z "_3_z"
OpName %_4_0_foo "_4_0_foo"
OpName %_5_a "_5_a"
OpName %_6_1_arr "_6_1_arr"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %_arr_float_int_2 ArrayStride 16
OpDecorate %_arr__arr_float_int_2_int_3 ArrayStride 32
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%float_10 = OpConstant %float 10
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%int_0 = OpConstant %int 0
%float_20 = OpConstant %float 20
%int_1 = OpConstant %int 1
%int_3 = OpConstant %int 3
%_arr__arr_float_int_2_int_3 = OpTypeArray %_arr_float_int_2 %int_3
%_ptr_Function__arr__arr_float_int_2_int_3 = OpTypePointer Function %_arr__arr_float_int_2_int_3
%float_123 = OpConstant %float 123
%float_456 = OpConstant %float 456
%main = OpFunction %void None %11
%12 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%_2_y = OpVariable %_ptr_Function__arr_float_int_2 Function
%_3_z = OpVariable %_ptr_Function_float Function
%_4_0_foo = OpVariable %_ptr_Function_float Function
%_5_a = OpVariable %_ptr_Function__arr__arr_float_int_2_int_3 Function
%_6_1_arr = OpVariable %_ptr_Function_float Function
OpStore %x %float_10
%23 = OpAccessChain %_ptr_Function_float %_2_y %int_0
OpStore %23 %float_10
%26 = OpAccessChain %_ptr_Function_float %_2_y %int_1
OpStore %26 %float_20
%28 = OpAccessChain %_ptr_Function_float %_2_y %int_0
%29 = OpLoad %float %28
%30 = OpAccessChain %_ptr_Function_float %_2_y %int_1
%31 = OpLoad %float %30
%32 = OpFMul %float %29 %31
OpStore %_4_0_foo %32
%33 = OpLoad %float %_4_0_foo
OpStore %_3_z %33
%39 = OpAccessChain %_ptr_Function_float %_5_a %int_0 %int_0
OpStore %39 %float_123
%41 = OpAccessChain %_ptr_Function_float %_5_a %int_1 %int_2
OpStore %41 %float_456
%43 = OpAccessChain %_ptr_Function_float %_5_a %int_0 %int_0
%44 = OpLoad %float %43
%45 = OpAccessChain %_ptr_Function_float %_5_a %int_1 %int_2
%46 = OpLoad %float %45
%47 = OpFMul %float %44 %46
OpStore %_6_1_arr %47
%48 = OpLoad %float %_3_z
%49 = OpLoad %float %_6_1_arr
%50 = OpFAdd %float %48 %49
OpStore %x %50
%51 = OpLoad %float %x
%52 = OpCompositeConstruct %v4float %51 %51 %51 %51
OpStore %sk_FragColor %52
OpReturn
OpFunctionEnd
