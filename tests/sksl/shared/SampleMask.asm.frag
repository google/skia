OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_SampleMask %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_SampleMask "sk_SampleMask"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpDecorate %_arr_int_int_1 ArrayStride 16
OpDecorate %sk_SampleMask BuiltIn SampleMask
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%_arr_int_int_1 = OpTypeArray %int %int_1
%_ptr_Output__arr_int_int_1 = OpTypePointer Output %_arr_int_int_1
%sk_SampleMask = OpVariable %_ptr_Output__arr_int_int_1 Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%12 = OpTypeFunction %void
%int_0 = OpConstant %int 0
%_ptr_Output_int = OpTypePointer Output %int
%int_8 = OpConstant %int 8
%main = OpFunction %void None %12
%13 = OpLabel
%15 = OpAccessChain %_ptr_Output_int %sk_SampleMask %int_0
%17 = OpLoad %int %15
%19 = OpBitwiseOr %int %17 %int_8
OpStore %15 %19
OpReturn
OpFunctionEnd
