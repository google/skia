OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %bar "bar"
OpName %y "y"
OpName %z "z"
OpName %main "main"
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%8 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%bar = OpFunction %void None %8
%9 = OpLabel
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_float Function
OpStore %y %float_0
%15 = OpLoad %float %y
OpStore %z %15
OpReturn
OpFunctionEnd
%main = OpFunction %void None %8
%16 = OpLabel
%17 = OpFunctionCall %void %bar
OpReturn
OpFunctionEnd
