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
OpName %m23 "m23"
OpName %m24 "m24"
OpName %m32 "m32"
OpName %m34 "m34"
OpName %m42 "m42"
OpName %m43 "m43"
OpName %m22 "m22"
OpName %m33 "m33"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m23 "_1_m23"
OpName %_2_m24 "_2_m24"
OpName %_3_m32 "_3_m32"
OpName %_4_m34 "_4_m34"
OpName %_5_m42 "_5_m42"
OpName %_6_m43 "_6_m43"
OpName %_7_m22 "_7_m22"
OpName %_8_m33 "_8_m33"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %m23 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %m24 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %m32 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %m34 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %m42 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %m22 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%float_2 = OpConstant %float 2
%34 = OpConstantComposite %v3float %float_2 %float_0 %float_0
%35 = OpConstantComposite %v3float %float_0 %float_2 %float_0
%36 = OpConstantComposite %mat2v3float %34 %35
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_3 = OpConstant %float 3
%51 = OpConstantComposite %v4float %float_3 %float_0 %float_0 %float_0
%52 = OpConstantComposite %v4float %float_0 %float_3 %float_0 %float_0
%53 = OpConstantComposite %mat2v4float %51 %52
%v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%67 = OpConstantComposite %v2float %float_4 %float_0
%68 = OpConstantComposite %v2float %float_0 %float_4
%69 = OpConstantComposite %mat3v2float %67 %68 %20
%v2bool = OpTypeVector %bool 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%86 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
%87 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
%88 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
%89 = OpConstantComposite %mat3v4float %86 %87 %88
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%105 = OpConstantComposite %v2float %float_6 %float_0
%106 = OpConstantComposite %v2float %float_0 %float_6
%107 = OpConstantComposite %mat4v2float %105 %106 %20 %20
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%126 = OpConstantComposite %v3float %float_7 %float_0 %float_0
%127 = OpConstantComposite %v3float %float_0 %float_7 %float_0
%128 = OpConstantComposite %v3float %float_0 %float_0 %float_7
%129 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%130 = OpConstantComposite %mat4v3float %126 %127 %128 %129
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_8 = OpConstant %float 8
%152 = OpConstantComposite %v2float %float_8 %float_0
%153 = OpConstantComposite %v2float %float_0 %float_8
%154 = OpConstantComposite %mat2v2float %152 %153
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%170 = OpConstantComposite %v3float %float_35 %float_0 %float_0
%171 = OpConstantComposite %v3float %float_0 %float_35 %float_0
%172 = OpConstantComposite %v3float %float_0 %float_0 %float_35
%173 = OpConstantComposite %mat3v3float %170 %171 %172
%float_1 = OpConstant %float 1
%187 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%188 = OpConstantComposite %mat2v3float %187 %187
%194 = OpConstantComposite %v3float %float_3 %float_1 %float_1
%195 = OpConstantComposite %v3float %float_1 %float_3 %float_1
%196 = OpConstantComposite %mat2v3float %194 %195
%203 = OpConstantComposite %v2float %float_2 %float_2
%204 = OpConstantComposite %mat3v2float %203 %203 %203
%float_n2 = OpConstant %float -2
%212 = OpConstantComposite %v2float %float_2 %float_n2
%213 = OpConstantComposite %v2float %float_n2 %float_2
%214 = OpConstantComposite %v2float %float_n2 %float_n2
%215 = OpConstantComposite %mat3v2float %212 %213 %214
%225 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%226 = OpConstantComposite %mat2v4float %225 %225
%float_0_75 = OpConstant %float 0.75
%233 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0 %float_0
%234 = OpConstantComposite %v4float %float_0 %float_0_75 %float_0 %float_0
%235 = OpConstantComposite %mat2v4float %233 %234
%242 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %24
%25 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%m23 = OpVariable %_ptr_Function_mat2v3float Function
%m24 = OpVariable %_ptr_Function_mat2v4float Function
%m32 = OpVariable %_ptr_Function_mat3v2float Function
%m34 = OpVariable %_ptr_Function_mat3v4float Function
%m42 = OpVariable %_ptr_Function_mat4v2float Function
%m43 = OpVariable %_ptr_Function_mat4v3float Function
%m22 = OpVariable %_ptr_Function_mat2v2float Function
%m33 = OpVariable %_ptr_Function_mat3v3float Function
OpStore %ok %true
OpStore %m23 %36
OpSelectionMerge %39 None
OpBranchConditional %true %38 %39
%38 = OpLabel
%41 = OpFOrdEqual %v3bool %34 %34
%42 = OpAll %bool %41
%43 = OpFOrdEqual %v3bool %35 %35
%44 = OpAll %bool %43
%45 = OpLogicalAnd %bool %42 %44
OpBranch %39
%39 = OpLabel
%46 = OpPhi %bool %false %25 %45 %38
OpStore %ok %46
OpStore %m24 %53
OpSelectionMerge %55 None
OpBranchConditional %46 %54 %55
%54 = OpLabel
%57 = OpFOrdEqual %v4bool %51 %51
%58 = OpAll %bool %57
%59 = OpFOrdEqual %v4bool %52 %52
%60 = OpAll %bool %59
%61 = OpLogicalAnd %bool %58 %60
OpBranch %55
%55 = OpLabel
%62 = OpPhi %bool %false %39 %61 %54
OpStore %ok %62
OpStore %m32 %69
OpSelectionMerge %71 None
OpBranchConditional %62 %70 %71
%70 = OpLabel
%73 = OpFOrdEqual %v2bool %67 %67
%74 = OpAll %bool %73
%75 = OpFOrdEqual %v2bool %68 %68
%76 = OpAll %bool %75
%77 = OpLogicalAnd %bool %74 %76
%78 = OpFOrdEqual %v2bool %20 %20
%79 = OpAll %bool %78
%80 = OpLogicalAnd %bool %77 %79
OpBranch %71
%71 = OpLabel
%81 = OpPhi %bool %false %55 %80 %70
OpStore %ok %81
OpStore %m34 %89
OpSelectionMerge %91 None
OpBranchConditional %81 %90 %91
%90 = OpLabel
%92 = OpFOrdEqual %v4bool %86 %86
%93 = OpAll %bool %92
%94 = OpFOrdEqual %v4bool %87 %87
%95 = OpAll %bool %94
%96 = OpLogicalAnd %bool %93 %95
%97 = OpFOrdEqual %v4bool %88 %88
%98 = OpAll %bool %97
%99 = OpLogicalAnd %bool %96 %98
OpBranch %91
%91 = OpLabel
%100 = OpPhi %bool %false %71 %99 %90
OpStore %ok %100
OpStore %m42 %107
OpSelectionMerge %109 None
OpBranchConditional %100 %108 %109
%108 = OpLabel
%110 = OpFOrdEqual %v2bool %105 %105
%111 = OpAll %bool %110
%112 = OpFOrdEqual %v2bool %106 %106
%113 = OpAll %bool %112
%114 = OpLogicalAnd %bool %111 %113
%115 = OpFOrdEqual %v2bool %20 %20
%116 = OpAll %bool %115
%117 = OpLogicalAnd %bool %114 %116
%118 = OpFOrdEqual %v2bool %20 %20
%119 = OpAll %bool %118
%120 = OpLogicalAnd %bool %117 %119
OpBranch %109
%109 = OpLabel
%121 = OpPhi %bool %false %91 %120 %108
OpStore %ok %121
OpStore %m43 %130
OpSelectionMerge %132 None
OpBranchConditional %121 %131 %132
%131 = OpLabel
%133 = OpFOrdEqual %v3bool %126 %126
%134 = OpAll %bool %133
%135 = OpFOrdEqual %v3bool %127 %127
%136 = OpAll %bool %135
%137 = OpLogicalAnd %bool %134 %136
%138 = OpFOrdEqual %v3bool %128 %128
%139 = OpAll %bool %138
%140 = OpLogicalAnd %bool %137 %139
%141 = OpFOrdEqual %v3bool %129 %129
%142 = OpAll %bool %141
%143 = OpLogicalAnd %bool %140 %142
OpBranch %132
%132 = OpLabel
%144 = OpPhi %bool %false %109 %143 %131
OpStore %ok %144
%148 = OpMatrixTimesMatrix %mat2v2float %69 %36
OpStore %m22 %148
OpSelectionMerge %150 None
OpBranchConditional %144 %149 %150
%149 = OpLabel
%155 = OpCompositeExtract %v2float %148 0
%156 = OpFOrdEqual %v2bool %155 %152
%157 = OpAll %bool %156
%158 = OpCompositeExtract %v2float %148 1
%159 = OpFOrdEqual %v2bool %158 %153
%160 = OpAll %bool %159
%161 = OpLogicalAnd %bool %157 %160
OpBranch %150
%150 = OpLabel
%162 = OpPhi %bool %false %132 %161 %149
OpStore %ok %162
%166 = OpMatrixTimesMatrix %mat3v3float %130 %89
OpStore %m33 %166
OpSelectionMerge %168 None
OpBranchConditional %162 %167 %168
%167 = OpLabel
%174 = OpCompositeExtract %v3float %166 0
%175 = OpFOrdEqual %v3bool %174 %170
%176 = OpAll %bool %175
%177 = OpCompositeExtract %v3float %166 1
%178 = OpFOrdEqual %v3bool %177 %171
%179 = OpAll %bool %178
%180 = OpLogicalAnd %bool %176 %179
%181 = OpCompositeExtract %v3float %166 2
%182 = OpFOrdEqual %v3bool %181 %172
%183 = OpAll %bool %182
%184 = OpLogicalAnd %bool %180 %183
OpBranch %168
%168 = OpLabel
%185 = OpPhi %bool %false %150 %184 %167
OpStore %ok %185
%189 = OpFAdd %v3float %34 %187
%190 = OpFAdd %v3float %35 %187
%191 = OpCompositeConstruct %mat2v3float %189 %190
OpStore %m23 %191
OpSelectionMerge %193 None
OpBranchConditional %185 %192 %193
%192 = OpLabel
%197 = OpFOrdEqual %v3bool %189 %194
%198 = OpAll %bool %197
%199 = OpFOrdEqual %v3bool %190 %195
%200 = OpAll %bool %199
%201 = OpLogicalAnd %bool %198 %200
OpBranch %193
%193 = OpLabel
%202 = OpPhi %bool %false %168 %201 %192
OpStore %ok %202
%205 = OpFSub %v2float %67 %203
%206 = OpFSub %v2float %68 %203
%207 = OpFSub %v2float %20 %203
%208 = OpCompositeConstruct %mat3v2float %205 %206 %207
OpStore %m32 %208
OpSelectionMerge %210 None
OpBranchConditional %202 %209 %210
%209 = OpLabel
%216 = OpFOrdEqual %v2bool %205 %212
%217 = OpAll %bool %216
%218 = OpFOrdEqual %v2bool %206 %213
%219 = OpAll %bool %218
%220 = OpLogicalAnd %bool %217 %219
%221 = OpFOrdEqual %v2bool %207 %214
%222 = OpAll %bool %221
%223 = OpLogicalAnd %bool %220 %222
OpBranch %210
%210 = OpLabel
%224 = OpPhi %bool %false %193 %223 %209
OpStore %ok %224
%227 = OpFDiv %v4float %51 %225
%228 = OpFDiv %v4float %52 %225
%229 = OpCompositeConstruct %mat2v4float %227 %228
OpStore %m24 %229
OpSelectionMerge %231 None
OpBranchConditional %224 %230 %231
%230 = OpLabel
%236 = OpFOrdEqual %v4bool %227 %233
%237 = OpAll %bool %236
%238 = OpFOrdEqual %v4bool %228 %234
%239 = OpAll %bool %238
%240 = OpLogicalAnd %bool %237 %239
OpBranch %231
%231 = OpLabel
%241 = OpPhi %bool %false %210 %240 %230
OpStore %ok %241
OpReturnValue %241
OpFunctionEnd
%main = OpFunction %v4float None %242
%243 = OpFunctionParameter %_ptr_Function_v2float
%244 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%387 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
OpStore %_1_m23 %36
OpSelectionMerge %248 None
OpBranchConditional %true %247 %248
%247 = OpLabel
%249 = OpFOrdEqual %v3bool %34 %34
%250 = OpAll %bool %249
%251 = OpFOrdEqual %v3bool %35 %35
%252 = OpAll %bool %251
%253 = OpLogicalAnd %bool %250 %252
OpBranch %248
%248 = OpLabel
%254 = OpPhi %bool %false %244 %253 %247
OpStore %_0_ok %254
OpStore %_2_m24 %53
OpSelectionMerge %257 None
OpBranchConditional %254 %256 %257
%256 = OpLabel
%258 = OpFOrdEqual %v4bool %51 %51
%259 = OpAll %bool %258
%260 = OpFOrdEqual %v4bool %52 %52
%261 = OpAll %bool %260
%262 = OpLogicalAnd %bool %259 %261
OpBranch %257
%257 = OpLabel
%263 = OpPhi %bool %false %248 %262 %256
OpStore %_0_ok %263
OpStore %_3_m32 %69
OpSelectionMerge %266 None
OpBranchConditional %263 %265 %266
%265 = OpLabel
%267 = OpFOrdEqual %v2bool %67 %67
%268 = OpAll %bool %267
%269 = OpFOrdEqual %v2bool %68 %68
%270 = OpAll %bool %269
%271 = OpLogicalAnd %bool %268 %270
%272 = OpFOrdEqual %v2bool %20 %20
%273 = OpAll %bool %272
%274 = OpLogicalAnd %bool %271 %273
OpBranch %266
%266 = OpLabel
%275 = OpPhi %bool %false %257 %274 %265
OpStore %_0_ok %275
OpStore %_4_m34 %89
OpSelectionMerge %278 None
OpBranchConditional %275 %277 %278
%277 = OpLabel
%279 = OpFOrdEqual %v4bool %86 %86
%280 = OpAll %bool %279
%281 = OpFOrdEqual %v4bool %87 %87
%282 = OpAll %bool %281
%283 = OpLogicalAnd %bool %280 %282
%284 = OpFOrdEqual %v4bool %88 %88
%285 = OpAll %bool %284
%286 = OpLogicalAnd %bool %283 %285
OpBranch %278
%278 = OpLabel
%287 = OpPhi %bool %false %266 %286 %277
OpStore %_0_ok %287
OpStore %_5_m42 %107
OpSelectionMerge %290 None
OpBranchConditional %287 %289 %290
%289 = OpLabel
%291 = OpFOrdEqual %v2bool %105 %105
%292 = OpAll %bool %291
%293 = OpFOrdEqual %v2bool %106 %106
%294 = OpAll %bool %293
%295 = OpLogicalAnd %bool %292 %294
%296 = OpFOrdEqual %v2bool %20 %20
%297 = OpAll %bool %296
%298 = OpLogicalAnd %bool %295 %297
%299 = OpFOrdEqual %v2bool %20 %20
%300 = OpAll %bool %299
%301 = OpLogicalAnd %bool %298 %300
OpBranch %290
%290 = OpLabel
%302 = OpPhi %bool %false %278 %301 %289
OpStore %_0_ok %302
OpStore %_6_m43 %130
OpSelectionMerge %305 None
OpBranchConditional %302 %304 %305
%304 = OpLabel
%306 = OpFOrdEqual %v3bool %126 %126
%307 = OpAll %bool %306
%308 = OpFOrdEqual %v3bool %127 %127
%309 = OpAll %bool %308
%310 = OpLogicalAnd %bool %307 %309
%311 = OpFOrdEqual %v3bool %128 %128
%312 = OpAll %bool %311
%313 = OpLogicalAnd %bool %310 %312
%314 = OpFOrdEqual %v3bool %129 %129
%315 = OpAll %bool %314
%316 = OpLogicalAnd %bool %313 %315
OpBranch %305
%305 = OpLabel
%317 = OpPhi %bool %false %290 %316 %304
OpStore %_0_ok %317
%319 = OpMatrixTimesMatrix %mat2v2float %69 %36
OpStore %_7_m22 %319
OpSelectionMerge %321 None
OpBranchConditional %317 %320 %321
%320 = OpLabel
%322 = OpCompositeExtract %v2float %319 0
%323 = OpFOrdEqual %v2bool %322 %152
%324 = OpAll %bool %323
%325 = OpCompositeExtract %v2float %319 1
%326 = OpFOrdEqual %v2bool %325 %153
%327 = OpAll %bool %326
%328 = OpLogicalAnd %bool %324 %327
OpBranch %321
%321 = OpLabel
%329 = OpPhi %bool %false %305 %328 %320
OpStore %_0_ok %329
%331 = OpMatrixTimesMatrix %mat3v3float %130 %89
OpStore %_8_m33 %331
OpSelectionMerge %333 None
OpBranchConditional %329 %332 %333
%332 = OpLabel
%334 = OpCompositeExtract %v3float %331 0
%335 = OpFOrdEqual %v3bool %334 %170
%336 = OpAll %bool %335
%337 = OpCompositeExtract %v3float %331 1
%338 = OpFOrdEqual %v3bool %337 %171
%339 = OpAll %bool %338
%340 = OpLogicalAnd %bool %336 %339
%341 = OpCompositeExtract %v3float %331 2
%342 = OpFOrdEqual %v3bool %341 %172
%343 = OpAll %bool %342
%344 = OpLogicalAnd %bool %340 %343
OpBranch %333
%333 = OpLabel
%345 = OpPhi %bool %false %321 %344 %332
OpStore %_0_ok %345
%346 = OpFAdd %v3float %34 %187
%347 = OpFAdd %v3float %35 %187
%348 = OpCompositeConstruct %mat2v3float %346 %347
OpStore %_1_m23 %348
OpSelectionMerge %350 None
OpBranchConditional %345 %349 %350
%349 = OpLabel
%351 = OpFOrdEqual %v3bool %346 %194
%352 = OpAll %bool %351
%353 = OpFOrdEqual %v3bool %347 %195
%354 = OpAll %bool %353
%355 = OpLogicalAnd %bool %352 %354
OpBranch %350
%350 = OpLabel
%356 = OpPhi %bool %false %333 %355 %349
OpStore %_0_ok %356
%357 = OpFSub %v2float %67 %203
%358 = OpFSub %v2float %68 %203
%359 = OpFSub %v2float %20 %203
%360 = OpCompositeConstruct %mat3v2float %357 %358 %359
OpStore %_3_m32 %360
OpSelectionMerge %362 None
OpBranchConditional %356 %361 %362
%361 = OpLabel
%363 = OpFOrdEqual %v2bool %357 %212
%364 = OpAll %bool %363
%365 = OpFOrdEqual %v2bool %358 %213
%366 = OpAll %bool %365
%367 = OpLogicalAnd %bool %364 %366
%368 = OpFOrdEqual %v2bool %359 %214
%369 = OpAll %bool %368
%370 = OpLogicalAnd %bool %367 %369
OpBranch %362
%362 = OpLabel
%371 = OpPhi %bool %false %350 %370 %361
OpStore %_0_ok %371
%372 = OpFDiv %v4float %51 %225
%373 = OpFDiv %v4float %52 %225
%374 = OpCompositeConstruct %mat2v4float %372 %373
OpStore %_2_m24 %374
OpSelectionMerge %376 None
OpBranchConditional %371 %375 %376
%375 = OpLabel
%377 = OpFOrdEqual %v4bool %372 %233
%378 = OpAll %bool %377
%379 = OpFOrdEqual %v4bool %373 %234
%380 = OpAll %bool %379
%381 = OpLogicalAnd %bool %378 %380
OpBranch %376
%376 = OpLabel
%382 = OpPhi %bool %false %362 %381 %375
OpStore %_0_ok %382
OpSelectionMerge %384 None
OpBranchConditional %382 %383 %384
%383 = OpLabel
%385 = OpFunctionCall %bool %test_half_b
OpBranch %384
%384 = OpLabel
%386 = OpPhi %bool %false %376 %385 %383
OpSelectionMerge %391 None
OpBranchConditional %386 %389 %390
%389 = OpLabel
%392 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%396 = OpLoad %v4float %392
OpStore %387 %396
OpBranch %391
%390 = OpLabel
%397 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%399 = OpLoad %v4float %397
OpStore %387 %399
OpBranch %391
%391 = OpLabel
%400 = OpLoad %v4float %387
OpReturnValue %400
OpFunctionEnd
