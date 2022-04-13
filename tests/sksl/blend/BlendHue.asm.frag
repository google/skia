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
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %r RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %_2_mn RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %_3_mx RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %_4_lum RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %_5_result RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %_6_minComp RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %_7_maxComp RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
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
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
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
%125 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%133 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%void = OpTypeVoid
%235 = OpTypeFunction %void
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
%75 = OpVariable %_ptr_Function_v3float Function
%_2_mn = OpVariable %_ptr_Function_float Function
%_3_mx = OpVariable %_ptr_Function_float Function
%106 = OpVariable %_ptr_Function_v3float Function
%115 = OpVariable %_ptr_Function_v3float Function
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
%70 = OpLoad %v3float %dsa
OpStore %66 %70
OpBranch %69
%68 = OpLabel
%71 = OpLoad %v3float %sda
OpStore %66 %71
OpBranch %69
%69 = OpLabel
%72 = OpLoad %v3float %66
OpStore %l %72
%74 = OpLoad %bool %42
OpSelectionMerge %78 None
OpBranchConditional %74 %76 %77
%76 = OpLabel
%79 = OpLoad %v3float %sda
OpStore %75 %79
OpBranch %78
%77 = OpLabel
%80 = OpLoad %v3float %dsa
OpStore %75 %80
OpBranch %78
%78 = OpLabel
%81 = OpLoad %v3float %75
OpStore %r %81
%82 = OpLoad %bool %43
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%88 = OpLoad %v3float %l
%89 = OpCompositeExtract %float %88 0
%90 = OpLoad %v3float %l
%91 = OpCompositeExtract %float %90 1
%87 = OpExtInst %float %1 FMin %89 %91
%92 = OpLoad %v3float %l
%93 = OpCompositeExtract %float %92 2
%86 = OpExtInst %float %1 FMin %87 %93
OpStore %_2_mn %86
%97 = OpLoad %v3float %l
%98 = OpCompositeExtract %float %97 0
%99 = OpLoad %v3float %l
%100 = OpCompositeExtract %float %99 1
%96 = OpExtInst %float %1 FMax %98 %100
%101 = OpLoad %v3float %l
%102 = OpCompositeExtract %float %101 2
%95 = OpExtInst %float %1 FMax %96 %102
OpStore %_3_mx %95
%103 = OpLoad %float %_3_mx
%104 = OpLoad %float %_2_mn
%105 = OpFOrdGreaterThan %bool %103 %104
OpSelectionMerge %109 None
OpBranchConditional %105 %107 %108
%107 = OpLabel
%110 = OpLoad %v3float %l
%111 = OpLoad %float %_2_mn
%112 = OpCompositeConstruct %v3float %111 %111 %111
%113 = OpFSub %v3float %110 %112
%114 = OpLoad %v3float %r
OpStore %115 %114
%116 = OpFunctionCall %float %blend_color_saturation_Qhh3 %115
%117 = OpVectorTimesScalar %v3float %113 %116
%118 = OpLoad %float %_3_mx
%119 = OpLoad %float %_2_mn
%120 = OpFSub %float %118 %119
%122 = OpFDiv %float %float_1 %120
%123 = OpVectorTimesScalar %v3float %117 %122
OpStore %106 %123
OpBranch %109
%108 = OpLabel
OpStore %106 %125
OpBranch %109
%109 = OpLabel
%126 = OpLoad %v3float %106
OpStore %l %126
%127 = OpLoad %v3float %dsa
OpStore %r %127
OpBranch %84
%84 = OpLabel
%134 = OpLoad %v3float %r
%129 = OpDot %float %133 %134
OpStore %_4_lum %129
%136 = OpLoad %float %_4_lum
%138 = OpLoad %v3float %l
%137 = OpDot %float %133 %138
%139 = OpFSub %float %136 %137
%140 = OpLoad %v3float %l
%141 = OpCompositeConstruct %v3float %139 %139 %139
%142 = OpFAdd %v3float %141 %140
OpStore %_5_result %142
%146 = OpLoad %v3float %_5_result
%147 = OpCompositeExtract %float %146 0
%148 = OpLoad %v3float %_5_result
%149 = OpCompositeExtract %float %148 1
%145 = OpExtInst %float %1 FMin %147 %149
%150 = OpLoad %v3float %_5_result
%151 = OpCompositeExtract %float %150 2
%144 = OpExtInst %float %1 FMin %145 %151
OpStore %_6_minComp %144
%155 = OpLoad %v3float %_5_result
%156 = OpCompositeExtract %float %155 0
%157 = OpLoad %v3float %_5_result
%158 = OpCompositeExtract %float %157 1
%154 = OpExtInst %float %1 FMax %156 %158
%159 = OpLoad %v3float %_5_result
%160 = OpCompositeExtract %float %159 2
%153 = OpExtInst %float %1 FMax %154 %160
OpStore %_7_maxComp %153
%162 = OpLoad %float %_6_minComp
%163 = OpFOrdLessThan %bool %162 %float_0
OpSelectionMerge %165 None
OpBranchConditional %163 %164 %165
%164 = OpLabel
%166 = OpLoad %float %_4_lum
%167 = OpLoad %float %_6_minComp
%168 = OpFUnordNotEqual %bool %166 %167
OpBranch %165
%165 = OpLabel
%169 = OpPhi %bool %false %84 %168 %164
OpSelectionMerge %171 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%172 = OpLoad %float %_4_lum
%173 = OpLoad %v3float %_5_result
%174 = OpLoad %float %_4_lum
%175 = OpCompositeConstruct %v3float %174 %174 %174
%176 = OpFSub %v3float %173 %175
%177 = OpLoad %float %_4_lum
%178 = OpLoad %float %_4_lum
%179 = OpLoad %float %_6_minComp
%180 = OpFSub %float %178 %179
%181 = OpFDiv %float %177 %180
%182 = OpVectorTimesScalar %v3float %176 %181
%183 = OpCompositeConstruct %v3float %172 %172 %172
%184 = OpFAdd %v3float %183 %182
OpStore %_5_result %184
OpBranch %171
%171 = OpLabel
%185 = OpLoad %float %_7_maxComp
%186 = OpLoad %float %alpha
%187 = OpFOrdGreaterThan %bool %185 %186
OpSelectionMerge %189 None
OpBranchConditional %187 %188 %189
%188 = OpLabel
%190 = OpLoad %float %_7_maxComp
%191 = OpLoad %float %_4_lum
%192 = OpFUnordNotEqual %bool %190 %191
OpBranch %189
%189 = OpLabel
%193 = OpPhi %bool %false %171 %192 %188
OpSelectionMerge %195 None
OpBranchConditional %193 %194 %195
%194 = OpLabel
%196 = OpLoad %float %_4_lum
%197 = OpLoad %v3float %_5_result
%198 = OpLoad %float %_4_lum
%199 = OpCompositeConstruct %v3float %198 %198 %198
%200 = OpFSub %v3float %197 %199
%201 = OpLoad %float %alpha
%202 = OpLoad %float %_4_lum
%203 = OpFSub %float %201 %202
%204 = OpVectorTimesScalar %v3float %200 %203
%205 = OpLoad %float %_7_maxComp
%206 = OpLoad %float %_4_lum
%207 = OpFSub %float %205 %206
%208 = OpFDiv %float %float_1 %207
%209 = OpVectorTimesScalar %v3float %204 %208
%210 = OpCompositeConstruct %v3float %196 %196 %196
%211 = OpFAdd %v3float %210 %209
OpStore %_5_result %211
OpBranch %195
%195 = OpLabel
%212 = OpLoad %v3float %_5_result
%213 = OpLoad %v4float %41
%214 = OpVectorShuffle %v3float %213 %213 0 1 2
%215 = OpFAdd %v3float %212 %214
%216 = OpLoad %v3float %dsa
%217 = OpFSub %v3float %215 %216
%218 = OpLoad %v4float %40
%219 = OpVectorShuffle %v3float %218 %218 0 1 2
%220 = OpFAdd %v3float %217 %219
%221 = OpLoad %v3float %sda
%222 = OpFSub %v3float %220 %221
%223 = OpCompositeExtract %float %222 0
%224 = OpCompositeExtract %float %222 1
%225 = OpCompositeExtract %float %222 2
%226 = OpLoad %v4float %40
%227 = OpCompositeExtract %float %226 3
%228 = OpLoad %v4float %41
%229 = OpCompositeExtract %float %228 3
%230 = OpFAdd %float %227 %229
%231 = OpLoad %float %alpha
%232 = OpFSub %float %230 %231
%233 = OpCompositeConstruct %v4float %223 %224 %225 %232
OpReturnValue %233
OpFunctionEnd
%main = OpFunction %void None %235
%236 = OpLabel
%242 = OpVariable %_ptr_Function_v4float Function
%246 = OpVariable %_ptr_Function_v4float Function
%247 = OpVariable %_ptr_Function_bool Function
%249 = OpVariable %_ptr_Function_bool Function
%237 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%241 = OpLoad %v4float %237
OpStore %242 %241
%243 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%245 = OpLoad %v4float %243
OpStore %246 %245
OpStore %247 %false
OpStore %249 %true
%250 = OpFunctionCall %v4float %blend_hslc_h4h4h4bb %242 %246 %247 %249
OpStore %sk_FragColor %250
OpReturn
OpFunctionEnd
