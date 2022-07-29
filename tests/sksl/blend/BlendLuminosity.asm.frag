OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %blend_color_saturation_Qhh3 "blend_color_saturation_Qhh3"
OpName %blend_hslc_h4h4h4h2 "blend_hslc_h4h4h4h2"
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
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
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
OpDecorate %66 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %_2_mn RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %_3_mx RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %_4_lum RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %_5_result RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %_6_minComp RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %_7_maxComp RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
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
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%17 = OpTypeFunction %float %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%40 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%111 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%118 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%void = OpTypeVoid
%186 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%198 = OpConstantComposite %v2float %float_1 %float_0
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
%blend_hslc_h4h4h4h2 = OpFunction %v4float None %40
%41 = OpFunctionParameter %_ptr_Function_v4float
%42 = OpFunctionParameter %_ptr_Function_v4float
%43 = OpFunctionParameter %_ptr_Function_v2float
%44 = OpLabel
%alpha = OpVariable %_ptr_Function_float Function
%sda = OpVariable %_ptr_Function_v3float Function
%dsa = OpVariable %_ptr_Function_v3float Function
%l = OpVariable %_ptr_Function_v3float Function
%69 = OpVariable %_ptr_Function_v3float Function
%r = OpVariable %_ptr_Function_v3float Function
%78 = OpVariable %_ptr_Function_v3float Function
%_2_mn = OpVariable %_ptr_Function_float Function
%_3_mx = OpVariable %_ptr_Function_float Function
%98 = OpVariable %_ptr_Function_v3float Function
%104 = OpVariable %_ptr_Function_v3float Function
%_4_lum = OpVariable %_ptr_Function_float Function
%_5_result = OpVariable %_ptr_Function_v3float Function
%_6_minComp = OpVariable %_ptr_Function_float Function
%_7_maxComp = OpVariable %_ptr_Function_float Function
%47 = OpLoad %v4float %42
%48 = OpCompositeExtract %float %47 3
%49 = OpLoad %v4float %41
%50 = OpCompositeExtract %float %49 3
%51 = OpFMul %float %48 %50
OpStore %alpha %51
%53 = OpLoad %v4float %41
%54 = OpVectorShuffle %v3float %53 %53 0 1 2
%55 = OpLoad %v4float %42
%56 = OpCompositeExtract %float %55 3
%57 = OpVectorTimesScalar %v3float %54 %56
OpStore %sda %57
%59 = OpLoad %v4float %42
%60 = OpVectorShuffle %v3float %59 %59 0 1 2
%61 = OpLoad %v4float %41
%62 = OpCompositeExtract %float %61 3
%63 = OpVectorTimesScalar %v3float %60 %62
OpStore %dsa %63
%65 = OpLoad %v2float %43
%66 = OpCompositeExtract %float %65 0
%67 = OpFUnordNotEqual %bool %66 %float_0
OpSelectionMerge %72 None
OpBranchConditional %67 %70 %71
%70 = OpLabel
OpStore %69 %63
OpBranch %72
%71 = OpLabel
OpStore %69 %57
OpBranch %72
%72 = OpLabel
%73 = OpLoad %v3float %69
OpStore %l %73
%75 = OpLoad %v2float %43
%76 = OpCompositeExtract %float %75 0
%77 = OpFUnordNotEqual %bool %76 %float_0
OpSelectionMerge %81 None
OpBranchConditional %77 %79 %80
%79 = OpLabel
OpStore %78 %57
OpBranch %81
%80 = OpLabel
OpStore %78 %63
OpBranch %81
%81 = OpLabel
%82 = OpLoad %v3float %78
OpStore %r %82
%83 = OpLoad %v2float %43
%84 = OpCompositeExtract %float %83 1
%85 = OpFUnordNotEqual %bool %84 %float_0
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%91 = OpCompositeExtract %float %73 0
%92 = OpCompositeExtract %float %73 1
%90 = OpExtInst %float %1 FMin %91 %92
%93 = OpCompositeExtract %float %73 2
%89 = OpExtInst %float %1 FMin %90 %93
OpStore %_2_mn %89
%96 = OpExtInst %float %1 FMax %91 %92
%95 = OpExtInst %float %1 FMax %96 %93
OpStore %_3_mx %95
%97 = OpFOrdGreaterThan %bool %95 %89
OpSelectionMerge %101 None
OpBranchConditional %97 %99 %100
%99 = OpLabel
%102 = OpCompositeConstruct %v3float %89 %89 %89
%103 = OpFSub %v3float %73 %102
OpStore %104 %82
%105 = OpFunctionCall %float %blend_color_saturation_Qhh3 %104
%106 = OpVectorTimesScalar %v3float %103 %105
%107 = OpFSub %float %95 %89
%109 = OpFDiv %float %float_1 %107
%110 = OpVectorTimesScalar %v3float %106 %109
OpStore %98 %110
OpBranch %101
%100 = OpLabel
OpStore %98 %111
OpBranch %101
%101 = OpLabel
%112 = OpLoad %v3float %98
OpStore %l %112
OpStore %r %63
OpBranch %87
%87 = OpLabel
%119 = OpLoad %v3float %r
%114 = OpDot %float %118 %119
OpStore %_4_lum %114
%122 = OpLoad %v3float %l
%121 = OpDot %float %118 %122
%123 = OpFSub %float %114 %121
%124 = OpLoad %v3float %l
%125 = OpCompositeConstruct %v3float %123 %123 %123
%126 = OpFAdd %v3float %125 %124
OpStore %_5_result %126
%130 = OpCompositeExtract %float %126 0
%131 = OpCompositeExtract %float %126 1
%129 = OpExtInst %float %1 FMin %130 %131
%132 = OpCompositeExtract %float %126 2
%128 = OpExtInst %float %1 FMin %129 %132
OpStore %_6_minComp %128
%135 = OpExtInst %float %1 FMax %130 %131
%134 = OpExtInst %float %1 FMax %135 %132
OpStore %_7_maxComp %134
%137 = OpFOrdLessThan %bool %128 %float_0
OpSelectionMerge %139 None
OpBranchConditional %137 %138 %139
%138 = OpLabel
%140 = OpFUnordNotEqual %bool %114 %128
OpBranch %139
%139 = OpLabel
%141 = OpPhi %bool %false %87 %140 %138
OpSelectionMerge %143 None
OpBranchConditional %141 %142 %143
%142 = OpLabel
%144 = OpCompositeConstruct %v3float %114 %114 %114
%145 = OpFSub %v3float %126 %144
%146 = OpFSub %float %114 %128
%147 = OpFDiv %float %114 %146
%148 = OpVectorTimesScalar %v3float %145 %147
%149 = OpFAdd %v3float %144 %148
OpStore %_5_result %149
OpBranch %143
%143 = OpLabel
%150 = OpFOrdGreaterThan %bool %134 %51
OpSelectionMerge %152 None
OpBranchConditional %150 %151 %152
%151 = OpLabel
%153 = OpFUnordNotEqual %bool %134 %114
OpBranch %152
%152 = OpLabel
%154 = OpPhi %bool %false %143 %153 %151
OpSelectionMerge %156 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
%157 = OpLoad %v3float %_5_result
%158 = OpCompositeConstruct %v3float %114 %114 %114
%159 = OpFSub %v3float %157 %158
%160 = OpFSub %float %51 %114
%161 = OpVectorTimesScalar %v3float %159 %160
%162 = OpFSub %float %134 %114
%163 = OpFDiv %float %float_1 %162
%164 = OpVectorTimesScalar %v3float %161 %163
%165 = OpFAdd %v3float %158 %164
OpStore %_5_result %165
OpBranch %156
%156 = OpLabel
%166 = OpLoad %v3float %_5_result
%167 = OpLoad %v4float %42
%168 = OpVectorShuffle %v3float %167 %167 0 1 2
%169 = OpFAdd %v3float %166 %168
%170 = OpFSub %v3float %169 %63
%171 = OpLoad %v4float %41
%172 = OpVectorShuffle %v3float %171 %171 0 1 2
%173 = OpFAdd %v3float %170 %172
%174 = OpFSub %v3float %173 %57
%175 = OpCompositeExtract %float %174 0
%176 = OpCompositeExtract %float %174 1
%177 = OpCompositeExtract %float %174 2
%178 = OpLoad %v4float %41
%179 = OpCompositeExtract %float %178 3
%180 = OpLoad %v4float %42
%181 = OpCompositeExtract %float %180 3
%182 = OpFAdd %float %179 %181
%183 = OpFSub %float %182 %51
%184 = OpCompositeConstruct %v4float %175 %176 %177 %183
OpReturnValue %184
OpFunctionEnd
%main = OpFunction %void None %186
%187 = OpLabel
%193 = OpVariable %_ptr_Function_v4float Function
%197 = OpVariable %_ptr_Function_v4float Function
%199 = OpVariable %_ptr_Function_v2float Function
%188 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%192 = OpLoad %v4float %188
OpStore %193 %192
%194 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%196 = OpLoad %v4float %194
OpStore %197 %196
OpStore %199 %198
%200 = OpFunctionCall %v4float %blend_hslc_h4h4h4h2 %193 %197 %199
OpStore %sk_FragColor %200
OpReturn
OpFunctionEnd
