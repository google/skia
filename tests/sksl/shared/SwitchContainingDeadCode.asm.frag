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
%float_2 = OpConstant %float 2
%int = OpTypeInt 32 1
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%main = OpFunction %void None %11
%12 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%15 = OpExtInst %float %1 Sqrt %float_2
%17 = OpConvertFToS %int %15
OpSelectionMerge %19 None
OpSwitch %17 %22 0 %20 1 %21
%20 = OpLabel
OpStore %x %float_0
OpBranch %21
%21 = OpLabel
OpStore %x %float_1
OpBranch %22
%22 = OpLabel
OpStore %x %float_2
OpBranch %19
%19 = OpLabel
%25 = OpLoad %float %x
%26 = OpCompositeConstruct %v4float %25 %25 %25 %25
OpStore %sk_FragColor %26
OpReturn
OpFunctionEnd
