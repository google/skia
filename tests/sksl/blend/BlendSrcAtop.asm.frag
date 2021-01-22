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
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %16 RelaxedPrecision
OpDecorate %18 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%main = OpFunction %void None %14
%15 = OpLabel
%16 = OpLoad %v4float %dst
%17 = OpCompositeExtract %float %16 3
%18 = OpLoad %v4float %src
%19 = OpVectorTimesScalar %v4float %18 %17
%21 = OpLoad %v4float %src
%22 = OpCompositeExtract %float %21 3
%23 = OpFSub %float %float_1 %22
%24 = OpLoad %v4float %dst
%25 = OpVectorTimesScalar %v4float %24 %23
%26 = OpFAdd %v4float %19 %25
OpStore %sk_FragColor %26
OpReturn
OpFunctionEnd
