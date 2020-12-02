OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %arr "arr"
OpName %foo "foo"
OpName %bar "bar"
OpName %y "y"
OpName %z "z"
OpName %a "a"
OpName %main "main"
OpName %x "x"
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
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%int_3 = OpConstant %int 3
%_arr__arr_float_int_2_int_3 = OpTypeArray %_arr_float_int_2 %int_3
%_ptr_Function__arr__arr_float_int_2_int_3 = OpTypePointer Function %_arr__arr_float_int_2_int_3
%18 = OpTypeFunction %float %_ptr_Function__arr__arr_float_int_2_int_3
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%int_1 = OpConstant %int 1
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%30 = OpTypeFunction %float %_ptr_Function__arr_float_int_2
%void = OpTypeVoid
%40 = OpTypeFunction %void %_ptr_Function_float
%float_2 = OpConstant %float 2
%float_123 = OpConstant %float 123
%float_456 = OpConstant %float 456
%64 = OpTypeFunction %void
%float_10 = OpConstant %float 10
%arr = OpFunction %float None %18
%20 = OpFunctionParameter %_ptr_Function__arr__arr_float_int_2_int_3
%21 = OpLabel
%23 = OpAccessChain %_ptr_Function_float %20 %int_0 %int_0
%25 = OpLoad %float %23
%27 = OpAccessChain %_ptr_Function_float %20 %int_1 %int_2
%28 = OpLoad %float %27
%29 = OpFMul %float %25 %28
OpReturnValue %29
OpFunctionEnd
%foo = OpFunction %float None %30
%32 = OpFunctionParameter %_ptr_Function__arr_float_int_2
%33 = OpLabel
%34 = OpAccessChain %_ptr_Function_float %32 %int_0
%35 = OpLoad %float %34
%36 = OpAccessChain %_ptr_Function_float %32 %int_1
%37 = OpLoad %float %36
%38 = OpFMul %float %35 %37
OpReturnValue %38
OpFunctionEnd
%bar = OpFunction %void None %40
%41 = OpFunctionParameter %_ptr_Function_float
%42 = OpLabel
%y = OpVariable %_ptr_Function__arr_float_int_2 Function
%z = OpVariable %_ptr_Function_float Function
%52 = OpVariable %_ptr_Function__arr_float_int_2 Function
%a = OpVariable %_ptr_Function__arr__arr_float_int_2_int_3 Function
%61 = OpVariable %_ptr_Function__arr__arr_float_int_2_int_3 Function
%45 = OpLoad %float %41
%46 = OpAccessChain %_ptr_Function_float %y %int_0
OpStore %46 %45
%47 = OpLoad %float %41
%49 = OpFMul %float %47 %float_2
%50 = OpAccessChain %_ptr_Function_float %y %int_1
OpStore %50 %49
%51 = OpLoad %_arr_float_int_2 %y
OpStore %52 %51
%53 = OpFunctionCall %float %foo %52
OpStore %z %53
%56 = OpAccessChain %_ptr_Function_float %a %int_0 %int_0
OpStore %56 %float_123
%58 = OpAccessChain %_ptr_Function_float %a %int_1 %int_2
OpStore %58 %float_456
%59 = OpLoad %float %z
%60 = OpLoad %_arr__arr_float_int_2_int_3 %a
OpStore %61 %60
%62 = OpFunctionCall %float %arr %61
%63 = OpFAdd %float %59 %62
OpStore %41 %63
OpReturn
OpFunctionEnd
%main = OpFunction %void None %64
%65 = OpLabel
%x = OpVariable %_ptr_Function_float Function
OpStore %x %float_10
%68 = OpFunctionCall %void %bar %x
%69 = OpLoad %float %x
%70 = OpCompositeConstruct %v4float %69 %69 %69 %69
OpStore %sk_FragColor %70
OpReturn
OpFunctionEnd
