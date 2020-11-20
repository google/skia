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
OpName %_0_blend_hard_light "_0_blend_hard_light"
OpName %_1_8_blend_overlay "_1_8_blend_overlay"
OpName %_2_9_1_blend_overlay_component "_2_9_1_blend_overlay_component"
OpName %_3_76_blend_overlay_component "_3_76_blend_overlay_component"
OpName %_4_80_blend_overlay_component "_4_80_blend_overlay_component"
OpName %_5_10_result "_5_10_result"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
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
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
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
%_0_blend_hard_light = OpVariable %_ptr_Function_v4float Function
%_1_8_blend_overlay = OpVariable %_ptr_Function_v4float Function
%_2_9_1_blend_overlay_component = OpVariable %_ptr_Function_float Function
%28 = OpVariable %_ptr_Function_float Function
%_3_76_blend_overlay_component = OpVariable %_ptr_Function_float Function
%64 = OpVariable %_ptr_Function_float Function
%_4_80_blend_overlay_component = OpVariable %_ptr_Function_float Function
%100 = OpVariable %_ptr_Function_float Function
%_5_10_result = OpVariable %_ptr_Function_v4float Function
%22 = OpLoad %v4float %src
%23 = OpCompositeExtract %float %22 0
%24 = OpFMul %float %float_2 %23
%25 = OpLoad %v4float %src
%26 = OpCompositeExtract %float %25 3
%27 = OpFOrdLessThanEqual %bool %24 %26
OpSelectionMerge %31 None
OpBranchConditional %27 %29 %30
%29 = OpLabel
%32 = OpLoad %v4float %dst
%33 = OpCompositeExtract %float %32 0
%34 = OpFMul %float %float_2 %33
%35 = OpLoad %v4float %src
%36 = OpCompositeExtract %float %35 0
%37 = OpFMul %float %34 %36
OpStore %28 %37
OpBranch %31
%30 = OpLabel
%38 = OpLoad %v4float %dst
%39 = OpCompositeExtract %float %38 3
%40 = OpLoad %v4float %src
%41 = OpCompositeExtract %float %40 3
%42 = OpFMul %float %39 %41
%43 = OpLoad %v4float %src
%44 = OpCompositeExtract %float %43 3
%45 = OpLoad %v4float %src
%46 = OpCompositeExtract %float %45 0
%47 = OpFSub %float %44 %46
%48 = OpFMul %float %float_2 %47
%49 = OpLoad %v4float %dst
%50 = OpCompositeExtract %float %49 3
%51 = OpLoad %v4float %dst
%52 = OpCompositeExtract %float %51 0
%53 = OpFSub %float %50 %52
%54 = OpFMul %float %48 %53
%55 = OpFSub %float %42 %54
OpStore %28 %55
OpBranch %31
%31 = OpLabel
%56 = OpLoad %float %28
OpStore %_2_9_1_blend_overlay_component %56
%58 = OpLoad %v4float %src
%59 = OpCompositeExtract %float %58 1
%60 = OpFMul %float %float_2 %59
%61 = OpLoad %v4float %src
%62 = OpCompositeExtract %float %61 3
%63 = OpFOrdLessThanEqual %bool %60 %62
OpSelectionMerge %67 None
OpBranchConditional %63 %65 %66
%65 = OpLabel
%68 = OpLoad %v4float %dst
%69 = OpCompositeExtract %float %68 1
%70 = OpFMul %float %float_2 %69
%71 = OpLoad %v4float %src
%72 = OpCompositeExtract %float %71 1
%73 = OpFMul %float %70 %72
OpStore %64 %73
OpBranch %67
%66 = OpLabel
%74 = OpLoad %v4float %dst
%75 = OpCompositeExtract %float %74 3
%76 = OpLoad %v4float %src
%77 = OpCompositeExtract %float %76 3
%78 = OpFMul %float %75 %77
%79 = OpLoad %v4float %src
%80 = OpCompositeExtract %float %79 3
%81 = OpLoad %v4float %src
%82 = OpCompositeExtract %float %81 1
%83 = OpFSub %float %80 %82
%84 = OpFMul %float %float_2 %83
%85 = OpLoad %v4float %dst
%86 = OpCompositeExtract %float %85 3
%87 = OpLoad %v4float %dst
%88 = OpCompositeExtract %float %87 1
%89 = OpFSub %float %86 %88
%90 = OpFMul %float %84 %89
%91 = OpFSub %float %78 %90
OpStore %64 %91
OpBranch %67
%67 = OpLabel
%92 = OpLoad %float %64
OpStore %_3_76_blend_overlay_component %92
%94 = OpLoad %v4float %src
%95 = OpCompositeExtract %float %94 2
%96 = OpFMul %float %float_2 %95
%97 = OpLoad %v4float %src
%98 = OpCompositeExtract %float %97 3
%99 = OpFOrdLessThanEqual %bool %96 %98
OpSelectionMerge %103 None
OpBranchConditional %99 %101 %102
%101 = OpLabel
%104 = OpLoad %v4float %dst
%105 = OpCompositeExtract %float %104 2
%106 = OpFMul %float %float_2 %105
%107 = OpLoad %v4float %src
%108 = OpCompositeExtract %float %107 2
%109 = OpFMul %float %106 %108
OpStore %100 %109
OpBranch %103
%102 = OpLabel
%110 = OpLoad %v4float %dst
%111 = OpCompositeExtract %float %110 3
%112 = OpLoad %v4float %src
%113 = OpCompositeExtract %float %112 3
%114 = OpFMul %float %111 %113
%115 = OpLoad %v4float %src
%116 = OpCompositeExtract %float %115 3
%117 = OpLoad %v4float %src
%118 = OpCompositeExtract %float %117 2
%119 = OpFSub %float %116 %118
%120 = OpFMul %float %float_2 %119
%121 = OpLoad %v4float %dst
%122 = OpCompositeExtract %float %121 3
%123 = OpLoad %v4float %dst
%124 = OpCompositeExtract %float %123 2
%125 = OpFSub %float %122 %124
%126 = OpFMul %float %120 %125
%127 = OpFSub %float %114 %126
OpStore %100 %127
OpBranch %103
%103 = OpLabel
%128 = OpLoad %float %100
OpStore %_4_80_blend_overlay_component %128
%130 = OpLoad %float %_2_9_1_blend_overlay_component
%131 = OpLoad %float %_3_76_blend_overlay_component
%132 = OpLoad %float %_4_80_blend_overlay_component
%133 = OpLoad %v4float %dst
%134 = OpCompositeExtract %float %133 3
%136 = OpLoad %v4float %dst
%137 = OpCompositeExtract %float %136 3
%138 = OpFSub %float %float_1 %137
%139 = OpLoad %v4float %src
%140 = OpCompositeExtract %float %139 3
%141 = OpFMul %float %138 %140
%142 = OpFAdd %float %134 %141
%143 = OpCompositeConstruct %v4float %130 %131 %132 %142
OpStore %_5_10_result %143
%144 = OpLoad %v4float %_5_10_result
%145 = OpVectorShuffle %v3float %144 %144 0 1 2
%147 = OpLoad %v4float %src
%148 = OpVectorShuffle %v3float %147 %147 0 1 2
%149 = OpLoad %v4float %dst
%150 = OpCompositeExtract %float %149 3
%151 = OpFSub %float %float_1 %150
%152 = OpVectorTimesScalar %v3float %148 %151
%153 = OpLoad %v4float %dst
%154 = OpVectorShuffle %v3float %153 %153 0 1 2
%155 = OpLoad %v4float %src
%156 = OpCompositeExtract %float %155 3
%157 = OpFSub %float %float_1 %156
%158 = OpVectorTimesScalar %v3float %154 %157
%159 = OpFAdd %v3float %152 %158
%160 = OpFAdd %v3float %145 %159
%161 = OpLoad %v4float %_5_10_result
%162 = OpVectorShuffle %v4float %161 %160 4 5 6 3
OpStore %_5_10_result %162
%163 = OpLoad %v4float %_5_10_result
OpStore %_1_8_blend_overlay %163
%164 = OpLoad %v4float %_1_8_blend_overlay
OpStore %_0_blend_hard_light %164
%165 = OpLoad %v4float %_0_blend_hard_light
OpStore %sk_FragColor %165
OpReturn
OpFunctionEnd
