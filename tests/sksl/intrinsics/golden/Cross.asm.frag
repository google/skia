OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %a %b
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %a "a"
OpName %b "b"
OpName %main "main"
OpName %_0_cross "_0_cross"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %19 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
%a = OpVariable %_ptr_Input_v2float Input
%b = OpVariable %_ptr_Input_v2float Input
%void = OpTypeVoid
%15 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Output_float = OpTypePointer Output %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%main = OpFunction %void None %15
%16 = OpLabel
%_0_cross = OpVariable %_ptr_Function_float Function
%19 = OpLoad %v2float %a
%20 = OpCompositeExtract %float %19 0
%21 = OpLoad %v2float %b
%22 = OpCompositeExtract %float %21 1
%23 = OpFMul %float %20 %22
%24 = OpLoad %v2float %a
%25 = OpCompositeExtract %float %24 1
%26 = OpLoad %v2float %b
%27 = OpCompositeExtract %float %26 0
%28 = OpFMul %float %25 %27
%29 = OpFSub %float %23 %28
OpStore %_0_cross %29
%30 = OpLoad %float %_0_cross
%31 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %31 %30
OpReturn
OpFunctionEnd
