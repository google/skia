OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %foo "foo"
OpName %bar "bar"
OpName %y "y"
OpName %z "z"
OpName %main "main"
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%9 = OpTypeFunction %float %_ptr_Function_float
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%foo = OpFunction %float None %9
%11 = OpFunctionParameter %_ptr_Function_float
%12 = OpLabel
%13 = OpLoad %float %11
OpReturnValue %13
OpFunctionEnd
%bar = OpFunction %void None %15
%16 = OpLabel
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_float Function
%21 = OpVariable %_ptr_Function_float Function
OpStore %y %float_0
%20 = OpLoad %float %y
OpStore %21 %20
%22 = OpFunctionCall %float %foo %21
OpStore %z %22
OpReturn
OpFunctionEnd
%main = OpFunction %void None %15
%23 = OpLabel
%24 = OpFunctionCall %void %bar
OpReturn
OpFunctionEnd
