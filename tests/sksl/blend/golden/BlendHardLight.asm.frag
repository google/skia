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
OpName %_0_blend_hard_light "_0_blend_hard_light"
OpName %_1_blend_overlay "_1_blend_overlay"
OpName %_2_result "_2_result"
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
OpDecorate %64 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
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
%_0_blend_hard_light = OpVariable %_ptr_Function_v4float Function
%_1_blend_overlay = OpVariable %_ptr_Function_v4float Function
%_2_result = OpVariable %_ptr_Function_v4float Function
%66 = OpVariable %_ptr_Function_v2float Function
%69 = OpVariable %_ptr_Function_v2float Function
%73 = OpVariable %_ptr_Function_v2float Function
%76 = OpVariable %_ptr_Function_v2float Function
%80 = OpVariable %_ptr_Function_v2float Function
%83 = OpVariable %_ptr_Function_v2float Function
%64 = OpLoad %v4float %dst
%65 = OpVectorShuffle %v2float %64 %64 0 3
OpStore %66 %65
%67 = OpLoad %v4float %src
%68 = OpVectorShuffle %v2float %67 %67 0 3
OpStore %69 %68
%70 = OpFunctionCall %float %_blend_overlay_component %66 %69
%71 = OpLoad %v4float %dst
%72 = OpVectorShuffle %v2float %71 %71 1 3
OpStore %73 %72
%74 = OpLoad %v4float %src
%75 = OpVectorShuffle %v2float %74 %74 1 3
OpStore %76 %75
%77 = OpFunctionCall %float %_blend_overlay_component %73 %76
%78 = OpLoad %v4float %dst
%79 = OpVectorShuffle %v2float %78 %78 2 3
OpStore %80 %79
%81 = OpLoad %v4float %src
%82 = OpVectorShuffle %v2float %81 %81 2 3
OpStore %83 %82
%84 = OpFunctionCall %float %_blend_overlay_component %80 %83
%85 = OpLoad %v4float %dst
%86 = OpCompositeExtract %float %85 3
%88 = OpLoad %v4float %dst
%89 = OpCompositeExtract %float %88 3
%90 = OpFSub %float %float_1 %89
%91 = OpLoad %v4float %src
%92 = OpCompositeExtract %float %91 3
%93 = OpFMul %float %90 %92
%94 = OpFAdd %float %86 %93
%95 = OpCompositeConstruct %v4float %70 %77 %84 %94
OpStore %_2_result %95
%96 = OpLoad %v4float %_2_result
%97 = OpVectorShuffle %v3float %96 %96 0 1 2
%99 = OpLoad %v4float %src
%100 = OpVectorShuffle %v3float %99 %99 0 1 2
%101 = OpLoad %v4float %dst
%102 = OpCompositeExtract %float %101 3
%103 = OpFSub %float %float_1 %102
%104 = OpVectorTimesScalar %v3float %100 %103
%105 = OpLoad %v4float %dst
%106 = OpVectorShuffle %v3float %105 %105 0 1 2
%107 = OpLoad %v4float %src
%108 = OpCompositeExtract %float %107 3
%109 = OpFSub %float %float_1 %108
%110 = OpVectorTimesScalar %v3float %106 %109
%111 = OpFAdd %v3float %104 %110
%112 = OpFAdd %v3float %97 %111
%113 = OpLoad %v4float %_2_result
%114 = OpVectorShuffle %v4float %113 %112 4 5 6 3
OpStore %_2_result %114
%115 = OpLoad %v4float %_2_result
OpStore %_1_blend_overlay %115
%116 = OpLoad %v4float %_1_blend_overlay
OpStore %_0_blend_hard_light %116
%117 = OpLoad %v4float %_0_blend_hard_light
OpStore %sk_FragColor %117
OpReturn
OpFunctionEnd
