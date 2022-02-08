### Compilation failed:

error: SPIR-V validation error: Member index 0 is missing a location assignment
  %s = OpTypeStruct %int

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %3 %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %s "s"
OpMemberName %s 0 "I"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpMemberDecorate %s 0 Offset 0
OpDecorate %s Block
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%int = OpTypeInt 32 1
%s = OpTypeStruct %int
%_ptr_Input_s = OpTypePointer Input %s
%3 = OpVariable %_ptr_Input_s Input
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%main = OpFunction %void None %11
%12 = OpLabel
OpReturn
OpFunctionEnd

1 error
