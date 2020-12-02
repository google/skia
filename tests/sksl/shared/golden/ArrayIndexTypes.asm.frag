OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %array "array"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %_arr_float_int_4 ArrayStride 16
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
%int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4
%_ptr_Function__arr_float_int_4 = OpTypePointer Function %_arr_float_int_4
%float_0 = OpConstant %float 0
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%int_1 = OpConstant %int 1
%float_2 = OpConstant %float 2
%int_2 = OpConstant %int 2
%float_3 = OpConstant %float 3
%int_3 = OpConstant %int 3
%uint = OpTypeInt 32 0
%uint_3 = OpConstant %uint 3
%main = OpFunction %void None %11
%12 = OpLabel
%array = OpVariable %_ptr_Function__arr_float_int_4 Function
%20 = OpAccessChain %_ptr_Function_float %array %int_0
OpStore %20 %float_0
%24 = OpAccessChain %_ptr_Function_float %array %int_1
OpStore %24 %float_1
%27 = OpAccessChain %_ptr_Function_float %array %int_2
OpStore %27 %float_2
%30 = OpAccessChain %_ptr_Function_float %array %int_3
OpStore %30 %float_3
%31 = OpAccessChain %_ptr_Function_float %array %int_0
%32 = OpLoad %float %31
%33 = OpAccessChain %_ptr_Function_float %array %int_1
%34 = OpLoad %float %33
%35 = OpAccessChain %_ptr_Function_float %array %int_2
%36 = OpLoad %float %35
%39 = OpAccessChain %_ptr_Function_float %array %uint_3
%40 = OpLoad %float %39
%41 = OpCompositeConstruct %v4float %32 %34 %36 %40
OpStore %sk_FragColor %41
OpReturn
OpFunctionEnd
