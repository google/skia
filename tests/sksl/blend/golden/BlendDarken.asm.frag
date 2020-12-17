OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %main "main"
OpName %_1_result "_1_result"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %18 RelaxedPrecision
OpDecorate %20 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
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
%void = OpTypeVoid
%14 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%main = OpFunction %void None %14
%15 = OpLabel
%_1_result = OpVariable %_ptr_Function_v4float Function
%18 = OpLoad %v4float %src
%20 = OpLoad %v4float %src
%21 = OpCompositeExtract %float %20 3
%22 = OpFSub %float %float_1 %21
%23 = OpLoad %v4float %dst
%24 = OpVectorTimesScalar %v4float %23 %22
%25 = OpFAdd %v4float %18 %24
OpStore %_1_result %25
%27 = OpLoad %v4float %_1_result
%28 = OpVectorShuffle %v3float %27 %27 0 1 2
%30 = OpLoad %v4float %dst
%31 = OpCompositeExtract %float %30 3
%32 = OpFSub %float %float_1 %31
%33 = OpLoad %v4float %src
%34 = OpVectorShuffle %v3float %33 %33 0 1 2
%35 = OpVectorTimesScalar %v3float %34 %32
%36 = OpLoad %v4float %dst
%37 = OpVectorShuffle %v3float %36 %36 0 1 2
%38 = OpFAdd %v3float %35 %37
%26 = OpExtInst %v3float %1 FMin %28 %38
%39 = OpLoad %v4float %_1_result
%40 = OpVectorShuffle %v4float %39 %26 4 5 6 3
OpStore %_1_result %40
%41 = OpLoad %v4float %_1_result
OpStore %sk_FragColor %41
OpReturn
OpFunctionEnd
