OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %blend_src_over "blend_src_over"
OpName %blend_lighten "blend_lighten"
OpName %result "result"
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
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
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
%15 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%void = OpTypeVoid
%56 = OpTypeFunction %void
%blend_src_over = OpFunction %v4float None %15
%17 = OpFunctionParameter %_ptr_Function_v4float
%18 = OpFunctionParameter %_ptr_Function_v4float
%19 = OpLabel
%20 = OpLoad %v4float %17
%22 = OpLoad %v4float %17
%23 = OpCompositeExtract %float %22 3
%24 = OpFSub %float %float_1 %23
%25 = OpLoad %v4float %18
%26 = OpVectorTimesScalar %v4float %25 %24
%27 = OpFAdd %v4float %20 %26
OpReturnValue %27
OpFunctionEnd
%blend_lighten = OpFunction %v4float None %15
%28 = OpFunctionParameter %_ptr_Function_v4float
%29 = OpFunctionParameter %_ptr_Function_v4float
%30 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%32 = OpLoad %v4float %28
%33 = OpLoad %v4float %28
%34 = OpCompositeExtract %float %33 3
%35 = OpFSub %float %float_1 %34
%36 = OpLoad %v4float %29
%37 = OpVectorTimesScalar %v4float %36 %35
%38 = OpFAdd %v4float %32 %37
OpStore %result %38
%40 = OpLoad %v4float %result
%41 = OpVectorShuffle %v3float %40 %40 0 1 2
%43 = OpLoad %v4float %29
%44 = OpCompositeExtract %float %43 3
%45 = OpFSub %float %float_1 %44
%46 = OpLoad %v4float %28
%47 = OpVectorShuffle %v3float %46 %46 0 1 2
%48 = OpVectorTimesScalar %v3float %47 %45
%49 = OpLoad %v4float %29
%50 = OpVectorShuffle %v3float %49 %49 0 1 2
%51 = OpFAdd %v3float %48 %50
%39 = OpExtInst %v3float %1 FMax %41 %51
%52 = OpLoad %v4float %result
%53 = OpVectorShuffle %v4float %52 %39 4 5 6 3
OpStore %result %53
%54 = OpLoad %v4float %result
OpReturnValue %54
OpFunctionEnd
%main = OpFunction %void None %56
%57 = OpLabel
%59 = OpVariable %_ptr_Function_v4float Function
%61 = OpVariable %_ptr_Function_v4float Function
%58 = OpLoad %v4float %src
OpStore %59 %58
%60 = OpLoad %v4float %dst
OpStore %61 %60
%62 = OpFunctionCall %v4float %blend_lighten %59 %61
OpStore %sk_FragColor %62
OpReturn
OpFunctionEnd
