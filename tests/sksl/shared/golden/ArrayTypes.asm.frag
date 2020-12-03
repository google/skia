OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %x "x"
OpName %y "y"
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
%int_0 = OpConstant %int 0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%float_2 = OpConstant %float 2
%24 = OpConstantComposite %v2float %float_2 %float_2
%int_1 = OpConstant %int 1
%float_3 = OpConstant %float 3
%29 = OpConstantComposite %v2float %float_3 %float_3
%float_4 = OpConstant %float 4
%32 = OpConstantComposite %v2float %float_4 %float_4
%main = OpFunction %void None %11
%12 = OpLabel
%x = OpVariable %_ptr_Function__arr_v2float_int_2 Function
%y = OpVariable %_ptr_Function__arr_v2float_int_2 Function
%22 = OpAccessChain %_ptr_Function_v2float %x %int_0
OpStore %22 %19
%27 = OpAccessChain %_ptr_Function_v2float %x %int_1
OpStore %27 %24
%31 = OpAccessChain %_ptr_Function_v2float %y %int_0
OpStore %31 %29
%34 = OpAccessChain %_ptr_Function_v2float %y %int_1
OpStore %34 %32
%35 = OpAccessChain %_ptr_Function_v2float %x %int_0
%36 = OpLoad %v2float %35
%37 = OpCompositeExtract %float %36 0
%38 = OpCompositeExtract %float %36 1
%39 = OpAccessChain %_ptr_Function_v2float %y %int_1
%40 = OpLoad %v2float %39
%41 = OpCompositeExtract %float %40 0
%42 = OpCompositeExtract %float %40 1
%43 = OpCompositeConstruct %v4float %37 %38 %41 %42
OpStore %sk_FragColor %43
OpReturn
OpFunctionEnd
