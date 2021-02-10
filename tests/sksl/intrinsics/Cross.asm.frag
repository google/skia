OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %a %b
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %a "a"
OpName %b "b"
OpName %cross "cross"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %20 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
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
%_ptr_Function_v2float = OpTypePointer Function %v2float
%15 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%void = OpTypeVoid
%32 = OpTypeFunction %void
%_ptr_Output_float = OpTypePointer Output %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%cross = OpFunction %float None %15
%17 = OpFunctionParameter %_ptr_Function_v2float
%18 = OpFunctionParameter %_ptr_Function_v2float
%19 = OpLabel
%20 = OpLoad %v2float %17
%21 = OpCompositeExtract %float %20 0
%22 = OpLoad %v2float %18
%23 = OpCompositeExtract %float %22 1
%24 = OpFMul %float %21 %23
%25 = OpLoad %v2float %17
%26 = OpCompositeExtract %float %25 1
%27 = OpLoad %v2float %18
%28 = OpCompositeExtract %float %27 0
%29 = OpFMul %float %26 %28
%30 = OpFSub %float %24 %29
OpReturnValue %30
OpFunctionEnd
%main = OpFunction %void None %32
%33 = OpLabel
%35 = OpVariable %_ptr_Function_v2float Function
%37 = OpVariable %_ptr_Function_v2float Function
%34 = OpLoad %v2float %a
OpStore %35 %34
%36 = OpLoad %v2float %b
OpStore %37 %36
%38 = OpFunctionCall %float %cross %35 %37
%39 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %39 %38
OpReturn
OpFunctionEnd
