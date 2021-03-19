OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %b "b"
OpName %c "c"
OpName %d "d"
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
%12 = OpTypeFunction %void
%15 = OpTypeFunction %v4float
%_ptr_Function_float = OpTypePointer Function %float
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%float_5 = OpConstant %float 5
%float_4 = OpConstant %float 4
%_entrypoint = OpFunction %void None %12
%13 = OpLabel
%14 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %14
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %15
%16 = OpLabel
%b = OpVariable %_ptr_Function_float Function
%c = OpVariable %_ptr_Function_float Function
%d = OpVariable %_ptr_Function_float Function
OpStore %b %float_2
OpStore %c %float_3
%23 = OpLoad %float %c
OpStore %d %23
%24 = OpLoad %float %b
%26 = OpFAdd %float %24 %float_1
OpStore %b %26
%27 = OpLoad %float %d
%28 = OpFAdd %float %27 %float_1
OpStore %d %28
%29 = OpLoad %float %b
%30 = OpFOrdEqual %bool %29 %float_2
%31 = OpSelect %float %30 %float_1 %float_0
%33 = OpLoad %float %b
%34 = OpFOrdEqual %bool %33 %float_3
%35 = OpSelect %float %34 %float_1 %float_0
%36 = OpLoad %float %d
%38 = OpFOrdEqual %bool %36 %float_5
%39 = OpSelect %float %38 %float_1 %float_0
%40 = OpLoad %float %d
%42 = OpFOrdEqual %bool %40 %float_4
%43 = OpSelect %float %42 %float_1 %float_0
%44 = OpCompositeConstruct %v4float %31 %35 %39 %43
OpReturnValue %44
OpFunctionEnd
