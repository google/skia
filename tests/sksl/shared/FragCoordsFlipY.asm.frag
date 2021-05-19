### Compilation failed:

error: SPIR-V validation error: [VUID-FragCoord-FragCoord-04211] Vulkan spec allows BuiltIn FragCoord to be only used for variables with Input storage class. ID <7> (OpVariable) is referencing ID <7> (OpVariable) which is decorated with BuiltIn FragCoord. ID <7> (OpVariable) uses storage class Private.
  %__device_FragCoord = OpVariable %_ptr_Private_v4float Private

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
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
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Private_v4float = OpTypePointer Private %v4float
%__device_FragCoord = OpVariable %_ptr_Private_v4float Private
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%sk_RTAdjust = OpVariable %_ptr_Private_v4float Private
%void = OpTypeVoid
%14 = OpTypeFunction %void
%float_2 = OpConstant %float 2
%v2float = OpTypeVector %float 2
%main = OpFunction %void None %14
%15 = OpLabel
%16 = OpLoad %v4float %__device_FragCoord
%17 = OpCompositeExtract %float %16 0
%19 = OpLoad %v4float %sk_RTAdjust
%20 = OpCompositeExtract %float %19 3
%21 = OpFMul %float %float_2 %20
%22 = OpLoad %v4float %__device_FragCoord
%23 = OpCompositeExtract %float %22 1
%24 = OpFSub %float %21 %23
%25 = OpCompositeConstruct %v2float %17 %24
%27 = OpLoad %v4float %sk_FragColor
%28 = OpVectorShuffle %v4float %27 %25 4 5 2 3
OpStore %sk_FragColor %28
OpReturn
OpFunctionEnd

1 error
