OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %y "y"
OpName %_0_v "_0_v"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %_arr_float_int_2 ArrayStride 16
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
%int_0 = OpConstant %int 0
%main = OpFunction %void None %7
%8 = OpLabel
%y = OpVariable %_ptr_Function__arr_float_int_2 Function
%_0_v = OpVariable %_ptr_Function__arr_float_int_2 Function
%16 = OpLoad %_arr_float_int_2 %y
OpStore %_0_v %16
%18 = OpAccessChain %_ptr_Function_float %_0_v %int_1
%20 = OpLoad %float %18
%22 = OpAccessChain %_ptr_Function_float %_0_v %int_0
OpStore %22 %20
OpReturn
OpFunctionEnd
