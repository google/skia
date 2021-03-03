OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %_0_y "_0_y"
OpName %_1_z "_1_z"
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
%_0_y = OpVariable %_ptr_Function_float Function
%_1_z = OpVariable %_ptr_Function_float Function
OpStore %_0_y %float_0
%14 = OpLoad %float %_0_y
OpStore %_1_z %14
OpReturn
OpFunctionEnd
