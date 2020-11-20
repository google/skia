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
OpName %_0_blend_overlay "_0_blend_overlay"
OpName %_1_1_blend_overlay_component "_1_1_blend_overlay_component"
OpName %_2_75_blend_overlay_component "_2_75_blend_overlay_component"
OpName %_3_79_blend_overlay_component "_3_79_blend_overlay_component"
OpName %_4_result "_4_result"
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
OpDecorate %31 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%float_2 = OpConstant %float 2
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%main = OpFunction %void None %14
%15 = OpLabel
%_0_blend_overlay = OpVariable %_ptr_Function_v4float Function
%_1_1_blend_overlay_component = OpVariable %_ptr_Function_float Function
%27 = OpVariable %_ptr_Function_float Function
%_2_75_blend_overlay_component = OpVariable %_ptr_Function_float Function
%63 = OpVariable %_ptr_Function_float Function
%_3_79_blend_overlay_component = OpVariable %_ptr_Function_float Function
%99 = OpVariable %_ptr_Function_float Function
%_4_result = OpVariable %_ptr_Function_v4float Function
%21 = OpLoad %v4float %dst
%22 = OpCompositeExtract %float %21 0
%23 = OpFMul %float %float_2 %22
%24 = OpLoad %v4float %dst
%25 = OpCompositeExtract %float %24 3
%26 = OpFOrdLessThanEqual %bool %23 %25
OpSelectionMerge %30 None
OpBranchConditional %26 %28 %29
%28 = OpLabel
%31 = OpLoad %v4float %src
%32 = OpCompositeExtract %float %31 0
%33 = OpFMul %float %float_2 %32
%34 = OpLoad %v4float %dst
%35 = OpCompositeExtract %float %34 0
%36 = OpFMul %float %33 %35
OpStore %27 %36
OpBranch %30
%29 = OpLabel
%37 = OpLoad %v4float %src
%38 = OpCompositeExtract %float %37 3
%39 = OpLoad %v4float %dst
%40 = OpCompositeExtract %float %39 3
%41 = OpFMul %float %38 %40
%42 = OpLoad %v4float %dst
%43 = OpCompositeExtract %float %42 3
%44 = OpLoad %v4float %dst
%45 = OpCompositeExtract %float %44 0
%46 = OpFSub %float %43 %45
%47 = OpFMul %float %float_2 %46
%48 = OpLoad %v4float %src
%49 = OpCompositeExtract %float %48 3
%50 = OpLoad %v4float %src
%51 = OpCompositeExtract %float %50 0
%52 = OpFSub %float %49 %51
%53 = OpFMul %float %47 %52
%54 = OpFSub %float %41 %53
OpStore %27 %54
OpBranch %30
%30 = OpLabel
%55 = OpLoad %float %27
OpStore %_1_1_blend_overlay_component %55
%57 = OpLoad %v4float %dst
%58 = OpCompositeExtract %float %57 1
%59 = OpFMul %float %float_2 %58
%60 = OpLoad %v4float %dst
%61 = OpCompositeExtract %float %60 3
%62 = OpFOrdLessThanEqual %bool %59 %61
OpSelectionMerge %66 None
OpBranchConditional %62 %64 %65
%64 = OpLabel
%67 = OpLoad %v4float %src
%68 = OpCompositeExtract %float %67 1
%69 = OpFMul %float %float_2 %68
%70 = OpLoad %v4float %dst
%71 = OpCompositeExtract %float %70 1
%72 = OpFMul %float %69 %71
OpStore %63 %72
OpBranch %66
%65 = OpLabel
%73 = OpLoad %v4float %src
%74 = OpCompositeExtract %float %73 3
%75 = OpLoad %v4float %dst
%76 = OpCompositeExtract %float %75 3
%77 = OpFMul %float %74 %76
%78 = OpLoad %v4float %dst
%79 = OpCompositeExtract %float %78 3
%80 = OpLoad %v4float %dst
%81 = OpCompositeExtract %float %80 1
%82 = OpFSub %float %79 %81
%83 = OpFMul %float %float_2 %82
%84 = OpLoad %v4float %src
%85 = OpCompositeExtract %float %84 3
%86 = OpLoad %v4float %src
%87 = OpCompositeExtract %float %86 1
%88 = OpFSub %float %85 %87
%89 = OpFMul %float %83 %88
%90 = OpFSub %float %77 %89
OpStore %63 %90
OpBranch %66
%66 = OpLabel
%91 = OpLoad %float %63
OpStore %_2_75_blend_overlay_component %91
%93 = OpLoad %v4float %dst
%94 = OpCompositeExtract %float %93 2
%95 = OpFMul %float %float_2 %94
%96 = OpLoad %v4float %dst
%97 = OpCompositeExtract %float %96 3
%98 = OpFOrdLessThanEqual %bool %95 %97
OpSelectionMerge %102 None
OpBranchConditional %98 %100 %101
%100 = OpLabel
%103 = OpLoad %v4float %src
%104 = OpCompositeExtract %float %103 2
%105 = OpFMul %float %float_2 %104
%106 = OpLoad %v4float %dst
%107 = OpCompositeExtract %float %106 2
%108 = OpFMul %float %105 %107
OpStore %99 %108
OpBranch %102
%101 = OpLabel
%109 = OpLoad %v4float %src
%110 = OpCompositeExtract %float %109 3
%111 = OpLoad %v4float %dst
%112 = OpCompositeExtract %float %111 3
%113 = OpFMul %float %110 %112
%114 = OpLoad %v4float %dst
%115 = OpCompositeExtract %float %114 3
%116 = OpLoad %v4float %dst
%117 = OpCompositeExtract %float %116 2
%118 = OpFSub %float %115 %117
%119 = OpFMul %float %float_2 %118
%120 = OpLoad %v4float %src
%121 = OpCompositeExtract %float %120 3
%122 = OpLoad %v4float %src
%123 = OpCompositeExtract %float %122 2
%124 = OpFSub %float %121 %123
%125 = OpFMul %float %119 %124
%126 = OpFSub %float %113 %125
OpStore %99 %126
OpBranch %102
%102 = OpLabel
%127 = OpLoad %float %99
OpStore %_3_79_blend_overlay_component %127
%129 = OpLoad %float %_1_1_blend_overlay_component
%130 = OpLoad %float %_2_75_blend_overlay_component
%131 = OpLoad %float %_3_79_blend_overlay_component
%132 = OpLoad %v4float %src
%133 = OpCompositeExtract %float %132 3
%135 = OpLoad %v4float %src
%136 = OpCompositeExtract %float %135 3
%137 = OpFSub %float %float_1 %136
%138 = OpLoad %v4float %dst
%139 = OpCompositeExtract %float %138 3
%140 = OpFMul %float %137 %139
%141 = OpFAdd %float %133 %140
%142 = OpCompositeConstruct %v4float %129 %130 %131 %141
OpStore %_4_result %142
%143 = OpLoad %v4float %_4_result
%144 = OpVectorShuffle %v3float %143 %143 0 1 2
%146 = OpLoad %v4float %dst
%147 = OpVectorShuffle %v3float %146 %146 0 1 2
%148 = OpLoad %v4float %src
%149 = OpCompositeExtract %float %148 3
%150 = OpFSub %float %float_1 %149
%151 = OpVectorTimesScalar %v3float %147 %150
%152 = OpLoad %v4float %src
%153 = OpVectorShuffle %v3float %152 %152 0 1 2
%154 = OpLoad %v4float %dst
%155 = OpCompositeExtract %float %154 3
%156 = OpFSub %float %float_1 %155
%157 = OpVectorTimesScalar %v3float %153 %156
%158 = OpFAdd %v3float %151 %157
%159 = OpFAdd %v3float %144 %158
%160 = OpLoad %v4float %_4_result
%161 = OpVectorShuffle %v4float %160 %159 4 5 6 3
OpStore %_4_result %161
%162 = OpLoad %v4float %_4_result
OpStore %_0_blend_overlay %162
%163 = OpLoad %v4float %_0_blend_overlay
OpStore %sk_FragColor %163
OpReturn
OpFunctionEnd
