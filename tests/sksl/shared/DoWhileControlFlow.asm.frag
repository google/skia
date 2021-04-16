OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %34 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%12 = OpTypeFunction %void
%15 = OpTypeFunction %v4float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_3 = OpConstant %float 3
%float_2 = OpConstant %float 2
%float_0 = OpConstant %float 0
%34 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%_entrypoint_v = OpFunction %void None %12
%13 = OpLabel
%14 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %14
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %15
%16 = OpLabel
%x = OpVariable %_ptr_Function_float Function
OpStore %x %float_1
OpBranch %20
%20 = OpLabel
OpLoopMerge %24 %23 None
OpBranch %21
%21 = OpLabel
%25 = OpLoad %float %x
%27 = OpFOrdEqual %bool %25 %float_3
OpSelectionMerge %29 None
OpBranchConditional %27 %28 %29
%28 = OpLabel
OpBranch %23
%29 = OpLabel
OpBranch %22
%22 = OpLabel
OpBranch %23
%23 = OpLabel
%30 = OpLoad %float %x
%32 = OpFOrdEqual %bool %30 %float_2
OpBranchConditional %32 %20 %24
%24 = OpLabel
OpReturnValue %34
OpFunctionEnd
