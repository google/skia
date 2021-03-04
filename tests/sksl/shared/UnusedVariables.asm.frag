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
OpName %d "d"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
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
%d = OpVariable %_ptr_Function_float Function
OpStore %b %float_2
OpStore %d %float_3
%22 = OpLoad %float %b
%24 = OpFAdd %float %22 %float_1
OpStore %b %24
%25 = OpLoad %float %d
%26 = OpFAdd %float %25 %float_1
OpStore %d %26
%27 = OpLoad %float %b
%28 = OpFOrdEqual %bool %27 %float_2
%29 = OpSelect %float %28 %float_1 %float_0
%31 = OpLoad %float %b
%32 = OpFOrdEqual %bool %31 %float_3
%33 = OpSelect %float %32 %float_1 %float_0
%34 = OpLoad %float %d
%36 = OpFOrdEqual %bool %34 %float_5
%37 = OpSelect %float %36 %float_1 %float_0
%38 = OpLoad %float %d
%40 = OpFOrdEqual %bool %38 %float_4
%41 = OpSelect %float %40 %float_1 %float_0
%42 = OpCompositeConstruct %v4float %29 %33 %37 %41
OpReturnValue %42
OpFunctionEnd
