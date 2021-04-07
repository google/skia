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
OpName %a4 "a4"
OpName %B "B"
OpMemberName %B 0 "x"
OpMemberName %B 1 "y"
OpMemberName %B 2 "z"
OpName %b1 "b1"
OpName %b4 "b4"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %A 0 Offset 0
OpMemberDecorate %A 1 Offset 4
OpMemberDecorate %A 1 RelaxedPrecision
OpDecorate %_arr_float_int_2 ArrayStride 16
OpMemberDecorate %B 0 Offset 0
OpMemberDecorate %B 0 RelaxedPrecision
OpMemberDecorate %B 1 Offset 16
OpMemberDecorate %B 2 Binding 1
OpMemberDecorate %B 2 Offset 48
OpMemberDecorate %B 2 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%A = OpTypeStruct %int %float
%_ptr_Private_A = OpTypePointer Private %A
%a1 = OpVariable %_ptr_Private_A Private
%a4 = OpVariable %_ptr_Private_A Private
%int_1 = OpConstant %int 1
%float_2 = OpConstant %float 2
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%B = OpTypeStruct %float %_arr_float_int_2 %A
%_ptr_Private_B = OpTypePointer Private %B
%b1 = OpVariable %_ptr_Private_B Private
%b4 = OpVariable %_ptr_Private_B Private
%float_1 = OpConstant %float 1
%float_3 = OpConstant %float 3
%int_4 = OpConstant %int 4
%float_5 = OpConstant %float 5
%void = OpTypeVoid
%32 = OpTypeFunction %void
%int_0 = OpConstant %int 0
%_ptr_Private_int = OpTypePointer Private %int
%float_0 = OpConstant %float 0
%_ptr_Private_float = OpTypePointer Private %float
%_ptr_Function_A = OpTypePointer Function %A
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Output_float = OpTypePointer Output %float
%main = OpFunction %void None %32
%33 = OpLabel
%52 = OpVariable %_ptr_Function_A Function
%17 = OpCompositeConstruct %A %int_1 %float_2
OpStore %a4 %17
%26 = OpCompositeConstruct %_arr_float_int_2 %float_2 %float_3
%29 = OpCompositeConstruct %A %int_4 %float_5
%30 = OpCompositeConstruct %B %float_1 %26 %29
OpStore %b4 %30
%35 = OpAccessChain %_ptr_Private_int %a1 %int_0
OpStore %35 %int_0
%38 = OpAccessChain %_ptr_Private_float %b1 %int_0
OpStore %38 %float_0
%40 = OpAccessChain %_ptr_Private_int %a1 %int_0
%41 = OpLoad %int %40
%42 = OpConvertSToF %float %41
%43 = OpAccessChain %_ptr_Private_float %b1 %int_0
%44 = OpLoad %float %43
%45 = OpFAdd %float %42 %44
%46 = OpAccessChain %_ptr_Private_float %a4 %int_1
%47 = OpLoad %float %46
%48 = OpFAdd %float %45 %47
%49 = OpAccessChain %_ptr_Private_float %b4 %int_0
%50 = OpLoad %float %49
%51 = OpFAdd %float %48 %50
%54 = OpCompositeConstruct %A %int_1 %float_2
OpStore %52 %54
%55 = OpAccessChain %_ptr_Function_float %52 %int_1
%57 = OpLoad %float %55
%58 = OpFAdd %float %51 %57
%59 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %59 %58
OpReturn
OpFunctionEnd
