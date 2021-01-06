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
%void = OpTypeVoid
%11 = OpTypeFunction %void
%float_2 = OpConstant %float 2
%int = OpTypeInt 32 1
%21 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%main = OpFunction %void None %11
%12 = OpLabel
%14 = OpExtInst %float %1 Sqrt %float_2
%13 = OpConvertFToS %int %14
OpSelectionMerge %17 None
OpSwitch %13 %20 0 %18 1 %19
%18 = OpLabel
OpBranch %19
%19 = OpLabel
OpBranch %20
%20 = OpLabel
OpBranch %17
%17 = OpLabel
OpStore %sk_FragColor %21
OpReturn
OpFunctionEnd
