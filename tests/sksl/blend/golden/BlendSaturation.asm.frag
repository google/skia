OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %_blend_set_color_saturation_helper "_blend_set_color_saturation_helper"
OpName %main "main"
OpName %_0_blend_saturation "_0_blend_saturation"
OpName %_1_alpha "_1_alpha"
OpName %_2_sda "_2_sda"
OpName %_3_dsa "_3_dsa"
OpName %_4_blend_set_color_saturation "_4_blend_set_color_saturation"
OpName %_5_hueLumColor "_5_hueLumColor"
OpName %_6_13_blend_color_saturation "_6_13_blend_color_saturation"
OpName %_7_sat "_7_sat"
OpName %_8_blend_set_color_luminance "_8_blend_set_color_luminance"
OpName %_9_11_blend_color_luminance "_9_11_blend_color_luminance"
OpName %_10_lum "_10_lum"
OpName %_11_12_blend_color_luminance "_11_12_blend_color_luminance"
OpName %_12_result "_12_result"
OpName %_13_minComp "_13_minComp"
OpName %_14_maxComp "_14_maxComp"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%46 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%void = OpTypeVoid
%49 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%185 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%194 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%_blend_set_color_saturation_helper = OpFunction %v3float None %15
%18 = OpFunctionParameter %_ptr_Function_v3float
%19 = OpFunctionParameter %_ptr_Function_float
%20 = OpLabel
%26 = OpVariable %_ptr_Function_v3float Function
%21 = OpLoad %v3float %18
%22 = OpCompositeExtract %float %21 0
%23 = OpLoad %v3float %18
%24 = OpCompositeExtract %float %23 2
%25 = OpFOrdLessThan %bool %22 %24
OpSelectionMerge %29 None
OpBranchConditional %25 %27 %28
%27 = OpLabel
%31 = OpLoad %float %19
%32 = OpLoad %v3float %18
%33 = OpCompositeExtract %float %32 1
%34 = OpLoad %v3float %18
%35 = OpCompositeExtract %float %34 0
%36 = OpFSub %float %33 %35
%37 = OpFMul %float %31 %36
%38 = OpLoad %v3float %18
%39 = OpCompositeExtract %float %38 2
%40 = OpLoad %v3float %18
%41 = OpCompositeExtract %float %40 0
%42 = OpFSub %float %39 %41
%43 = OpFDiv %float %37 %42
%44 = OpLoad %float %19
%45 = OpCompositeConstruct %v3float %float_0 %43 %44
OpStore %26 %45
OpBranch %29
%28 = OpLabel
OpStore %26 %46
OpBranch %29
%29 = OpLabel
%47 = OpLoad %v3float %26
OpReturnValue %47
OpFunctionEnd
%main = OpFunction %void None %49
%50 = OpLabel
%_0_blend_saturation = OpVariable %_ptr_Function_v4float Function
%_1_alpha = OpVariable %_ptr_Function_float Function
%_2_sda = OpVariable %_ptr_Function_v3float Function
%_3_dsa = OpVariable %_ptr_Function_v3float Function
%_4_blend_set_color_saturation = OpVariable %_ptr_Function_v3float Function
%_5_hueLumColor = OpVariable %_ptr_Function_v3float Function
%_6_13_blend_color_saturation = OpVariable %_ptr_Function_float Function
%_7_sat = OpVariable %_ptr_Function_float Function
%111 = OpVariable %_ptr_Function_v3float Function
%113 = OpVariable %_ptr_Function_float Function
%127 = OpVariable %_ptr_Function_v3float Function
%129 = OpVariable %_ptr_Function_float Function
%135 = OpVariable %_ptr_Function_v3float Function
%137 = OpVariable %_ptr_Function_float Function
%151 = OpVariable %_ptr_Function_v3float Function
%153 = OpVariable %_ptr_Function_float Function
%167 = OpVariable %_ptr_Function_v3float Function
%169 = OpVariable %_ptr_Function_float Function
%175 = OpVariable %_ptr_Function_v3float Function
%177 = OpVariable %_ptr_Function_float Function
%_8_blend_set_color_luminance = OpVariable %_ptr_Function_v3float Function
%_9_11_blend_color_luminance = OpVariable %_ptr_Function_float Function
%_10_lum = OpVariable %_ptr_Function_float Function
%_11_12_blend_color_luminance = OpVariable %_ptr_Function_float Function
%_12_result = OpVariable %_ptr_Function_v3float Function
%_13_minComp = OpVariable %_ptr_Function_float Function
%_14_maxComp = OpVariable %_ptr_Function_float Function
%256 = OpVariable %_ptr_Function_v3float Function
%54 = OpLoad %v4float %dst
%55 = OpCompositeExtract %float %54 3
%56 = OpLoad %v4float %src
%57 = OpCompositeExtract %float %56 3
%58 = OpFMul %float %55 %57
OpStore %_1_alpha %58
%60 = OpLoad %v4float %src
%61 = OpVectorShuffle %v3float %60 %60 0 1 2
%62 = OpLoad %v4float %dst
%63 = OpCompositeExtract %float %62 3
%64 = OpVectorTimesScalar %v3float %61 %63
OpStore %_2_sda %64
%66 = OpLoad %v4float %dst
%67 = OpVectorShuffle %v3float %66 %66 0 1 2
%68 = OpLoad %v4float %src
%69 = OpCompositeExtract %float %68 3
%70 = OpVectorTimesScalar %v3float %67 %69
OpStore %_3_dsa %70
%73 = OpLoad %v3float %_3_dsa
OpStore %_5_hueLumColor %73
%77 = OpLoad %v3float %_2_sda
%78 = OpCompositeExtract %float %77 0
%79 = OpLoad %v3float %_2_sda
%80 = OpCompositeExtract %float %79 1
%76 = OpExtInst %float %1 FMax %78 %80
%81 = OpLoad %v3float %_2_sda
%82 = OpCompositeExtract %float %81 2
%75 = OpExtInst %float %1 FMax %76 %82
%85 = OpLoad %v3float %_2_sda
%86 = OpCompositeExtract %float %85 0
%87 = OpLoad %v3float %_2_sda
%88 = OpCompositeExtract %float %87 1
%84 = OpExtInst %float %1 FMin %86 %88
%89 = OpLoad %v3float %_2_sda
%90 = OpCompositeExtract %float %89 2
%83 = OpExtInst %float %1 FMin %84 %90
%91 = OpFSub %float %75 %83
OpStore %_6_13_blend_color_saturation %91
%93 = OpLoad %float %_6_13_blend_color_saturation
OpStore %_7_sat %93
%94 = OpLoad %v3float %_5_hueLumColor
%95 = OpCompositeExtract %float %94 0
%96 = OpLoad %v3float %_5_hueLumColor
%97 = OpCompositeExtract %float %96 1
%98 = OpFOrdLessThanEqual %bool %95 %97
OpSelectionMerge %101 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%102 = OpLoad %v3float %_5_hueLumColor
%103 = OpCompositeExtract %float %102 1
%104 = OpLoad %v3float %_5_hueLumColor
%105 = OpCompositeExtract %float %104 2
%106 = OpFOrdLessThanEqual %bool %103 %105
OpSelectionMerge %109 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%110 = OpLoad %v3float %_5_hueLumColor
OpStore %111 %110
%112 = OpLoad %float %_7_sat
OpStore %113 %112
%114 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %111 %113
%115 = OpLoad %v3float %_5_hueLumColor
%116 = OpVectorShuffle %v3float %115 %114 3 4 5
OpStore %_5_hueLumColor %116
OpBranch %109
%108 = OpLabel
%117 = OpLoad %v3float %_5_hueLumColor
%118 = OpCompositeExtract %float %117 0
%119 = OpLoad %v3float %_5_hueLumColor
%120 = OpCompositeExtract %float %119 2
%121 = OpFOrdLessThanEqual %bool %118 %120
OpSelectionMerge %124 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%125 = OpLoad %v3float %_5_hueLumColor
%126 = OpVectorShuffle %v3float %125 %125 0 2 1
OpStore %127 %126
%128 = OpLoad %float %_7_sat
OpStore %129 %128
%130 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %127 %129
%131 = OpLoad %v3float %_5_hueLumColor
%132 = OpVectorShuffle %v3float %131 %130 3 5 4
OpStore %_5_hueLumColor %132
OpBranch %124
%123 = OpLabel
%133 = OpLoad %v3float %_5_hueLumColor
%134 = OpVectorShuffle %v3float %133 %133 2 0 1
OpStore %135 %134
%136 = OpLoad %float %_7_sat
OpStore %137 %136
%138 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %135 %137
%139 = OpLoad %v3float %_5_hueLumColor
%140 = OpVectorShuffle %v3float %139 %138 4 5 3
OpStore %_5_hueLumColor %140
OpBranch %124
%124 = OpLabel
OpBranch %109
%109 = OpLabel
OpBranch %101
%100 = OpLabel
%141 = OpLoad %v3float %_5_hueLumColor
%142 = OpCompositeExtract %float %141 0
%143 = OpLoad %v3float %_5_hueLumColor
%144 = OpCompositeExtract %float %143 2
%145 = OpFOrdLessThanEqual %bool %142 %144
OpSelectionMerge %148 None
OpBranchConditional %145 %146 %147
%146 = OpLabel
%149 = OpLoad %v3float %_5_hueLumColor
%150 = OpVectorShuffle %v3float %149 %149 1 0 2
OpStore %151 %150
%152 = OpLoad %float %_7_sat
OpStore %153 %152
%154 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %151 %153
%155 = OpLoad %v3float %_5_hueLumColor
%156 = OpVectorShuffle %v3float %155 %154 4 3 5
OpStore %_5_hueLumColor %156
OpBranch %148
%147 = OpLabel
%157 = OpLoad %v3float %_5_hueLumColor
%158 = OpCompositeExtract %float %157 1
%159 = OpLoad %v3float %_5_hueLumColor
%160 = OpCompositeExtract %float %159 2
%161 = OpFOrdLessThanEqual %bool %158 %160
OpSelectionMerge %164 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%165 = OpLoad %v3float %_5_hueLumColor
%166 = OpVectorShuffle %v3float %165 %165 1 2 0
OpStore %167 %166
%168 = OpLoad %float %_7_sat
OpStore %169 %168
%170 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %167 %169
%171 = OpLoad %v3float %_5_hueLumColor
%172 = OpVectorShuffle %v3float %171 %170 5 3 4
OpStore %_5_hueLumColor %172
OpBranch %164
%163 = OpLabel
%173 = OpLoad %v3float %_5_hueLumColor
%174 = OpVectorShuffle %v3float %173 %173 2 1 0
OpStore %175 %174
%176 = OpLoad %float %_7_sat
OpStore %177 %176
%178 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %175 %177
%179 = OpLoad %v3float %_5_hueLumColor
%180 = OpVectorShuffle %v3float %179 %178 5 4 3
OpStore %_5_hueLumColor %180
OpBranch %164
%164 = OpLabel
OpBranch %148
%148 = OpLabel
OpBranch %101
%101 = OpLabel
%181 = OpLoad %v3float %_5_hueLumColor
OpStore %_4_blend_set_color_saturation %181
%189 = OpLoad %v3float %_3_dsa
%184 = OpDot %float %185 %189
OpStore %_9_11_blend_color_luminance %184
%191 = OpLoad %float %_9_11_blend_color_luminance
OpStore %_10_lum %191
%195 = OpLoad %v3float %_4_blend_set_color_saturation
%193 = OpDot %float %194 %195
OpStore %_11_12_blend_color_luminance %193
%197 = OpLoad %float %_10_lum
%198 = OpLoad %float %_11_12_blend_color_luminance
%199 = OpFSub %float %197 %198
%200 = OpLoad %v3float %_4_blend_set_color_saturation
%201 = OpCompositeConstruct %v3float %199 %199 %199
%202 = OpFAdd %v3float %201 %200
OpStore %_12_result %202
%206 = OpLoad %v3float %_12_result
%207 = OpCompositeExtract %float %206 0
%208 = OpLoad %v3float %_12_result
%209 = OpCompositeExtract %float %208 1
%205 = OpExtInst %float %1 FMin %207 %209
%210 = OpLoad %v3float %_12_result
%211 = OpCompositeExtract %float %210 2
%204 = OpExtInst %float %1 FMin %205 %211
OpStore %_13_minComp %204
%215 = OpLoad %v3float %_12_result
%216 = OpCompositeExtract %float %215 0
%217 = OpLoad %v3float %_12_result
%218 = OpCompositeExtract %float %217 1
%214 = OpExtInst %float %1 FMax %216 %218
%219 = OpLoad %v3float %_12_result
%220 = OpCompositeExtract %float %219 2
%213 = OpExtInst %float %1 FMax %214 %220
OpStore %_14_maxComp %213
%222 = OpLoad %float %_13_minComp
%223 = OpFOrdLessThan %bool %222 %float_0
OpSelectionMerge %225 None
OpBranchConditional %223 %224 %225
%224 = OpLabel
%226 = OpLoad %float %_10_lum
%227 = OpLoad %float %_13_minComp
%228 = OpFOrdNotEqual %bool %226 %227
OpBranch %225
%225 = OpLabel
%229 = OpPhi %bool %false %101 %228 %224
OpSelectionMerge %231 None
OpBranchConditional %229 %230 %231
%230 = OpLabel
%232 = OpLoad %float %_10_lum
%233 = OpLoad %v3float %_12_result
%234 = OpLoad %float %_10_lum
%235 = OpCompositeConstruct %v3float %234 %234 %234
%236 = OpFSub %v3float %233 %235
%237 = OpLoad %float %_10_lum
%238 = OpVectorTimesScalar %v3float %236 %237
%239 = OpLoad %float %_10_lum
%240 = OpLoad %float %_13_minComp
%241 = OpFSub %float %239 %240
%243 = OpFDiv %float %float_1 %241
%244 = OpVectorTimesScalar %v3float %238 %243
%245 = OpCompositeConstruct %v3float %232 %232 %232
%246 = OpFAdd %v3float %245 %244
OpStore %_12_result %246
OpBranch %231
%231 = OpLabel
%247 = OpLoad %float %_14_maxComp
%248 = OpLoad %float %_1_alpha
%249 = OpFOrdGreaterThan %bool %247 %248
OpSelectionMerge %251 None
OpBranchConditional %249 %250 %251
%250 = OpLabel
%252 = OpLoad %float %_14_maxComp
%253 = OpLoad %float %_10_lum
%254 = OpFOrdNotEqual %bool %252 %253
OpBranch %251
%251 = OpLabel
%255 = OpPhi %bool %false %231 %254 %250
OpSelectionMerge %259 None
OpBranchConditional %255 %257 %258
%257 = OpLabel
%260 = OpLoad %float %_10_lum
%261 = OpLoad %v3float %_12_result
%262 = OpLoad %float %_10_lum
%263 = OpCompositeConstruct %v3float %262 %262 %262
%264 = OpFSub %v3float %261 %263
%265 = OpLoad %float %_1_alpha
%266 = OpLoad %float %_10_lum
%267 = OpFSub %float %265 %266
%268 = OpVectorTimesScalar %v3float %264 %267
%269 = OpLoad %float %_14_maxComp
%270 = OpLoad %float %_10_lum
%271 = OpFSub %float %269 %270
%272 = OpFDiv %float %float_1 %271
%273 = OpVectorTimesScalar %v3float %268 %272
%274 = OpCompositeConstruct %v3float %260 %260 %260
%275 = OpFAdd %v3float %274 %273
OpStore %256 %275
OpBranch %259
%258 = OpLabel
%276 = OpLoad %v3float %_12_result
OpStore %256 %276
OpBranch %259
%259 = OpLabel
%277 = OpLoad %v3float %256
OpStore %_8_blend_set_color_luminance %277
%278 = OpLoad %v3float %_8_blend_set_color_luminance
%279 = OpLoad %v4float %dst
%280 = OpVectorShuffle %v3float %279 %279 0 1 2
%281 = OpFAdd %v3float %278 %280
%282 = OpLoad %v3float %_3_dsa
%283 = OpFSub %v3float %281 %282
%284 = OpLoad %v4float %src
%285 = OpVectorShuffle %v3float %284 %284 0 1 2
%286 = OpFAdd %v3float %283 %285
%287 = OpLoad %v3float %_2_sda
%288 = OpFSub %v3float %286 %287
%289 = OpCompositeExtract %float %288 0
%290 = OpCompositeExtract %float %288 1
%291 = OpCompositeExtract %float %288 2
%292 = OpLoad %v4float %src
%293 = OpCompositeExtract %float %292 3
%294 = OpLoad %v4float %dst
%295 = OpCompositeExtract %float %294 3
%296 = OpFAdd %float %293 %295
%297 = OpLoad %float %_1_alpha
%298 = OpFSub %float %296 %297
%299 = OpCompositeConstruct %v4float %289 %290 %291 %298
OpStore %_0_blend_saturation %299
%300 = OpLoad %v4float %_0_blend_saturation
OpStore %sk_FragColor %300
OpReturn
OpFunctionEnd
