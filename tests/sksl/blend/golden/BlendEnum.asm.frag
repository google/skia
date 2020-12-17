OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %main "main"
OpName %_0_blend "_0_blend"
OpName %_1_loop "_1_loop"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%src = OpVariable %_ptr_Input_v4float Input
%dst = OpVariable %_ptr_Input_v4float Input
%void = OpTypeVoid
%14 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%main = OpFunction %void None %14
%15 = OpLabel
%_0_blend = OpVariable %_ptr_Function_v4float Function
%_1_loop = OpVariable %_ptr_Function_int Function
OpStore %_1_loop %int_0
OpBranch %22
%22 = OpLabel
OpLoopMerge %26 %25 None
OpBranch %23
%23 = OpLabel
%27 = OpLoad %int %_1_loop
%29 = OpSLessThan %bool %27 %int_1
OpBranchConditional %29 %24 %26
%24 = OpLabel
%30 = OpLoad %v4float %src
%31 = OpLoad %v4float %dst
%32 = OpFMul %v4float %30 %31
OpStore %_0_blend %32
OpBranch %25
%25 = OpLabel
%33 = OpLoad %int %_1_loop
%34 = OpIAdd %int %33 %int_1
OpStore %_1_loop %34
OpBranch %22
%26 = OpLabel
%35 = OpLoad %v4float %_0_blend
OpStore %sk_FragColor %35
OpReturn
OpFunctionEnd
