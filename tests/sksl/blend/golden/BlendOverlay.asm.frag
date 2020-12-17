OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %_blend_overlay_component "_blend_overlay_component"
OpName %main "main"
OpName %_1_result "_1_result"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%15 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%_ptr_Function_float = OpTypePointer Function %float
%void = OpTypeVoid
%58 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%_blend_overlay_component = OpFunction %float None %15
%17 = OpFunctionParameter %_ptr_Function_v2float
%18 = OpFunctionParameter %_ptr_Function_v2float
%19 = OpLabel
%27 = OpVariable %_ptr_Function_float Function
%21 = OpLoad %v2float %18
%22 = OpCompositeExtract %float %21 0
%23 = OpFMul %float %float_2 %22
%24 = OpLoad %v2float %18
%25 = OpCompositeExtract %float %24 1
%26 = OpFOrdLessThanEqual %bool %23 %25
OpSelectionMerge %31 None
OpBranchConditional %26 %29 %30
%29 = OpLabel
%32 = OpLoad %v2float %17
%33 = OpCompositeExtract %float %32 0
%34 = OpFMul %float %float_2 %33
%35 = OpLoad %v2float %18
%36 = OpCompositeExtract %float %35 0
%37 = OpFMul %float %34 %36
OpStore %27 %37
OpBranch %31
%30 = OpLabel
%38 = OpLoad %v2float %17
%39 = OpCompositeExtract %float %38 1
%40 = OpLoad %v2float %18
%41 = OpCompositeExtract %float %40 1
%42 = OpFMul %float %39 %41
%43 = OpLoad %v2float %18
%44 = OpCompositeExtract %float %43 1
%45 = OpLoad %v2float %18
%46 = OpCompositeExtract %float %45 0
%47 = OpFSub %float %44 %46
%48 = OpFMul %float %float_2 %47
%49 = OpLoad %v2float %17
%50 = OpCompositeExtract %float %49 1
%51 = OpLoad %v2float %17
%52 = OpCompositeExtract %float %51 0
%53 = OpFSub %float %50 %52
%54 = OpFMul %float %48 %53
%55 = OpFSub %float %42 %54
OpStore %27 %55
OpBranch %31
%31 = OpLabel
%56 = OpLoad %float %27
OpReturnValue %56
OpFunctionEnd
%main = OpFunction %void None %58
%59 = OpLabel
%_1_result = OpVariable %_ptr_Function_v4float Function
%64 = OpVariable %_ptr_Function_v2float Function
%67 = OpVariable %_ptr_Function_v2float Function
%71 = OpVariable %_ptr_Function_v2float Function
%74 = OpVariable %_ptr_Function_v2float Function
%78 = OpVariable %_ptr_Function_v2float Function
%81 = OpVariable %_ptr_Function_v2float Function
%62 = OpLoad %v4float %src
%63 = OpVectorShuffle %v2float %62 %62 0 3
OpStore %64 %63
%65 = OpLoad %v4float %dst
%66 = OpVectorShuffle %v2float %65 %65 0 3
OpStore %67 %66
%68 = OpFunctionCall %float %_blend_overlay_component %64 %67
%69 = OpLoad %v4float %src
%70 = OpVectorShuffle %v2float %69 %69 1 3
OpStore %71 %70
%72 = OpLoad %v4float %dst
%73 = OpVectorShuffle %v2float %72 %72 1 3
OpStore %74 %73
%75 = OpFunctionCall %float %_blend_overlay_component %71 %74
%76 = OpLoad %v4float %src
%77 = OpVectorShuffle %v2float %76 %76 2 3
OpStore %78 %77
%79 = OpLoad %v4float %dst
%80 = OpVectorShuffle %v2float %79 %79 2 3
OpStore %81 %80
%82 = OpFunctionCall %float %_blend_overlay_component %78 %81
%83 = OpLoad %v4float %src
%84 = OpCompositeExtract %float %83 3
%86 = OpLoad %v4float %src
%87 = OpCompositeExtract %float %86 3
%88 = OpFSub %float %float_1 %87
%89 = OpLoad %v4float %dst
%90 = OpCompositeExtract %float %89 3
%91 = OpFMul %float %88 %90
%92 = OpFAdd %float %84 %91
%93 = OpCompositeConstruct %v4float %68 %75 %82 %92
OpStore %_1_result %93
%94 = OpLoad %v4float %_1_result
%95 = OpVectorShuffle %v3float %94 %94 0 1 2
%97 = OpLoad %v4float %dst
%98 = OpVectorShuffle %v3float %97 %97 0 1 2
%99 = OpLoad %v4float %src
%100 = OpCompositeExtract %float %99 3
%101 = OpFSub %float %float_1 %100
%102 = OpVectorTimesScalar %v3float %98 %101
%103 = OpLoad %v4float %src
%104 = OpVectorShuffle %v3float %103 %103 0 1 2
%105 = OpLoad %v4float %dst
%106 = OpCompositeExtract %float %105 3
%107 = OpFSub %float %float_1 %106
%108 = OpVectorTimesScalar %v3float %104 %107
%109 = OpFAdd %v3float %102 %108
%110 = OpFAdd %v3float %95 %109
%111 = OpLoad %v4float %_1_result
%112 = OpVectorShuffle %v4float %111 %110 4 5 6 3
OpStore %_1_result %112
%113 = OpLoad %v4float %_1_result
OpStore %sk_FragColor %113
OpReturn
OpFunctionEnd
