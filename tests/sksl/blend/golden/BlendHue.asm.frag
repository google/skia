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
OpName %_0_blend_hue "_0_blend_hue"
OpName %_1_alpha "_1_alpha"
OpName %_2_sda "_2_sda"
OpName %_3_dsa "_3_dsa"
OpName %_4_blend_set_color_saturation "_4_blend_set_color_saturation"
OpName %_5_hueLumColor "_5_hueLumColor"
OpName %_6_17_blend_color_saturation "_6_17_blend_color_saturation"
OpName %_7_sat "_7_sat"
OpName %_8_18_blend_set_color_saturation_helper "_8_18_blend_set_color_saturation_helper"
OpName %_9_19_blend_set_color_saturation_helper "_9_19_blend_set_color_saturation_helper"
OpName %_10_20_blend_set_color_saturation_helper "_10_20_blend_set_color_saturation_helper"
OpName %_11_21_blend_set_color_saturation_helper "_11_21_blend_set_color_saturation_helper"
OpName %_12_22_blend_set_color_saturation_helper "_12_22_blend_set_color_saturation_helper"
OpName %_13_23_blend_set_color_saturation_helper "_13_23_blend_set_color_saturation_helper"
OpName %_14_blend_set_color_luminance "_14_blend_set_color_luminance"
OpName %_15_15_blend_color_luminance "_15_15_blend_color_luminance"
OpName %_16_lum "_16_lum"
OpName %_17_16_blend_color_luminance "_17_16_blend_color_luminance"
OpName %_18_result "_18_result"
OpName %_19_minComp "_19_minComp"
OpName %_20_maxComp "_20_maxComp"
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
OpDecorate %41 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%104 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%142 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%172 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%210 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%248 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%278 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%287 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%296 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%main = OpFunction %void None %14
%15 = OpLabel
%_0_blend_hue = OpVariable %_ptr_Function_v4float Function
%_1_alpha = OpVariable %_ptr_Function_float Function
%_2_sda = OpVariable %_ptr_Function_v3float Function
%_3_dsa = OpVariable %_ptr_Function_v3float Function
%_4_blend_set_color_saturation = OpVariable %_ptr_Function_v3float Function
%_5_hueLumColor = OpVariable %_ptr_Function_v3float Function
%_6_17_blend_color_saturation = OpVariable %_ptr_Function_float Function
%_7_sat = OpVariable %_ptr_Function_float Function
%_8_18_blend_set_color_saturation_helper = OpVariable %_ptr_Function_v3float Function
%84 = OpVariable %_ptr_Function_v3float Function
%_9_19_blend_set_color_saturation_helper = OpVariable %_ptr_Function_v3float Function
%123 = OpVariable %_ptr_Function_v3float Function
%_10_20_blend_set_color_saturation_helper = OpVariable %_ptr_Function_v3float Function
%153 = OpVariable %_ptr_Function_v3float Function
%_11_21_blend_set_color_saturation_helper = OpVariable %_ptr_Function_v3float Function
%191 = OpVariable %_ptr_Function_v3float Function
%_12_22_blend_set_color_saturation_helper = OpVariable %_ptr_Function_v3float Function
%229 = OpVariable %_ptr_Function_v3float Function
%_13_23_blend_set_color_saturation_helper = OpVariable %_ptr_Function_v3float Function
%259 = OpVariable %_ptr_Function_v3float Function
%_14_blend_set_color_luminance = OpVariable %_ptr_Function_v3float Function
%_15_15_blend_color_luminance = OpVariable %_ptr_Function_float Function
%_16_lum = OpVariable %_ptr_Function_float Function
%_17_16_blend_color_luminance = OpVariable %_ptr_Function_float Function
%_18_result = OpVariable %_ptr_Function_v3float Function
%_19_minComp = OpVariable %_ptr_Function_float Function
%_20_maxComp = OpVariable %_ptr_Function_float Function
%358 = OpVariable %_ptr_Function_v3float Function
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
%41 = OpLoad %v3float %_2_sda
OpStore %_5_hueLumColor %41
%45 = OpLoad %v3float %_3_dsa
%46 = OpCompositeExtract %float %45 0
%47 = OpLoad %v3float %_3_dsa
%48 = OpCompositeExtract %float %47 1
%44 = OpExtInst %float %1 FMax %46 %48
%49 = OpLoad %v3float %_3_dsa
%50 = OpCompositeExtract %float %49 2
%43 = OpExtInst %float %1 FMax %44 %50
%53 = OpLoad %v3float %_3_dsa
%54 = OpCompositeExtract %float %53 0
%55 = OpLoad %v3float %_3_dsa
%56 = OpCompositeExtract %float %55 1
%52 = OpExtInst %float %1 FMin %54 %56
%57 = OpLoad %v3float %_3_dsa
%58 = OpCompositeExtract %float %57 2
%51 = OpExtInst %float %1 FMin %52 %58
%59 = OpFSub %float %43 %51
OpStore %_6_17_blend_color_saturation %59
%61 = OpLoad %float %_6_17_blend_color_saturation
OpStore %_7_sat %61
%62 = OpLoad %v3float %_5_hueLumColor
%63 = OpCompositeExtract %float %62 0
%64 = OpLoad %v3float %_5_hueLumColor
%65 = OpCompositeExtract %float %64 1
%66 = OpFOrdLessThanEqual %bool %63 %65
OpSelectionMerge %69 None
OpBranchConditional %66 %67 %68
%67 = OpLabel
%70 = OpLoad %v3float %_5_hueLumColor
%71 = OpCompositeExtract %float %70 1
%72 = OpLoad %v3float %_5_hueLumColor
%73 = OpCompositeExtract %float %72 2
%74 = OpFOrdLessThanEqual %bool %71 %73
OpSelectionMerge %77 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%79 = OpLoad %v3float %_5_hueLumColor
%80 = OpCompositeExtract %float %79 0
%81 = OpLoad %v3float %_5_hueLumColor
%82 = OpCompositeExtract %float %81 2
%83 = OpFOrdLessThan %bool %80 %82
OpSelectionMerge %87 None
OpBranchConditional %83 %85 %86
%85 = OpLabel
%89 = OpLoad %float %_7_sat
%90 = OpLoad %v3float %_5_hueLumColor
%91 = OpCompositeExtract %float %90 1
%92 = OpLoad %v3float %_5_hueLumColor
%93 = OpCompositeExtract %float %92 0
%94 = OpFSub %float %91 %93
%95 = OpFMul %float %89 %94
%96 = OpLoad %v3float %_5_hueLumColor
%97 = OpCompositeExtract %float %96 2
%98 = OpLoad %v3float %_5_hueLumColor
%99 = OpCompositeExtract %float %98 0
%100 = OpFSub %float %97 %99
%101 = OpFDiv %float %95 %100
%102 = OpLoad %float %_7_sat
%103 = OpCompositeConstruct %v3float %float_0 %101 %102
OpStore %84 %103
OpBranch %87
%86 = OpLabel
OpStore %84 %104
OpBranch %87
%87 = OpLabel
%105 = OpLoad %v3float %84
OpStore %_8_18_blend_set_color_saturation_helper %105
%106 = OpLoad %v3float %_8_18_blend_set_color_saturation_helper
%107 = OpLoad %v3float %_5_hueLumColor
%108 = OpVectorShuffle %v3float %107 %106 3 4 5
OpStore %_5_hueLumColor %108
OpBranch %77
%76 = OpLabel
%109 = OpLoad %v3float %_5_hueLumColor
%110 = OpCompositeExtract %float %109 0
%111 = OpLoad %v3float %_5_hueLumColor
%112 = OpCompositeExtract %float %111 2
%113 = OpFOrdLessThanEqual %bool %110 %112
OpSelectionMerge %116 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%118 = OpLoad %v3float %_5_hueLumColor
%119 = OpCompositeExtract %float %118 0
%120 = OpLoad %v3float %_5_hueLumColor
%121 = OpCompositeExtract %float %120 1
%122 = OpFOrdLessThan %bool %119 %121
OpSelectionMerge %126 None
OpBranchConditional %122 %124 %125
%124 = OpLabel
%127 = OpLoad %float %_7_sat
%128 = OpLoad %v3float %_5_hueLumColor
%129 = OpCompositeExtract %float %128 2
%130 = OpLoad %v3float %_5_hueLumColor
%131 = OpCompositeExtract %float %130 0
%132 = OpFSub %float %129 %131
%133 = OpFMul %float %127 %132
%134 = OpLoad %v3float %_5_hueLumColor
%135 = OpCompositeExtract %float %134 1
%136 = OpLoad %v3float %_5_hueLumColor
%137 = OpCompositeExtract %float %136 0
%138 = OpFSub %float %135 %137
%139 = OpFDiv %float %133 %138
%140 = OpLoad %float %_7_sat
%141 = OpCompositeConstruct %v3float %float_0 %139 %140
OpStore %123 %141
OpBranch %126
%125 = OpLabel
OpStore %123 %142
OpBranch %126
%126 = OpLabel
%143 = OpLoad %v3float %123
OpStore %_9_19_blend_set_color_saturation_helper %143
%144 = OpLoad %v3float %_9_19_blend_set_color_saturation_helper
%145 = OpLoad %v3float %_5_hueLumColor
%146 = OpVectorShuffle %v3float %145 %144 3 5 4
OpStore %_5_hueLumColor %146
OpBranch %116
%115 = OpLabel
%148 = OpLoad %v3float %_5_hueLumColor
%149 = OpCompositeExtract %float %148 2
%150 = OpLoad %v3float %_5_hueLumColor
%151 = OpCompositeExtract %float %150 1
%152 = OpFOrdLessThan %bool %149 %151
OpSelectionMerge %156 None
OpBranchConditional %152 %154 %155
%154 = OpLabel
%157 = OpLoad %float %_7_sat
%158 = OpLoad %v3float %_5_hueLumColor
%159 = OpCompositeExtract %float %158 0
%160 = OpLoad %v3float %_5_hueLumColor
%161 = OpCompositeExtract %float %160 2
%162 = OpFSub %float %159 %161
%163 = OpFMul %float %157 %162
%164 = OpLoad %v3float %_5_hueLumColor
%165 = OpCompositeExtract %float %164 1
%166 = OpLoad %v3float %_5_hueLumColor
%167 = OpCompositeExtract %float %166 2
%168 = OpFSub %float %165 %167
%169 = OpFDiv %float %163 %168
%170 = OpLoad %float %_7_sat
%171 = OpCompositeConstruct %v3float %float_0 %169 %170
OpStore %153 %171
OpBranch %156
%155 = OpLabel
OpStore %153 %172
OpBranch %156
%156 = OpLabel
%173 = OpLoad %v3float %153
OpStore %_10_20_blend_set_color_saturation_helper %173
%174 = OpLoad %v3float %_10_20_blend_set_color_saturation_helper
%175 = OpLoad %v3float %_5_hueLumColor
%176 = OpVectorShuffle %v3float %175 %174 4 5 3
OpStore %_5_hueLumColor %176
OpBranch %116
%116 = OpLabel
OpBranch %77
%77 = OpLabel
OpBranch %69
%68 = OpLabel
%177 = OpLoad %v3float %_5_hueLumColor
%178 = OpCompositeExtract %float %177 0
%179 = OpLoad %v3float %_5_hueLumColor
%180 = OpCompositeExtract %float %179 2
%181 = OpFOrdLessThanEqual %bool %178 %180
OpSelectionMerge %184 None
OpBranchConditional %181 %182 %183
%182 = OpLabel
%186 = OpLoad %v3float %_5_hueLumColor
%187 = OpCompositeExtract %float %186 1
%188 = OpLoad %v3float %_5_hueLumColor
%189 = OpCompositeExtract %float %188 2
%190 = OpFOrdLessThan %bool %187 %189
OpSelectionMerge %194 None
OpBranchConditional %190 %192 %193
%192 = OpLabel
%195 = OpLoad %float %_7_sat
%196 = OpLoad %v3float %_5_hueLumColor
%197 = OpCompositeExtract %float %196 0
%198 = OpLoad %v3float %_5_hueLumColor
%199 = OpCompositeExtract %float %198 1
%200 = OpFSub %float %197 %199
%201 = OpFMul %float %195 %200
%202 = OpLoad %v3float %_5_hueLumColor
%203 = OpCompositeExtract %float %202 2
%204 = OpLoad %v3float %_5_hueLumColor
%205 = OpCompositeExtract %float %204 1
%206 = OpFSub %float %203 %205
%207 = OpFDiv %float %201 %206
%208 = OpLoad %float %_7_sat
%209 = OpCompositeConstruct %v3float %float_0 %207 %208
OpStore %191 %209
OpBranch %194
%193 = OpLabel
OpStore %191 %210
OpBranch %194
%194 = OpLabel
%211 = OpLoad %v3float %191
OpStore %_11_21_blend_set_color_saturation_helper %211
%212 = OpLoad %v3float %_11_21_blend_set_color_saturation_helper
%213 = OpLoad %v3float %_5_hueLumColor
%214 = OpVectorShuffle %v3float %213 %212 4 3 5
OpStore %_5_hueLumColor %214
OpBranch %184
%183 = OpLabel
%215 = OpLoad %v3float %_5_hueLumColor
%216 = OpCompositeExtract %float %215 1
%217 = OpLoad %v3float %_5_hueLumColor
%218 = OpCompositeExtract %float %217 2
%219 = OpFOrdLessThanEqual %bool %216 %218
OpSelectionMerge %222 None
OpBranchConditional %219 %220 %221
%220 = OpLabel
%224 = OpLoad %v3float %_5_hueLumColor
%225 = OpCompositeExtract %float %224 1
%226 = OpLoad %v3float %_5_hueLumColor
%227 = OpCompositeExtract %float %226 0
%228 = OpFOrdLessThan %bool %225 %227
OpSelectionMerge %232 None
OpBranchConditional %228 %230 %231
%230 = OpLabel
%233 = OpLoad %float %_7_sat
%234 = OpLoad %v3float %_5_hueLumColor
%235 = OpCompositeExtract %float %234 2
%236 = OpLoad %v3float %_5_hueLumColor
%237 = OpCompositeExtract %float %236 1
%238 = OpFSub %float %235 %237
%239 = OpFMul %float %233 %238
%240 = OpLoad %v3float %_5_hueLumColor
%241 = OpCompositeExtract %float %240 0
%242 = OpLoad %v3float %_5_hueLumColor
%243 = OpCompositeExtract %float %242 1
%244 = OpFSub %float %241 %243
%245 = OpFDiv %float %239 %244
%246 = OpLoad %float %_7_sat
%247 = OpCompositeConstruct %v3float %float_0 %245 %246
OpStore %229 %247
OpBranch %232
%231 = OpLabel
OpStore %229 %248
OpBranch %232
%232 = OpLabel
%249 = OpLoad %v3float %229
OpStore %_12_22_blend_set_color_saturation_helper %249
%250 = OpLoad %v3float %_12_22_blend_set_color_saturation_helper
%251 = OpLoad %v3float %_5_hueLumColor
%252 = OpVectorShuffle %v3float %251 %250 5 3 4
OpStore %_5_hueLumColor %252
OpBranch %222
%221 = OpLabel
%254 = OpLoad %v3float %_5_hueLumColor
%255 = OpCompositeExtract %float %254 2
%256 = OpLoad %v3float %_5_hueLumColor
%257 = OpCompositeExtract %float %256 0
%258 = OpFOrdLessThan %bool %255 %257
OpSelectionMerge %262 None
OpBranchConditional %258 %260 %261
%260 = OpLabel
%263 = OpLoad %float %_7_sat
%264 = OpLoad %v3float %_5_hueLumColor
%265 = OpCompositeExtract %float %264 1
%266 = OpLoad %v3float %_5_hueLumColor
%267 = OpCompositeExtract %float %266 2
%268 = OpFSub %float %265 %267
%269 = OpFMul %float %263 %268
%270 = OpLoad %v3float %_5_hueLumColor
%271 = OpCompositeExtract %float %270 0
%272 = OpLoad %v3float %_5_hueLumColor
%273 = OpCompositeExtract %float %272 2
%274 = OpFSub %float %271 %273
%275 = OpFDiv %float %269 %274
%276 = OpLoad %float %_7_sat
%277 = OpCompositeConstruct %v3float %float_0 %275 %276
OpStore %259 %277
OpBranch %262
%261 = OpLabel
OpStore %259 %278
OpBranch %262
%262 = OpLabel
%279 = OpLoad %v3float %259
OpStore %_13_23_blend_set_color_saturation_helper %279
%280 = OpLoad %v3float %_13_23_blend_set_color_saturation_helper
%281 = OpLoad %v3float %_5_hueLumColor
%282 = OpVectorShuffle %v3float %281 %280 5 4 3
OpStore %_5_hueLumColor %282
OpBranch %222
%222 = OpLabel
OpBranch %184
%184 = OpLabel
OpBranch %69
%69 = OpLabel
%283 = OpLoad %v3float %_5_hueLumColor
OpStore %_4_blend_set_color_saturation %283
%291 = OpLoad %v3float %_3_dsa
%286 = OpDot %float %287 %291
OpStore %_15_15_blend_color_luminance %286
%293 = OpLoad %float %_15_15_blend_color_luminance
OpStore %_16_lum %293
%297 = OpLoad %v3float %_4_blend_set_color_saturation
%295 = OpDot %float %296 %297
OpStore %_17_16_blend_color_luminance %295
%299 = OpLoad %float %_16_lum
%300 = OpLoad %float %_17_16_blend_color_luminance
%301 = OpFSub %float %299 %300
%302 = OpLoad %v3float %_4_blend_set_color_saturation
%303 = OpCompositeConstruct %v3float %301 %301 %301
%304 = OpFAdd %v3float %303 %302
OpStore %_18_result %304
%308 = OpLoad %v3float %_18_result
%309 = OpCompositeExtract %float %308 0
%310 = OpLoad %v3float %_18_result
%311 = OpCompositeExtract %float %310 1
%307 = OpExtInst %float %1 FMin %309 %311
%312 = OpLoad %v3float %_18_result
%313 = OpCompositeExtract %float %312 2
%306 = OpExtInst %float %1 FMin %307 %313
OpStore %_19_minComp %306
%317 = OpLoad %v3float %_18_result
%318 = OpCompositeExtract %float %317 0
%319 = OpLoad %v3float %_18_result
%320 = OpCompositeExtract %float %319 1
%316 = OpExtInst %float %1 FMax %318 %320
%321 = OpLoad %v3float %_18_result
%322 = OpCompositeExtract %float %321 2
%315 = OpExtInst %float %1 FMax %316 %322
OpStore %_20_maxComp %315
%324 = OpLoad %float %_19_minComp
%325 = OpFOrdLessThan %bool %324 %float_0
OpSelectionMerge %327 None
OpBranchConditional %325 %326 %327
%326 = OpLabel
%328 = OpLoad %float %_16_lum
%329 = OpLoad %float %_19_minComp
%330 = OpFOrdNotEqual %bool %328 %329
OpBranch %327
%327 = OpLabel
%331 = OpPhi %bool %false %69 %330 %326
OpSelectionMerge %333 None
OpBranchConditional %331 %332 %333
%332 = OpLabel
%334 = OpLoad %float %_16_lum
%335 = OpLoad %v3float %_18_result
%336 = OpLoad %float %_16_lum
%337 = OpCompositeConstruct %v3float %336 %336 %336
%338 = OpFSub %v3float %335 %337
%339 = OpLoad %float %_16_lum
%340 = OpVectorTimesScalar %v3float %338 %339
%341 = OpLoad %float %_16_lum
%342 = OpLoad %float %_19_minComp
%343 = OpFSub %float %341 %342
%345 = OpFDiv %float %float_1 %343
%346 = OpVectorTimesScalar %v3float %340 %345
%347 = OpCompositeConstruct %v3float %334 %334 %334
%348 = OpFAdd %v3float %347 %346
OpStore %_18_result %348
OpBranch %333
%333 = OpLabel
%349 = OpLoad %float %_20_maxComp
%350 = OpLoad %float %_1_alpha
%351 = OpFOrdGreaterThan %bool %349 %350
OpSelectionMerge %353 None
OpBranchConditional %351 %352 %353
%352 = OpLabel
%354 = OpLoad %float %_20_maxComp
%355 = OpLoad %float %_16_lum
%356 = OpFOrdNotEqual %bool %354 %355
OpBranch %353
%353 = OpLabel
%357 = OpPhi %bool %false %333 %356 %352
OpSelectionMerge %361 None
OpBranchConditional %357 %359 %360
%359 = OpLabel
%362 = OpLoad %float %_16_lum
%363 = OpLoad %v3float %_18_result
%364 = OpLoad %float %_16_lum
%365 = OpCompositeConstruct %v3float %364 %364 %364
%366 = OpFSub %v3float %363 %365
%367 = OpLoad %float %_1_alpha
%368 = OpLoad %float %_16_lum
%369 = OpFSub %float %367 %368
%370 = OpVectorTimesScalar %v3float %366 %369
%371 = OpLoad %float %_20_maxComp
%372 = OpLoad %float %_16_lum
%373 = OpFSub %float %371 %372
%374 = OpFDiv %float %float_1 %373
%375 = OpVectorTimesScalar %v3float %370 %374
%376 = OpCompositeConstruct %v3float %362 %362 %362
%377 = OpFAdd %v3float %376 %375
OpStore %358 %377
OpBranch %361
%360 = OpLabel
%378 = OpLoad %v3float %_18_result
OpStore %358 %378
OpBranch %361
%361 = OpLabel
%379 = OpLoad %v3float %358
OpStore %_14_blend_set_color_luminance %379
%380 = OpLoad %v3float %_14_blend_set_color_luminance
%381 = OpLoad %v4float %dst
%382 = OpVectorShuffle %v3float %381 %381 0 1 2
%383 = OpFAdd %v3float %380 %382
%384 = OpLoad %v3float %_3_dsa
%385 = OpFSub %v3float %383 %384
%386 = OpLoad %v4float %src
%387 = OpVectorShuffle %v3float %386 %386 0 1 2
%388 = OpFAdd %v3float %385 %387
%389 = OpLoad %v3float %_2_sda
%390 = OpFSub %v3float %388 %389
%391 = OpCompositeExtract %float %390 0
%392 = OpCompositeExtract %float %390 1
%393 = OpCompositeExtract %float %390 2
%394 = OpLoad %v4float %src
%395 = OpCompositeExtract %float %394 3
%396 = OpLoad %v4float %dst
%397 = OpCompositeExtract %float %396 3
%398 = OpFAdd %float %395 %397
%399 = OpLoad %float %_1_alpha
%400 = OpFSub %float %398 %399
%401 = OpCompositeConstruct %v4float %391 %392 %393 %400
OpStore %_0_blend_hue %401
%402 = OpLoad %v4float %_0_blend_hue
OpStore %sk_FragColor %402
OpReturn
OpFunctionEnd
