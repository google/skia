OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %blend_multiply "blend_multiply"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %20 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%void = OpTypeVoid
%54 = OpTypeFunction %void
%blend_multiply = OpFunction %v4float None %14
%16 = OpFunctionParameter %_ptr_Function_v4float
%17 = OpFunctionParameter %_ptr_Function_v4float
%18 = OpLabel
%20 = OpLoad %v4float %16
%21 = OpCompositeExtract %float %20 3
%22 = OpFSub %float %float_1 %21
%23 = OpLoad %v4float %17
%24 = OpVectorShuffle %v3float %23 %23 0 1 2
%26 = OpVectorTimesScalar %v3float %24 %22
%27 = OpLoad %v4float %17
%28 = OpCompositeExtract %float %27 3
%29 = OpFSub %float %float_1 %28
%30 = OpLoad %v4float %16
%31 = OpVectorShuffle %v3float %30 %30 0 1 2
%32 = OpVectorTimesScalar %v3float %31 %29
%33 = OpFAdd %v3float %26 %32
%34 = OpLoad %v4float %16
%35 = OpVectorShuffle %v3float %34 %34 0 1 2
%36 = OpLoad %v4float %17
%37 = OpVectorShuffle %v3float %36 %36 0 1 2
%38 = OpFMul %v3float %35 %37
%39 = OpFAdd %v3float %33 %38
%40 = OpCompositeExtract %float %39 0
%41 = OpCompositeExtract %float %39 1
%42 = OpCompositeExtract %float %39 2
%43 = OpLoad %v4float %16
%44 = OpCompositeExtract %float %43 3
%45 = OpLoad %v4float %16
%46 = OpCompositeExtract %float %45 3
%47 = OpFSub %float %float_1 %46
%48 = OpLoad %v4float %17
%49 = OpCompositeExtract %float %48 3
%50 = OpFMul %float %47 %49
%51 = OpFAdd %float %44 %50
%52 = OpCompositeConstruct %v4float %40 %41 %42 %51
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
%60 = OpFunctionCall %v4float %blend_multiply %57 %59
OpStore %sk_FragColor %60
OpReturn
OpFunctionEnd
