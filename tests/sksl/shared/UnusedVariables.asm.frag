OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpName %e "e"
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
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
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
%a = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
%c = OpVariable %_ptr_Function_float Function
%d = OpVariable %_ptr_Function_float Function
%e = OpVariable %_ptr_Function_float Function
OpStore %a %float_1
OpStore %b %float_2
OpStore %c %float_3
%25 = OpLoad %float %c
OpStore %d %25
%27 = OpLoad %float %d
OpStore %e %27
%28 = OpLoad %float %b
%29 = OpFAdd %float %28 %float_1
OpStore %b %29
%30 = OpLoad %float %d
%31 = OpFAdd %float %30 %float_1
OpStore %d %31
%32 = OpLoad %float %b
%33 = OpFOrdEqual %bool %32 %float_2
%34 = OpSelect %float %33 %float_1 %float_0
%36 = OpLoad %float %b
%37 = OpFOrdEqual %bool %36 %float_3
%38 = OpSelect %float %37 %float_1 %float_0
%39 = OpLoad %float %d
%41 = OpFOrdEqual %bool %39 %float_5
%42 = OpSelect %float %41 %float_1 %float_0
%43 = OpLoad %float %d
%45 = OpFOrdEqual %bool %43 %float_4
%46 = OpSelect %float %45 %float_1 %float_0
%47 = OpCompositeConstruct %v4float %34 %38 %42 %46
OpReturnValue %47
OpFunctionEnd
