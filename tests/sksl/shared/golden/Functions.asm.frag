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
OpDecorate %_arr_float_int_3 ArrayStride 16
OpDecorate %_arr__arr_float_int_3_int_2 ArrayStride 48
OpDecorate %_arr_float_int_2 ArrayStride 16
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%int_3 = OpConstant %int 3
%_arr_float_int_3 = OpTypeArray %float %int_3
%int_2 = OpConstant %int 2
%_arr__arr_float_int_3_int_2 = OpTypeArray %_arr_float_int_3 %int_2
%_ptr_Function__arr__arr_float_int_3_int_2 = OpTypePointer Function %_arr__arr_float_int_3_int_2
%18 = OpTypeFunction %float %_ptr_Function__arr__arr_float_int_3_int_2
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%int_1 = OpConstant %int 1
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%31 = OpTypeFunction %float %_ptr_Function__arr_float_int_2
%void = OpTypeVoid
%41 = OpTypeFunction %void %_ptr_Function_float
%float_2 = OpConstant %float 2
%float_123 = OpConstant %float 123
%float_456 = OpConstant %float 456
%65 = OpTypeFunction %void
%float_10 = OpConstant %float 10
%arr = OpFunction %float None %18
%20 = OpFunctionParameter %_ptr_Function__arr__arr_float_int_3_int_2
%21 = OpLabel
%23 = OpAccessChain %_ptr_Function_float %20 %int_0 %int_0
%25 = OpLoad %float %23
%27 = OpAccessChain %_ptr_Function_float %20 %int_1 %int_2
%28 = OpLoad %float %27
%29 = OpFMul %float %25 %28
OpReturnValue %29
OpFunctionEnd
%foo = OpFunction %float None %31
%33 = OpFunctionParameter %_ptr_Function__arr_float_int_2
%34 = OpLabel
%35 = OpAccessChain %_ptr_Function_float %33 %int_0
%36 = OpLoad %float %35
%37 = OpAccessChain %_ptr_Function_float %33 %int_1
%38 = OpLoad %float %37
%39 = OpFMul %float %36 %38
OpReturnValue %39
OpFunctionEnd
%bar = OpFunction %void None %41
%42 = OpFunctionParameter %_ptr_Function_float
%43 = OpLabel
%y = OpVariable %_ptr_Function__arr_float_int_2 Function
%z = OpVariable %_ptr_Function_float Function
%53 = OpVariable %_ptr_Function__arr_float_int_2 Function
%a = OpVariable %_ptr_Function__arr__arr_float_int_3_int_2 Function
%62 = OpVariable %_ptr_Function__arr__arr_float_int_3_int_2 Function
%46 = OpLoad %float %42
%47 = OpAccessChain %_ptr_Function_float %y %int_0
OpStore %47 %46
%48 = OpLoad %float %42
%50 = OpFMul %float %48 %float_2
%51 = OpAccessChain %_ptr_Function_float %y %int_1
OpStore %51 %50
%52 = OpLoad %_arr_float_int_2 %y
OpStore %53 %52
%54 = OpFunctionCall %float %foo %53
OpStore %z %54
%57 = OpAccessChain %_ptr_Function_float %a %int_0 %int_0
OpStore %57 %float_123
%59 = OpAccessChain %_ptr_Function_float %a %int_1 %int_2
OpStore %59 %float_456
%60 = OpLoad %float %z
%61 = OpLoad %_arr__arr_float_int_3_int_2 %a
OpStore %62 %61
%63 = OpFunctionCall %float %arr %62
%64 = OpFAdd %float %60 %63
OpStore %42 %64
OpReturn
OpFunctionEnd
%main = OpFunction %void None %65
%66 = OpLabel
%x = OpVariable %_ptr_Function_float Function
OpStore %x %float_10
%69 = OpFunctionCall %void %bar %x
%70 = OpLoad %float %x
%71 = OpCompositeConstruct %v4float %70 %70 %70 %70
OpStore %sk_FragColor %71
OpReturn
OpFunctionEnd
