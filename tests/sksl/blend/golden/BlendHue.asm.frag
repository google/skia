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
OpName %_5_17_blend_color_saturation "_5_17_blend_color_saturation"
OpName %_6_sat "_6_sat"
OpName %_7_18_blend_set_color_saturation_helper "_7_18_blend_set_color_saturation_helper"
OpName %_8_19_blend_set_color_saturation_helper "_8_19_blend_set_color_saturation_helper"
OpName %_9_20_blend_set_color_saturation_helper "_9_20_blend_set_color_saturation_helper"
OpName %_10_21_blend_set_color_saturation_helper "_10_21_blend_set_color_saturation_helper"
OpName %_11_22_blend_set_color_saturation_helper "_11_22_blend_set_color_saturation_helper"
OpName %_12_23_blend_set_color_saturation_helper "_12_23_blend_set_color_saturation_helper"
OpName %_13_blend_set_color_luminance "_13_blend_set_color_luminance"
OpName %_14_15_blend_color_luminance "_14_15_blend_color_luminance"
OpName %_15_lum "_15_lum"
OpName %_16_16_blend_color_luminance "_16_16_blend_color_luminance"
OpName %_17_result "_17_result"
OpName %_18_minComp "_18_minComp"
OpName %_19_maxComp "_19_maxComp"
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
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
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
%102 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%138 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%167 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%204 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%241 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%270 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%277 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%286 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%main = OpFunction %void None %14
%15 = OpLabel
%_0_blend_hue = OpVariable %_ptr_Function_v4float Function
%_1_alpha = OpVariable %_ptr_Function_float Function
%_2_sda = OpVariable %_ptr_Function_v3float Function
%_3_dsa = OpVariable %_ptr_Function_v3float Function
%_4_blend_set_color_saturation = OpVariable %_ptr_Function_v3float Function
%_5_17_blend_color_saturation = OpVariable %_ptr_Function_float Function
%_6_sat = OpVariable %_ptr_Function_float Function
%_7_18_blend_set_color_saturation_helper = OpVariable %_ptr_Function_v3float Function
%82 = OpVariable %_ptr_Function_v3float Function
%_8_19_blend_set_color_saturation_helper = OpVariable %_ptr_Function_v3float Function
%119 = OpVariable %_ptr_Function_v3float Function
%_9_20_blend_set_color_saturation_helper = OpVariable %_ptr_Function_v3float Function
%148 = OpVariable %_ptr_Function_v3float Function
%_10_21_blend_set_color_saturation_helper = OpVariable %_ptr_Function_v3float Function
%185 = OpVariable %_ptr_Function_v3float Function
%_11_22_blend_set_color_saturation_helper = OpVariable %_ptr_Function_v3float Function
%222 = OpVariable %_ptr_Function_v3float Function
%_12_23_blend_set_color_saturation_helper = OpVariable %_ptr_Function_v3float Function
%251 = OpVariable %_ptr_Function_v3float Function
%_13_blend_set_color_luminance = OpVariable %_ptr_Function_v3float Function
%_14_15_blend_color_luminance = OpVariable %_ptr_Function_float Function
%_15_lum = OpVariable %_ptr_Function_float Function
%_16_16_blend_color_luminance = OpVariable %_ptr_Function_float Function
%_17_result = OpVariable %_ptr_Function_v3float Function
%_18_minComp = OpVariable %_ptr_Function_float Function
%_19_maxComp = OpVariable %_ptr_Function_float Function
%348 = OpVariable %_ptr_Function_v3float Function
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
%43 = OpLoad %v3float %_3_dsa
%44 = OpCompositeExtract %float %43 0
%45 = OpLoad %v3float %_3_dsa
%46 = OpCompositeExtract %float %45 1
%42 = OpExtInst %float %1 FMax %44 %46
%47 = OpLoad %v3float %_3_dsa
%48 = OpCompositeExtract %float %47 2
%41 = OpExtInst %float %1 FMax %42 %48
%51 = OpLoad %v3float %_3_dsa
%52 = OpCompositeExtract %float %51 0
%53 = OpLoad %v3float %_3_dsa
%54 = OpCompositeExtract %float %53 1
%50 = OpExtInst %float %1 FMin %52 %54
%55 = OpLoad %v3float %_3_dsa
%56 = OpCompositeExtract %float %55 2
%49 = OpExtInst %float %1 FMin %50 %56
%57 = OpFSub %float %41 %49
OpStore %_5_17_blend_color_saturation %57
%59 = OpLoad %float %_5_17_blend_color_saturation
OpStore %_6_sat %59
%60 = OpLoad %v3float %_2_sda
%61 = OpCompositeExtract %float %60 0
%62 = OpLoad %v3float %_2_sda
%63 = OpCompositeExtract %float %62 1
%64 = OpFOrdLessThanEqual %bool %61 %63
OpSelectionMerge %67 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%68 = OpLoad %v3float %_2_sda
%69 = OpCompositeExtract %float %68 1
%70 = OpLoad %v3float %_2_sda
%71 = OpCompositeExtract %float %70 2
%72 = OpFOrdLessThanEqual %bool %69 %71
OpSelectionMerge %75 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%77 = OpLoad %v3float %_2_sda
%78 = OpCompositeExtract %float %77 0
%79 = OpLoad %v3float %_2_sda
%80 = OpCompositeExtract %float %79 2
%81 = OpFOrdLessThan %bool %78 %80
OpSelectionMerge %85 None
OpBranchConditional %81 %83 %84
%83 = OpLabel
%87 = OpLoad %float %_6_sat
%88 = OpLoad %v3float %_2_sda
%89 = OpCompositeExtract %float %88 1
%90 = OpLoad %v3float %_2_sda
%91 = OpCompositeExtract %float %90 0
%92 = OpFSub %float %89 %91
%93 = OpFMul %float %87 %92
%94 = OpLoad %v3float %_2_sda
%95 = OpCompositeExtract %float %94 2
%96 = OpLoad %v3float %_2_sda
%97 = OpCompositeExtract %float %96 0
%98 = OpFSub %float %95 %97
%99 = OpFDiv %float %93 %98
%100 = OpLoad %float %_6_sat
%101 = OpCompositeConstruct %v3float %float_0 %99 %100
OpStore %82 %101
OpBranch %85
%84 = OpLabel
OpStore %82 %102
OpBranch %85
%85 = OpLabel
%103 = OpLoad %v3float %82
OpStore %_7_18_blend_set_color_saturation_helper %103
%104 = OpLoad %v3float %_7_18_blend_set_color_saturation_helper
OpStore %_4_blend_set_color_saturation %104
OpBranch %75
%74 = OpLabel
%105 = OpLoad %v3float %_2_sda
%106 = OpCompositeExtract %float %105 0
%107 = OpLoad %v3float %_2_sda
%108 = OpCompositeExtract %float %107 2
%109 = OpFOrdLessThanEqual %bool %106 %108
OpSelectionMerge %112 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%114 = OpLoad %v3float %_2_sda
%115 = OpCompositeExtract %float %114 0
%116 = OpLoad %v3float %_2_sda
%117 = OpCompositeExtract %float %116 1
%118 = OpFOrdLessThan %bool %115 %117
OpSelectionMerge %122 None
OpBranchConditional %118 %120 %121
%120 = OpLabel
%123 = OpLoad %float %_6_sat
%124 = OpLoad %v3float %_2_sda
%125 = OpCompositeExtract %float %124 2
%126 = OpLoad %v3float %_2_sda
%127 = OpCompositeExtract %float %126 0
%128 = OpFSub %float %125 %127
%129 = OpFMul %float %123 %128
%130 = OpLoad %v3float %_2_sda
%131 = OpCompositeExtract %float %130 1
%132 = OpLoad %v3float %_2_sda
%133 = OpCompositeExtract %float %132 0
%134 = OpFSub %float %131 %133
%135 = OpFDiv %float %129 %134
%136 = OpLoad %float %_6_sat
%137 = OpCompositeConstruct %v3float %float_0 %135 %136
OpStore %119 %137
OpBranch %122
%121 = OpLabel
OpStore %119 %138
OpBranch %122
%122 = OpLabel
%139 = OpLoad %v3float %119
OpStore %_8_19_blend_set_color_saturation_helper %139
%140 = OpLoad %v3float %_8_19_blend_set_color_saturation_helper
%141 = OpVectorShuffle %v3float %140 %140 0 2 1
OpStore %_4_blend_set_color_saturation %141
OpBranch %112
%111 = OpLabel
%143 = OpLoad %v3float %_2_sda
%144 = OpCompositeExtract %float %143 2
%145 = OpLoad %v3float %_2_sda
%146 = OpCompositeExtract %float %145 1
%147 = OpFOrdLessThan %bool %144 %146
OpSelectionMerge %151 None
OpBranchConditional %147 %149 %150
%149 = OpLabel
%152 = OpLoad %float %_6_sat
%153 = OpLoad %v3float %_2_sda
%154 = OpCompositeExtract %float %153 0
%155 = OpLoad %v3float %_2_sda
%156 = OpCompositeExtract %float %155 2
%157 = OpFSub %float %154 %156
%158 = OpFMul %float %152 %157
%159 = OpLoad %v3float %_2_sda
%160 = OpCompositeExtract %float %159 1
%161 = OpLoad %v3float %_2_sda
%162 = OpCompositeExtract %float %161 2
%163 = OpFSub %float %160 %162
%164 = OpFDiv %float %158 %163
%165 = OpLoad %float %_6_sat
%166 = OpCompositeConstruct %v3float %float_0 %164 %165
OpStore %148 %166
OpBranch %151
%150 = OpLabel
OpStore %148 %167
OpBranch %151
%151 = OpLabel
%168 = OpLoad %v3float %148
OpStore %_9_20_blend_set_color_saturation_helper %168
%169 = OpLoad %v3float %_9_20_blend_set_color_saturation_helper
%170 = OpVectorShuffle %v3float %169 %169 1 2 0
OpStore %_4_blend_set_color_saturation %170
OpBranch %112
%112 = OpLabel
OpBranch %75
%75 = OpLabel
OpBranch %67
%66 = OpLabel
%171 = OpLoad %v3float %_2_sda
%172 = OpCompositeExtract %float %171 0
%173 = OpLoad %v3float %_2_sda
%174 = OpCompositeExtract %float %173 2
%175 = OpFOrdLessThanEqual %bool %172 %174
OpSelectionMerge %178 None
OpBranchConditional %175 %176 %177
%176 = OpLabel
%180 = OpLoad %v3float %_2_sda
%181 = OpCompositeExtract %float %180 1
%182 = OpLoad %v3float %_2_sda
%183 = OpCompositeExtract %float %182 2
%184 = OpFOrdLessThan %bool %181 %183
OpSelectionMerge %188 None
OpBranchConditional %184 %186 %187
%186 = OpLabel
%189 = OpLoad %float %_6_sat
%190 = OpLoad %v3float %_2_sda
%191 = OpCompositeExtract %float %190 0
%192 = OpLoad %v3float %_2_sda
%193 = OpCompositeExtract %float %192 1
%194 = OpFSub %float %191 %193
%195 = OpFMul %float %189 %194
%196 = OpLoad %v3float %_2_sda
%197 = OpCompositeExtract %float %196 2
%198 = OpLoad %v3float %_2_sda
%199 = OpCompositeExtract %float %198 1
%200 = OpFSub %float %197 %199
%201 = OpFDiv %float %195 %200
%202 = OpLoad %float %_6_sat
%203 = OpCompositeConstruct %v3float %float_0 %201 %202
OpStore %185 %203
OpBranch %188
%187 = OpLabel
OpStore %185 %204
OpBranch %188
%188 = OpLabel
%205 = OpLoad %v3float %185
OpStore %_10_21_blend_set_color_saturation_helper %205
%206 = OpLoad %v3float %_10_21_blend_set_color_saturation_helper
%207 = OpVectorShuffle %v3float %206 %206 1 0 2
OpStore %_4_blend_set_color_saturation %207
OpBranch %178
%177 = OpLabel
%208 = OpLoad %v3float %_2_sda
%209 = OpCompositeExtract %float %208 1
%210 = OpLoad %v3float %_2_sda
%211 = OpCompositeExtract %float %210 2
%212 = OpFOrdLessThanEqual %bool %209 %211
OpSelectionMerge %215 None
OpBranchConditional %212 %213 %214
%213 = OpLabel
%217 = OpLoad %v3float %_2_sda
%218 = OpCompositeExtract %float %217 1
%219 = OpLoad %v3float %_2_sda
%220 = OpCompositeExtract %float %219 0
%221 = OpFOrdLessThan %bool %218 %220
OpSelectionMerge %225 None
OpBranchConditional %221 %223 %224
%223 = OpLabel
%226 = OpLoad %float %_6_sat
%227 = OpLoad %v3float %_2_sda
%228 = OpCompositeExtract %float %227 2
%229 = OpLoad %v3float %_2_sda
%230 = OpCompositeExtract %float %229 1
%231 = OpFSub %float %228 %230
%232 = OpFMul %float %226 %231
%233 = OpLoad %v3float %_2_sda
%234 = OpCompositeExtract %float %233 0
%235 = OpLoad %v3float %_2_sda
%236 = OpCompositeExtract %float %235 1
%237 = OpFSub %float %234 %236
%238 = OpFDiv %float %232 %237
%239 = OpLoad %float %_6_sat
%240 = OpCompositeConstruct %v3float %float_0 %238 %239
OpStore %222 %240
OpBranch %225
%224 = OpLabel
OpStore %222 %241
OpBranch %225
%225 = OpLabel
%242 = OpLoad %v3float %222
OpStore %_11_22_blend_set_color_saturation_helper %242
%243 = OpLoad %v3float %_11_22_blend_set_color_saturation_helper
%244 = OpVectorShuffle %v3float %243 %243 2 0 1
OpStore %_4_blend_set_color_saturation %244
OpBranch %215
%214 = OpLabel
%246 = OpLoad %v3float %_2_sda
%247 = OpCompositeExtract %float %246 2
%248 = OpLoad %v3float %_2_sda
%249 = OpCompositeExtract %float %248 0
%250 = OpFOrdLessThan %bool %247 %249
OpSelectionMerge %254 None
OpBranchConditional %250 %252 %253
%252 = OpLabel
%255 = OpLoad %float %_6_sat
%256 = OpLoad %v3float %_2_sda
%257 = OpCompositeExtract %float %256 1
%258 = OpLoad %v3float %_2_sda
%259 = OpCompositeExtract %float %258 2
%260 = OpFSub %float %257 %259
%261 = OpFMul %float %255 %260
%262 = OpLoad %v3float %_2_sda
%263 = OpCompositeExtract %float %262 0
%264 = OpLoad %v3float %_2_sda
%265 = OpCompositeExtract %float %264 2
%266 = OpFSub %float %263 %265
%267 = OpFDiv %float %261 %266
%268 = OpLoad %float %_6_sat
%269 = OpCompositeConstruct %v3float %float_0 %267 %268
OpStore %251 %269
OpBranch %254
%253 = OpLabel
OpStore %251 %270
OpBranch %254
%254 = OpLabel
%271 = OpLoad %v3float %251
OpStore %_12_23_blend_set_color_saturation_helper %271
%272 = OpLoad %v3float %_12_23_blend_set_color_saturation_helper
%273 = OpVectorShuffle %v3float %272 %272 2 1 0
OpStore %_4_blend_set_color_saturation %273
OpBranch %215
%215 = OpLabel
OpBranch %178
%178 = OpLabel
OpBranch %67
%67 = OpLabel
%281 = OpLoad %v3float %_3_dsa
%276 = OpDot %float %277 %281
OpStore %_14_15_blend_color_luminance %276
%283 = OpLoad %float %_14_15_blend_color_luminance
OpStore %_15_lum %283
%287 = OpLoad %v3float %_4_blend_set_color_saturation
%285 = OpDot %float %286 %287
OpStore %_16_16_blend_color_luminance %285
%289 = OpLoad %float %_15_lum
%290 = OpLoad %float %_16_16_blend_color_luminance
%291 = OpFSub %float %289 %290
%292 = OpLoad %v3float %_4_blend_set_color_saturation
%293 = OpCompositeConstruct %v3float %291 %291 %291
%294 = OpFAdd %v3float %293 %292
OpStore %_17_result %294
%298 = OpLoad %v3float %_17_result
%299 = OpCompositeExtract %float %298 0
%300 = OpLoad %v3float %_17_result
%301 = OpCompositeExtract %float %300 1
%297 = OpExtInst %float %1 FMin %299 %301
%302 = OpLoad %v3float %_17_result
%303 = OpCompositeExtract %float %302 2
%296 = OpExtInst %float %1 FMin %297 %303
OpStore %_18_minComp %296
%307 = OpLoad %v3float %_17_result
%308 = OpCompositeExtract %float %307 0
%309 = OpLoad %v3float %_17_result
%310 = OpCompositeExtract %float %309 1
%306 = OpExtInst %float %1 FMax %308 %310
%311 = OpLoad %v3float %_17_result
%312 = OpCompositeExtract %float %311 2
%305 = OpExtInst %float %1 FMax %306 %312
OpStore %_19_maxComp %305
%314 = OpLoad %float %_18_minComp
%315 = OpFOrdLessThan %bool %314 %float_0
OpSelectionMerge %317 None
OpBranchConditional %315 %316 %317
%316 = OpLabel
%318 = OpLoad %float %_15_lum
%319 = OpLoad %float %_18_minComp
%320 = OpFOrdNotEqual %bool %318 %319
OpBranch %317
%317 = OpLabel
%321 = OpPhi %bool %false %67 %320 %316
OpSelectionMerge %323 None
OpBranchConditional %321 %322 %323
%322 = OpLabel
%324 = OpLoad %float %_15_lum
%325 = OpLoad %v3float %_17_result
%326 = OpLoad %float %_15_lum
%327 = OpCompositeConstruct %v3float %326 %326 %326
%328 = OpFSub %v3float %325 %327
%329 = OpLoad %float %_15_lum
%330 = OpVectorTimesScalar %v3float %328 %329
%331 = OpLoad %float %_15_lum
%332 = OpLoad %float %_18_minComp
%333 = OpFSub %float %331 %332
%335 = OpFDiv %float %float_1 %333
%336 = OpVectorTimesScalar %v3float %330 %335
%337 = OpCompositeConstruct %v3float %324 %324 %324
%338 = OpFAdd %v3float %337 %336
OpStore %_17_result %338
OpBranch %323
%323 = OpLabel
%339 = OpLoad %float %_19_maxComp
%340 = OpLoad %float %_1_alpha
%341 = OpFOrdGreaterThan %bool %339 %340
OpSelectionMerge %343 None
OpBranchConditional %341 %342 %343
%342 = OpLabel
%344 = OpLoad %float %_19_maxComp
%345 = OpLoad %float %_15_lum
%346 = OpFOrdNotEqual %bool %344 %345
OpBranch %343
%343 = OpLabel
%347 = OpPhi %bool %false %323 %346 %342
OpSelectionMerge %351 None
OpBranchConditional %347 %349 %350
%349 = OpLabel
%352 = OpLoad %float %_15_lum
%353 = OpLoad %v3float %_17_result
%354 = OpLoad %float %_15_lum
%355 = OpCompositeConstruct %v3float %354 %354 %354
%356 = OpFSub %v3float %353 %355
%357 = OpLoad %float %_1_alpha
%358 = OpLoad %float %_15_lum
%359 = OpFSub %float %357 %358
%360 = OpVectorTimesScalar %v3float %356 %359
%361 = OpLoad %float %_19_maxComp
%362 = OpLoad %float %_15_lum
%363 = OpFSub %float %361 %362
%364 = OpFDiv %float %float_1 %363
%365 = OpVectorTimesScalar %v3float %360 %364
%366 = OpCompositeConstruct %v3float %352 %352 %352
%367 = OpFAdd %v3float %366 %365
OpStore %348 %367
OpBranch %351
%350 = OpLabel
%368 = OpLoad %v3float %_17_result
OpStore %348 %368
OpBranch %351
%351 = OpLabel
%369 = OpLoad %v3float %348
OpStore %_13_blend_set_color_luminance %369
%370 = OpLoad %v3float %_13_blend_set_color_luminance
%371 = OpLoad %v4float %dst
%372 = OpVectorShuffle %v3float %371 %371 0 1 2
%373 = OpFAdd %v3float %370 %372
%374 = OpLoad %v3float %_3_dsa
%375 = OpFSub %v3float %373 %374
%376 = OpLoad %v4float %src
%377 = OpVectorShuffle %v3float %376 %376 0 1 2
%378 = OpFAdd %v3float %375 %377
%379 = OpLoad %v3float %_2_sda
%380 = OpFSub %v3float %378 %379
%381 = OpCompositeExtract %float %380 0
%382 = OpCompositeExtract %float %380 1
%383 = OpCompositeExtract %float %380 2
%384 = OpLoad %v4float %src
%385 = OpCompositeExtract %float %384 3
%386 = OpLoad %v4float %dst
%387 = OpCompositeExtract %float %386 3
%388 = OpFAdd %float %385 %387
%389 = OpLoad %float %_1_alpha
%390 = OpFSub %float %388 %389
%391 = OpCompositeConstruct %v4float %381 %382 %383 %390
OpStore %_0_blend_hue %391
%392 = OpLoad %v4float %_0_blend_hue
OpStore %sk_FragColor %392
OpReturn
OpFunctionEnd
