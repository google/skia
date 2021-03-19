OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %main "main"
OpName %_0_alpha "_0_alpha"
OpName %_1_sda "_1_sda"
OpName %_2_dsa "_2_dsa"
OpName %_3_blend_set_color_luminance "_3_blend_set_color_luminance"
OpName %_4_lum "_4_lum"
OpName %_5_result "_5_result"
OpName %_6_minComp "_6_minComp"
OpName %_7_maxComp "_7_maxComp"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %22 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%14 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%53 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%main = OpFunction %void None %14
%15 = OpLabel
%_0_alpha = OpVariable %_ptr_Function_float Function
%_1_sda = OpVariable %_ptr_Function_v3float Function
%_2_dsa = OpVariable %_ptr_Function_v3float Function
%_3_blend_set_color_luminance = OpVariable %_ptr_Function_v3float Function
%_4_lum = OpVariable %_ptr_Function_float Function
%_5_result = OpVariable %_ptr_Function_v3float Function
%_6_minComp = OpVariable %_ptr_Function_float Function
%_7_maxComp = OpVariable %_ptr_Function_float Function
%18 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%22 = OpLoad %v4float %18
%23 = OpCompositeExtract %float %22 3
%24 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %24
%27 = OpCompositeExtract %float %26 3
%28 = OpFMul %float %23 %27
OpStore %_0_alpha %28
%32 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%33 = OpLoad %v4float %32
%34 = OpVectorShuffle %v3float %33 %33 0 1 2
%35 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%36 = OpLoad %v4float %35
%37 = OpCompositeExtract %float %36 3
%38 = OpVectorTimesScalar %v3float %34 %37
OpStore %_1_sda %38
%40 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%41 = OpLoad %v4float %40
%42 = OpVectorShuffle %v3float %41 %41 0 1 2
%43 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%44 = OpLoad %v4float %43
%45 = OpCompositeExtract %float %44 3
%46 = OpVectorTimesScalar %v3float %42 %45
OpStore %_2_dsa %46
%54 = OpLoad %v3float %_2_dsa
%49 = OpDot %float %53 %54
OpStore %_4_lum %49
%56 = OpLoad %float %_4_lum
%58 = OpLoad %v3float %_1_sda
%57 = OpDot %float %53 %58
%59 = OpFSub %float %56 %57
%60 = OpLoad %v3float %_1_sda
%61 = OpCompositeConstruct %v3float %59 %59 %59
%62 = OpFAdd %v3float %61 %60
OpStore %_5_result %62
%66 = OpLoad %v3float %_5_result
%67 = OpCompositeExtract %float %66 0
%68 = OpLoad %v3float %_5_result
%69 = OpCompositeExtract %float %68 1
%65 = OpExtInst %float %1 FMin %67 %69
%70 = OpLoad %v3float %_5_result
%71 = OpCompositeExtract %float %70 2
%64 = OpExtInst %float %1 FMin %65 %71
OpStore %_6_minComp %64
%75 = OpLoad %v3float %_5_result
%76 = OpCompositeExtract %float %75 0
%77 = OpLoad %v3float %_5_result
%78 = OpCompositeExtract %float %77 1
%74 = OpExtInst %float %1 FMax %76 %78
%79 = OpLoad %v3float %_5_result
%80 = OpCompositeExtract %float %79 2
%73 = OpExtInst %float %1 FMax %74 %80
OpStore %_7_maxComp %73
%82 = OpLoad %float %_6_minComp
%84 = OpFOrdLessThan %bool %82 %float_0
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpLoad %float %_4_lum
%88 = OpLoad %float %_6_minComp
%89 = OpFOrdNotEqual %bool %87 %88
OpBranch %86
%86 = OpLabel
%90 = OpPhi %bool %false %15 %89 %85
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%93 = OpLoad %float %_4_lum
%94 = OpLoad %v3float %_5_result
%95 = OpLoad %float %_4_lum
%96 = OpCompositeConstruct %v3float %95 %95 %95
%97 = OpFSub %v3float %94 %96
%98 = OpLoad %float %_4_lum
%99 = OpLoad %float %_4_lum
%100 = OpLoad %float %_6_minComp
%101 = OpFSub %float %99 %100
%102 = OpFDiv %float %98 %101
%103 = OpVectorTimesScalar %v3float %97 %102
%104 = OpCompositeConstruct %v3float %93 %93 %93
%105 = OpFAdd %v3float %104 %103
OpStore %_5_result %105
OpBranch %92
%92 = OpLabel
%106 = OpLoad %float %_7_maxComp
%107 = OpLoad %float %_0_alpha
%108 = OpFOrdGreaterThan %bool %106 %107
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%111 = OpLoad %float %_7_maxComp
%112 = OpLoad %float %_4_lum
%113 = OpFOrdNotEqual %bool %111 %112
OpBranch %110
%110 = OpLabel
%114 = OpPhi %bool %false %92 %113 %109
OpSelectionMerge %117 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%118 = OpLoad %float %_4_lum
%119 = OpLoad %v3float %_5_result
%120 = OpLoad %float %_4_lum
%121 = OpCompositeConstruct %v3float %120 %120 %120
%122 = OpFSub %v3float %119 %121
%123 = OpLoad %float %_0_alpha
%124 = OpLoad %float %_4_lum
%125 = OpFSub %float %123 %124
%126 = OpVectorTimesScalar %v3float %122 %125
%127 = OpLoad %float %_7_maxComp
%128 = OpLoad %float %_4_lum
%129 = OpFSub %float %127 %128
%131 = OpFDiv %float %float_1 %129
%132 = OpVectorTimesScalar %v3float %126 %131
%133 = OpCompositeConstruct %v3float %118 %118 %118
%134 = OpFAdd %v3float %133 %132
OpStore %_3_blend_set_color_luminance %134
OpBranch %117
%116 = OpLabel
%135 = OpLoad %v3float %_5_result
OpStore %_3_blend_set_color_luminance %135
OpBranch %117
%117 = OpLabel
%136 = OpLoad %v3float %_3_blend_set_color_luminance
%137 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%138 = OpLoad %v4float %137
%139 = OpVectorShuffle %v3float %138 %138 0 1 2
%140 = OpFAdd %v3float %136 %139
%141 = OpLoad %v3float %_2_dsa
%142 = OpFSub %v3float %140 %141
%143 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%144 = OpLoad %v4float %143
%145 = OpVectorShuffle %v3float %144 %144 0 1 2
%146 = OpFAdd %v3float %142 %145
%147 = OpLoad %v3float %_1_sda
%148 = OpFSub %v3float %146 %147
%149 = OpCompositeExtract %float %148 0
%150 = OpCompositeExtract %float %148 1
%151 = OpCompositeExtract %float %148 2
%152 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%153 = OpLoad %v4float %152
%154 = OpCompositeExtract %float %153 3
%155 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%156 = OpLoad %v4float %155
%157 = OpCompositeExtract %float %156 3
%158 = OpFAdd %float %154 %157
%159 = OpLoad %float %_0_alpha
%160 = OpFSub %float %158 %159
%161 = OpCompositeConstruct %v4float %149 %150 %151 %160
OpStore %sk_FragColor %161
OpReturn
OpFunctionEnd
