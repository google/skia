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
OpDecorate %_arr_float_int_3 ArrayStride 16
OpDecorate %_arr__arr_float_int_3_int_2 ArrayStride 48
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
%_arr_float_int_3 = OpTypeArray %float %int_3
%_arr__arr_float_int_3_int_2 = OpTypeArray %_arr_float_int_3 %int_2
%_ptr_Function__arr__arr_float_int_3_int_2 = OpTypePointer Function %_arr__arr_float_int_3_int_2
%float_123 = OpConstant %float 123
%float_456 = OpConstant %float 456
%main = OpFunction %void None %11
%12 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%_2_y = OpVariable %_ptr_Function__arr_float_int_2 Function
%_3_z = OpVariable %_ptr_Function_float Function
%_4_0_foo = OpVariable %_ptr_Function_float Function
%_5_a = OpVariable %_ptr_Function__arr__arr_float_int_3_int_2 Function
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
%40 = OpAccessChain %_ptr_Function_float %_5_a %int_0 %int_0
OpStore %40 %float_123
%42 = OpAccessChain %_ptr_Function_float %_5_a %int_1 %int_2
OpStore %42 %float_456
%44 = OpAccessChain %_ptr_Function_float %_5_a %int_0 %int_0
%45 = OpLoad %float %44
%46 = OpAccessChain %_ptr_Function_float %_5_a %int_1 %int_2
%47 = OpLoad %float %46
%48 = OpFMul %float %45 %47
OpStore %_6_1_arr %48
%49 = OpLoad %float %_3_z
%50 = OpLoad %float %_6_1_arr
%51 = OpFAdd %float %49 %50
OpStore %x %51
%52 = OpLoad %float %x
%53 = OpCompositeConstruct %v4float %52 %52 %52 %52
OpStore %sk_FragColor %53
OpReturn
OpFunctionEnd
