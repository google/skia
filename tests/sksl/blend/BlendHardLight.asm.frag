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
OpName %blend_overlay "blend_overlay"
OpName %result "result"
OpName %blend_hard_light "blend_hard_light"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
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
%17 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%59 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%void = OpTypeVoid
%126 = OpTypeFunction %void
%_blend_overlay_component = OpFunction %float None %17
%19 = OpFunctionParameter %_ptr_Function_v2float
%20 = OpFunctionParameter %_ptr_Function_v2float
%21 = OpLabel
%29 = OpVariable %_ptr_Function_float Function
%23 = OpLoad %v2float %20
%24 = OpCompositeExtract %float %23 0
%25 = OpFMul %float %float_2 %24
%26 = OpLoad %v2float %20
%27 = OpCompositeExtract %float %26 1
%28 = OpFOrdLessThanEqual %bool %25 %27
OpSelectionMerge %33 None
OpBranchConditional %28 %31 %32
%31 = OpLabel
%34 = OpLoad %v2float %19
%35 = OpCompositeExtract %float %34 0
%36 = OpFMul %float %float_2 %35
%37 = OpLoad %v2float %20
%38 = OpCompositeExtract %float %37 0
%39 = OpFMul %float %36 %38
OpStore %29 %39
OpBranch %33
%32 = OpLabel
%40 = OpLoad %v2float %19
%41 = OpCompositeExtract %float %40 1
%42 = OpLoad %v2float %20
%43 = OpCompositeExtract %float %42 1
%44 = OpFMul %float %41 %43
%45 = OpLoad %v2float %20
%46 = OpCompositeExtract %float %45 1
%47 = OpLoad %v2float %20
%48 = OpCompositeExtract %float %47 0
%49 = OpFSub %float %46 %48
%50 = OpFMul %float %float_2 %49
%51 = OpLoad %v2float %19
%52 = OpCompositeExtract %float %51 1
%53 = OpLoad %v2float %19
%54 = OpCompositeExtract %float %53 0
%55 = OpFSub %float %52 %54
%56 = OpFMul %float %50 %55
%57 = OpFSub %float %44 %56
OpStore %29 %57
OpBranch %33
%33 = OpLabel
%58 = OpLoad %float %29
OpReturnValue %58
OpFunctionEnd
%blend_overlay = OpFunction %v4float None %59
%61 = OpFunctionParameter %_ptr_Function_v4float
%62 = OpFunctionParameter %_ptr_Function_v4float
%63 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%67 = OpVariable %_ptr_Function_v2float Function
%70 = OpVariable %_ptr_Function_v2float Function
%74 = OpVariable %_ptr_Function_v2float Function
%77 = OpVariable %_ptr_Function_v2float Function
%81 = OpVariable %_ptr_Function_v2float Function
%84 = OpVariable %_ptr_Function_v2float Function
%65 = OpLoad %v4float %61
%66 = OpVectorShuffle %v2float %65 %65 0 3
OpStore %67 %66
%68 = OpLoad %v4float %62
%69 = OpVectorShuffle %v2float %68 %68 0 3
OpStore %70 %69
%71 = OpFunctionCall %float %_blend_overlay_component %67 %70
%72 = OpLoad %v4float %61
%73 = OpVectorShuffle %v2float %72 %72 1 3
OpStore %74 %73
%75 = OpLoad %v4float %62
%76 = OpVectorShuffle %v2float %75 %75 1 3
OpStore %77 %76
%78 = OpFunctionCall %float %_blend_overlay_component %74 %77
%79 = OpLoad %v4float %61
%80 = OpVectorShuffle %v2float %79 %79 2 3
OpStore %81 %80
%82 = OpLoad %v4float %62
%83 = OpVectorShuffle %v2float %82 %82 2 3
OpStore %84 %83
%85 = OpFunctionCall %float %_blend_overlay_component %81 %84
%86 = OpLoad %v4float %61
%87 = OpCompositeExtract %float %86 3
%89 = OpLoad %v4float %61
%90 = OpCompositeExtract %float %89 3
%91 = OpFSub %float %float_1 %90
%92 = OpLoad %v4float %62
%93 = OpCompositeExtract %float %92 3
%94 = OpFMul %float %91 %93
%95 = OpFAdd %float %87 %94
%96 = OpCompositeConstruct %v4float %71 %78 %85 %95
OpStore %result %96
%97 = OpLoad %v4float %result
%98 = OpVectorShuffle %v3float %97 %97 0 1 2
%100 = OpLoad %v4float %62
%101 = OpVectorShuffle %v3float %100 %100 0 1 2
%102 = OpLoad %v4float %61
%103 = OpCompositeExtract %float %102 3
%104 = OpFSub %float %float_1 %103
%105 = OpVectorTimesScalar %v3float %101 %104
%106 = OpLoad %v4float %61
%107 = OpVectorShuffle %v3float %106 %106 0 1 2
%108 = OpLoad %v4float %62
%109 = OpCompositeExtract %float %108 3
%110 = OpFSub %float %float_1 %109
%111 = OpVectorTimesScalar %v3float %107 %110
%112 = OpFAdd %v3float %105 %111
%113 = OpFAdd %v3float %98 %112
%114 = OpLoad %v4float %result
%115 = OpVectorShuffle %v4float %114 %113 4 5 6 3
OpStore %result %115
%116 = OpLoad %v4float %result
OpReturnValue %116
OpFunctionEnd
%blend_hard_light = OpFunction %v4float None %59
%117 = OpFunctionParameter %_ptr_Function_v4float
%118 = OpFunctionParameter %_ptr_Function_v4float
%119 = OpLabel
%121 = OpVariable %_ptr_Function_v4float Function
%123 = OpVariable %_ptr_Function_v4float Function
%120 = OpLoad %v4float %118
OpStore %121 %120
%122 = OpLoad %v4float %117
OpStore %123 %122
%124 = OpFunctionCall %v4float %blend_overlay %121 %123
OpReturnValue %124
OpFunctionEnd
%main = OpFunction %void None %126
%127 = OpLabel
%129 = OpVariable %_ptr_Function_v4float Function
%131 = OpVariable %_ptr_Function_v4float Function
%128 = OpLoad %v4float %src
OpStore %129 %128
%130 = OpLoad %v4float %dst
OpStore %131 %130
%132 = OpFunctionCall %v4float %blend_hard_light %129 %131
OpStore %sk_FragColor %132
OpReturn
OpFunctionEnd
