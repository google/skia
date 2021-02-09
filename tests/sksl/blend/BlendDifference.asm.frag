OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %blend_difference "blend_difference"
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
OpDecorate %27 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
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
%54 = OpTypeFunction %void
%blend_difference = OpFunction %v4float None %14
%16 = OpFunctionParameter %_ptr_Function_v4float
%17 = OpFunctionParameter %_ptr_Function_v4float
%18 = OpLabel
%19 = OpLoad %v4float %16
%20 = OpVectorShuffle %v3float %19 %19 0 1 2
%22 = OpLoad %v4float %17
%23 = OpVectorShuffle %v3float %22 %22 0 1 2
%24 = OpFAdd %v3float %20 %23
%27 = OpLoad %v4float %16
%28 = OpVectorShuffle %v3float %27 %27 0 1 2
%29 = OpLoad %v4float %17
%30 = OpCompositeExtract %float %29 3
%31 = OpVectorTimesScalar %v3float %28 %30
%32 = OpLoad %v4float %17
%33 = OpVectorShuffle %v3float %32 %32 0 1 2
%34 = OpLoad %v4float %16
%35 = OpCompositeExtract %float %34 3
%36 = OpVectorTimesScalar %v3float %33 %35
%26 = OpExtInst %v3float %1 FMin %31 %36
%37 = OpVectorTimesScalar %v3float %26 %float_2
%38 = OpFSub %v3float %24 %37
%39 = OpCompositeExtract %float %38 0
%40 = OpCompositeExtract %float %38 1
%41 = OpCompositeExtract %float %38 2
%42 = OpLoad %v4float %16
%43 = OpCompositeExtract %float %42 3
%45 = OpLoad %v4float %16
%46 = OpCompositeExtract %float %45 3
%47 = OpFSub %float %float_1 %46
%48 = OpLoad %v4float %17
%49 = OpCompositeExtract %float %48 3
%50 = OpFMul %float %47 %49
%51 = OpFAdd %float %43 %50
%52 = OpCompositeConstruct %v4float %39 %40 %41 %51
OpReturnValue %52
OpFunctionEnd
%main = OpFunction %void None %54
%55 = OpLabel
%57 = OpVariable %_ptr_Function_v4float Function
%59 = OpVariable %_ptr_Function_v4float Function
%56 = OpLoad %v4float %src
OpStore %57 %56
%58 = OpLoad %v4float %dst
OpStore %59 %58
%60 = OpFunctionCall %v4float %blend_difference %57 %59
OpStore %sk_FragColor %60
OpReturn
OpFunctionEnd
