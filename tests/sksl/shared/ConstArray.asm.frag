OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %test "test"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %test RelaxedPrecision
OpDecorate %_arr_float_int_4 ArrayStride 16
OpDecorate %26 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%void = OpTypeVoid
%21 = OpTypeFunction %void
%int_0 = OpConstant %int 0
%_ptr_Private_float = OpTypePointer Private %float
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%main = OpFunction %void None %21
%22 = OpLabel
%19 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %test %19
%24 = OpAccessChain %_ptr_Private_float %test %int_0
%26 = OpLoad %float %24
%28 = OpAccessChain %_ptr_Private_float %test %int_1
%29 = OpLoad %float %28
%31 = OpAccessChain %_ptr_Private_float %test %int_2
%32 = OpLoad %float %31
%34 = OpAccessChain %_ptr_Private_float %test %int_3
%35 = OpLoad %float %34
%36 = OpCompositeConstruct %v4float %26 %29 %32 %35
OpStore %sk_FragColor %36
OpReturn
OpFunctionEnd
