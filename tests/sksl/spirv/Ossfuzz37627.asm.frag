OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%7 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%uint_1 = OpConstant %uint 1
%main = OpFunction %void None %7
%8 = OpLabel
%x = OpVariable %_ptr_Function_uint Function
%14 = OpLoad %uint %x
%15 = OpIAdd %uint %14 %uint_1
OpStore %x %15
%12 = OpSNegate %uint %15
OpReturn
OpFunctionEnd
