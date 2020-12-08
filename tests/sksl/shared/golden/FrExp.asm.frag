OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %exp1 "exp1"
OpName %a "a"
OpName %exp3 "exp3"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %31 RelaxedPrecision
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
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
%float_0_5 = OpConstant %float 0.5
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%float_3_5 = OpConstant %float 3.5
%v3float = OpTypeVector %float 3
%27 = OpConstantComposite %v3float %float_3_5 %float_3_5 %float_3_5
%main = OpFunction %void None %11
%12 = OpLabel
%exp1 = OpVariable %_ptr_Function_int Function
%a = OpVariable %_ptr_Function_float Function
%exp3 = OpVariable %_ptr_Function_v3int Function
%18 = OpExtInst %float %1 Frexp %float_0_5 %exp1
OpStore %a %18
%21 = OpLoad %int %exp1
%20 = OpConvertSToF %float %21
%22 = OpCompositeConstruct %v4float %20 %20 %20 %20
OpStore %sk_FragColor %22
%26 = OpExtInst %v3float %1 Frexp %27 %exp3
%30 = OpLoad %v4float %sk_FragColor
%31 = OpVectorShuffle %v4float %30 %26 4 5 6 3
OpStore %sk_FragColor %31
OpReturn
OpFunctionEnd
