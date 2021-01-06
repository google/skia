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
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%24 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%main = OpFunction %void None %11
%12 = OpLabel
OpSelectionMerge %15 None
OpSwitch %int_0 %15 0 %16 1 %17
%16 = OpLabel
%19 = OpExtInst %float %1 Sqrt %float_1
%21 = OpFOrdLessThan %bool %float_0 %19
OpSelectionMerge %23 None
OpBranchConditional %21 %22 %23
%22 = OpLabel
OpStore %sk_FragColor %24
OpBranch %15
%23 = OpLabel
OpBranch %17
%17 = OpLabel
OpBranch %15
%15 = OpLabel
OpReturn
OpFunctionEnd
