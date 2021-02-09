OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %blend_exclusion "blend_exclusion"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %19 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%src = OpVariable %_ptr_Input_v4float Input
%dst = OpVariable %_ptr_Input_v4float Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%14 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%v3float = OpTypeVector %float 3
%float_2 = OpConstant %float 2
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%48 = OpTypeFunction %void
%blend_exclusion = OpFunction %v4float None %14
%16 = OpFunctionParameter %_ptr_Function_v4float
%17 = OpFunctionParameter %_ptr_Function_v4float
%18 = OpLabel
%19 = OpLoad %v4float %17
%20 = OpVectorShuffle %v3float %19 %19 0 1 2
%22 = OpLoad %v4float %16
%23 = OpVectorShuffle %v3float %22 %22 0 1 2
%24 = OpFAdd %v3float %20 %23
%26 = OpLoad %v4float %17
%27 = OpVectorShuffle %v3float %26 %26 0 1 2
%28 = OpVectorTimesScalar %v3float %27 %float_2
%29 = OpLoad %v4float %16
%30 = OpVectorShuffle %v3float %29 %29 0 1 2
%31 = OpFMul %v3float %28 %30
%32 = OpFSub %v3float %24 %31
%33 = OpCompositeExtract %float %32 0
%34 = OpCompositeExtract %float %32 1
%35 = OpCompositeExtract %float %32 2
%36 = OpLoad %v4float %16
%37 = OpCompositeExtract %float %36 3
%39 = OpLoad %v4float %16
%40 = OpCompositeExtract %float %39 3
%41 = OpFSub %float %float_1 %40
%42 = OpLoad %v4float %17
%43 = OpCompositeExtract %float %42 3
%44 = OpFMul %float %41 %43
%45 = OpFAdd %float %37 %44
%46 = OpCompositeConstruct %v4float %33 %34 %35 %45
OpReturnValue %46
OpFunctionEnd
%main = OpFunction %void None %48
%49 = OpLabel
%51 = OpVariable %_ptr_Function_v4float Function
%53 = OpVariable %_ptr_Function_v4float Function
%50 = OpLoad %v4float %src
OpStore %51 %50
%52 = OpLoad %v4float %dst
OpStore %53 %52
%54 = OpFunctionCall %v4float %blend_exclusion %51 %53
OpStore %sk_FragColor %54
OpReturn
OpFunctionEnd
