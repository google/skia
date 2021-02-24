OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %_1_y "_1_y"
OpName %_2_foo "_2_foo"
OpName %_3_z "_3_z"
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%false = OpConstantFalse %bool
%main = OpFunction %void None %7
%8 = OpLabel
%_1_y = OpVariable %_ptr_Function_float Function
%_2_foo = OpVariable %_ptr_Function_float Function
%_3_z = OpVariable %_ptr_Function_float Function
OpStore %_1_y %float_0
%15 = OpLoad %float %_1_y
OpStore %_3_z %15
OpReturn
OpFunctionEnd
