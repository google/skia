OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %_guarded_divide "_guarded_divide"
OpName %main "main"
OpName %_1_alpha "_1_alpha"
OpName %_2_sda "_2_sda"
OpName %_3_dsa "_3_dsa"
OpName %_5_lum "_5_lum"
OpName %_6_result "_6_result"
OpName %_7_minComp "_7_minComp"
OpName %_8_maxComp "_8_maxComp"
OpName %_9_d "_9_d"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
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
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_float = OpTypePointer Function %float
%15 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%27 = OpTypeFunction %void
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%52 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%_guarded_divide = OpFunction %v3float None %15
%18 = OpFunctionParameter %_ptr_Function_v3float
%19 = OpFunctionParameter %_ptr_Function_float
%20 = OpLabel
%21 = OpLoad %v3float %18
%22 = OpLoad %float %19
%24 = OpFDiv %float %float_1 %22
%25 = OpVectorTimesScalar %v3float %21 %24
OpReturnValue %25
OpFunctionEnd
%main = OpFunction %void None %27
%28 = OpLabel
%_1_alpha = OpVariable %_ptr_Function_float Function
%_2_sda = OpVariable %_ptr_Function_v3float Function
%_3_dsa = OpVariable %_ptr_Function_v3float Function
%_5_lum = OpVariable %_ptr_Function_float Function
%_6_result = OpVariable %_ptr_Function_v3float Function
%_7_minComp = OpVariable %_ptr_Function_float Function
%_8_maxComp = OpVariable %_ptr_Function_float Function
%_9_d = OpVariable %_ptr_Function_float Function
%116 = OpVariable %_ptr_Function_v3float Function
%129 = OpVariable %_ptr_Function_v3float Function
%133 = OpVariable %_ptr_Function_float Function
%30 = OpLoad %v4float %dst
%31 = OpCompositeExtract %float %30 3
%32 = OpLoad %v4float %src
%33 = OpCompositeExtract %float %32 3
%34 = OpFMul %float %31 %33
OpStore %_1_alpha %34
%36 = OpLoad %v4float %src
%37 = OpVectorShuffle %v3float %36 %36 0 1 2
%38 = OpLoad %v4float %dst
%39 = OpCompositeExtract %float %38 3
%40 = OpVectorTimesScalar %v3float %37 %39
OpStore %_2_sda %40
%42 = OpLoad %v4float %dst
%43 = OpVectorShuffle %v3float %42 %42 0 1 2
%44 = OpLoad %v4float %src
%45 = OpCompositeExtract %float %44 3
%46 = OpVectorTimesScalar %v3float %43 %45
OpStore %_3_dsa %46
%53 = OpLoad %v3float %_3_dsa
%48 = OpDot %float %52 %53
OpStore %_5_lum %48
%55 = OpLoad %float %_5_lum
%57 = OpLoad %v3float %_2_sda
%56 = OpDot %float %52 %57
%58 = OpFSub %float %55 %56
%59 = OpLoad %v3float %_2_sda
%60 = OpCompositeConstruct %v3float %58 %58 %58
%61 = OpFAdd %v3float %60 %59
OpStore %_6_result %61
%65 = OpLoad %v3float %_6_result
%66 = OpCompositeExtract %float %65 0
%67 = OpLoad %v3float %_6_result
%68 = OpCompositeExtract %float %67 1
%64 = OpExtInst %float %1 FMin %66 %68
%69 = OpLoad %v3float %_6_result
%70 = OpCompositeExtract %float %69 2
%63 = OpExtInst %float %1 FMin %64 %70
OpStore %_7_minComp %63
%74 = OpLoad %v3float %_6_result
%75 = OpCompositeExtract %float %74 0
%76 = OpLoad %v3float %_6_result
%77 = OpCompositeExtract %float %76 1
%73 = OpExtInst %float %1 FMax %75 %77
%78 = OpLoad %v3float %_6_result
%79 = OpCompositeExtract %float %78 2
%72 = OpExtInst %float %1 FMax %73 %79
OpStore %_8_maxComp %72
%81 = OpLoad %float %_7_minComp
%83 = OpFOrdLessThan %bool %81 %float_0
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpLoad %float %_5_lum
%87 = OpLoad %float %_7_minComp
%88 = OpFOrdNotEqual %bool %86 %87
OpBranch %85
%85 = OpLabel
%89 = OpPhi %bool %false %28 %88 %84
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%93 = OpLoad %float %_5_lum
%94 = OpLoad %float %_7_minComp
%95 = OpFSub %float %93 %94
OpStore %_9_d %95
%96 = OpLoad %float %_5_lum
%97 = OpLoad %v3float %_6_result
%98 = OpLoad %float %_5_lum
%99 = OpCompositeConstruct %v3float %98 %98 %98
%100 = OpFSub %v3float %97 %99
%101 = OpLoad %float %_5_lum
%102 = OpLoad %float %_9_d
%103 = OpFDiv %float %101 %102
%104 = OpVectorTimesScalar %v3float %100 %103
%105 = OpCompositeConstruct %v3float %96 %96 %96
%106 = OpFAdd %v3float %105 %104
OpStore %_6_result %106
OpBranch %91
%91 = OpLabel
%107 = OpLoad %float %_8_maxComp
%108 = OpLoad %float %_1_alpha
%109 = OpFOrdGreaterThan %bool %107 %108
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%112 = OpLoad %float %_8_maxComp
%113 = OpLoad %float %_5_lum
%114 = OpFOrdNotEqual %bool %112 %113
OpBranch %111
%111 = OpLabel
%115 = OpPhi %bool %false %91 %114 %110
OpSelectionMerge %119 None
OpBranchConditional %115 %117 %118
%117 = OpLabel
%120 = OpLoad %float %_5_lum
%121 = OpLoad %v3float %_6_result
%122 = OpLoad %float %_5_lum
%123 = OpCompositeConstruct %v3float %122 %122 %122
%124 = OpFSub %v3float %121 %123
%125 = OpLoad %float %_1_alpha
%126 = OpLoad %float %_5_lum
%127 = OpFSub %float %125 %126
%128 = OpVectorTimesScalar %v3float %124 %127
OpStore %129 %128
%130 = OpLoad %float %_8_maxComp
%131 = OpLoad %float %_5_lum
%132 = OpFSub %float %130 %131
OpStore %133 %132
%134 = OpFunctionCall %v3float %_guarded_divide %129 %133
%135 = OpCompositeConstruct %v3float %120 %120 %120
%136 = OpFAdd %v3float %135 %134
OpStore %116 %136
OpBranch %119
%118 = OpLabel
%137 = OpLoad %v3float %_6_result
OpStore %116 %137
OpBranch %119
%119 = OpLabel
%138 = OpLoad %v3float %116
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
