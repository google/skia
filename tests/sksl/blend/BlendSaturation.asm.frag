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
OpName %_18_guarded_divide "_18_guarded_divide"
OpName %_19_n "_19_n"
OpName %_20_d "_20_d"
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
OpName %_14_guarded_divide "_14_guarded_divide"
OpName %_15_d "_15_d"
OpName %_16_guarded_divide "_16_guarded_divide"
OpName %_17_n "_17_n"
OpName %_18_d "_18_d"
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
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
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
OpDecorate %115 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
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
%50 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%void = OpTypeVoid
%52 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
%181 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%_blend_set_color_saturation_helper = OpFunction %v3float None %15
%18 = OpFunctionParameter %_ptr_Function_v3float
%19 = OpFunctionParameter %_ptr_Function_float
%20 = OpLabel
%_18_guarded_divide = OpVariable %_ptr_Function_float Function
%_19_n = OpVariable %_ptr_Function_float Function
%_20_d = OpVariable %_ptr_Function_float Function
%21 = OpLoad %v3float %18
%22 = OpCompositeExtract %float %21 0
%23 = OpLoad %v3float %18
%24 = OpCompositeExtract %float %23 2
%25 = OpFOrdLessThan %bool %22 %24
OpSelectionMerge %28 None
OpBranchConditional %25 %26 %27
%26 = OpLabel
%31 = OpLoad %float %19
%32 = OpLoad %v3float %18
%33 = OpCompositeExtract %float %32 1
%34 = OpLoad %v3float %18
%35 = OpCompositeExtract %float %34 0
%36 = OpFSub %float %33 %35
%37 = OpFMul %float %31 %36
OpStore %_19_n %37
%39 = OpLoad %v3float %18
%40 = OpCompositeExtract %float %39 2
%41 = OpLoad %v3float %18
%42 = OpCompositeExtract %float %41 0
%43 = OpFSub %float %40 %42
OpStore %_20_d %43
%45 = OpLoad %float %_19_n
%46 = OpLoad %float %_20_d
%47 = OpFDiv %float %45 %46
%48 = OpLoad %float %19
%49 = OpCompositeConstruct %v3float %float_0 %47 %48
OpReturnValue %49
%27 = OpLabel
OpReturnValue %50
%28 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %52
%53 = OpLabel
%_0_blend_saturation = OpVariable %_ptr_Function_v4float Function
%_1_alpha = OpVariable %_ptr_Function_float Function
%_2_sda = OpVariable %_ptr_Function_v3float Function
%_3_dsa = OpVariable %_ptr_Function_v3float Function
%_4_blend_set_color_saturation = OpVariable %_ptr_Function_v3float Function
%_5_blend_color_saturation = OpVariable %_ptr_Function_float Function
%_6_sat = OpVariable %_ptr_Function_float Function
%111 = OpVariable %_ptr_Function_v3float Function
%113 = OpVariable %_ptr_Function_float Function
%125 = OpVariable %_ptr_Function_v3float Function
%127 = OpVariable %_ptr_Function_float Function
%132 = OpVariable %_ptr_Function_v3float Function
%134 = OpVariable %_ptr_Function_float Function
%147 = OpVariable %_ptr_Function_v3float Function
%149 = OpVariable %_ptr_Function_float Function
%162 = OpVariable %_ptr_Function_v3float Function
%164 = OpVariable %_ptr_Function_float Function
%169 = OpVariable %_ptr_Function_v3float Function
%171 = OpVariable %_ptr_Function_float Function
%_7_blend_set_color_luminance = OpVariable %_ptr_Function_v3float Function
%_8_blend_color_luminance = OpVariable %_ptr_Function_float Function
%_9_lum = OpVariable %_ptr_Function_float Function
%_10_blend_color_luminance = OpVariable %_ptr_Function_float Function
%_11_result = OpVariable %_ptr_Function_v3float Function
%_12_minComp = OpVariable %_ptr_Function_float Function
%_13_maxComp = OpVariable %_ptr_Function_float Function
%_14_guarded_divide = OpVariable %_ptr_Function_float Function
%_15_d = OpVariable %_ptr_Function_float Function
%_16_guarded_divide = OpVariable %_ptr_Function_v3float Function
%_17_n = OpVariable %_ptr_Function_v3float Function
%_18_d = OpVariable %_ptr_Function_float Function
%57 = OpLoad %v4float %dst
%58 = OpCompositeExtract %float %57 3
%59 = OpLoad %v4float %src
%60 = OpCompositeExtract %float %59 3
%61 = OpFMul %float %58 %60
OpStore %_1_alpha %61
%63 = OpLoad %v4float %src
%64 = OpVectorShuffle %v3float %63 %63 0 1 2
%65 = OpLoad %v4float %dst
%66 = OpCompositeExtract %float %65 3
%67 = OpVectorTimesScalar %v3float %64 %66
OpStore %_2_sda %67
%69 = OpLoad %v4float %dst
%70 = OpVectorShuffle %v3float %69 %69 0 1 2
%71 = OpLoad %v4float %src
%72 = OpCompositeExtract %float %71 3
%73 = OpVectorTimesScalar %v3float %70 %72
OpStore %_3_dsa %73
%79 = OpLoad %v3float %_2_sda
%80 = OpCompositeExtract %float %79 0
%81 = OpLoad %v3float %_2_sda
%82 = OpCompositeExtract %float %81 1
%78 = OpExtInst %float %1 FMax %80 %82
%83 = OpLoad %v3float %_2_sda
%84 = OpCompositeExtract %float %83 2
%77 = OpExtInst %float %1 FMax %78 %84
%87 = OpLoad %v3float %_2_sda
%88 = OpCompositeExtract %float %87 0
%89 = OpLoad %v3float %_2_sda
%90 = OpCompositeExtract %float %89 1
%86 = OpExtInst %float %1 FMin %88 %90
%91 = OpLoad %v3float %_2_sda
%92 = OpCompositeExtract %float %91 2
%85 = OpExtInst %float %1 FMin %86 %92
%93 = OpFSub %float %77 %85
OpStore %_6_sat %93
%94 = OpLoad %v3float %_3_dsa
%95 = OpCompositeExtract %float %94 0
%96 = OpLoad %v3float %_3_dsa
%97 = OpCompositeExtract %float %96 1
%98 = OpFOrdLessThanEqual %bool %95 %97
OpSelectionMerge %101 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%102 = OpLoad %v3float %_3_dsa
%103 = OpCompositeExtract %float %102 1
%104 = OpLoad %v3float %_3_dsa
%105 = OpCompositeExtract %float %104 2
%106 = OpFOrdLessThanEqual %bool %103 %105
OpSelectionMerge %109 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%110 = OpLoad %v3float %_3_dsa
OpStore %111 %110
%112 = OpLoad %float %_6_sat
OpStore %113 %112
%114 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %111 %113
OpStore %_4_blend_set_color_saturation %114
OpBranch %109
%108 = OpLabel
%115 = OpLoad %v3float %_3_dsa
%116 = OpCompositeExtract %float %115 0
%117 = OpLoad %v3float %_3_dsa
%118 = OpCompositeExtract %float %117 2
%119 = OpFOrdLessThanEqual %bool %116 %118
OpSelectionMerge %122 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%123 = OpLoad %v3float %_3_dsa
%124 = OpVectorShuffle %v3float %123 %123 0 2 1
OpStore %125 %124
%126 = OpLoad %float %_6_sat
OpStore %127 %126
%128 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %125 %127
%129 = OpVectorShuffle %v3float %128 %128 0 2 1
OpStore %_4_blend_set_color_saturation %129
OpBranch %122
%121 = OpLabel
%130 = OpLoad %v3float %_3_dsa
%131 = OpVectorShuffle %v3float %130 %130 2 0 1
OpStore %132 %131
%133 = OpLoad %float %_6_sat
OpStore %134 %133
%135 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %132 %134
%136 = OpVectorShuffle %v3float %135 %135 1 2 0
OpStore %_4_blend_set_color_saturation %136
OpBranch %122
%122 = OpLabel
OpBranch %109
%109 = OpLabel
OpBranch %101
%100 = OpLabel
%137 = OpLoad %v3float %_3_dsa
%138 = OpCompositeExtract %float %137 0
%139 = OpLoad %v3float %_3_dsa
%140 = OpCompositeExtract %float %139 2
%141 = OpFOrdLessThanEqual %bool %138 %140
OpSelectionMerge %144 None
OpBranchConditional %141 %142 %143
%142 = OpLabel
%145 = OpLoad %v3float %_3_dsa
%146 = OpVectorShuffle %v3float %145 %145 1 0 2
OpStore %147 %146
%148 = OpLoad %float %_6_sat
OpStore %149 %148
%150 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %147 %149
%151 = OpVectorShuffle %v3float %150 %150 1 0 2
OpStore %_4_blend_set_color_saturation %151
OpBranch %144
%143 = OpLabel
%152 = OpLoad %v3float %_3_dsa
%153 = OpCompositeExtract %float %152 1
%154 = OpLoad %v3float %_3_dsa
%155 = OpCompositeExtract %float %154 2
%156 = OpFOrdLessThanEqual %bool %153 %155
OpSelectionMerge %159 None
OpBranchConditional %156 %157 %158
%157 = OpLabel
%160 = OpLoad %v3float %_3_dsa
%161 = OpVectorShuffle %v3float %160 %160 1 2 0
OpStore %162 %161
%163 = OpLoad %float %_6_sat
OpStore %164 %163
%165 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %162 %164
%166 = OpVectorShuffle %v3float %165 %165 2 0 1
OpStore %_4_blend_set_color_saturation %166
OpBranch %159
%158 = OpLabel
%167 = OpLoad %v3float %_3_dsa
%168 = OpVectorShuffle %v3float %167 %167 2 1 0
OpStore %169 %168
%170 = OpLoad %float %_6_sat
OpStore %171 %170
%172 = OpFunctionCall %v3float %_blend_set_color_saturation_helper %169 %171
%173 = OpVectorShuffle %v3float %172 %172 2 1 0
OpStore %_4_blend_set_color_saturation %173
OpBranch %159
%159 = OpLabel
OpBranch %144
%144 = OpLabel
OpBranch %101
%101 = OpLabel
%182 = OpLoad %v3float %_3_dsa
%177 = OpDot %float %181 %182
OpStore %_9_lum %177
%185 = OpLoad %float %_9_lum
%187 = OpLoad %v3float %_4_blend_set_color_saturation
%186 = OpDot %float %181 %187
%188 = OpFSub %float %185 %186
%189 = OpLoad %v3float %_4_blend_set_color_saturation
%190 = OpCompositeConstruct %v3float %188 %188 %188
%191 = OpFAdd %v3float %190 %189
OpStore %_11_result %191
%195 = OpLoad %v3float %_11_result
%196 = OpCompositeExtract %float %195 0
%197 = OpLoad %v3float %_11_result
%198 = OpCompositeExtract %float %197 1
%194 = OpExtInst %float %1 FMin %196 %198
%199 = OpLoad %v3float %_11_result
%200 = OpCompositeExtract %float %199 2
%193 = OpExtInst %float %1 FMin %194 %200
OpStore %_12_minComp %193
%204 = OpLoad %v3float %_11_result
%205 = OpCompositeExtract %float %204 0
%206 = OpLoad %v3float %_11_result
%207 = OpCompositeExtract %float %206 1
%203 = OpExtInst %float %1 FMax %205 %207
%208 = OpLoad %v3float %_11_result
%209 = OpCompositeExtract %float %208 2
%202 = OpExtInst %float %1 FMax %203 %209
OpStore %_13_maxComp %202
%211 = OpLoad %float %_12_minComp
%212 = OpFOrdLessThan %bool %211 %float_0
OpSelectionMerge %214 None
OpBranchConditional %212 %213 %214
%213 = OpLabel
%215 = OpLoad %float %_9_lum
%216 = OpLoad %float %_12_minComp
%217 = OpFOrdNotEqual %bool %215 %216
OpBranch %214
%214 = OpLabel
%218 = OpPhi %bool %false %101 %217 %213
OpSelectionMerge %220 None
OpBranchConditional %218 %219 %220
%219 = OpLabel
%223 = OpLoad %float %_9_lum
%224 = OpLoad %float %_12_minComp
%225 = OpFSub %float %223 %224
OpStore %_15_d %225
%226 = OpLoad %float %_9_lum
%227 = OpLoad %v3float %_11_result
%228 = OpLoad %float %_9_lum
%229 = OpCompositeConstruct %v3float %228 %228 %228
%230 = OpFSub %v3float %227 %229
%231 = OpLoad %float %_9_lum
%232 = OpLoad %float %_15_d
%233 = OpFDiv %float %231 %232
%234 = OpVectorTimesScalar %v3float %230 %233
%235 = OpCompositeConstruct %v3float %226 %226 %226
%236 = OpFAdd %v3float %235 %234
OpStore %_11_result %236
OpBranch %220
%220 = OpLabel
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
%245 = OpPhi %bool %false %220 %244 %240
OpSelectionMerge %248 None
OpBranchConditional %245 %246 %247
%246 = OpLabel
%251 = OpLoad %v3float %_11_result
%252 = OpLoad %float %_9_lum
%253 = OpCompositeConstruct %v3float %252 %252 %252
%254 = OpFSub %v3float %251 %253
%255 = OpLoad %float %_1_alpha
%256 = OpLoad %float %_9_lum
%257 = OpFSub %float %255 %256
%258 = OpVectorTimesScalar %v3float %254 %257
OpStore %_17_n %258
%260 = OpLoad %float %_13_maxComp
%261 = OpLoad %float %_9_lum
%262 = OpFSub %float %260 %261
OpStore %_18_d %262
%263 = OpLoad %float %_9_lum
%264 = OpLoad %v3float %_17_n
%265 = OpLoad %float %_18_d
%267 = OpFDiv %float %float_1 %265
%268 = OpVectorTimesScalar %v3float %264 %267
%269 = OpCompositeConstruct %v3float %263 %263 %263
%270 = OpFAdd %v3float %269 %268
OpStore %_7_blend_set_color_luminance %270
OpBranch %248
%247 = OpLabel
%271 = OpLoad %v3float %_11_result
OpStore %_7_blend_set_color_luminance %271
OpBranch %248
%248 = OpLabel
%272 = OpLoad %v3float %_7_blend_set_color_luminance
%273 = OpLoad %v4float %dst
%274 = OpVectorShuffle %v3float %273 %273 0 1 2
%275 = OpFAdd %v3float %272 %274
%276 = OpLoad %v3float %_3_dsa
%277 = OpFSub %v3float %275 %276
%278 = OpLoad %v4float %src
%279 = OpVectorShuffle %v3float %278 %278 0 1 2
%280 = OpFAdd %v3float %277 %279
%281 = OpLoad %v3float %_2_sda
%282 = OpFSub %v3float %280 %281
%283 = OpCompositeExtract %float %282 0
%284 = OpCompositeExtract %float %282 1
%285 = OpCompositeExtract %float %282 2
%286 = OpLoad %v4float %src
%287 = OpCompositeExtract %float %286 3
%288 = OpLoad %v4float %dst
%289 = OpCompositeExtract %float %288 3
%290 = OpFAdd %float %287 %289
%291 = OpLoad %float %_1_alpha
%292 = OpFSub %float %290 %291
%293 = OpCompositeConstruct %v4float %283 %284 %285 %292
OpStore %sk_FragColor %293
OpReturn
OpFunctionEnd
