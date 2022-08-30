OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_float_b "test_float_b"
OpName %ok "ok"
OpName %m1 "m1"
OpName %m3 "m3"
OpName %m4 "m4"
OpName %m5 "m5"
OpName %m7 "m7"
OpName %m9 "m9"
OpName %m10 "m10"
OpName %m11 "m11"
OpName %test_half_b "test_half_b"
OpName %ok_0 "ok"
OpName %m1_0 "m1"
OpName %m3_0 "m3"
OpName %m4_0 "m4"
OpName %m5_0 "m5"
OpName %m7_0 "m7"
OpName %m9_0 "m9"
OpName %m10_0 "m10"
OpName %m11_0 "m11"
OpName %test_comma_b "test_comma_b"
OpName %x "x"
OpName %y "y"
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
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %m1_0 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %m3_0 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %m4_0 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %m5_0 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %m7_0 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %m9_0 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %m10_0 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %m11_0 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%38 = OpConstantComposite %v2float %float_1 %float_2
%39 = OpConstantComposite %v2float %float_3 %float_4
%40 = OpConstantComposite %mat2v2float %38 %39
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%float_6 = OpConstant %float 6
%62 = OpConstantComposite %v2float %float_6 %float_0
%63 = OpConstantComposite %v2float %float_0 %float_6
%64 = OpConstantComposite %mat2v2float %62 %63
%float_12 = OpConstant %float 12
%float_18 = OpConstant %float 18
%float_24 = OpConstant %float 24
%79 = OpConstantComposite %v2float %float_6 %float_12
%80 = OpConstantComposite %v2float %float_18 %float_24
%81 = OpConstantComposite %mat2v2float %79 %80
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%101 = OpConstantComposite %v2float %float_4 %float_0
%102 = OpConstantComposite %v2float %float_0 %float_4
%103 = OpConstantComposite %mat2v2float %101 %102
%float_5 = OpConstant %float 5
%float_8 = OpConstant %float 8
%117 = OpConstantComposite %v2float %float_5 %float_2
%118 = OpConstantComposite %v2float %float_3 %float_8
%119 = OpConstantComposite %mat2v2float %117 %118
%float_7 = OpConstant %float 7
%128 = OpConstantComposite %v2float %float_5 %float_6
%129 = OpConstantComposite %v2float %float_7 %float_8
%130 = OpConstantComposite %mat2v2float %128 %129
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_9 = OpConstant %float 9
%144 = OpConstantComposite %v3float %float_9 %float_0 %float_0
%145 = OpConstantComposite %v3float %float_0 %float_9 %float_0
%146 = OpConstantComposite %v3float %float_0 %float_0 %float_9
%147 = OpConstantComposite %mat3v3float %144 %145 %146
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_11 = OpConstant %float 11
%164 = OpConstantComposite %v4float %float_11 %float_0 %float_0 %float_0
%165 = OpConstantComposite %v4float %float_0 %float_11 %float_0 %float_0
%166 = OpConstantComposite %v4float %float_0 %float_0 %float_11 %float_0
%167 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_11
%168 = OpConstantComposite %mat4v4float %164 %165 %166 %167
%v4bool = OpTypeVector %bool 4
%float_20 = OpConstant %float 20
%186 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_20
%187 = OpConstantComposite %mat4v4float %186 %186 %186 %186
%195 = OpConstantComposite %v4float %float_9 %float_20 %float_20 %float_20
%196 = OpConstantComposite %v4float %float_20 %float_9 %float_20 %float_20
%197 = OpConstantComposite %v4float %float_20 %float_20 %float_9 %float_20
%198 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_9
%199 = OpConstantComposite %mat4v4float %195 %196 %197 %198
%342 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %18
%19 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%test_float_b = OpFunction %bool None %26
%27 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%m1 = OpVariable %_ptr_Function_mat2v2float Function
%m3 = OpVariable %_ptr_Function_mat2v2float Function
%m4 = OpVariable %_ptr_Function_mat2v2float Function
%m5 = OpVariable %_ptr_Function_mat2v2float Function
%m7 = OpVariable %_ptr_Function_mat2v2float Function
%m9 = OpVariable %_ptr_Function_mat3v3float Function
%m10 = OpVariable %_ptr_Function_mat4v4float Function
%m11 = OpVariable %_ptr_Function_mat4v4float Function
OpStore %ok %true
OpStore %m1 %40
OpSelectionMerge %43 None
OpBranchConditional %true %42 %43
%42 = OpLabel
%45 = OpFOrdEqual %v2bool %38 %38
%46 = OpAll %bool %45
%47 = OpFOrdEqual %v2bool %39 %39
%48 = OpAll %bool %47
%49 = OpLogicalAnd %bool %46 %48
OpBranch %43
%43 = OpLabel
%50 = OpPhi %bool %false %27 %49 %42
OpStore %ok %50
OpStore %m3 %40
OpSelectionMerge %53 None
OpBranchConditional %50 %52 %53
%52 = OpLabel
%54 = OpFOrdEqual %v2bool %38 %38
%55 = OpAll %bool %54
%56 = OpFOrdEqual %v2bool %39 %39
%57 = OpAll %bool %56
%58 = OpLogicalAnd %bool %55 %57
OpBranch %53
%53 = OpLabel
%59 = OpPhi %bool %false %43 %58 %52
OpStore %ok %59
OpStore %m4 %64
OpSelectionMerge %66 None
OpBranchConditional %59 %65 %66
%65 = OpLabel
%67 = OpFOrdEqual %v2bool %62 %62
%68 = OpAll %bool %67
%69 = OpFOrdEqual %v2bool %63 %63
%70 = OpAll %bool %69
%71 = OpLogicalAnd %bool %68 %70
OpBranch %66
%66 = OpLabel
%72 = OpPhi %bool %false %53 %71 %65
OpStore %ok %72
%73 = OpMatrixTimesMatrix %mat2v2float %40 %64
OpStore %m3 %73
OpSelectionMerge %75 None
OpBranchConditional %72 %74 %75
%74 = OpLabel
%82 = OpCompositeExtract %v2float %73 0
%83 = OpFOrdEqual %v2bool %82 %79
%84 = OpAll %bool %83
%85 = OpCompositeExtract %v2float %73 1
%86 = OpFOrdEqual %v2bool %85 %80
%87 = OpAll %bool %86
%88 = OpLogicalAnd %bool %84 %87
OpBranch %75
%75 = OpLabel
%89 = OpPhi %bool %false %66 %88 %74
OpStore %ok %89
%93 = OpAccessChain %_ptr_Function_v2float %m1 %int_1
%94 = OpLoad %v2float %93
%95 = OpCompositeExtract %float %94 1
%96 = OpCompositeConstruct %v2float %95 %float_0
%97 = OpCompositeConstruct %v2float %float_0 %95
%98 = OpCompositeConstruct %mat2v2float %96 %97
OpStore %m5 %98
OpSelectionMerge %100 None
OpBranchConditional %89 %99 %100
%99 = OpLabel
%104 = OpFOrdEqual %v2bool %96 %101
%105 = OpAll %bool %104
%106 = OpFOrdEqual %v2bool %97 %102
%107 = OpAll %bool %106
%108 = OpLogicalAnd %bool %105 %107
OpBranch %100
%100 = OpLabel
%109 = OpPhi %bool %false %75 %108 %99
OpStore %ok %109
%110 = OpFAdd %v2float %38 %96
%111 = OpFAdd %v2float %39 %97
%112 = OpCompositeConstruct %mat2v2float %110 %111
OpStore %m1 %112
OpSelectionMerge %114 None
OpBranchConditional %109 %113 %114
%113 = OpLabel
%120 = OpFOrdEqual %v2bool %110 %117
%121 = OpAll %bool %120
%122 = OpFOrdEqual %v2bool %111 %118
%123 = OpAll %bool %122
%124 = OpLogicalAnd %bool %121 %123
OpBranch %114
%114 = OpLabel
%125 = OpPhi %bool %false %100 %124 %113
OpStore %ok %125
OpStore %m7 %130
OpSelectionMerge %132 None
OpBranchConditional %125 %131 %132
%131 = OpLabel
%133 = OpFOrdEqual %v2bool %128 %128
%134 = OpAll %bool %133
%135 = OpFOrdEqual %v2bool %129 %129
%136 = OpAll %bool %135
%137 = OpLogicalAnd %bool %134 %136
OpBranch %132
%132 = OpLabel
%138 = OpPhi %bool %false %114 %137 %131
OpStore %ok %138
OpStore %m9 %147
OpSelectionMerge %149 None
OpBranchConditional %138 %148 %149
%148 = OpLabel
%151 = OpFOrdEqual %v3bool %144 %144
%152 = OpAll %bool %151
%153 = OpFOrdEqual %v3bool %145 %145
%154 = OpAll %bool %153
%155 = OpLogicalAnd %bool %152 %154
%156 = OpFOrdEqual %v3bool %146 %146
%157 = OpAll %bool %156
%158 = OpLogicalAnd %bool %155 %157
OpBranch %149
%149 = OpLabel
%159 = OpPhi %bool %false %132 %158 %148
OpStore %ok %159
OpStore %m10 %168
OpSelectionMerge %170 None
OpBranchConditional %159 %169 %170
%169 = OpLabel
%172 = OpFOrdEqual %v4bool %164 %164
%173 = OpAll %bool %172
%174 = OpFOrdEqual %v4bool %165 %165
%175 = OpAll %bool %174
%176 = OpLogicalAnd %bool %173 %175
%177 = OpFOrdEqual %v4bool %166 %166
%178 = OpAll %bool %177
%179 = OpLogicalAnd %bool %176 %178
%180 = OpFOrdEqual %v4bool %167 %167
%181 = OpAll %bool %180
%182 = OpLogicalAnd %bool %179 %181
OpBranch %170
%170 = OpLabel
%183 = OpPhi %bool %false %149 %182 %169
OpStore %ok %183
OpStore %m11 %187
%188 = OpFSub %v4float %186 %164
%189 = OpFSub %v4float %186 %165
%190 = OpFSub %v4float %186 %166
%191 = OpFSub %v4float %186 %167
%192 = OpCompositeConstruct %mat4v4float %188 %189 %190 %191
OpStore %m11 %192
OpSelectionMerge %194 None
OpBranchConditional %183 %193 %194
%193 = OpLabel
%200 = OpFOrdEqual %v4bool %188 %195
%201 = OpAll %bool %200
%202 = OpFOrdEqual %v4bool %189 %196
%203 = OpAll %bool %202
%204 = OpLogicalAnd %bool %201 %203
%205 = OpFOrdEqual %v4bool %190 %197
%206 = OpAll %bool %205
%207 = OpLogicalAnd %bool %204 %206
%208 = OpFOrdEqual %v4bool %191 %198
%209 = OpAll %bool %208
%210 = OpLogicalAnd %bool %207 %209
OpBranch %194
%194 = OpLabel
%211 = OpPhi %bool %false %170 %210 %193
OpStore %ok %211
OpReturnValue %211
OpFunctionEnd
%test_half_b = OpFunction %bool None %26
%212 = OpLabel
%ok_0 = OpVariable %_ptr_Function_bool Function
%m1_0 = OpVariable %_ptr_Function_mat2v2float Function
%m3_0 = OpVariable %_ptr_Function_mat2v2float Function
%m4_0 = OpVariable %_ptr_Function_mat2v2float Function
%m5_0 = OpVariable %_ptr_Function_mat2v2float Function
%m7_0 = OpVariable %_ptr_Function_mat2v2float Function
%m9_0 = OpVariable %_ptr_Function_mat3v3float Function
%m10_0 = OpVariable %_ptr_Function_mat4v4float Function
%m11_0 = OpVariable %_ptr_Function_mat4v4float Function
OpStore %ok_0 %true
OpStore %m1_0 %40
OpSelectionMerge %216 None
OpBranchConditional %true %215 %216
%215 = OpLabel
%217 = OpFOrdEqual %v2bool %38 %38
%218 = OpAll %bool %217
%219 = OpFOrdEqual %v2bool %39 %39
%220 = OpAll %bool %219
%221 = OpLogicalAnd %bool %218 %220
OpBranch %216
%216 = OpLabel
%222 = OpPhi %bool %false %212 %221 %215
OpStore %ok_0 %222
OpStore %m3_0 %40
OpSelectionMerge %225 None
OpBranchConditional %222 %224 %225
%224 = OpLabel
%226 = OpFOrdEqual %v2bool %38 %38
%227 = OpAll %bool %226
%228 = OpFOrdEqual %v2bool %39 %39
%229 = OpAll %bool %228
%230 = OpLogicalAnd %bool %227 %229
OpBranch %225
%225 = OpLabel
%231 = OpPhi %bool %false %216 %230 %224
OpStore %ok_0 %231
OpStore %m4_0 %64
OpSelectionMerge %234 None
OpBranchConditional %231 %233 %234
%233 = OpLabel
%235 = OpFOrdEqual %v2bool %62 %62
%236 = OpAll %bool %235
%237 = OpFOrdEqual %v2bool %63 %63
%238 = OpAll %bool %237
%239 = OpLogicalAnd %bool %236 %238
OpBranch %234
%234 = OpLabel
%240 = OpPhi %bool %false %225 %239 %233
OpStore %ok_0 %240
%241 = OpMatrixTimesMatrix %mat2v2float %40 %64
OpStore %m3_0 %241
OpSelectionMerge %243 None
OpBranchConditional %240 %242 %243
%242 = OpLabel
%244 = OpCompositeExtract %v2float %241 0
%245 = OpFOrdEqual %v2bool %244 %79
%246 = OpAll %bool %245
%247 = OpCompositeExtract %v2float %241 1
%248 = OpFOrdEqual %v2bool %247 %80
%249 = OpAll %bool %248
%250 = OpLogicalAnd %bool %246 %249
OpBranch %243
%243 = OpLabel
%251 = OpPhi %bool %false %234 %250 %242
OpStore %ok_0 %251
%253 = OpAccessChain %_ptr_Function_v2float %m1_0 %int_1
%254 = OpLoad %v2float %253
%255 = OpCompositeExtract %float %254 1
%256 = OpCompositeConstruct %v2float %255 %float_0
%257 = OpCompositeConstruct %v2float %float_0 %255
%258 = OpCompositeConstruct %mat2v2float %256 %257
OpStore %m5_0 %258
OpSelectionMerge %260 None
OpBranchConditional %251 %259 %260
%259 = OpLabel
%261 = OpFOrdEqual %v2bool %256 %101
%262 = OpAll %bool %261
%263 = OpFOrdEqual %v2bool %257 %102
%264 = OpAll %bool %263
%265 = OpLogicalAnd %bool %262 %264
OpBranch %260
%260 = OpLabel
%266 = OpPhi %bool %false %243 %265 %259
OpStore %ok_0 %266
%267 = OpFAdd %v2float %38 %256
%268 = OpFAdd %v2float %39 %257
%269 = OpCompositeConstruct %mat2v2float %267 %268
OpStore %m1_0 %269
OpSelectionMerge %271 None
OpBranchConditional %266 %270 %271
%270 = OpLabel
%272 = OpFOrdEqual %v2bool %267 %117
%273 = OpAll %bool %272
%274 = OpFOrdEqual %v2bool %268 %118
%275 = OpAll %bool %274
%276 = OpLogicalAnd %bool %273 %275
OpBranch %271
%271 = OpLabel
%277 = OpPhi %bool %false %260 %276 %270
OpStore %ok_0 %277
OpStore %m7_0 %130
OpSelectionMerge %280 None
OpBranchConditional %277 %279 %280
%279 = OpLabel
%281 = OpFOrdEqual %v2bool %128 %128
%282 = OpAll %bool %281
%283 = OpFOrdEqual %v2bool %129 %129
%284 = OpAll %bool %283
%285 = OpLogicalAnd %bool %282 %284
OpBranch %280
%280 = OpLabel
%286 = OpPhi %bool %false %271 %285 %279
OpStore %ok_0 %286
OpStore %m9_0 %147
OpSelectionMerge %289 None
OpBranchConditional %286 %288 %289
%288 = OpLabel
%290 = OpFOrdEqual %v3bool %144 %144
%291 = OpAll %bool %290
%292 = OpFOrdEqual %v3bool %145 %145
%293 = OpAll %bool %292
%294 = OpLogicalAnd %bool %291 %293
%295 = OpFOrdEqual %v3bool %146 %146
%296 = OpAll %bool %295
%297 = OpLogicalAnd %bool %294 %296
OpBranch %289
%289 = OpLabel
%298 = OpPhi %bool %false %280 %297 %288
OpStore %ok_0 %298
OpStore %m10_0 %168
OpSelectionMerge %301 None
OpBranchConditional %298 %300 %301
%300 = OpLabel
%302 = OpFOrdEqual %v4bool %164 %164
%303 = OpAll %bool %302
%304 = OpFOrdEqual %v4bool %165 %165
%305 = OpAll %bool %304
%306 = OpLogicalAnd %bool %303 %305
%307 = OpFOrdEqual %v4bool %166 %166
%308 = OpAll %bool %307
%309 = OpLogicalAnd %bool %306 %308
%310 = OpFOrdEqual %v4bool %167 %167
%311 = OpAll %bool %310
%312 = OpLogicalAnd %bool %309 %311
OpBranch %301
%301 = OpLabel
%313 = OpPhi %bool %false %289 %312 %300
OpStore %ok_0 %313
OpStore %m11_0 %187
%315 = OpFSub %v4float %186 %164
%316 = OpFSub %v4float %186 %165
%317 = OpFSub %v4float %186 %166
%318 = OpFSub %v4float %186 %167
%319 = OpCompositeConstruct %mat4v4float %315 %316 %317 %318
OpStore %m11_0 %319
OpSelectionMerge %321 None
OpBranchConditional %313 %320 %321
%320 = OpLabel
%322 = OpFOrdEqual %v4bool %315 %195
%323 = OpAll %bool %322
%324 = OpFOrdEqual %v4bool %316 %196
%325 = OpAll %bool %324
%326 = OpLogicalAnd %bool %323 %325
%327 = OpFOrdEqual %v4bool %317 %197
%328 = OpAll %bool %327
%329 = OpLogicalAnd %bool %326 %328
%330 = OpFOrdEqual %v4bool %318 %198
%331 = OpAll %bool %330
%332 = OpLogicalAnd %bool %329 %331
OpBranch %321
%321 = OpLabel
%333 = OpPhi %bool %false %301 %332 %320
OpStore %ok_0 %333
OpReturnValue %333
OpFunctionEnd
%test_comma_b = OpFunction %bool None %26
%334 = OpLabel
%x = OpVariable %_ptr_Function_mat2v2float Function
%y = OpVariable %_ptr_Function_mat2v2float Function
OpStore %x %40
OpStore %y %40
%337 = OpFOrdEqual %v2bool %38 %38
%338 = OpAll %bool %337
%339 = OpFOrdEqual %v2bool %39 %39
%340 = OpAll %bool %339
%341 = OpLogicalAnd %bool %338 %340
OpReturnValue %341
OpFunctionEnd
%main = OpFunction %v4float None %342
%343 = OpFunctionParameter %_ptr_Function_v2float
%344 = OpLabel
%354 = OpVariable %_ptr_Function_v4float Function
%345 = OpFunctionCall %bool %test_float_b
OpSelectionMerge %347 None
OpBranchConditional %345 %346 %347
%346 = OpLabel
%348 = OpFunctionCall %bool %test_half_b
OpBranch %347
%347 = OpLabel
%349 = OpPhi %bool %false %344 %348 %346
OpSelectionMerge %351 None
OpBranchConditional %349 %350 %351
%350 = OpLabel
%352 = OpFunctionCall %bool %test_comma_b
OpBranch %351
%351 = OpLabel
%353 = OpPhi %bool %false %347 %352 %350
OpSelectionMerge %358 None
OpBranchConditional %353 %356 %357
%356 = OpLabel
%359 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%362 = OpLoad %v4float %359
OpStore %354 %362
OpBranch %358
%357 = OpLabel
%363 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%364 = OpLoad %v4float %363
OpStore %354 %364
OpBranch %358
%358 = OpLabel
%365 = OpLoad %v4float %354
OpReturnValue %365
OpFunctionEnd
