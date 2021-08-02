### Compilation failed:

error: SPIR-V validation error: Member index 0 is missing a location assignment
  %T = OpTypeStruct %int %v2float

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %4 %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %T "T"
OpMemberName %T 0 "x"
OpMemberName %T 1 "u_skRTFlip"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpMemberDecorate %T 0 Offset 0
OpMemberDecorate %T 1 Offset 32
OpDecorate %T Block
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%int = OpTypeInt 32 1
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%T = OpTypeStruct %int %v2float
%_ptr_Input_T = OpTypePointer Input %T
%4 = OpVariable %_ptr_Input_T Input
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%14 = OpTypeFunction %void
%main = OpFunction %void None %14
%15 = OpLabel
OpReturn
OpFunctionEnd

1 error
