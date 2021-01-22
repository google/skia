OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%7 = OpTypeFunction %void
%true = OpConstantTrue %bool
%main = OpFunction %void None %7
%8 = OpLabel
OpBranch %9
%9 = OpLabel
OpLoopMerge %13 %12 None
OpBranch %10
%10 = OpLabel
OpBranchConditional %true %11 %13
%11 = OpLabel
OpBranch %13
%12 = OpLabel
OpBranch %9
%13 = OpLabel
OpReturn
OpFunctionEnd
