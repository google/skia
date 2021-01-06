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
OpName %_1_alpha "_1_alpha"
OpName %_2_sda "_2_sda"
OpName %_3_dsa "_3_dsa"
OpName %_5_lum "_5_lum"
OpName %_6_result "_6_result"
OpName %_7_minComp "_7_minComp"
OpName %_8_maxComp "_8_maxComp"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %18 RelaxedPrecision
OpDecorate %20 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%42 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%main = OpFunction %void None %14
%15 = OpLabel
%_1_alpha = OpVariable %_ptr_Function_float Function
%_2_sda = OpVariable %_ptr_Function_v3float Function
%_3_dsa = OpVariable %_ptr_Function_v3float Function
%_5_lum = OpVariable %_ptr_Function_float Function
%_6_result = OpVariable %_ptr_Function_v3float Function
%_7_minComp = OpVariable %_ptr_Function_float Function
%_8_maxComp = OpVariable %_ptr_Function_float Function
%106 = OpVariable %_ptr_Function_v3float Function
%18 = OpLoad %v4float %dst
%19 = OpCompositeExtract %float %18 3
%20 = OpLoad %v4float %src
%21 = OpCompositeExtract %float %20 3
%22 = OpFMul %float %19 %21
OpStore %_1_alpha %22
%26 = OpLoad %v4float %src
%27 = OpVectorShuffle %v3float %26 %26 0 1 2
%28 = OpLoad %v4float %dst
%29 = OpCompositeExtract %float %28 3
%30 = OpVectorTimesScalar %v3float %27 %29
OpStore %_2_sda %30
%32 = OpLoad %v4float %dst
%33 = OpVectorShuffle %v3float %32 %32 0 1 2
%34 = OpLoad %v4float %src
%35 = OpCompositeExtract %float %34 3
%36 = OpVectorTimesScalar %v3float %33 %35
OpStore %_3_dsa %36
%43 = OpLoad %v3float %_3_dsa
%38 = OpDot %float %42 %43
OpStore %_5_lum %38
%45 = OpLoad %float %_5_lum
%47 = OpLoad %v3float %_2_sda
%46 = OpDot %float %42 %47
%48 = OpFSub %float %45 %46
%49 = OpLoad %v3float %_2_sda
%50 = OpCompositeConstruct %v3float %48 %48 %48
%51 = OpFAdd %v3float %50 %49
OpStore %_6_result %51
%55 = OpLoad %v3float %_6_result
%56 = OpCompositeExtract %float %55 0
%57 = OpLoad %v3float %_6_result
%58 = OpCompositeExtract %float %57 1
%54 = OpExtInst %float %1 FMin %56 %58
%59 = OpLoad %v3float %_6_result
%60 = OpCompositeExtract %float %59 2
%53 = OpExtInst %float %1 FMin %54 %60
OpStore %_7_minComp %53
%64 = OpLoad %v3float %_6_result
%65 = OpCompositeExtract %float %64 0
%66 = OpLoad %v3float %_6_result
%67 = OpCompositeExtract %float %66 1
%63 = OpExtInst %float %1 FMax %65 %67
%68 = OpLoad %v3float %_6_result
%69 = OpCompositeExtract %float %68 2
%62 = OpExtInst %float %1 FMax %63 %69
OpStore %_8_maxComp %62
%71 = OpLoad %float %_7_minComp
%73 = OpFOrdLessThan %bool %71 %float_0
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%76 = OpLoad %float %_5_lum
%77 = OpLoad %float %_7_minComp
%78 = OpFOrdNotEqual %bool %76 %77
OpBranch %75
%75 = OpLabel
%79 = OpPhi %bool %false %15 %78 %74
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%82 = OpLoad %float %_5_lum
%83 = OpLoad %v3float %_6_result
%84 = OpLoad %float %_5_lum
%85 = OpCompositeConstruct %v3float %84 %84 %84
%86 = OpFSub %v3float %83 %85
%87 = OpLoad %float %_5_lum
%88 = OpVectorTimesScalar %v3float %86 %87
%89 = OpLoad %float %_5_lum
%90 = OpLoad %float %_7_minComp
%91 = OpFSub %float %89 %90
%93 = OpFDiv %float %float_1 %91
%94 = OpVectorTimesScalar %v3float %88 %93
%95 = OpCompositeConstruct %v3float %82 %82 %82
%96 = OpFAdd %v3float %95 %94
OpStore %_6_result %96
OpBranch %81
%81 = OpLabel
%97 = OpLoad %float %_8_maxComp
%98 = OpLoad %float %_1_alpha
%99 = OpFOrdGreaterThan %bool %97 %98
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpLoad %float %_8_maxComp
%103 = OpLoad %float %_5_lum
%104 = OpFOrdNotEqual %bool %102 %103
OpBranch %101
%101 = OpLabel
%105 = OpPhi %bool %false %81 %104 %100
OpSelectionMerge %109 None
OpBranchConditional %105 %107 %108
%107 = OpLabel
%110 = OpLoad %float %_5_lum
%111 = OpLoad %v3float %_6_result
%112 = OpLoad %float %_5_lum
%113 = OpCompositeConstruct %v3float %112 %112 %112
%114 = OpFSub %v3float %111 %113
%115 = OpLoad %float %_1_alpha
%116 = OpLoad %float %_5_lum
%117 = OpFSub %float %115 %116
%118 = OpVectorTimesScalar %v3float %114 %117
%119 = OpLoad %float %_8_maxComp
%120 = OpLoad %float %_5_lum
%121 = OpFSub %float %119 %120
%122 = OpFDiv %float %float_1 %121
%123 = OpVectorTimesScalar %v3float %118 %122
%124 = OpCompositeConstruct %v3float %110 %110 %110
%125 = OpFAdd %v3float %124 %123
OpStore %106 %125
OpBranch %109
%108 = OpLabel
%126 = OpLoad %v3float %_6_result
OpStore %106 %126
OpBranch %109
%109 = OpLabel
%127 = OpLoad %v3float %106
%128 = OpLoad %v4float %dst
%129 = OpVectorShuffle %v3float %128 %128 0 1 2
%130 = OpFAdd %v3float %127 %129
%131 = OpLoad %v3float %_3_dsa
%132 = OpFSub %v3float %130 %131
%133 = OpLoad %v4float %src
%134 = OpVectorShuffle %v3float %133 %133 0 1 2
%135 = OpFAdd %v3float %132 %134
%136 = OpLoad %v3float %_2_sda
%137 = OpFSub %v3float %135 %136
%138 = OpCompositeExtract %float %137 0
%139 = OpCompositeExtract %float %137 1
%140 = OpCompositeExtract %float %137 2
%141 = OpLoad %v4float %src
%142 = OpCompositeExtract %float %141 3
%143 = OpLoad %v4float %dst
%144 = OpCompositeExtract %float %143 3
%145 = OpFAdd %float %142 %144
%146 = OpLoad %float %_1_alpha
%147 = OpFSub %float %145 %146
%148 = OpCompositeConstruct %v4float %138 %139 %140 %147
OpStore %sk_FragColor %148
OpReturn
OpFunctionEnd
