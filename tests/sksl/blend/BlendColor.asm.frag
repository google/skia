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
OpName %_11_guarded_divide "_11_guarded_divide"
OpName %_12_d "_12_d"
OpName %_13_guarded_divide "_13_guarded_divide"
OpName %_14_n "_14_n"
OpName %_15_d "_15_d"
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
OpDecorate %47 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
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
%46 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
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
%_11_guarded_divide = OpVariable %_ptr_Function_float Function
%_12_d = OpVariable %_ptr_Function_float Function
%_13_guarded_divide = OpVariable %_ptr_Function_v3float Function
%_14_n = OpVariable %_ptr_Function_v3float Function
%_15_d = OpVariable %_ptr_Function_float Function
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
%47 = OpLoad %v3float %_3_dsa
%42 = OpDot %float %46 %47
OpStore %_6_lum %42
%50 = OpLoad %float %_6_lum
%52 = OpLoad %v3float %_2_sda
%51 = OpDot %float %46 %52
%53 = OpFSub %float %50 %51
%54 = OpLoad %v3float %_2_sda
%55 = OpCompositeConstruct %v3float %53 %53 %53
%56 = OpFAdd %v3float %55 %54
OpStore %_8_result %56
%60 = OpLoad %v3float %_8_result
%61 = OpCompositeExtract %float %60 0
%62 = OpLoad %v3float %_8_result
%63 = OpCompositeExtract %float %62 1
%59 = OpExtInst %float %1 FMin %61 %63
%64 = OpLoad %v3float %_8_result
%65 = OpCompositeExtract %float %64 2
%58 = OpExtInst %float %1 FMin %59 %65
OpStore %_9_minComp %58
%69 = OpLoad %v3float %_8_result
%70 = OpCompositeExtract %float %69 0
%71 = OpLoad %v3float %_8_result
%72 = OpCompositeExtract %float %71 1
%68 = OpExtInst %float %1 FMax %70 %72
%73 = OpLoad %v3float %_8_result
%74 = OpCompositeExtract %float %73 2
%67 = OpExtInst %float %1 FMax %68 %74
OpStore %_10_maxComp %67
%76 = OpLoad %float %_9_minComp
%78 = OpFOrdLessThan %bool %76 %float_0
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%81 = OpLoad %float %_6_lum
%82 = OpLoad %float %_9_minComp
%83 = OpFOrdNotEqual %bool %81 %82
OpBranch %80
%80 = OpLabel
%84 = OpPhi %bool %false %15 %83 %79
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%89 = OpLoad %float %_6_lum
%90 = OpLoad %float %_9_minComp
%91 = OpFSub %float %89 %90
OpStore %_12_d %91
%92 = OpLoad %float %_6_lum
%93 = OpLoad %v3float %_8_result
%94 = OpLoad %float %_6_lum
%95 = OpCompositeConstruct %v3float %94 %94 %94
%96 = OpFSub %v3float %93 %95
%97 = OpLoad %float %_6_lum
%98 = OpLoad %float %_12_d
%99 = OpFDiv %float %97 %98
%100 = OpVectorTimesScalar %v3float %96 %99
%101 = OpCompositeConstruct %v3float %92 %92 %92
%102 = OpFAdd %v3float %101 %100
OpStore %_8_result %102
OpBranch %86
%86 = OpLabel
%103 = OpLoad %float %_10_maxComp
%104 = OpLoad %float %_1_alpha
%105 = OpFOrdGreaterThan %bool %103 %104
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%108 = OpLoad %float %_10_maxComp
%109 = OpLoad %float %_6_lum
%110 = OpFOrdNotEqual %bool %108 %109
OpBranch %107
%107 = OpLabel
%111 = OpPhi %bool %false %86 %110 %106
OpSelectionMerge %114 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%117 = OpLoad %v3float %_8_result
%118 = OpLoad %float %_6_lum
%119 = OpCompositeConstruct %v3float %118 %118 %118
%120 = OpFSub %v3float %117 %119
%121 = OpLoad %float %_1_alpha
%122 = OpLoad %float %_6_lum
%123 = OpFSub %float %121 %122
%124 = OpVectorTimesScalar %v3float %120 %123
OpStore %_14_n %124
%126 = OpLoad %float %_10_maxComp
%127 = OpLoad %float %_6_lum
%128 = OpFSub %float %126 %127
OpStore %_15_d %128
%129 = OpLoad %float %_6_lum
%130 = OpLoad %v3float %_14_n
%131 = OpLoad %float %_15_d
%133 = OpFDiv %float %float_1 %131
%134 = OpVectorTimesScalar %v3float %130 %133
%135 = OpCompositeConstruct %v3float %129 %129 %129
%136 = OpFAdd %v3float %135 %134
OpStore %_4_blend_set_color_luminance %136
OpBranch %114
%113 = OpLabel
%137 = OpLoad %v3float %_8_result
OpStore %_4_blend_set_color_luminance %137
OpBranch %114
%114 = OpLabel
%138 = OpLoad %v3float %_4_blend_set_color_luminance
%139 = OpLoad %v4float %dst
%140 = OpVectorShuffle %v3float %139 %139 0 1 2
%141 = OpFAdd %v3float %138 %140
%142 = OpLoad %v3float %_3_dsa
%143 = OpFSub %v3float %141 %142
%144 = OpLoad %v4float %src
%145 = OpVectorShuffle %v3float %144 %144 0 1 2
%146 = OpFAdd %v3float %143 %145
%147 = OpLoad %v3float %_2_sda
%148 = OpFSub %v3float %146 %147
%149 = OpCompositeExtract %float %148 0
%150 = OpCompositeExtract %float %148 1
%151 = OpCompositeExtract %float %148 2
%152 = OpLoad %v4float %src
%153 = OpCompositeExtract %float %152 3
%154 = OpLoad %v4float %dst
%155 = OpCompositeExtract %float %154 3
%156 = OpFAdd %float %153 %155
%157 = OpLoad %float %_1_alpha
%158 = OpFSub %float %156 %157
%159 = OpCompositeConstruct %v4float %149 %150 %151 %158
OpStore %sk_FragColor %159
OpReturn
OpFunctionEnd
