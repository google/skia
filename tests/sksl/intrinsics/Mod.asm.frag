OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %a %b %c %d
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %c RelaxedPrecision
OpDecorate %d RelaxedPrecision
OpDecorate %20 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Input_float = OpTypePointer Input %float
%a = OpVariable %_ptr_Input_float Input
%b = OpVariable %_ptr_Input_float Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%c = OpVariable %_ptr_Input_v4float Input
%d = OpVariable %_ptr_Input_v4float Input
%void = OpTypeVoid
%17 = OpTypeFunction %void
%_ptr_Output_float = OpTypePointer Output %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%main = OpFunction %void None %17
%18 = OpLabel
%20 = OpLoad %float %a
%21 = OpLoad %float %b
%19 = OpFMod %float %20 %21
%22 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %22 %19
%27 = OpLoad %v4float %c
%28 = OpLoad %float %b
%29 = OpCompositeConstruct %v4float %28 %28 %28 %28
%26 = OpFMod %v4float %27 %29
OpStore %sk_FragColor %26
%31 = OpLoad %v4float %c
%32 = OpLoad %v4float %d
%30 = OpFMod %v4float %31 %32
OpStore %sk_FragColor %30
OpReturn
OpFunctionEnd
