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
OpName %blend_color_saturation_Qhh3 "blend_color_saturation_Qhh3"
OpName %blend_hslc_h4h4h4bb "blend_hslc_h4h4h4bb"
OpName %alpha "alpha"
OpName %sda "sda"
OpName %dsa "dsa"
OpName %l "l"
OpName %r "r"
OpName %_2_mn "_2_mn"
OpName %_3_mx "_3_mx"
OpName %_4_lum "_4_lum"
OpName %_5_result "_5_result"
OpName %_6_minComp "_6_minComp"
OpName %_7_maxComp "_7_maxComp"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %alpha RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %sda RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %dsa RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %l RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %_2_mn RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %_3_mx RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %_4_lum RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %_5_result RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %_6_minComp RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %_7_maxComp RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%17 = OpTypeFunction %float %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%39 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_bool %_ptr_Function_bool
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%105 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%112 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%void = OpTypeVoid
%180 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%true = OpConstantTrue %bool
%blend_color_saturation_Qhh3 = OpFunction %float None %17
%18 = OpFunctionParameter %_ptr_Function_v3float
%19 = OpLabel
%22 = OpLoad %v3float %18
%23 = OpCompositeExtract %float %22 0
%24 = OpLoad %v3float %18
%25 = OpCompositeExtract %float %24 1
%21 = OpExtInst %float %1 FMax %23 %25
%26 = OpLoad %v3float %18
%27 = OpCompositeExtract %float %26 2
%20 = OpExtInst %float %1 FMax %21 %27
%30 = OpLoad %v3float %18
%31 = OpCompositeExtract %float %30 0
%32 = OpLoad %v3float %18
%33 = OpCompositeExtract %float %32 1
%29 = OpExtInst %float %1 FMin %31 %33
%34 = OpLoad %v3float %18
%35 = OpCompositeExtract %float %34 2
%28 = OpExtInst %float %1 FMin %29 %35
%36 = OpFSub %float %20 %28
OpReturnValue %36
OpFunctionEnd
%blend_hslc_h4h4h4bb = OpFunction %v4float None %39
%40 = OpFunctionParameter %_ptr_Function_v4float
%41 = OpFunctionParameter %_ptr_Function_v4float
%42 = OpFunctionParameter %_ptr_Function_bool
%43 = OpFunctionParameter %_ptr_Function_bool
%44 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%l = OpVariable %_ptr_Function_v3float Function
%66 = OpVariable %_ptr_Function_v3float Function
%r = OpVariable %_ptr_Function_v3float Function
%73 = OpVariable %_ptr_Function_v3float Function
%_2_mn = OpVariable %_ptr_Function_float Function
%_3_mx = OpVariable %_ptr_Function_float Function
%91 = OpVariable %_ptr_Function_v3float Function
%97 = OpVariable %_ptr_Function_v3float Function
%_4_lum = OpVariable %_ptr_Function_float Function
%_5_result = OpVariable %_ptr_Function_v3float Function
%_6_minComp = OpVariable %_ptr_Function_float Function
%_7_maxComp = OpVariable %_ptr_Function_float Function
%47 = OpLoad %v4float %41
%48 = OpCompositeExtract %float %47 3
%49 = OpLoad %v4float %40
%50 = OpCompositeExtract %float %49 3
%51 = OpFMul %float %48 %50
OpStore %alpha %51
%53 = OpLoad %v4float %40
%54 = OpVectorShuffle %v3float %53 %53 0 1 2
%55 = OpLoad %v4float %41
%56 = OpCompositeExtract %float %55 3
%57 = OpVectorTimesScalar %v3float %54 %56
OpStore %sda %57
%59 = OpLoad %v4float %41
%60 = OpVectorShuffle %v3float %59 %59 0 1 2
%61 = OpLoad %v4float %40
%62 = OpCompositeExtract %float %61 3
%63 = OpVectorTimesScalar %v3float %60 %62
OpStore %dsa %63
%65 = OpLoad %bool %42
OpSelectionMerge %69 None
OpBranchConditional %65 %67 %68
%67 = OpLabel
OpStore %66 %63
OpBranch %69
%68 = OpLabel
OpStore %66 %57
OpBranch %69
%69 = OpLabel
%70 = OpLoad %v3float %66
OpStore %l %70
%72 = OpLoad %bool %42
OpSelectionMerge %76 None
OpBranchConditional %72 %74 %75
%74 = OpLabel
OpStore %73 %57
OpBranch %76
%75 = OpLabel
OpStore %73 %63
OpBranch %76
%76 = OpLabel
%77 = OpLoad %v3float %73
OpStore %r %77
%78 = OpLoad %bool %43
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%84 = OpCompositeExtract %float %70 0
%85 = OpCompositeExtract %float %70 1
%83 = OpExtInst %float %1 FMin %84 %85
%86 = OpCompositeExtract %float %70 2
%82 = OpExtInst %float %1 FMin %83 %86
OpStore %_2_mn %82
%89 = OpExtInst %float %1 FMax %84 %85
%88 = OpExtInst %float %1 FMax %89 %86
OpStore %_3_mx %88
%90 = OpFOrdGreaterThan %bool %88 %82
OpSelectionMerge %94 None
OpBranchConditional %90 %92 %93
%92 = OpLabel
%95 = OpCompositeConstruct %v3float %82 %82 %82
%96 = OpFSub %v3float %70 %95
OpStore %97 %77
%98 = OpFunctionCall %float %blend_color_saturation_Qhh3 %97
%99 = OpVectorTimesScalar %v3float %96 %98
%100 = OpFSub %float %88 %82
%102 = OpFDiv %float %float_1 %100
%103 = OpVectorTimesScalar %v3float %99 %102
OpStore %91 %103
OpBranch %94
%93 = OpLabel
OpStore %91 %105
OpBranch %94
%94 = OpLabel
%106 = OpLoad %v3float %91
OpStore %l %106
OpStore %r %63
OpBranch %80
%80 = OpLabel
%113 = OpLoad %v3float %r
%108 = OpDot %float %112 %113
OpStore %_4_lum %108
%116 = OpLoad %v3float %l
%115 = OpDot %float %112 %116
%117 = OpFSub %float %108 %115
%118 = OpLoad %v3float %l
%119 = OpCompositeConstruct %v3float %117 %117 %117
%120 = OpFAdd %v3float %119 %118
OpStore %_5_result %120
%124 = OpCompositeExtract %float %120 0
%125 = OpCompositeExtract %float %120 1
%123 = OpExtInst %float %1 FMin %124 %125
%126 = OpCompositeExtract %float %120 2
%122 = OpExtInst %float %1 FMin %123 %126
OpStore %_6_minComp %122
%129 = OpExtInst %float %1 FMax %124 %125
%128 = OpExtInst %float %1 FMax %129 %126
OpStore %_7_maxComp %128
%131 = OpFOrdLessThan %bool %122 %float_0
OpSelectionMerge %133 None
OpBranchConditional %131 %132 %133
%132 = OpLabel
%134 = OpFUnordNotEqual %bool %108 %122
OpBranch %133
%133 = OpLabel
%135 = OpPhi %bool %false %80 %134 %132
OpSelectionMerge %137 None
OpBranchConditional %135 %136 %137
%136 = OpLabel
%138 = OpCompositeConstruct %v3float %108 %108 %108
%139 = OpFSub %v3float %120 %138
%140 = OpFSub %float %108 %122
%141 = OpFDiv %float %108 %140
%142 = OpVectorTimesScalar %v3float %139 %141
%143 = OpFAdd %v3float %138 %142
OpStore %_5_result %143
OpBranch %137
%137 = OpLabel
%144 = OpFOrdGreaterThan %bool %128 %51
OpSelectionMerge %146 None
OpBranchConditional %144 %145 %146
%145 = OpLabel
%147 = OpFUnordNotEqual %bool %128 %108
OpBranch %146
%146 = OpLabel
%148 = OpPhi %bool %false %137 %147 %145
OpSelectionMerge %150 None
OpBranchConditional %148 %149 %150
%149 = OpLabel
%151 = OpLoad %v3float %_5_result
%152 = OpCompositeConstruct %v3float %108 %108 %108
%153 = OpFSub %v3float %151 %152
%154 = OpFSub %float %51 %108
%155 = OpVectorTimesScalar %v3float %153 %154
%156 = OpFSub %float %128 %108
%157 = OpFDiv %float %float_1 %156
%158 = OpVectorTimesScalar %v3float %155 %157
%159 = OpFAdd %v3float %152 %158
OpStore %_5_result %159
OpBranch %150
%150 = OpLabel
%160 = OpLoad %v3float %_5_result
%161 = OpLoad %v4float %41
%162 = OpVectorShuffle %v3float %161 %161 0 1 2
%163 = OpFAdd %v3float %160 %162
%164 = OpFSub %v3float %163 %63
%165 = OpLoad %v4float %40
%166 = OpVectorShuffle %v3float %165 %165 0 1 2
%167 = OpFAdd %v3float %164 %166
%168 = OpFSub %v3float %167 %57
%169 = OpCompositeExtract %float %168 0
%170 = OpCompositeExtract %float %168 1
%171 = OpCompositeExtract %float %168 2
%172 = OpLoad %v4float %40
%173 = OpCompositeExtract %float %172 3
%174 = OpLoad %v4float %41
%175 = OpCompositeExtract %float %174 3
%176 = OpFAdd %float %173 %175
%177 = OpFSub %float %176 %51
%178 = OpCompositeConstruct %v4float %169 %170 %171 %177
OpReturnValue %178
OpFunctionEnd
%main = OpFunction %void None %180
%181 = OpLabel
%187 = OpVariable %_ptr_Function_v4float Function
%191 = OpVariable %_ptr_Function_v4float Function
%193 = OpVariable %_ptr_Function_bool Function
%194 = OpVariable %_ptr_Function_bool Function
%182 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%186 = OpLoad %v4float %182
OpStore %187 %186
%188 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%190 = OpLoad %v4float %188
OpStore %191 %190
OpStore %193 %true
OpStore %194 %false
%195 = OpFunctionCall %v4float %blend_hslc_h4h4h4bb %187 %191 %193 %194
OpStore %sk_FragColor %195
OpReturn
OpFunctionEnd
