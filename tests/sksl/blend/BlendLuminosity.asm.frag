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
OpName %_0_blend_luminosity "_0_blend_luminosity"
OpName %_1_alpha "_1_alpha"
OpName %_2_sda "_2_sda"
OpName %_3_dsa "_3_dsa"
OpName %_4_blend_set_color_luminance "_4_blend_set_color_luminance"
OpName %_5_lum "_5_lum"
OpName %_6_result "_6_result"
OpName %_7_minComp "_7_minComp"
OpName %_8_maxComp "_8_maxComp"
OpName %_9_d "_9_d"
OpName %_10_n "_10_n"
OpName %_11_d "_11_d"
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
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
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
%45 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%main = OpFunction %void None %14
%15 = OpLabel
%_0_blend_luminosity = OpVariable %_ptr_Function_v4float Function
%_1_alpha = OpVariable %_ptr_Function_float Function
%_2_sda = OpVariable %_ptr_Function_v3float Function
%_3_dsa = OpVariable %_ptr_Function_v3float Function
%_4_blend_set_color_luminance = OpVariable %_ptr_Function_v3float Function
%_5_lum = OpVariable %_ptr_Function_float Function
%_6_result = OpVariable %_ptr_Function_v3float Function
%_7_minComp = OpVariable %_ptr_Function_float Function
%_8_maxComp = OpVariable %_ptr_Function_float Function
%_9_d = OpVariable %_ptr_Function_float Function
%_10_n = OpVariable %_ptr_Function_v3float Function
%_11_d = OpVariable %_ptr_Function_float Function
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
%46 = OpLoad %v3float %_2_sda
%41 = OpDot %float %45 %46
OpStore %_5_lum %41
%48 = OpLoad %float %_5_lum
%50 = OpLoad %v3float %_3_dsa
%49 = OpDot %float %45 %50
%51 = OpFSub %float %48 %49
%52 = OpLoad %v3float %_3_dsa
%53 = OpCompositeConstruct %v3float %51 %51 %51
%54 = OpFAdd %v3float %53 %52
OpStore %_6_result %54
%58 = OpLoad %v3float %_6_result
%59 = OpCompositeExtract %float %58 0
%60 = OpLoad %v3float %_6_result
%61 = OpCompositeExtract %float %60 1
%57 = OpExtInst %float %1 FMin %59 %61
%62 = OpLoad %v3float %_6_result
%63 = OpCompositeExtract %float %62 2
%56 = OpExtInst %float %1 FMin %57 %63
OpStore %_7_minComp %56
%67 = OpLoad %v3float %_6_result
%68 = OpCompositeExtract %float %67 0
%69 = OpLoad %v3float %_6_result
%70 = OpCompositeExtract %float %69 1
%66 = OpExtInst %float %1 FMax %68 %70
%71 = OpLoad %v3float %_6_result
%72 = OpCompositeExtract %float %71 2
%65 = OpExtInst %float %1 FMax %66 %72
OpStore %_8_maxComp %65
%74 = OpLoad %float %_7_minComp
%76 = OpFOrdLessThan %bool %74 %float_0
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%79 = OpLoad %float %_5_lum
%80 = OpLoad %float %_7_minComp
%81 = OpFOrdNotEqual %bool %79 %80
OpBranch %78
%78 = OpLabel
%82 = OpPhi %bool %false %15 %81 %77
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%86 = OpLoad %float %_5_lum
%87 = OpLoad %float %_7_minComp
%88 = OpFSub %float %86 %87
OpStore %_9_d %88
%89 = OpLoad %float %_5_lum
%90 = OpLoad %v3float %_6_result
%91 = OpLoad %float %_5_lum
%92 = OpCompositeConstruct %v3float %91 %91 %91
%93 = OpFSub %v3float %90 %92
%94 = OpLoad %float %_5_lum
%95 = OpLoad %float %_9_d
%96 = OpFDiv %float %94 %95
%97 = OpVectorTimesScalar %v3float %93 %96
%98 = OpCompositeConstruct %v3float %89 %89 %89
%99 = OpFAdd %v3float %98 %97
OpStore %_6_result %99
OpBranch %84
%84 = OpLabel
%100 = OpLoad %float %_8_maxComp
%101 = OpLoad %float %_1_alpha
%102 = OpFOrdGreaterThan %bool %100 %101
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpLoad %float %_8_maxComp
%106 = OpLoad %float %_5_lum
%107 = OpFOrdNotEqual %bool %105 %106
OpBranch %104
%104 = OpLabel
%108 = OpPhi %bool %false %84 %107 %103
OpSelectionMerge %111 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%113 = OpLoad %v3float %_6_result
%114 = OpLoad %float %_5_lum
%115 = OpCompositeConstruct %v3float %114 %114 %114
%116 = OpFSub %v3float %113 %115
%117 = OpLoad %float %_1_alpha
%118 = OpLoad %float %_5_lum
%119 = OpFSub %float %117 %118
%120 = OpVectorTimesScalar %v3float %116 %119
OpStore %_10_n %120
%122 = OpLoad %float %_8_maxComp
%123 = OpLoad %float %_5_lum
%124 = OpFSub %float %122 %123
OpStore %_11_d %124
%125 = OpLoad %float %_5_lum
%126 = OpLoad %v3float %_10_n
%127 = OpLoad %float %_11_d
%129 = OpFDiv %float %float_1 %127
%130 = OpVectorTimesScalar %v3float %126 %129
%131 = OpCompositeConstruct %v3float %125 %125 %125
%132 = OpFAdd %v3float %131 %130
OpStore %_4_blend_set_color_luminance %132
OpBranch %111
%110 = OpLabel
%133 = OpLoad %v3float %_6_result
OpStore %_4_blend_set_color_luminance %133
OpBranch %111
%111 = OpLabel
%134 = OpLoad %v3float %_4_blend_set_color_luminance
%135 = OpLoad %v4float %dst
%136 = OpVectorShuffle %v3float %135 %135 0 1 2
%137 = OpFAdd %v3float %134 %136
%138 = OpLoad %v3float %_3_dsa
%139 = OpFSub %v3float %137 %138
%140 = OpLoad %v4float %src
%141 = OpVectorShuffle %v3float %140 %140 0 1 2
%142 = OpFAdd %v3float %139 %141
%143 = OpLoad %v3float %_2_sda
%144 = OpFSub %v3float %142 %143
%145 = OpCompositeExtract %float %144 0
%146 = OpCompositeExtract %float %144 1
%147 = OpCompositeExtract %float %144 2
%148 = OpLoad %v4float %src
%149 = OpCompositeExtract %float %148 3
%150 = OpLoad %v4float %dst
%151 = OpCompositeExtract %float %150 3
%152 = OpFAdd %float %149 %151
%153 = OpLoad %float %_1_alpha
%154 = OpFSub %float %152 %153
%155 = OpCompositeConstruct %v4float %145 %146 %147 %154
OpStore %sk_FragColor %155
OpReturn
OpFunctionEnd
