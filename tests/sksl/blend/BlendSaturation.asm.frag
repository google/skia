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
OpName %_guarded_divide_0 "_guarded_divide"
OpName %_blend_set_color_saturation_helper "_blend_set_color_saturation_helper"
OpName %main "main"
OpName %_1_alpha "_1_alpha"
OpName %_2_sda "_2_sda"
OpName %_3_dsa "_3_dsa"
OpName %_4_blend_set_color_saturation "_4_blend_set_color_saturation"
OpName %_5_sat "_5_sat"
OpName %_7_lum "_7_lum"
OpName %_8_result "_8_result"
OpName %_9_minComp "_9_minComp"
OpName %_10_maxComp "_10_maxComp"
OpName %_11_d "_11_d"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%16 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%25 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%65 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%void = OpTypeVoid
%68 = OpTypeFunction %void
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%192 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%_guarded_divide = OpFunction %float None %16
%18 = OpFunctionParameter %_ptr_Function_float
%19 = OpFunctionParameter %_ptr_Function_float
%20 = OpLabel
%21 = OpLoad %float %18
%22 = OpLoad %float %19
%23 = OpFDiv %float %21 %22
OpReturnValue %23
OpFunctionEnd
%_guarded_divide_0 = OpFunction %v3float None %25
%27 = OpFunctionParameter %_ptr_Function_v3float
%28 = OpFunctionParameter %_ptr_Function_float
%29 = OpLabel
%30 = OpLoad %v3float %27
%31 = OpLoad %float %28
%33 = OpFDiv %float %float_1 %31
%34 = OpVectorTimesScalar %v3float %30 %33
OpReturnValue %34
OpFunctionEnd
%_blend_set_color_saturation_helper = OpFunction %v3float None %25
%35 = OpFunctionParameter %_ptr_Function_v3float
%36 = OpFunctionParameter %_ptr_Function_float
%37 = OpLabel
%43 = OpVariable %_ptr_Function_v3float Function
%55 = OpVariable %_ptr_Function_float Function
%61 = OpVariable %_ptr_Function_float Function
%38 = OpLoad %v3float %35
%39 = OpCompositeExtract %float %38 0
%40 = OpLoad %v3float %35
%41 = OpCompositeExtract %float %40 2
%42 = OpFOrdLessThan %bool %39 %41
OpSelectionMerge %46 None
OpBranchConditional %42 %44 %45
%44 = OpLabel
%48 = OpLoad %float %36
%49 = OpLoad %v3float %35
%50 = OpCompositeExtract %float %49 1
%51 = OpLoad %v3float %35
%52 = OpCompositeExtract %float %51 0
%53 = OpFSub %float %50 %52
%54 = OpFMul %float %48 %53
OpStore %55 %54
%56 = OpLoad %v3float %35
%57 = OpCompositeExtract %float %56 2
%58 = OpLoad %v3float %35
%59 = OpCompositeExtract %float %58 0
%60 = OpFSub %float %57 %59
OpStore %61 %60
%62 = OpFunctionCall %float %_guarded_divide %55 %61
%63 = OpLoad %float %36
%64 = OpCompositeConstruct %v3float %float_0 %62 %63
OpStore %43 %64
OpBranch %46
%45 = OpLabel
OpStore %43 %65
OpBranch %46
%46 = OpLabel
%66 = OpLoad %v3float %43
OpReturnValue %66
OpFunctionEnd
%main = OpFunction %void None %68
%69 = OpLabel
%_1_alpha = OpVariable %_ptr_Function_float Function
%_2_sda = OpVariable %_ptr_Function_v3float Function
%_3_dsa = OpVariable %_ptr_Function_v3float Function
%_4_blend_set_color_saturation = OpVariable %_ptr_Function_v3float Function
%_5_sat = OpVariable %_ptr_Function_float Function
%124 = OpVariable %_ptr_Function_v3float Function
%126 = OpVariable %_ptr_Function_float Function
%138 = OpVariable %_ptr_Function_v3float Function
%140 = OpVariable %_ptr_Function_float Function
%145 = OpVariable %_ptr_Function_v3float Function
%147 = OpVariable %_ptr_Function_float Function
%160 = OpVariable %_ptr_Function_v3float Function
%162 = OpVariable %_ptr_Function_float Function
%175 = OpVariable %_ptr_Function_v3float Function
%177 = OpVariable %_ptr_Function_float Function
%182 = OpVariable %_ptr_Function_v3float Function
%184 = OpVariable %_ptr_Function_float Function
%_7_lum = OpVariable %_ptr_Function_float Function
%_8_result = OpVariable %_ptr_Function_v3float Function
%_9_minComp = OpVariable %_ptr_Function_float Function
%_10_maxComp = OpVariable %_ptr_Function_float Function
%_11_d = OpVariable %_ptr_Function_float Function
%255 = OpVariable %_ptr_Function_v3float Function
%268 = OpVariable %_ptr_Function_v3float Function
%272 = OpVariable %_ptr_Function_float Function
%71 = OpLoad %v4float %dst
%72 = OpCompositeExtract %float %71 3
%73 = OpLoad %v4float %src
%74 = OpCompositeExtract %float %73 3
%75 = OpFMul %float %72 %74
OpStore %_1_alpha %75
%77 = OpLoad %v4float %src
%78 = OpVectorShuffle %v3float %77 %77 0 1 2
%79 = OpLoad %v4float %dst
%80 = OpCompositeExtract %float %79 3
%81 = OpVectorTimesScalar %v3float %78 %80
OpStore %_2_sda %81
%83 = OpLoad %v4float %dst
%84 = OpVectorShuffle %v3float %83 %83 0 1 2
%85 = OpLoad %v4float %src
%86 = OpCompositeExtract %float %85 3
%87 = OpVectorTimesScalar %v3float %84 %86
OpStore %_3_dsa %87
%92 = OpLoad %v3float %_2_sda
%93 = OpCompositeExtract %float %92 0
%94 = OpLoad %v3float %_2_sda
%95 = OpCompositeExtract %float %94 1
%91 = OpExtInst %float %1 FMax %93 %95
%96 = OpLoad %v3float %_2_sda
%97 = OpCompositeExtract %float %96 2
%90 = OpExtInst %float %1 FMax %91 %97
%100 = OpLoad %v3float %_2_sda
%101 = OpCompositeExtract %float %100 0
%102 = OpLoad %v3float %_2_sda
%103 = OpCompositeExtract %float %102 1
%99 = OpExtInst %float %1 FMin %101 %103
%104 = OpLoad %v3float %_2_sda
%105 = OpCompositeExtract %float %104 2
%98 = OpExtInst %float %1 FMin %99 %105
%106 = OpFSub %float %90 %98
OpStore %_5_sat %106
%107 = OpLoad %v3float %_3_dsa
%108 = OpCompositeExtract %float %107 0
%109 = OpLoad %v3float %_3_dsa
%110 = OpCompositeExtract %float %109 1
%111 = OpFOrdLessThanEqual %bool %108 %110
OpSelectionMerge %114 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%115 = OpLoad %v3float %_3_dsa
%116 = OpCompositeExtract %float %115 1
%117 = OpLoad %v3float %_3_dsa
%118 = OpCompositeExtract %float %117 2
%119 = OpFOrdLessThanEqual %bool %116 %118
OpSelectionMerge %122 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%123 = OpLoad %v3float %_3_dsa
OpStore %124 %123
%125 = OpLoad %float %_5_sat
OpStore %126 %125
%127 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %124 %126
OpStore %_4_blend_set_color_saturation %127
OpBranch %122
%121 = OpLabel
%128 = OpLoad %v3float %_3_dsa
%129 = OpCompositeExtract %float %128 0
%130 = OpLoad %v3float %_3_dsa
%131 = OpCompositeExtract %float %130 2
%132 = OpFOrdLessThanEqual %bool %129 %131
OpSelectionMerge %135 None
OpBranchConditional %132 %133 %134
%133 = OpLabel
%136 = OpLoad %v3float %_3_dsa
%137 = OpVectorShuffle %v3float %136 %136 0 2 1
OpStore %138 %137
%139 = OpLoad %float %_5_sat
OpStore %140 %139
%141 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %138 %140
%142 = OpVectorShuffle %v3float %141 %141 0 2 1
OpStore %_4_blend_set_color_saturation %142
OpBranch %135
%134 = OpLabel
%143 = OpLoad %v3float %_3_dsa
%144 = OpVectorShuffle %v3float %143 %143 2 0 1
OpStore %145 %144
%146 = OpLoad %float %_5_sat
OpStore %147 %146
%148 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %145 %147
%149 = OpVectorShuffle %v3float %148 %148 1 2 0
OpStore %_4_blend_set_color_saturation %149
OpBranch %135
%135 = OpLabel
OpBranch %122
%122 = OpLabel
OpBranch %114
%113 = OpLabel
%150 = OpLoad %v3float %_3_dsa
%151 = OpCompositeExtract %float %150 0
%152 = OpLoad %v3float %_3_dsa
%153 = OpCompositeExtract %float %152 2
%154 = OpFOrdLessThanEqual %bool %151 %153
OpSelectionMerge %157 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
%158 = OpLoad %v3float %_3_dsa
%159 = OpVectorShuffle %v3float %158 %158 1 0 2
OpStore %160 %159
%161 = OpLoad %float %_5_sat
OpStore %162 %161
%163 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %160 %162
%164 = OpVectorShuffle %v3float %163 %163 1 0 2
OpStore %_4_blend_set_color_saturation %164
OpBranch %157
%156 = OpLabel
%165 = OpLoad %v3float %_3_dsa
%166 = OpCompositeExtract %float %165 1
%167 = OpLoad %v3float %_3_dsa
%168 = OpCompositeExtract %float %167 2
%169 = OpFOrdLessThanEqual %bool %166 %168
OpSelectionMerge %172 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%173 = OpLoad %v3float %_3_dsa
%174 = OpVectorShuffle %v3float %173 %173 1 2 0
OpStore %175 %174
%176 = OpLoad %float %_5_sat
OpStore %177 %176
%178 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %175 %177
%179 = OpVectorShuffle %v3float %178 %178 2 0 1
OpStore %_4_blend_set_color_saturation %179
OpBranch %172
%171 = OpLabel
%180 = OpLoad %v3float %_3_dsa
%181 = OpVectorShuffle %v3float %180 %180 2 1 0
OpStore %182 %181
%183 = OpLoad %float %_5_sat
OpStore %184 %183
%185 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %182 %184
%186 = OpVectorShuffle %v3float %185 %185 2 1 0
OpStore %_4_blend_set_color_saturation %186
OpBranch %172
%172 = OpLabel
OpBranch %157
%157 = OpLabel
OpBranch %114
%114 = OpLabel
%193 = OpLoad %v3float %_3_dsa
%188 = OpDot %float %192 %193
OpStore %_7_lum %188
%195 = OpLoad %float %_7_lum
%197 = OpLoad %v3float %_4_blend_set_color_saturation
%196 = OpDot %float %192 %197
%198 = OpFSub %float %195 %196
%199 = OpLoad %v3float %_4_blend_set_color_saturation
%200 = OpCompositeConstruct %v3float %198 %198 %198
%201 = OpFAdd %v3float %200 %199
OpStore %_8_result %201
%205 = OpLoad %v3float %_8_result
%206 = OpCompositeExtract %float %205 0
%207 = OpLoad %v3float %_8_result
%208 = OpCompositeExtract %float %207 1
%204 = OpExtInst %float %1 FMin %206 %208
%209 = OpLoad %v3float %_8_result
%210 = OpCompositeExtract %float %209 2
%203 = OpExtInst %float %1 FMin %204 %210
OpStore %_9_minComp %203
%214 = OpLoad %v3float %_8_result
%215 = OpCompositeExtract %float %214 0
%216 = OpLoad %v3float %_8_result
%217 = OpCompositeExtract %float %216 1
%213 = OpExtInst %float %1 FMax %215 %217
%218 = OpLoad %v3float %_8_result
%219 = OpCompositeExtract %float %218 2
%212 = OpExtInst %float %1 FMax %213 %219
OpStore %_10_maxComp %212
%221 = OpLoad %float %_9_minComp
%222 = OpFOrdLessThan %bool %221 %float_0
OpSelectionMerge %224 None
OpBranchConditional %222 %223 %224
%223 = OpLabel
%225 = OpLoad %float %_7_lum
%226 = OpLoad %float %_9_minComp
%227 = OpFOrdNotEqual %bool %225 %226
OpBranch %224
%224 = OpLabel
%228 = OpPhi %bool %false %114 %227 %223
OpSelectionMerge %230 None
OpBranchConditional %228 %229 %230
%229 = OpLabel
%232 = OpLoad %float %_7_lum
%233 = OpLoad %float %_9_minComp
%234 = OpFSub %float %232 %233
OpStore %_11_d %234
%235 = OpLoad %float %_7_lum
%236 = OpLoad %v3float %_8_result
%237 = OpLoad %float %_7_lum
%238 = OpCompositeConstruct %v3float %237 %237 %237
%239 = OpFSub %v3float %236 %238
%240 = OpLoad %float %_7_lum
%241 = OpLoad %float %_11_d
%242 = OpFDiv %float %240 %241
%243 = OpVectorTimesScalar %v3float %239 %242
%244 = OpCompositeConstruct %v3float %235 %235 %235
%245 = OpFAdd %v3float %244 %243
OpStore %_8_result %245
OpBranch %230
%230 = OpLabel
%246 = OpLoad %float %_10_maxComp
%247 = OpLoad %float %_1_alpha
%248 = OpFOrdGreaterThan %bool %246 %247
OpSelectionMerge %250 None
OpBranchConditional %248 %249 %250
%249 = OpLabel
%251 = OpLoad %float %_10_maxComp
%252 = OpLoad %float %_7_lum
%253 = OpFOrdNotEqual %bool %251 %252
OpBranch %250
%250 = OpLabel
%254 = OpPhi %bool %false %230 %253 %249
OpSelectionMerge %258 None
OpBranchConditional %254 %256 %257
%256 = OpLabel
%259 = OpLoad %float %_7_lum
%260 = OpLoad %v3float %_8_result
%261 = OpLoad %float %_7_lum
%262 = OpCompositeConstruct %v3float %261 %261 %261
%263 = OpFSub %v3float %260 %262
%264 = OpLoad %float %_1_alpha
%265 = OpLoad %float %_7_lum
%266 = OpFSub %float %264 %265
%267 = OpVectorTimesScalar %v3float %263 %266
OpStore %268 %267
%269 = OpLoad %float %_10_maxComp
%270 = OpLoad %float %_7_lum
%271 = OpFSub %float %269 %270
OpStore %272 %271
%273 = OpFunctionCall %v3float %_guarded_divide_0 %268 %272
%274 = OpCompositeConstruct %v3float %259 %259 %259
%275 = OpFAdd %v3float %274 %273
OpStore %255 %275
OpBranch %258
%257 = OpLabel
%276 = OpLoad %v3float %_8_result
OpStore %255 %276
OpBranch %258
%258 = OpLabel
%277 = OpLoad %v3float %255
%278 = OpLoad %v4float %dst
%279 = OpVectorShuffle %v3float %278 %278 0 1 2
%280 = OpFAdd %v3float %277 %279
%281 = OpLoad %v3float %_3_dsa
%282 = OpFSub %v3float %280 %281
%283 = OpLoad %v4float %src
%284 = OpVectorShuffle %v3float %283 %283 0 1 2
%285 = OpFAdd %v3float %282 %284
%286 = OpLoad %v3float %_2_sda
%287 = OpFSub %v3float %285 %286
%288 = OpCompositeExtract %float %287 0
%289 = OpCompositeExtract %float %287 1
%290 = OpCompositeExtract %float %287 2
%291 = OpLoad %v4float %src
%292 = OpCompositeExtract %float %291 3
%293 = OpLoad %v4float %dst
%294 = OpCompositeExtract %float %293 3
%295 = OpFAdd %float %292 %294
%296 = OpLoad %float %_1_alpha
%297 = OpFSub %float %295 %296
%298 = OpCompositeConstruct %v4float %288 %289 %290 %297
OpStore %sk_FragColor %298
OpReturn
OpFunctionEnd
