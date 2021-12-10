### Compilation failed:

error: SPIR-V validation error: [VUID-StandaloneSpirv-None-04633] OpEntryPoint Entry Point <id> '2[%main]'s function return type is not void.
  OpEntryPoint Fragment %main "main" %sk_Clockwise

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%v3int = OpTypeVector %int 3
%8 = OpTypeFunction %v3int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%13 = OpConstantComposite %v3int %int_1 %int_2 %int_3
%main = OpFunction %v3int None %8
%9 = OpLabel
OpReturnValue %13
OpFunctionEnd

1 error
