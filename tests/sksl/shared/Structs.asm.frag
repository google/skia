OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %A "A"
OpMemberName %A 0 "x"
OpMemberName %A 1 "y"
OpName %a1 "a1"
OpName %B "B"
OpMemberName %B 0 "x"
OpMemberName %B 1 "y"
OpMemberName %B 2 "z"
OpName %b1 "b1"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %A 0 Offset 0
OpMemberDecorate %A 1 Offset 4
OpDecorate %_arr_float_int_2 ArrayStride 16
OpMemberDecorate %B 0 Offset 0
OpMemberDecorate %B 1 Offset 16
OpMemberDecorate %B 2 Binding 1
OpMemberDecorate %B 2 Offset 48
OpMemberDecorate %B 2 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%A = OpTypeStruct %int %int
%_ptr_Private_A = OpTypePointer Private %A
%a1 = OpVariable %_ptr_Private_A Private
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%B = OpTypeStruct %float %_arr_float_int_2 %A
%_ptr_Private_B = OpTypePointer Private %B
%b1 = OpVariable %_ptr_Private_B Private
%void = OpTypeVoid
%20 = OpTypeFunction %void
%int_0 = OpConstant %int 0
%_ptr_Private_int = OpTypePointer Private %int
%float_0 = OpConstant %float 0
%_ptr_Private_float = OpTypePointer Private %float
%_ptr_Output_float = OpTypePointer Output %float
%main = OpFunction %void None %20
%21 = OpLabel
%23 = OpAccessChain %_ptr_Private_int %a1 %int_0
OpStore %23 %int_0
%26 = OpAccessChain %_ptr_Private_float %b1 %int_0
OpStore %26 %float_0
%28 = OpAccessChain %_ptr_Private_int %a1 %int_0
%29 = OpLoad %int %28
%30 = OpConvertSToF %float %29
%31 = OpAccessChain %_ptr_Private_float %b1 %int_0
%32 = OpLoad %float %31
%33 = OpFAdd %float %30 %32
%34 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %34 %33
OpReturn
OpFunctionEnd
