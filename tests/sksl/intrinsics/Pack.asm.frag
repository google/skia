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
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %19 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
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
%_ptr_Input_v4float = OpTypePointer Input %v4float
%b = OpVariable %_ptr_Input_v4float Input
%void = OpTypeVoid
%16 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%_ptr_Output_float = OpTypePointer Output %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%main = OpFunction %void None %16
%17 = OpLabel
%19 = OpLoad %v2float %a
%18 = OpExtInst %uint %1 PackHalf2x16 %19
%21 = OpConvertUToF %float %18
%22 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %22 %21
%27 = OpLoad %v2float %a
%26 = OpExtInst %uint %1 PackUnorm2x16 %27
%28 = OpConvertUToF %float %26
%29 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %29 %28
%31 = OpLoad %v2float %a
%30 = OpExtInst %uint %1 PackSnorm2x16 %31
%32 = OpConvertUToF %float %30
%33 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %33 %32
%35 = OpLoad %v4float %b
%34 = OpExtInst %uint %1 PackUnorm4x8 %35
%36 = OpConvertUToF %float %34
%37 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %37 %36
%39 = OpLoad %v4float %b
%38 = OpExtInst %uint %1 PackSnorm4x8 %39
%40 = OpConvertUToF %float %38
%41 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %41 %40
OpReturn
OpFunctionEnd
