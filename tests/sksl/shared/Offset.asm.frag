OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %Test "Test"
OpMemberName %Test 0 "x"
OpMemberName %Test 1 "y"
OpMemberName %Test 2 "z"
OpName %t "t"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %Test 0 Offset 0
OpMemberDecorate %Test 1 Offset 4
OpMemberDecorate %Test 2 Offset 8
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
%Test = OpTypeStruct %int %int %int
%_ptr_Function_Test = OpTypePointer Function %Test
%int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Output_float = OpTypePointer Output %float
%main = OpFunction %void None %11
%12 = OpLabel
%t = OpVariable %_ptr_Function_Test Function
%18 = OpAccessChain %_ptr_Function_int %t %int_0
OpStore %18 %int_0
%20 = OpAccessChain %_ptr_Function_int %t %int_0
%21 = OpLoad %int %20
%22 = OpConvertSToF %float %21
%23 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %23 %22
OpReturn
OpFunctionEnd
