### Compilation failed:

error: SPIR-V validation error: OpEntryPoint Entry Point <id> '2[%main]'s function return type is not void.
  OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%v3int = OpTypeVector %int 3
%12 = OpTypeFunction %v3int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%17 = OpConstantComposite %v3int %int_1 %int_2 %int_3
%main = OpFunction %v3int None %12
%13 = OpLabel
OpReturnValue %17
OpFunctionEnd

1 error
