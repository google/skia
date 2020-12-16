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
OpName %_0_blend_color "_0_blend_color"
OpName %_1_alpha "_1_alpha"
OpName %_2_sda "_2_sda"
OpName %_3_dsa "_3_dsa"
OpName %_4_blend_set_color_luminance "_4_blend_set_color_luminance"
OpName %_5_blend_color_luminance "_5_blend_color_luminance"
OpName %_6_lum "_6_lum"
OpName %_7_blend_color_luminance "_7_blend_color_luminance"
OpName %_8_result "_8_result"
OpName %_9_minComp "_9_minComp"
OpName %_10_maxComp "_10_maxComp"
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
OpDecorate %28 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
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
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%42 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%51 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%main = OpFunction %void None %14
%15 = OpLabel
%_0_blend_color = OpVariable %_ptr_Function_v4float Function
%_1_alpha = OpVariable %_ptr_Function_float Function
%_2_sda = OpVariable %_ptr_Function_v3float Function
%_3_dsa = OpVariable %_ptr_Function_v3float Function
%_4_blend_set_color_luminance = OpVariable %_ptr_Function_v3float Function
%_5_blend_color_luminance = OpVariable %_ptr_Function_float Function
%_6_lum = OpVariable %_ptr_Function_float Function
%_7_blend_color_luminance = OpVariable %_ptr_Function_float Function
%_8_result = OpVariable %_ptr_Function_v3float Function
%_9_minComp = OpVariable %_ptr_Function_float Function
%_10_maxComp = OpVariable %_ptr_Function_float Function
%114 = OpVariable %_ptr_Function_v3float Function
%20 = OpLoad %v4float %dst
%21 = OpCompositeExtract %float %20 3
%22 = OpLoad %v4float %src
%23 = OpCompositeExtract %float %22 3
%24 = OpFMul %float %21 %23
OpStore %_1_alpha %24
%28 = OpLoad %v4float %src
%29 = OpVectorShuffle %v3float %28 %28 0 1 2
%30 = OpLoad %v4float %dst
%31 = OpCompositeExtract %float %30 3
%32 = OpVectorTimesScalar %v3float %29 %31
OpStore %_2_sda %32
%34 = OpLoad %v4float %dst
%35 = OpVectorShuffle %v3float %34 %34 0 1 2
%36 = OpLoad %v4float %src
%37 = OpCompositeExtract %float %36 3
%38 = OpVectorTimesScalar %v3float %35 %37
OpStore %_3_dsa %38
%46 = OpLoad %v3float %_3_dsa
%41 = OpDot %float %42 %46
OpStore %_5_blend_color_luminance %41
%48 = OpLoad %float %_5_blend_color_luminance
OpStore %_6_lum %48
%52 = OpLoad %v3float %_2_sda
%50 = OpDot %float %51 %52
OpStore %_7_blend_color_luminance %50
%54 = OpLoad %float %_6_lum
%55 = OpLoad %float %_7_blend_color_luminance
%56 = OpFSub %float %54 %55
%57 = OpLoad %v3float %_2_sda
%58 = OpCompositeConstruct %v3float %56 %56 %56
%59 = OpFAdd %v3float %58 %57
OpStore %_8_result %59
%63 = OpLoad %v3float %_8_result
%64 = OpCompositeExtract %float %63 0
%65 = OpLoad %v3float %_8_result
%66 = OpCompositeExtract %float %65 1
%62 = OpExtInst %float %1 FMin %64 %66
%67 = OpLoad %v3float %_8_result
%68 = OpCompositeExtract %float %67 2
%61 = OpExtInst %float %1 FMin %62 %68
OpStore %_9_minComp %61
%72 = OpLoad %v3float %_8_result
%73 = OpCompositeExtract %float %72 0
%74 = OpLoad %v3float %_8_result
%75 = OpCompositeExtract %float %74 1
%71 = OpExtInst %float %1 FMax %73 %75
%76 = OpLoad %v3float %_8_result
%77 = OpCompositeExtract %float %76 2
%70 = OpExtInst %float %1 FMax %71 %77
OpStore %_10_maxComp %70
%79 = OpLoad %float %_9_minComp
%81 = OpFOrdLessThan %bool %79 %float_0
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%84 = OpLoad %float %_6_lum
%85 = OpLoad %float %_9_minComp
%86 = OpFOrdNotEqual %bool %84 %85
OpBranch %83
%83 = OpLabel
%87 = OpPhi %bool %false %15 %86 %82
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpLoad %float %_6_lum
%91 = OpLoad %v3float %_8_result
%92 = OpLoad %float %_6_lum
%93 = OpCompositeConstruct %v3float %92 %92 %92
%94 = OpFSub %v3float %91 %93
%95 = OpLoad %float %_6_lum
%96 = OpVectorTimesScalar %v3float %94 %95
%97 = OpLoad %float %_6_lum
%98 = OpLoad %float %_9_minComp
%99 = OpFSub %float %97 %98
%101 = OpFDiv %float %float_1 %99
%102 = OpVectorTimesScalar %v3float %96 %101
%103 = OpCompositeConstruct %v3float %90 %90 %90
%104 = OpFAdd %v3float %103 %102
OpStore %_8_result %104
OpBranch %89
%89 = OpLabel
%105 = OpLoad %float %_10_maxComp
%106 = OpLoad %float %_1_alpha
%107 = OpFOrdGreaterThan %bool %105 %106
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%110 = OpLoad %float %_10_maxComp
%111 = OpLoad %float %_6_lum
%112 = OpFOrdNotEqual %bool %110 %111
OpBranch %109
%109 = OpLabel
%113 = OpPhi %bool %false %89 %112 %108
OpSelectionMerge %117 None
OpBranchConditional %113 %115 %116
%115 = OpLabel
%118 = OpLoad %float %_6_lum
%119 = OpLoad %v3float %_8_result
%120 = OpLoad %float %_6_lum
%121 = OpCompositeConstruct %v3float %120 %120 %120
%122 = OpFSub %v3float %119 %121
%123 = OpLoad %float %_1_alpha
%124 = OpLoad %float %_6_lum
%125 = OpFSub %float %123 %124
%126 = OpVectorTimesScalar %v3float %122 %125
%127 = OpLoad %float %_10_maxComp
%128 = OpLoad %float %_6_lum
%129 = OpFSub %float %127 %128
%130 = OpFDiv %float %float_1 %129
%131 = OpVectorTimesScalar %v3float %126 %130
%132 = OpCompositeConstruct %v3float %118 %118 %118
%133 = OpFAdd %v3float %132 %131
OpStore %114 %133
OpBranch %117
%116 = OpLabel
%134 = OpLoad %v3float %_8_result
OpStore %114 %134
OpBranch %117
%117 = OpLabel
%135 = OpLoad %v3float %114
OpStore %_4_blend_set_color_luminance %135
%136 = OpLoad %v3float %_4_blend_set_color_luminance
%137 = OpLoad %v4float %dst
%138 = OpVectorShuffle %v3float %137 %137 0 1 2
%139 = OpFAdd %v3float %136 %138
%140 = OpLoad %v3float %_3_dsa
%141 = OpFSub %v3float %139 %140
%142 = OpLoad %v4float %src
%143 = OpVectorShuffle %v3float %142 %142 0 1 2
%144 = OpFAdd %v3float %141 %143
%145 = OpLoad %v3float %_2_sda
%146 = OpFSub %v3float %144 %145
%147 = OpCompositeExtract %float %146 0
%148 = OpCompositeExtract %float %146 1
%149 = OpCompositeExtract %float %146 2
%150 = OpLoad %v4float %src
%151 = OpCompositeExtract %float %150 3
%152 = OpLoad %v4float %dst
%153 = OpCompositeExtract %float %152 3
%154 = OpFAdd %float %151 %153
%155 = OpLoad %float %_1_alpha
%156 = OpFSub %float %154 %155
%157 = OpCompositeConstruct %v4float %147 %148 %149 %156
OpStore %_0_blend_color %157
%158 = OpLoad %v4float %_0_blend_color
OpStore %sk_FragColor %158
OpReturn
OpFunctionEnd
