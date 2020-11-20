OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %17 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%float_4 = OpConstant %float 4
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%main = OpFunction %void None %11
%12 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%15 = OpExtInst %float %1 Sqrt %float_4
OpStore %x %15
%17 = OpLoad %float %x
%18 = OpCompositeConstruct %v2float %17 %17
%20 = OpCompositeExtract %float %18 0
%21 = OpCompositeExtract %float %18 1
%24 = OpCompositeConstruct %v4float %20 %21 %float_0 %float_1
OpStore %sk_FragColor %24
%25 = OpExtInst %float %1 Sqrt %float_4
%26 = OpCompositeConstruct %v2float %25 %25
%27 = OpCompositeExtract %float %26 0
%28 = OpCompositeExtract %float %26 1
%29 = OpCompositeConstruct %v4float %27 %28 %float_0 %float_1
OpStore %sk_FragColor %29
%30 = OpExtInst %float %1 Sqrt %float_4
%31 = OpCompositeConstruct %v4float %float_0 %30 %float_0 %float_1
OpStore %sk_FragColor %31
%32 = OpExtInst %float %1 Sqrt %float_4
%33 = OpCompositeConstruct %v2float %32 %32
%34 = OpCompositeExtract %float %33 0
%35 = OpCompositeExtract %float %33 1
%36 = OpCompositeConstruct %v3float %34 %35 %float_0
%38 = OpVectorShuffle %v4float %36 %36 2 0 2 1
OpStore %sk_FragColor %38
OpReturn
OpFunctionEnd
