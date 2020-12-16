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
OpName %_0_blend_overlay "_0_blend_overlay"
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
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
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
%_0_blend_overlay = OpVariable %_ptr_Function_v4float Function
%_1_result = OpVariable %_ptr_Function_v4float Function
%65 = OpVariable %_ptr_Function_v2float Function
%68 = OpVariable %_ptr_Function_v2float Function
%72 = OpVariable %_ptr_Function_v2float Function
%75 = OpVariable %_ptr_Function_v2float Function
%79 = OpVariable %_ptr_Function_v2float Function
%82 = OpVariable %_ptr_Function_v2float Function
%63 = OpLoad %v4float %src
%64 = OpVectorShuffle %v2float %63 %63 0 3
OpStore %65 %64
%66 = OpLoad %v4float %dst
%67 = OpVectorShuffle %v2float %66 %66 0 3
OpStore %68 %67
%69 = OpFunctionCall %float %_blend_overlay_component %65 %68
%70 = OpLoad %v4float %src
%71 = OpVectorShuffle %v2float %70 %70 1 3
OpStore %72 %71
%73 = OpLoad %v4float %dst
%74 = OpVectorShuffle %v2float %73 %73 1 3
OpStore %75 %74
%76 = OpFunctionCall %float %_blend_overlay_component %72 %75
%77 = OpLoad %v4float %src
%78 = OpVectorShuffle %v2float %77 %77 2 3
OpStore %79 %78
%80 = OpLoad %v4float %dst
%81 = OpVectorShuffle %v2float %80 %80 2 3
OpStore %82 %81
%83 = OpFunctionCall %float %_blend_overlay_component %79 %82
%84 = OpLoad %v4float %src
%85 = OpCompositeExtract %float %84 3
%87 = OpLoad %v4float %src
%88 = OpCompositeExtract %float %87 3
%89 = OpFSub %float %float_1 %88
%90 = OpLoad %v4float %dst
%91 = OpCompositeExtract %float %90 3
%92 = OpFMul %float %89 %91
%93 = OpFAdd %float %85 %92
%94 = OpCompositeConstruct %v4float %69 %76 %83 %93
OpStore %_1_result %94
%95 = OpLoad %v4float %_1_result
%96 = OpVectorShuffle %v3float %95 %95 0 1 2
%98 = OpLoad %v4float %dst
%99 = OpVectorShuffle %v3float %98 %98 0 1 2
%100 = OpLoad %v4float %src
%101 = OpCompositeExtract %float %100 3
%102 = OpFSub %float %float_1 %101
%103 = OpVectorTimesScalar %v3float %99 %102
%104 = OpLoad %v4float %src
%105 = OpVectorShuffle %v3float %104 %104 0 1 2
%106 = OpLoad %v4float %dst
%107 = OpCompositeExtract %float %106 3
%108 = OpFSub %float %float_1 %107
%109 = OpVectorTimesScalar %v3float %105 %108
%110 = OpFAdd %v3float %103 %109
%111 = OpFAdd %v3float %96 %110
%112 = OpLoad %v4float %_1_result
%113 = OpVectorShuffle %v4float %112 %111 4 5 6 3
OpStore %_1_result %113
%114 = OpLoad %v4float %_1_result
OpStore %_0_blend_overlay %114
%115 = OpLoad %v4float %_0_blend_overlay
OpStore %sk_FragColor %115
OpReturn
OpFunctionEnd
