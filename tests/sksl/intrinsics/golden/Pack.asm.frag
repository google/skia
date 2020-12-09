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
OpDecorate %20 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
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
%20 = OpLoad %v2float %a
%19 = OpExtInst %uint %1 PackHalf2x16 %20
%18 = OpConvertUToF %float %19
%22 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %22 %18
%28 = OpLoad %v2float %a
%27 = OpExtInst %uint %1 PackUnorm2x16 %28
%26 = OpConvertUToF %float %27
%29 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %29 %26
%32 = OpLoad %v2float %a
%31 = OpExtInst %uint %1 PackSnorm2x16 %32
%30 = OpConvertUToF %float %31
%33 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %33 %30
%36 = OpLoad %v4float %b
%35 = OpExtInst %uint %1 PackUnorm4x8 %36
%34 = OpConvertUToF %float %35
%37 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %37 %34
%40 = OpLoad %v4float %b
%39 = OpExtInst %uint %1 PackSnorm4x8 %40
%38 = OpConvertUToF %float %39
%41 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %41 %38
OpReturn
OpFunctionEnd
