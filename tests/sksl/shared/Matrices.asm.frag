OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_half_b "test_half_b"
OpName %ok "ok"
OpName %m1 "m1"
OpName %m3 "m3"
OpName %m4 "m4"
OpName %m5 "m5"
OpName %m7 "m7"
OpName %m9 "m9"
OpName %m10 "m10"
OpName %m11 "m11"
OpName %test_comma_b "test_comma_b"
OpName %x "x"
OpName %y "y"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m1 "_1_m1"
OpName %_2_m3 "_2_m3"
OpName %_3_m4 "_3_m4"
OpName %_4_m5 "_4_m5"
OpName %_5_m7 "_5_m7"
OpName %_6_m9 "_6_m9"
OpName %_7_m10 "_7_m10"
OpName %_8_m11 "_8_m11"
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
OpDecorate %m1 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %m3 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %m4 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %m5 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %m7 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %m9 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %m10 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %m11 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
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
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%37 = OpConstantComposite %v2float %float_1 %float_2
%38 = OpConstantComposite %v2float %float_3 %float_4
%39 = OpConstantComposite %mat2v2float %37 %38
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%float_6 = OpConstant %float 6
%61 = OpConstantComposite %v2float %float_6 %float_0
%62 = OpConstantComposite %v2float %float_0 %float_6
%63 = OpConstantComposite %mat2v2float %61 %62
%float_12 = OpConstant %float 12
%float_18 = OpConstant %float 18
%float_24 = OpConstant %float 24
%78 = OpConstantComposite %v2float %float_6 %float_12
%79 = OpConstantComposite %v2float %float_18 %float_24
%80 = OpConstantComposite %mat2v2float %78 %79
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%100 = OpConstantComposite %v2float %float_4 %float_0
%101 = OpConstantComposite %v2float %float_0 %float_4
%102 = OpConstantComposite %mat2v2float %100 %101
%float_5 = OpConstant %float 5
%float_8 = OpConstant %float 8
%116 = OpConstantComposite %v2float %float_5 %float_2
%117 = OpConstantComposite %v2float %float_3 %float_8
%118 = OpConstantComposite %mat2v2float %116 %117
%float_7 = OpConstant %float 7
%127 = OpConstantComposite %v2float %float_5 %float_6
%128 = OpConstantComposite %v2float %float_7 %float_8
%129 = OpConstantComposite %mat2v2float %127 %128
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_9 = OpConstant %float 9
%143 = OpConstantComposite %v3float %float_9 %float_0 %float_0
%144 = OpConstantComposite %v3float %float_0 %float_9 %float_0
%145 = OpConstantComposite %v3float %float_0 %float_0 %float_9
%146 = OpConstantComposite %mat3v3float %143 %144 %145
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_11 = OpConstant %float 11
%163 = OpConstantComposite %v4float %float_11 %float_0 %float_0 %float_0
%164 = OpConstantComposite %v4float %float_0 %float_11 %float_0 %float_0
%165 = OpConstantComposite %v4float %float_0 %float_0 %float_11 %float_0
%166 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_11
%167 = OpConstantComposite %mat4v4float %163 %164 %165 %166
%v4bool = OpTypeVector %bool 4
%float_20 = OpConstant %float 20
%185 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_20
%186 = OpConstantComposite %mat4v4float %185 %185 %185 %185
%194 = OpConstantComposite %v4float %float_9 %float_20 %float_20 %float_20
%195 = OpConstantComposite %v4float %float_20 %float_9 %float_20 %float_20
%196 = OpConstantComposite %v4float %float_20 %float_20 %float_9 %float_20
%197 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_9
%198 = OpConstantComposite %mat4v4float %194 %195 %196 %197
%219 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %25
%26 = OpLabel
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
OpStore %m1 %39
OpSelectionMerge %42 None
OpBranchConditional %true %41 %42
%41 = OpLabel
%44 = OpFOrdEqual %v2bool %37 %37
%45 = OpAll %bool %44
%46 = OpFOrdEqual %v2bool %38 %38
%47 = OpAll %bool %46
%48 = OpLogicalAnd %bool %45 %47
OpBranch %42
%42 = OpLabel
%49 = OpPhi %bool %false %26 %48 %41
OpStore %ok %49
OpStore %m3 %39
OpSelectionMerge %52 None
OpBranchConditional %49 %51 %52
%51 = OpLabel
%53 = OpFOrdEqual %v2bool %37 %37
%54 = OpAll %bool %53
%55 = OpFOrdEqual %v2bool %38 %38
%56 = OpAll %bool %55
%57 = OpLogicalAnd %bool %54 %56
OpBranch %52
%52 = OpLabel
%58 = OpPhi %bool %false %42 %57 %51
OpStore %ok %58
OpStore %m4 %63
OpSelectionMerge %65 None
OpBranchConditional %58 %64 %65
%64 = OpLabel
%66 = OpFOrdEqual %v2bool %61 %61
%67 = OpAll %bool %66
%68 = OpFOrdEqual %v2bool %62 %62
%69 = OpAll %bool %68
%70 = OpLogicalAnd %bool %67 %69
OpBranch %65
%65 = OpLabel
%71 = OpPhi %bool %false %52 %70 %64
OpStore %ok %71
%72 = OpMatrixTimesMatrix %mat2v2float %39 %63
OpStore %m3 %72
OpSelectionMerge %74 None
OpBranchConditional %71 %73 %74
%73 = OpLabel
%81 = OpCompositeExtract %v2float %72 0
%82 = OpFOrdEqual %v2bool %81 %78
%83 = OpAll %bool %82
%84 = OpCompositeExtract %v2float %72 1
%85 = OpFOrdEqual %v2bool %84 %79
%86 = OpAll %bool %85
%87 = OpLogicalAnd %bool %83 %86
OpBranch %74
%74 = OpLabel
%88 = OpPhi %bool %false %65 %87 %73
OpStore %ok %88
%92 = OpAccessChain %_ptr_Function_v2float %m1 %int_1
%93 = OpLoad %v2float %92
%94 = OpCompositeExtract %float %93 1
%95 = OpCompositeConstruct %v2float %94 %float_0
%96 = OpCompositeConstruct %v2float %float_0 %94
%97 = OpCompositeConstruct %mat2v2float %95 %96
OpStore %m5 %97
OpSelectionMerge %99 None
OpBranchConditional %88 %98 %99
%98 = OpLabel
%103 = OpFOrdEqual %v2bool %95 %100
%104 = OpAll %bool %103
%105 = OpFOrdEqual %v2bool %96 %101
%106 = OpAll %bool %105
%107 = OpLogicalAnd %bool %104 %106
OpBranch %99
%99 = OpLabel
%108 = OpPhi %bool %false %74 %107 %98
OpStore %ok %108
%109 = OpFAdd %v2float %37 %95
%110 = OpFAdd %v2float %38 %96
%111 = OpCompositeConstruct %mat2v2float %109 %110
OpStore %m1 %111
OpSelectionMerge %113 None
OpBranchConditional %108 %112 %113
%112 = OpLabel
%119 = OpFOrdEqual %v2bool %109 %116
%120 = OpAll %bool %119
%121 = OpFOrdEqual %v2bool %110 %117
%122 = OpAll %bool %121
%123 = OpLogicalAnd %bool %120 %122
OpBranch %113
%113 = OpLabel
%124 = OpPhi %bool %false %99 %123 %112
OpStore %ok %124
OpStore %m7 %129
OpSelectionMerge %131 None
OpBranchConditional %124 %130 %131
%130 = OpLabel
%132 = OpFOrdEqual %v2bool %127 %127
%133 = OpAll %bool %132
%134 = OpFOrdEqual %v2bool %128 %128
%135 = OpAll %bool %134
%136 = OpLogicalAnd %bool %133 %135
OpBranch %131
%131 = OpLabel
%137 = OpPhi %bool %false %113 %136 %130
OpStore %ok %137
OpStore %m9 %146
OpSelectionMerge %148 None
OpBranchConditional %137 %147 %148
%147 = OpLabel
%150 = OpFOrdEqual %v3bool %143 %143
%151 = OpAll %bool %150
%152 = OpFOrdEqual %v3bool %144 %144
%153 = OpAll %bool %152
%154 = OpLogicalAnd %bool %151 %153
%155 = OpFOrdEqual %v3bool %145 %145
%156 = OpAll %bool %155
%157 = OpLogicalAnd %bool %154 %156
OpBranch %148
%148 = OpLabel
%158 = OpPhi %bool %false %131 %157 %147
OpStore %ok %158
OpStore %m10 %167
OpSelectionMerge %169 None
OpBranchConditional %158 %168 %169
%168 = OpLabel
%171 = OpFOrdEqual %v4bool %163 %163
%172 = OpAll %bool %171
%173 = OpFOrdEqual %v4bool %164 %164
%174 = OpAll %bool %173
%175 = OpLogicalAnd %bool %172 %174
%176 = OpFOrdEqual %v4bool %165 %165
%177 = OpAll %bool %176
%178 = OpLogicalAnd %bool %175 %177
%179 = OpFOrdEqual %v4bool %166 %166
%180 = OpAll %bool %179
%181 = OpLogicalAnd %bool %178 %180
OpBranch %169
%169 = OpLabel
%182 = OpPhi %bool %false %148 %181 %168
OpStore %ok %182
OpStore %m11 %186
%187 = OpFSub %v4float %185 %163
%188 = OpFSub %v4float %185 %164
%189 = OpFSub %v4float %185 %165
%190 = OpFSub %v4float %185 %166
%191 = OpCompositeConstruct %mat4v4float %187 %188 %189 %190
OpStore %m11 %191
OpSelectionMerge %193 None
OpBranchConditional %182 %192 %193
%192 = OpLabel
%199 = OpFOrdEqual %v4bool %187 %194
%200 = OpAll %bool %199
%201 = OpFOrdEqual %v4bool %188 %195
%202 = OpAll %bool %201
%203 = OpLogicalAnd %bool %200 %202
%204 = OpFOrdEqual %v4bool %189 %196
%205 = OpAll %bool %204
%206 = OpLogicalAnd %bool %203 %205
%207 = OpFOrdEqual %v4bool %190 %197
%208 = OpAll %bool %207
%209 = OpLogicalAnd %bool %206 %208
OpBranch %193
%193 = OpLabel
%210 = OpPhi %bool %false %169 %209 %192
OpStore %ok %210
OpReturnValue %210
OpFunctionEnd
%test_comma_b = OpFunction %bool None %25
%211 = OpLabel
%x = OpVariable %_ptr_Function_mat2v2float Function
%y = OpVariable %_ptr_Function_mat2v2float Function
OpStore %x %39
OpStore %y %39
%214 = OpFOrdEqual %v2bool %37 %37
%215 = OpAll %bool %214
%216 = OpFOrdEqual %v2bool %38 %38
%217 = OpAll %bool %216
%218 = OpLogicalAnd %bool %215 %217
OpReturnValue %218
OpFunctionEnd
%main = OpFunction %v4float None %219
%220 = OpFunctionParameter %_ptr_Function_v2float
%221 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_7_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_8_m11 = OpVariable %_ptr_Function_mat4v4float Function
%351 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
OpStore %_1_m1 %39
OpSelectionMerge %225 None
OpBranchConditional %true %224 %225
%224 = OpLabel
%226 = OpFOrdEqual %v2bool %37 %37
%227 = OpAll %bool %226
%228 = OpFOrdEqual %v2bool %38 %38
%229 = OpAll %bool %228
%230 = OpLogicalAnd %bool %227 %229
OpBranch %225
%225 = OpLabel
%231 = OpPhi %bool %false %221 %230 %224
OpStore %_0_ok %231
OpStore %_2_m3 %39
OpSelectionMerge %234 None
OpBranchConditional %231 %233 %234
%233 = OpLabel
%235 = OpFOrdEqual %v2bool %37 %37
%236 = OpAll %bool %235
%237 = OpFOrdEqual %v2bool %38 %38
%238 = OpAll %bool %237
%239 = OpLogicalAnd %bool %236 %238
OpBranch %234
%234 = OpLabel
%240 = OpPhi %bool %false %225 %239 %233
OpStore %_0_ok %240
OpStore %_3_m4 %63
OpSelectionMerge %243 None
OpBranchConditional %240 %242 %243
%242 = OpLabel
%244 = OpFOrdEqual %v2bool %61 %61
%245 = OpAll %bool %244
%246 = OpFOrdEqual %v2bool %62 %62
%247 = OpAll %bool %246
%248 = OpLogicalAnd %bool %245 %247
OpBranch %243
%243 = OpLabel
%249 = OpPhi %bool %false %234 %248 %242
OpStore %_0_ok %249
%250 = OpMatrixTimesMatrix %mat2v2float %39 %63
OpStore %_2_m3 %250
OpSelectionMerge %252 None
OpBranchConditional %249 %251 %252
%251 = OpLabel
%253 = OpCompositeExtract %v2float %250 0
%254 = OpFOrdEqual %v2bool %253 %78
%255 = OpAll %bool %254
%256 = OpCompositeExtract %v2float %250 1
%257 = OpFOrdEqual %v2bool %256 %79
%258 = OpAll %bool %257
%259 = OpLogicalAnd %bool %255 %258
OpBranch %252
%252 = OpLabel
%260 = OpPhi %bool %false %243 %259 %251
OpStore %_0_ok %260
%262 = OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
%263 = OpLoad %v2float %262
%264 = OpCompositeExtract %float %263 1
%265 = OpCompositeConstruct %v2float %264 %float_0
%266 = OpCompositeConstruct %v2float %float_0 %264
%267 = OpCompositeConstruct %mat2v2float %265 %266
OpStore %_4_m5 %267
OpSelectionMerge %269 None
OpBranchConditional %260 %268 %269
%268 = OpLabel
%270 = OpFOrdEqual %v2bool %265 %100
%271 = OpAll %bool %270
%272 = OpFOrdEqual %v2bool %266 %101
%273 = OpAll %bool %272
%274 = OpLogicalAnd %bool %271 %273
OpBranch %269
%269 = OpLabel
%275 = OpPhi %bool %false %252 %274 %268
OpStore %_0_ok %275
%276 = OpFAdd %v2float %37 %265
%277 = OpFAdd %v2float %38 %266
%278 = OpCompositeConstruct %mat2v2float %276 %277
OpStore %_1_m1 %278
OpSelectionMerge %280 None
OpBranchConditional %275 %279 %280
%279 = OpLabel
%281 = OpFOrdEqual %v2bool %276 %116
%282 = OpAll %bool %281
%283 = OpFOrdEqual %v2bool %277 %117
%284 = OpAll %bool %283
%285 = OpLogicalAnd %bool %282 %284
OpBranch %280
%280 = OpLabel
%286 = OpPhi %bool %false %269 %285 %279
OpStore %_0_ok %286
OpStore %_5_m7 %129
OpSelectionMerge %289 None
OpBranchConditional %286 %288 %289
%288 = OpLabel
%290 = OpFOrdEqual %v2bool %127 %127
%291 = OpAll %bool %290
%292 = OpFOrdEqual %v2bool %128 %128
%293 = OpAll %bool %292
%294 = OpLogicalAnd %bool %291 %293
OpBranch %289
%289 = OpLabel
%295 = OpPhi %bool %false %280 %294 %288
OpStore %_0_ok %295
OpStore %_6_m9 %146
OpSelectionMerge %298 None
OpBranchConditional %295 %297 %298
%297 = OpLabel
%299 = OpFOrdEqual %v3bool %143 %143
%300 = OpAll %bool %299
%301 = OpFOrdEqual %v3bool %144 %144
%302 = OpAll %bool %301
%303 = OpLogicalAnd %bool %300 %302
%304 = OpFOrdEqual %v3bool %145 %145
%305 = OpAll %bool %304
%306 = OpLogicalAnd %bool %303 %305
OpBranch %298
%298 = OpLabel
%307 = OpPhi %bool %false %289 %306 %297
OpStore %_0_ok %307
OpStore %_7_m10 %167
OpSelectionMerge %310 None
OpBranchConditional %307 %309 %310
%309 = OpLabel
%311 = OpFOrdEqual %v4bool %163 %163
%312 = OpAll %bool %311
%313 = OpFOrdEqual %v4bool %164 %164
%314 = OpAll %bool %313
%315 = OpLogicalAnd %bool %312 %314
%316 = OpFOrdEqual %v4bool %165 %165
%317 = OpAll %bool %316
%318 = OpLogicalAnd %bool %315 %317
%319 = OpFOrdEqual %v4bool %166 %166
%320 = OpAll %bool %319
%321 = OpLogicalAnd %bool %318 %320
OpBranch %310
%310 = OpLabel
%322 = OpPhi %bool %false %298 %321 %309
OpStore %_0_ok %322
OpStore %_8_m11 %186
%324 = OpFSub %v4float %185 %163
%325 = OpFSub %v4float %185 %164
%326 = OpFSub %v4float %185 %165
%327 = OpFSub %v4float %185 %166
%328 = OpCompositeConstruct %mat4v4float %324 %325 %326 %327
OpStore %_8_m11 %328
OpSelectionMerge %330 None
OpBranchConditional %322 %329 %330
%329 = OpLabel
%331 = OpFOrdEqual %v4bool %324 %194
%332 = OpAll %bool %331
%333 = OpFOrdEqual %v4bool %325 %195
%334 = OpAll %bool %333
%335 = OpLogicalAnd %bool %332 %334
%336 = OpFOrdEqual %v4bool %326 %196
%337 = OpAll %bool %336
%338 = OpLogicalAnd %bool %335 %337
%339 = OpFOrdEqual %v4bool %327 %197
%340 = OpAll %bool %339
%341 = OpLogicalAnd %bool %338 %340
OpBranch %330
%330 = OpLabel
%342 = OpPhi %bool %false %310 %341 %329
OpStore %_0_ok %342
OpSelectionMerge %344 None
OpBranchConditional %342 %343 %344
%343 = OpLabel
%345 = OpFunctionCall %bool %test_half_b
OpBranch %344
%344 = OpLabel
%346 = OpPhi %bool %false %330 %345 %343
OpSelectionMerge %348 None
OpBranchConditional %346 %347 %348
%347 = OpLabel
%349 = OpFunctionCall %bool %test_comma_b
OpBranch %348
%348 = OpLabel
%350 = OpPhi %bool %false %344 %349 %347
OpSelectionMerge %355 None
OpBranchConditional %350 %353 %354
%353 = OpLabel
%356 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%359 = OpLoad %v4float %356
OpStore %351 %359
OpBranch %355
%354 = OpLabel
%360 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%361 = OpLoad %v4float %360
OpStore %351 %361
OpBranch %355
%355 = OpLabel
%362 = OpLoad %v4float %351
OpReturnValue %362
OpFunctionEnd
