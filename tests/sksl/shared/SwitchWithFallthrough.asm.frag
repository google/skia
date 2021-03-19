OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
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
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%float_3 = OpConstant %float 3
%int = OpTypeInt 32 1
%float_1 = OpConstant %float 1
%main = OpFunction %void None %11
%12 = OpLabel
%x = OpVariable %_ptr_Function_float Function
OpStore %x %float_0
%16 = OpExtInst %float %1 Sqrt %float_3
%18 = OpConvertFToS %int %16
OpSelectionMerge %20 None
OpSwitch %18 %20 0 %21 1 %22
%21 = OpLabel
OpStore %x %float_0
OpBranch %22
%22 = OpLabel
OpStore %x %float_1
OpBranch %20
%20 = OpLabel
%24 = OpLoad %float %x
%25 = OpCompositeConstruct %v4float %24 %24 %24 %24
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
