OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %__device_FragCoord %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %__device_FragCoord "__device_FragCoord"
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_RTAdjust "sk_RTAdjust"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %__device_FragCoord BuiltIn FragCoord
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%__device_FragCoord = OpVariable %_ptr_Input_v4float Input
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Private_v4float = OpTypePointer Private %v4float
%sk_RTAdjust = OpVariable %_ptr_Private_v4float Private
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_2 = OpConstant %float 2
%v2float = OpTypeVector %float 2
%main = OpFunction %void None %15
%16 = OpLabel
%17 = OpLoad %v4float %__device_FragCoord
%18 = OpCompositeExtract %float %17 0
%20 = OpLoad %v4float %sk_RTAdjust
%21 = OpCompositeExtract %float %20 3
%22 = OpFMul %float %float_2 %21
%23 = OpLoad %v4float %__device_FragCoord
%24 = OpCompositeExtract %float %23 1
%25 = OpFSub %float %22 %24
%26 = OpCompositeConstruct %v2float %18 %25
%28 = OpLoad %v4float %sk_FragColor
%29 = OpVectorShuffle %v4float %28 %26 4 5 2 3
OpStore %sk_FragColor %29
OpReturn
OpFunctionEnd
