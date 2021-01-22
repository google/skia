OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %foo "foo"
OpName %bar "bar"
OpName %y "y"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %_arr_float_int_2 ArrayStride 16
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%15 = OpTypeFunction %float %_ptr_Function__arr_float_int_2
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%int_1 = OpConstant %int 1
%void = OpTypeVoid
%28 = OpTypeFunction %void %_ptr_Function_float
%float_2 = OpConstant %float 2
%41 = OpTypeFunction %void
%float_10 = OpConstant %float 10
%foo = OpFunction %float None %15
%17 = OpFunctionParameter %_ptr_Function__arr_float_int_2
%18 = OpLabel
%20 = OpAccessChain %_ptr_Function_float %17 %int_0
%22 = OpLoad %float %20
%24 = OpAccessChain %_ptr_Function_float %17 %int_1
%25 = OpLoad %float %24
%26 = OpFMul %float %22 %25
OpReturnValue %26
OpFunctionEnd
%bar = OpFunction %void None %28
%29 = OpFunctionParameter %_ptr_Function_float
%30 = OpLabel
%y = OpVariable %_ptr_Function__arr_float_int_2 Function
%39 = OpVariable %_ptr_Function__arr_float_int_2 Function
%32 = OpLoad %float %29
%33 = OpAccessChain %_ptr_Function_float %y %int_0
OpStore %33 %32
%34 = OpLoad %float %29
%36 = OpFMul %float %34 %float_2
%37 = OpAccessChain %_ptr_Function_float %y %int_1
OpStore %37 %36
%38 = OpLoad %_arr_float_int_2 %y
OpStore %39 %38
%40 = OpFunctionCall %float %foo %39
OpReturn
OpFunctionEnd
%main = OpFunction %void None %41
%42 = OpLabel
%x = OpVariable %_ptr_Function_float Function
OpStore %x %float_10
%45 = OpFunctionCall %void %bar %x
%46 = OpLoad %float %x
%47 = OpCompositeConstruct %v4float %46 %46 %46 %46
OpStore %sk_FragColor %47
OpReturn
OpFunctionEnd
