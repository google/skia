OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %foo_ff "foo_ff"
OpName %bar_v "bar_v"
OpName %y "y"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%10 = OpTypeFunction %float %_ptr_Function_float
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%foo_ff = OpFunction %float None %10
%11 = OpFunctionParameter %_ptr_Function_float
%12 = OpLabel
%13 = OpLoad %float %11
OpReturnValue %13
OpFunctionEnd
%bar_v = OpFunction %void None %15
%16 = OpLabel
%y = OpVariable %_ptr_Function_float Function
%19 = OpVariable %_ptr_Function_float Function
OpStore %y %float_0
OpStore %19 %float_0
%20 = OpFunctionCall %float %foo_ff %19
OpReturn
OpFunctionEnd
%main = OpFunction %void None %15
%21 = OpLabel
%22 = OpFunctionCall %void %bar_v
OpReturn
OpFunctionEnd
