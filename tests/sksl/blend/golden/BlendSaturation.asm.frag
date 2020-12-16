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
OpName %_5_blend_color_saturation "_5_blend_color_saturation"
OpName %_6_sat "_6_sat"
OpName %_7_blend_set_color_luminance "_7_blend_set_color_luminance"
OpName %_8_blend_color_luminance "_8_blend_color_luminance"
OpName %_9_lum "_9_lum"
OpName %_10_blend_color_luminance "_10_blend_color_luminance"
OpName %_11_result "_11_result"
OpName %_12_minComp "_12_minComp"
OpName %_13_maxComp "_13_maxComp"
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
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
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
%175 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%184 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
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
%_5_blend_color_saturation = OpVariable %_ptr_Function_float Function
%_6_sat = OpVariable %_ptr_Function_float Function
%109 = OpVariable %_ptr_Function_v3float Function
%111 = OpVariable %_ptr_Function_float Function
%123 = OpVariable %_ptr_Function_v3float Function
%125 = OpVariable %_ptr_Function_float Function
%130 = OpVariable %_ptr_Function_v3float Function
%132 = OpVariable %_ptr_Function_float Function
%145 = OpVariable %_ptr_Function_v3float Function
%147 = OpVariable %_ptr_Function_float Function
%160 = OpVariable %_ptr_Function_v3float Function
%162 = OpVariable %_ptr_Function_float Function
%167 = OpVariable %_ptr_Function_v3float Function
%169 = OpVariable %_ptr_Function_float Function
%_7_blend_set_color_luminance = OpVariable %_ptr_Function_v3float Function
%_8_blend_color_luminance = OpVariable %_ptr_Function_float Function
%_9_lum = OpVariable %_ptr_Function_float Function
%_10_blend_color_luminance = OpVariable %_ptr_Function_float Function
%_11_result = OpVariable %_ptr_Function_v3float Function
%_12_minComp = OpVariable %_ptr_Function_float Function
%_13_maxComp = OpVariable %_ptr_Function_float Function
%246 = OpVariable %_ptr_Function_v3float Function
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
%75 = OpLoad %v3float %_2_sda
%76 = OpCompositeExtract %float %75 0
%77 = OpLoad %v3float %_2_sda
%78 = OpCompositeExtract %float %77 1
%74 = OpExtInst %float %1 FMax %76 %78
%79 = OpLoad %v3float %_2_sda
%80 = OpCompositeExtract %float %79 2
%73 = OpExtInst %float %1 FMax %74 %80
%83 = OpLoad %v3float %_2_sda
%84 = OpCompositeExtract %float %83 0
%85 = OpLoad %v3float %_2_sda
%86 = OpCompositeExtract %float %85 1
%82 = OpExtInst %float %1 FMin %84 %86
%87 = OpLoad %v3float %_2_sda
%88 = OpCompositeExtract %float %87 2
%81 = OpExtInst %float %1 FMin %82 %88
%89 = OpFSub %float %73 %81
OpStore %_5_blend_color_saturation %89
%91 = OpLoad %float %_5_blend_color_saturation
OpStore %_6_sat %91
%92 = OpLoad %v3float %_3_dsa
%93 = OpCompositeExtract %float %92 0
%94 = OpLoad %v3float %_3_dsa
%95 = OpCompositeExtract %float %94 1
%96 = OpFOrdLessThanEqual %bool %93 %95
OpSelectionMerge %99 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%100 = OpLoad %v3float %_3_dsa
%101 = OpCompositeExtract %float %100 1
%102 = OpLoad %v3float %_3_dsa
%103 = OpCompositeExtract %float %102 2
%104 = OpFOrdLessThanEqual %bool %101 %103
OpSelectionMerge %107 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%108 = OpLoad %v3float %_3_dsa
OpStore %109 %108
%110 = OpLoad %float %_6_sat
OpStore %111 %110
%112 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %109 %111
OpStore %_4_blend_set_color_saturation %112
OpBranch %107
%106 = OpLabel
%113 = OpLoad %v3float %_3_dsa
%114 = OpCompositeExtract %float %113 0
%115 = OpLoad %v3float %_3_dsa
%116 = OpCompositeExtract %float %115 2
%117 = OpFOrdLessThanEqual %bool %114 %116
OpSelectionMerge %120 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%121 = OpLoad %v3float %_3_dsa
%122 = OpVectorShuffle %v3float %121 %121 0 2 1
OpStore %123 %122
%124 = OpLoad %float %_6_sat
OpStore %125 %124
%126 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %123 %125
%127 = OpVectorShuffle %v3float %126 %126 0 2 1
OpStore %_4_blend_set_color_saturation %127
OpBranch %120
%119 = OpLabel
%128 = OpLoad %v3float %_3_dsa
%129 = OpVectorShuffle %v3float %128 %128 2 0 1
OpStore %130 %129
%131 = OpLoad %float %_6_sat
OpStore %132 %131
%133 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %130 %132
%134 = OpVectorShuffle %v3float %133 %133 1 2 0
OpStore %_4_blend_set_color_saturation %134
OpBranch %120
%120 = OpLabel
OpBranch %107
%107 = OpLabel
OpBranch %99
%98 = OpLabel
%135 = OpLoad %v3float %_3_dsa
%136 = OpCompositeExtract %float %135 0
%137 = OpLoad %v3float %_3_dsa
%138 = OpCompositeExtract %float %137 2
%139 = OpFOrdLessThanEqual %bool %136 %138
OpSelectionMerge %142 None
OpBranchConditional %139 %140 %141
%140 = OpLabel
%143 = OpLoad %v3float %_3_dsa
%144 = OpVectorShuffle %v3float %143 %143 1 0 2
OpStore %145 %144
%146 = OpLoad %float %_6_sat
OpStore %147 %146
%148 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %145 %147
%149 = OpVectorShuffle %v3float %148 %148 1 0 2
OpStore %_4_blend_set_color_saturation %149
OpBranch %142
%141 = OpLabel
%150 = OpLoad %v3float %_3_dsa
%151 = OpCompositeExtract %float %150 1
%152 = OpLoad %v3float %_3_dsa
%153 = OpCompositeExtract %float %152 2
%154 = OpFOrdLessThanEqual %bool %151 %153
OpSelectionMerge %157 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
%158 = OpLoad %v3float %_3_dsa
%159 = OpVectorShuffle %v3float %158 %158 1 2 0
OpStore %160 %159
%161 = OpLoad %float %_6_sat
OpStore %162 %161
%163 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %160 %162
%164 = OpVectorShuffle %v3float %163 %163 2 0 1
OpStore %_4_blend_set_color_saturation %164
OpBranch %157
%156 = OpLabel
%165 = OpLoad %v3float %_3_dsa
%166 = OpVectorShuffle %v3float %165 %165 2 1 0
OpStore %167 %166
%168 = OpLoad %float %_6_sat
OpStore %169 %168
%170 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %167 %169
%171 = OpVectorShuffle %v3float %170 %170 2 1 0
OpStore %_4_blend_set_color_saturation %171
OpBranch %157
%157 = OpLabel
OpBranch %142
%142 = OpLabel
OpBranch %99
%99 = OpLabel
%179 = OpLoad %v3float %_3_dsa
%174 = OpDot %float %175 %179
OpStore %_8_blend_color_luminance %174
%181 = OpLoad %float %_8_blend_color_luminance
OpStore %_9_lum %181
%185 = OpLoad %v3float %_4_blend_set_color_saturation
%183 = OpDot %float %184 %185
OpStore %_10_blend_color_luminance %183
%187 = OpLoad %float %_9_lum
%188 = OpLoad %float %_10_blend_color_luminance
%189 = OpFSub %float %187 %188
%190 = OpLoad %v3float %_4_blend_set_color_saturation
%191 = OpCompositeConstruct %v3float %189 %189 %189
%192 = OpFAdd %v3float %191 %190
OpStore %_11_result %192
%196 = OpLoad %v3float %_11_result
%197 = OpCompositeExtract %float %196 0
%198 = OpLoad %v3float %_11_result
%199 = OpCompositeExtract %float %198 1
%195 = OpExtInst %float %1 FMin %197 %199
%200 = OpLoad %v3float %_11_result
%201 = OpCompositeExtract %float %200 2
%194 = OpExtInst %float %1 FMin %195 %201
OpStore %_12_minComp %194
%205 = OpLoad %v3float %_11_result
%206 = OpCompositeExtract %float %205 0
%207 = OpLoad %v3float %_11_result
%208 = OpCompositeExtract %float %207 1
%204 = OpExtInst %float %1 FMax %206 %208
%209 = OpLoad %v3float %_11_result
%210 = OpCompositeExtract %float %209 2
%203 = OpExtInst %float %1 FMax %204 %210
OpStore %_13_maxComp %203
%212 = OpLoad %float %_12_minComp
%213 = OpFOrdLessThan %bool %212 %float_0
OpSelectionMerge %215 None
OpBranchConditional %213 %214 %215
%214 = OpLabel
%216 = OpLoad %float %_9_lum
%217 = OpLoad %float %_12_minComp
%218 = OpFOrdNotEqual %bool %216 %217
OpBranch %215
%215 = OpLabel
%219 = OpPhi %bool %false %99 %218 %214
OpSelectionMerge %221 None
OpBranchConditional %219 %220 %221
%220 = OpLabel
%222 = OpLoad %float %_9_lum
%223 = OpLoad %v3float %_11_result
%224 = OpLoad %float %_9_lum
%225 = OpCompositeConstruct %v3float %224 %224 %224
%226 = OpFSub %v3float %223 %225
%227 = OpLoad %float %_9_lum
%228 = OpVectorTimesScalar %v3float %226 %227
%229 = OpLoad %float %_9_lum
%230 = OpLoad %float %_12_minComp
%231 = OpFSub %float %229 %230
%233 = OpFDiv %float %float_1 %231
%234 = OpVectorTimesScalar %v3float %228 %233
%235 = OpCompositeConstruct %v3float %222 %222 %222
%236 = OpFAdd %v3float %235 %234
OpStore %_11_result %236
OpBranch %221
%221 = OpLabel
%237 = OpLoad %float %_13_maxComp
%238 = OpLoad %float %_1_alpha
%239 = OpFOrdGreaterThan %bool %237 %238
OpSelectionMerge %241 None
OpBranchConditional %239 %240 %241
%240 = OpLabel
%242 = OpLoad %float %_13_maxComp
%243 = OpLoad %float %_9_lum
%244 = OpFOrdNotEqual %bool %242 %243
OpBranch %241
%241 = OpLabel
%245 = OpPhi %bool %false %221 %244 %240
OpSelectionMerge %249 None
OpBranchConditional %245 %247 %248
%247 = OpLabel
%250 = OpLoad %float %_9_lum
%251 = OpLoad %v3float %_11_result
%252 = OpLoad %float %_9_lum
%253 = OpCompositeConstruct %v3float %252 %252 %252
%254 = OpFSub %v3float %251 %253
%255 = OpLoad %float %_1_alpha
%256 = OpLoad %float %_9_lum
%257 = OpFSub %float %255 %256
%258 = OpVectorTimesScalar %v3float %254 %257
%259 = OpLoad %float %_13_maxComp
%260 = OpLoad %float %_9_lum
%261 = OpFSub %float %259 %260
%262 = OpFDiv %float %float_1 %261
%263 = OpVectorTimesScalar %v3float %258 %262
%264 = OpCompositeConstruct %v3float %250 %250 %250
%265 = OpFAdd %v3float %264 %263
OpStore %246 %265
OpBranch %249
%248 = OpLabel
%266 = OpLoad %v3float %_11_result
OpStore %246 %266
OpBranch %249
%249 = OpLabel
%267 = OpLoad %v3float %246
OpStore %_7_blend_set_color_luminance %267
%268 = OpLoad %v3float %_7_blend_set_color_luminance
%269 = OpLoad %v4float %dst
%270 = OpVectorShuffle %v3float %269 %269 0 1 2
%271 = OpFAdd %v3float %268 %270
%272 = OpLoad %v3float %_3_dsa
%273 = OpFSub %v3float %271 %272
%274 = OpLoad %v4float %src
%275 = OpVectorShuffle %v3float %274 %274 0 1 2
%276 = OpFAdd %v3float %273 %275
%277 = OpLoad %v3float %_2_sda
%278 = OpFSub %v3float %276 %277
%279 = OpCompositeExtract %float %278 0
%280 = OpCompositeExtract %float %278 1
%281 = OpCompositeExtract %float %278 2
%282 = OpLoad %v4float %src
%283 = OpCompositeExtract %float %282 3
%284 = OpLoad %v4float %dst
%285 = OpCompositeExtract %float %284 3
%286 = OpFAdd %float %283 %285
%287 = OpLoad %float %_1_alpha
%288 = OpFSub %float %286 %287
%289 = OpCompositeConstruct %v4float %279 %280 %281 %288
OpStore %_0_blend_saturation %289
%290 = OpLoad %v4float %_0_blend_saturation
OpStore %sk_FragColor %290
OpReturn
OpFunctionEnd
