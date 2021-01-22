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
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
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
%19 = OpExtInst %float %1 Atan %20
%21 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %21 %19
%26 = OpLoad %float %a
%27 = OpLoad %float %b
%25 = OpExtInst %float %1 Atan2 %26 %27
%28 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %28 %25
%30 = OpLoad %v4float %c
%29 = OpExtInst %v4float %1 Atan %30
OpStore %sk_FragColor %29
%32 = OpLoad %v4float %c
%33 = OpLoad %v4float %d
%31 = OpExtInst %v4float %1 Atan2 %32 %33
OpStore %sk_FragColor %31
OpReturn
OpFunctionEnd
