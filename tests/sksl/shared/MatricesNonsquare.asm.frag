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
OpDecorate %34 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %m24 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %m32 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %m34 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %m42 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %m22 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %481 RelaxedPrecision
OpDecorate %500 RelaxedPrecision
OpDecorate %528 RelaxedPrecision
OpDecorate %553 RelaxedPrecision
OpDecorate %580 RelaxedPrecision
OpDecorate %594 RelaxedPrecision
OpDecorate %608 RelaxedPrecision
OpDecorate %611 RelaxedPrecision
OpDecorate %612 RelaxedPrecision
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
%35 = OpConstantComposite %v3float %float_2 %float_0 %float_0
%36 = OpConstantComposite %v3float %float_0 %float_2 %float_0
%false = OpConstantFalse %bool
%42 = OpConstantComposite %mat2v3float %35 %36
%v3bool = OpTypeVector %bool 3
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_3 = OpConstant %float 3
%59 = OpConstantComposite %v4float %float_3 %float_0 %float_0 %float_0
%60 = OpConstantComposite %v4float %float_0 %float_3 %float_0 %float_0
%65 = OpConstantComposite %mat2v4float %59 %60
%v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%82 = OpConstantComposite %v2float %float_4 %float_0
%83 = OpConstantComposite %v2float %float_0 %float_4
%88 = OpConstantComposite %mat3v2float %82 %83 %20
%v2bool = OpTypeVector %bool 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%110 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
%111 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
%112 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
%117 = OpConstantComposite %mat3v4float %110 %111 %112
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%138 = OpConstantComposite %v2float %float_6 %float_0
%139 = OpConstantComposite %v2float %float_0 %float_6
%144 = OpConstantComposite %mat4v2float %138 %139 %20 %20
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%170 = OpConstantComposite %v3float %float_7 %float_0 %float_0
%171 = OpConstantComposite %v3float %float_0 %float_7 %float_0
%172 = OpConstantComposite %v3float %float_0 %float_0 %float_7
%173 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%178 = OpConstantComposite %mat4v3float %170 %171 %172 %173
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_8 = OpConstant %float 8
%211 = OpConstantComposite %v2float %float_8 %float_0
%212 = OpConstantComposite %v2float %float_0 %float_8
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%235 = OpConstantComposite %v3float %float_35 %float_0 %float_0
%236 = OpConstantComposite %v3float %float_0 %float_35 %float_0
%237 = OpConstantComposite %v3float %float_0 %float_0 %float_35
%float_1 = OpConstant %float 1
%255 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%256 = OpConstantComposite %mat2v3float %255 %255
%268 = OpConstantComposite %v3float %float_3 %float_1 %float_1
%269 = OpConstantComposite %v3float %float_1 %float_3 %float_1
%270 = OpConstantComposite %mat2v3float %268 %269
%282 = OpConstantComposite %v2float %float_2 %float_2
%283 = OpConstantComposite %mat3v2float %282 %282 %282
%float_n2 = OpConstant %float -2
%299 = OpConstantComposite %v2float %float_2 %float_n2
%300 = OpConstantComposite %v2float %float_n2 %float_2
%301 = OpConstantComposite %v2float %float_n2 %float_n2
%302 = OpConstantComposite %mat3v2float %299 %300 %301
%319 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%320 = OpConstantComposite %mat2v4float %319 %319
%float_0_75 = OpConstant %float 0.75
%333 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0 %float_0
%334 = OpConstantComposite %v4float %float_0 %float_0_75 %float_0 %float_0
%335 = OpConstantComposite %mat2v4float %333 %334
%347 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%34 = OpCompositeConstruct %mat2v3float %35 %36
OpStore %m23 %34
%38 = OpLoad %bool %ok
OpSelectionMerge %40 None
OpBranchConditional %38 %39 %40
%39 = OpLabel
%41 = OpLoad %mat2v3float %m23
%44 = OpCompositeExtract %v3float %41 0
%45 = OpCompositeExtract %v3float %42 0
%46 = OpFOrdEqual %v3bool %44 %45
%47 = OpAll %bool %46
%48 = OpCompositeExtract %v3float %41 1
%49 = OpCompositeExtract %v3float %42 1
%50 = OpFOrdEqual %v3bool %48 %49
%51 = OpAll %bool %50
%52 = OpLogicalAnd %bool %47 %51
OpBranch %40
%40 = OpLabel
%53 = OpPhi %bool %false %25 %52 %39
OpStore %ok %53
%58 = OpCompositeConstruct %mat2v4float %59 %60
OpStore %m24 %58
%61 = OpLoad %bool %ok
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%64 = OpLoad %mat2v4float %m24
%67 = OpCompositeExtract %v4float %64 0
%68 = OpCompositeExtract %v4float %65 0
%69 = OpFOrdEqual %v4bool %67 %68
%70 = OpAll %bool %69
%71 = OpCompositeExtract %v4float %64 1
%72 = OpCompositeExtract %v4float %65 1
%73 = OpFOrdEqual %v4bool %71 %72
%74 = OpAll %bool %73
%75 = OpLogicalAnd %bool %70 %74
OpBranch %63
%63 = OpLabel
%76 = OpPhi %bool %false %40 %75 %62
OpStore %ok %76
%81 = OpCompositeConstruct %mat3v2float %82 %83 %20
OpStore %m32 %81
%84 = OpLoad %bool %ok
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpLoad %mat3v2float %m32
%90 = OpCompositeExtract %v2float %87 0
%91 = OpCompositeExtract %v2float %88 0
%92 = OpFOrdEqual %v2bool %90 %91
%93 = OpAll %bool %92
%94 = OpCompositeExtract %v2float %87 1
%95 = OpCompositeExtract %v2float %88 1
%96 = OpFOrdEqual %v2bool %94 %95
%97 = OpAll %bool %96
%98 = OpLogicalAnd %bool %93 %97
%99 = OpCompositeExtract %v2float %87 2
%100 = OpCompositeExtract %v2float %88 2
%101 = OpFOrdEqual %v2bool %99 %100
%102 = OpAll %bool %101
%103 = OpLogicalAnd %bool %98 %102
OpBranch %86
%86 = OpLabel
%104 = OpPhi %bool %false %63 %103 %85
OpStore %ok %104
%109 = OpCompositeConstruct %mat3v4float %110 %111 %112
OpStore %m34 %109
%113 = OpLoad %bool %ok
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%116 = OpLoad %mat3v4float %m34
%118 = OpCompositeExtract %v4float %116 0
%119 = OpCompositeExtract %v4float %117 0
%120 = OpFOrdEqual %v4bool %118 %119
%121 = OpAll %bool %120
%122 = OpCompositeExtract %v4float %116 1
%123 = OpCompositeExtract %v4float %117 1
%124 = OpFOrdEqual %v4bool %122 %123
%125 = OpAll %bool %124
%126 = OpLogicalAnd %bool %121 %125
%127 = OpCompositeExtract %v4float %116 2
%128 = OpCompositeExtract %v4float %117 2
%129 = OpFOrdEqual %v4bool %127 %128
%130 = OpAll %bool %129
%131 = OpLogicalAnd %bool %126 %130
OpBranch %115
%115 = OpLabel
%132 = OpPhi %bool %false %86 %131 %114
OpStore %ok %132
%137 = OpCompositeConstruct %mat4v2float %138 %139 %20 %20
OpStore %m42 %137
%140 = OpLoad %bool %ok
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%143 = OpLoad %mat4v2float %m42
%145 = OpCompositeExtract %v2float %143 0
%146 = OpCompositeExtract %v2float %144 0
%147 = OpFOrdEqual %v2bool %145 %146
%148 = OpAll %bool %147
%149 = OpCompositeExtract %v2float %143 1
%150 = OpCompositeExtract %v2float %144 1
%151 = OpFOrdEqual %v2bool %149 %150
%152 = OpAll %bool %151
%153 = OpLogicalAnd %bool %148 %152
%154 = OpCompositeExtract %v2float %143 2
%155 = OpCompositeExtract %v2float %144 2
%156 = OpFOrdEqual %v2bool %154 %155
%157 = OpAll %bool %156
%158 = OpLogicalAnd %bool %153 %157
%159 = OpCompositeExtract %v2float %143 3
%160 = OpCompositeExtract %v2float %144 3
%161 = OpFOrdEqual %v2bool %159 %160
%162 = OpAll %bool %161
%163 = OpLogicalAnd %bool %158 %162
OpBranch %142
%142 = OpLabel
%164 = OpPhi %bool %false %115 %163 %141
OpStore %ok %164
%169 = OpCompositeConstruct %mat4v3float %170 %171 %172 %173
OpStore %m43 %169
%174 = OpLoad %bool %ok
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%177 = OpLoad %mat4v3float %m43
%179 = OpCompositeExtract %v3float %177 0
%180 = OpCompositeExtract %v3float %178 0
%181 = OpFOrdEqual %v3bool %179 %180
%182 = OpAll %bool %181
%183 = OpCompositeExtract %v3float %177 1
%184 = OpCompositeExtract %v3float %178 1
%185 = OpFOrdEqual %v3bool %183 %184
%186 = OpAll %bool %185
%187 = OpLogicalAnd %bool %182 %186
%188 = OpCompositeExtract %v3float %177 2
%189 = OpCompositeExtract %v3float %178 2
%190 = OpFOrdEqual %v3bool %188 %189
%191 = OpAll %bool %190
%192 = OpLogicalAnd %bool %187 %191
%193 = OpCompositeExtract %v3float %177 3
%194 = OpCompositeExtract %v3float %178 3
%195 = OpFOrdEqual %v3bool %193 %194
%196 = OpAll %bool %195
%197 = OpLogicalAnd %bool %192 %196
OpBranch %176
%176 = OpLabel
%198 = OpPhi %bool %false %142 %197 %175
OpStore %ok %198
%202 = OpLoad %mat3v2float %m32
%203 = OpLoad %mat2v3float %m23
%204 = OpMatrixTimesMatrix %mat2v2float %202 %203
OpStore %m22 %204
%205 = OpLoad %bool %ok
OpSelectionMerge %207 None
OpBranchConditional %205 %206 %207
%206 = OpLabel
%208 = OpLoad %mat2v2float %m22
%210 = OpCompositeConstruct %mat2v2float %211 %212
%213 = OpCompositeExtract %v2float %208 0
%214 = OpCompositeExtract %v2float %210 0
%215 = OpFOrdEqual %v2bool %213 %214
%216 = OpAll %bool %215
%217 = OpCompositeExtract %v2float %208 1
%218 = OpCompositeExtract %v2float %210 1
%219 = OpFOrdEqual %v2bool %217 %218
%220 = OpAll %bool %219
%221 = OpLogicalAnd %bool %216 %220
OpBranch %207
%207 = OpLabel
%222 = OpPhi %bool %false %176 %221 %206
OpStore %ok %222
%226 = OpLoad %mat4v3float %m43
%227 = OpLoad %mat3v4float %m34
%228 = OpMatrixTimesMatrix %mat3v3float %226 %227
OpStore %m33 %228
%229 = OpLoad %bool %ok
OpSelectionMerge %231 None
OpBranchConditional %229 %230 %231
%230 = OpLabel
%232 = OpLoad %mat3v3float %m33
%234 = OpCompositeConstruct %mat3v3float %235 %236 %237
%238 = OpCompositeExtract %v3float %232 0
%239 = OpCompositeExtract %v3float %234 0
%240 = OpFOrdEqual %v3bool %238 %239
%241 = OpAll %bool %240
%242 = OpCompositeExtract %v3float %232 1
%243 = OpCompositeExtract %v3float %234 1
%244 = OpFOrdEqual %v3bool %242 %243
%245 = OpAll %bool %244
%246 = OpLogicalAnd %bool %241 %245
%247 = OpCompositeExtract %v3float %232 2
%248 = OpCompositeExtract %v3float %234 2
%249 = OpFOrdEqual %v3bool %247 %248
%250 = OpAll %bool %249
%251 = OpLogicalAnd %bool %246 %250
OpBranch %231
%231 = OpLabel
%252 = OpPhi %bool %false %207 %251 %230
OpStore %ok %252
%253 = OpLoad %mat2v3float %m23
%257 = OpCompositeExtract %v3float %253 0
%258 = OpCompositeExtract %v3float %256 0
%259 = OpFAdd %v3float %257 %258
%260 = OpCompositeExtract %v3float %253 1
%261 = OpCompositeExtract %v3float %256 1
%262 = OpFAdd %v3float %260 %261
%263 = OpCompositeConstruct %mat2v3float %259 %262
OpStore %m23 %263
%264 = OpLoad %bool %ok
OpSelectionMerge %266 None
OpBranchConditional %264 %265 %266
%265 = OpLabel
%267 = OpLoad %mat2v3float %m23
%271 = OpCompositeExtract %v3float %267 0
%272 = OpCompositeExtract %v3float %270 0
%273 = OpFOrdEqual %v3bool %271 %272
%274 = OpAll %bool %273
%275 = OpCompositeExtract %v3float %267 1
%276 = OpCompositeExtract %v3float %270 1
%277 = OpFOrdEqual %v3bool %275 %276
%278 = OpAll %bool %277
%279 = OpLogicalAnd %bool %274 %278
OpBranch %266
%266 = OpLabel
%280 = OpPhi %bool %false %231 %279 %265
OpStore %ok %280
%281 = OpLoad %mat3v2float %m32
%284 = OpCompositeExtract %v2float %281 0
%285 = OpCompositeExtract %v2float %283 0
%286 = OpFSub %v2float %284 %285
%287 = OpCompositeExtract %v2float %281 1
%288 = OpCompositeExtract %v2float %283 1
%289 = OpFSub %v2float %287 %288
%290 = OpCompositeExtract %v2float %281 2
%291 = OpCompositeExtract %v2float %283 2
%292 = OpFSub %v2float %290 %291
%293 = OpCompositeConstruct %mat3v2float %286 %289 %292
OpStore %m32 %293
%294 = OpLoad %bool %ok
OpSelectionMerge %296 None
OpBranchConditional %294 %295 %296
%295 = OpLabel
%297 = OpLoad %mat3v2float %m32
%303 = OpCompositeExtract %v2float %297 0
%304 = OpCompositeExtract %v2float %302 0
%305 = OpFOrdEqual %v2bool %303 %304
%306 = OpAll %bool %305
%307 = OpCompositeExtract %v2float %297 1
%308 = OpCompositeExtract %v2float %302 1
%309 = OpFOrdEqual %v2bool %307 %308
%310 = OpAll %bool %309
%311 = OpLogicalAnd %bool %306 %310
%312 = OpCompositeExtract %v2float %297 2
%313 = OpCompositeExtract %v2float %302 2
%314 = OpFOrdEqual %v2bool %312 %313
%315 = OpAll %bool %314
%316 = OpLogicalAnd %bool %311 %315
OpBranch %296
%296 = OpLabel
%317 = OpPhi %bool %false %266 %316 %295
OpStore %ok %317
%318 = OpLoad %mat2v4float %m24
%321 = OpCompositeExtract %v4float %318 0
%322 = OpCompositeExtract %v4float %320 0
%323 = OpFDiv %v4float %321 %322
%324 = OpCompositeExtract %v4float %318 1
%325 = OpCompositeExtract %v4float %320 1
%326 = OpFDiv %v4float %324 %325
%327 = OpCompositeConstruct %mat2v4float %323 %326
OpStore %m24 %327
%328 = OpLoad %bool %ok
OpSelectionMerge %330 None
OpBranchConditional %328 %329 %330
%329 = OpLabel
%331 = OpLoad %mat2v4float %m24
%336 = OpCompositeExtract %v4float %331 0
%337 = OpCompositeExtract %v4float %335 0
%338 = OpFOrdEqual %v4bool %336 %337
%339 = OpAll %bool %338
%340 = OpCompositeExtract %v4float %331 1
%341 = OpCompositeExtract %v4float %335 1
%342 = OpFOrdEqual %v4bool %340 %341
%343 = OpAll %bool %342
%344 = OpLogicalAnd %bool %339 %343
OpBranch %330
%330 = OpLabel
%345 = OpPhi %bool %false %296 %344 %329
OpStore %ok %345
%346 = OpLoad %bool %ok
OpReturnValue %346
OpFunctionEnd
%main = OpFunction %v4float None %347
%348 = OpFunctionParameter %_ptr_Function_v2float
%349 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%599 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%352 = OpCompositeConstruct %mat2v3float %35 %36
OpStore %_1_m23 %352
%353 = OpLoad %bool %_0_ok
OpSelectionMerge %355 None
OpBranchConditional %353 %354 %355
%354 = OpLabel
%356 = OpLoad %mat2v3float %_1_m23
%357 = OpCompositeExtract %v3float %356 0
%358 = OpCompositeExtract %v3float %42 0
%359 = OpFOrdEqual %v3bool %357 %358
%360 = OpAll %bool %359
%361 = OpCompositeExtract %v3float %356 1
%362 = OpCompositeExtract %v3float %42 1
%363 = OpFOrdEqual %v3bool %361 %362
%364 = OpAll %bool %363
%365 = OpLogicalAnd %bool %360 %364
OpBranch %355
%355 = OpLabel
%366 = OpPhi %bool %false %349 %365 %354
OpStore %_0_ok %366
%368 = OpCompositeConstruct %mat2v4float %59 %60
OpStore %_2_m24 %368
%369 = OpLoad %bool %_0_ok
OpSelectionMerge %371 None
OpBranchConditional %369 %370 %371
%370 = OpLabel
%372 = OpLoad %mat2v4float %_2_m24
%373 = OpCompositeExtract %v4float %372 0
%374 = OpCompositeExtract %v4float %65 0
%375 = OpFOrdEqual %v4bool %373 %374
%376 = OpAll %bool %375
%377 = OpCompositeExtract %v4float %372 1
%378 = OpCompositeExtract %v4float %65 1
%379 = OpFOrdEqual %v4bool %377 %378
%380 = OpAll %bool %379
%381 = OpLogicalAnd %bool %376 %380
OpBranch %371
%371 = OpLabel
%382 = OpPhi %bool %false %355 %381 %370
OpStore %_0_ok %382
%384 = OpCompositeConstruct %mat3v2float %82 %83 %20
OpStore %_3_m32 %384
%385 = OpLoad %bool %_0_ok
OpSelectionMerge %387 None
OpBranchConditional %385 %386 %387
%386 = OpLabel
%388 = OpLoad %mat3v2float %_3_m32
%389 = OpCompositeExtract %v2float %388 0
%390 = OpCompositeExtract %v2float %88 0
%391 = OpFOrdEqual %v2bool %389 %390
%392 = OpAll %bool %391
%393 = OpCompositeExtract %v2float %388 1
%394 = OpCompositeExtract %v2float %88 1
%395 = OpFOrdEqual %v2bool %393 %394
%396 = OpAll %bool %395
%397 = OpLogicalAnd %bool %392 %396
%398 = OpCompositeExtract %v2float %388 2
%399 = OpCompositeExtract %v2float %88 2
%400 = OpFOrdEqual %v2bool %398 %399
%401 = OpAll %bool %400
%402 = OpLogicalAnd %bool %397 %401
OpBranch %387
%387 = OpLabel
%403 = OpPhi %bool %false %371 %402 %386
OpStore %_0_ok %403
%405 = OpCompositeConstruct %mat3v4float %110 %111 %112
OpStore %_4_m34 %405
%406 = OpLoad %bool %_0_ok
OpSelectionMerge %408 None
OpBranchConditional %406 %407 %408
%407 = OpLabel
%409 = OpLoad %mat3v4float %_4_m34
%410 = OpCompositeExtract %v4float %409 0
%411 = OpCompositeExtract %v4float %117 0
%412 = OpFOrdEqual %v4bool %410 %411
%413 = OpAll %bool %412
%414 = OpCompositeExtract %v4float %409 1
%415 = OpCompositeExtract %v4float %117 1
%416 = OpFOrdEqual %v4bool %414 %415
%417 = OpAll %bool %416
%418 = OpLogicalAnd %bool %413 %417
%419 = OpCompositeExtract %v4float %409 2
%420 = OpCompositeExtract %v4float %117 2
%421 = OpFOrdEqual %v4bool %419 %420
%422 = OpAll %bool %421
%423 = OpLogicalAnd %bool %418 %422
OpBranch %408
%408 = OpLabel
%424 = OpPhi %bool %false %387 %423 %407
OpStore %_0_ok %424
%426 = OpCompositeConstruct %mat4v2float %138 %139 %20 %20
OpStore %_5_m42 %426
%427 = OpLoad %bool %_0_ok
OpSelectionMerge %429 None
OpBranchConditional %427 %428 %429
%428 = OpLabel
%430 = OpLoad %mat4v2float %_5_m42
%431 = OpCompositeExtract %v2float %430 0
%432 = OpCompositeExtract %v2float %144 0
%433 = OpFOrdEqual %v2bool %431 %432
%434 = OpAll %bool %433
%435 = OpCompositeExtract %v2float %430 1
%436 = OpCompositeExtract %v2float %144 1
%437 = OpFOrdEqual %v2bool %435 %436
%438 = OpAll %bool %437
%439 = OpLogicalAnd %bool %434 %438
%440 = OpCompositeExtract %v2float %430 2
%441 = OpCompositeExtract %v2float %144 2
%442 = OpFOrdEqual %v2bool %440 %441
%443 = OpAll %bool %442
%444 = OpLogicalAnd %bool %439 %443
%445 = OpCompositeExtract %v2float %430 3
%446 = OpCompositeExtract %v2float %144 3
%447 = OpFOrdEqual %v2bool %445 %446
%448 = OpAll %bool %447
%449 = OpLogicalAnd %bool %444 %448
OpBranch %429
%429 = OpLabel
%450 = OpPhi %bool %false %408 %449 %428
OpStore %_0_ok %450
%452 = OpCompositeConstruct %mat4v3float %170 %171 %172 %173
OpStore %_6_m43 %452
%453 = OpLoad %bool %_0_ok
OpSelectionMerge %455 None
OpBranchConditional %453 %454 %455
%454 = OpLabel
%456 = OpLoad %mat4v3float %_6_m43
%457 = OpCompositeExtract %v3float %456 0
%458 = OpCompositeExtract %v3float %178 0
%459 = OpFOrdEqual %v3bool %457 %458
%460 = OpAll %bool %459
%461 = OpCompositeExtract %v3float %456 1
%462 = OpCompositeExtract %v3float %178 1
%463 = OpFOrdEqual %v3bool %461 %462
%464 = OpAll %bool %463
%465 = OpLogicalAnd %bool %460 %464
%466 = OpCompositeExtract %v3float %456 2
%467 = OpCompositeExtract %v3float %178 2
%468 = OpFOrdEqual %v3bool %466 %467
%469 = OpAll %bool %468
%470 = OpLogicalAnd %bool %465 %469
%471 = OpCompositeExtract %v3float %456 3
%472 = OpCompositeExtract %v3float %178 3
%473 = OpFOrdEqual %v3bool %471 %472
%474 = OpAll %bool %473
%475 = OpLogicalAnd %bool %470 %474
OpBranch %455
%455 = OpLabel
%476 = OpPhi %bool %false %429 %475 %454
OpStore %_0_ok %476
%478 = OpLoad %mat3v2float %_3_m32
%479 = OpLoad %mat2v3float %_1_m23
%480 = OpMatrixTimesMatrix %mat2v2float %478 %479
OpStore %_7_m22 %480
%481 = OpLoad %bool %_0_ok
OpSelectionMerge %483 None
OpBranchConditional %481 %482 %483
%482 = OpLabel
%484 = OpLoad %mat2v2float %_7_m22
%485 = OpCompositeConstruct %mat2v2float %211 %212
%486 = OpCompositeExtract %v2float %484 0
%487 = OpCompositeExtract %v2float %485 0
%488 = OpFOrdEqual %v2bool %486 %487
%489 = OpAll %bool %488
%490 = OpCompositeExtract %v2float %484 1
%491 = OpCompositeExtract %v2float %485 1
%492 = OpFOrdEqual %v2bool %490 %491
%493 = OpAll %bool %492
%494 = OpLogicalAnd %bool %489 %493
OpBranch %483
%483 = OpLabel
%495 = OpPhi %bool %false %455 %494 %482
OpStore %_0_ok %495
%497 = OpLoad %mat4v3float %_6_m43
%498 = OpLoad %mat3v4float %_4_m34
%499 = OpMatrixTimesMatrix %mat3v3float %497 %498
OpStore %_8_m33 %499
%500 = OpLoad %bool %_0_ok
OpSelectionMerge %502 None
OpBranchConditional %500 %501 %502
%501 = OpLabel
%503 = OpLoad %mat3v3float %_8_m33
%504 = OpCompositeConstruct %mat3v3float %235 %236 %237
%505 = OpCompositeExtract %v3float %503 0
%506 = OpCompositeExtract %v3float %504 0
%507 = OpFOrdEqual %v3bool %505 %506
%508 = OpAll %bool %507
%509 = OpCompositeExtract %v3float %503 1
%510 = OpCompositeExtract %v3float %504 1
%511 = OpFOrdEqual %v3bool %509 %510
%512 = OpAll %bool %511
%513 = OpLogicalAnd %bool %508 %512
%514 = OpCompositeExtract %v3float %503 2
%515 = OpCompositeExtract %v3float %504 2
%516 = OpFOrdEqual %v3bool %514 %515
%517 = OpAll %bool %516
%518 = OpLogicalAnd %bool %513 %517
OpBranch %502
%502 = OpLabel
%519 = OpPhi %bool %false %483 %518 %501
OpStore %_0_ok %519
%520 = OpLoad %mat2v3float %_1_m23
%521 = OpCompositeExtract %v3float %520 0
%522 = OpCompositeExtract %v3float %256 0
%523 = OpFAdd %v3float %521 %522
%524 = OpCompositeExtract %v3float %520 1
%525 = OpCompositeExtract %v3float %256 1
%526 = OpFAdd %v3float %524 %525
%527 = OpCompositeConstruct %mat2v3float %523 %526
OpStore %_1_m23 %527
%528 = OpLoad %bool %_0_ok
OpSelectionMerge %530 None
OpBranchConditional %528 %529 %530
%529 = OpLabel
%531 = OpLoad %mat2v3float %_1_m23
%532 = OpCompositeExtract %v3float %531 0
%533 = OpCompositeExtract %v3float %270 0
%534 = OpFOrdEqual %v3bool %532 %533
%535 = OpAll %bool %534
%536 = OpCompositeExtract %v3float %531 1
%537 = OpCompositeExtract %v3float %270 1
%538 = OpFOrdEqual %v3bool %536 %537
%539 = OpAll %bool %538
%540 = OpLogicalAnd %bool %535 %539
OpBranch %530
%530 = OpLabel
%541 = OpPhi %bool %false %502 %540 %529
OpStore %_0_ok %541
%542 = OpLoad %mat3v2float %_3_m32
%543 = OpCompositeExtract %v2float %542 0
%544 = OpCompositeExtract %v2float %283 0
%545 = OpFSub %v2float %543 %544
%546 = OpCompositeExtract %v2float %542 1
%547 = OpCompositeExtract %v2float %283 1
%548 = OpFSub %v2float %546 %547
%549 = OpCompositeExtract %v2float %542 2
%550 = OpCompositeExtract %v2float %283 2
%551 = OpFSub %v2float %549 %550
%552 = OpCompositeConstruct %mat3v2float %545 %548 %551
OpStore %_3_m32 %552
%553 = OpLoad %bool %_0_ok
OpSelectionMerge %555 None
OpBranchConditional %553 %554 %555
%554 = OpLabel
%556 = OpLoad %mat3v2float %_3_m32
%557 = OpCompositeExtract %v2float %556 0
%558 = OpCompositeExtract %v2float %302 0
%559 = OpFOrdEqual %v2bool %557 %558
%560 = OpAll %bool %559
%561 = OpCompositeExtract %v2float %556 1
%562 = OpCompositeExtract %v2float %302 1
%563 = OpFOrdEqual %v2bool %561 %562
%564 = OpAll %bool %563
%565 = OpLogicalAnd %bool %560 %564
%566 = OpCompositeExtract %v2float %556 2
%567 = OpCompositeExtract %v2float %302 2
%568 = OpFOrdEqual %v2bool %566 %567
%569 = OpAll %bool %568
%570 = OpLogicalAnd %bool %565 %569
OpBranch %555
%555 = OpLabel
%571 = OpPhi %bool %false %530 %570 %554
OpStore %_0_ok %571
%572 = OpLoad %mat2v4float %_2_m24
%573 = OpCompositeExtract %v4float %572 0
%574 = OpCompositeExtract %v4float %320 0
%575 = OpFDiv %v4float %573 %574
%576 = OpCompositeExtract %v4float %572 1
%577 = OpCompositeExtract %v4float %320 1
%578 = OpFDiv %v4float %576 %577
%579 = OpCompositeConstruct %mat2v4float %575 %578
OpStore %_2_m24 %579
%580 = OpLoad %bool %_0_ok
OpSelectionMerge %582 None
OpBranchConditional %580 %581 %582
%581 = OpLabel
%583 = OpLoad %mat2v4float %_2_m24
%584 = OpCompositeExtract %v4float %583 0
%585 = OpCompositeExtract %v4float %335 0
%586 = OpFOrdEqual %v4bool %584 %585
%587 = OpAll %bool %586
%588 = OpCompositeExtract %v4float %583 1
%589 = OpCompositeExtract %v4float %335 1
%590 = OpFOrdEqual %v4bool %588 %589
%591 = OpAll %bool %590
%592 = OpLogicalAnd %bool %587 %591
OpBranch %582
%582 = OpLabel
%593 = OpPhi %bool %false %555 %592 %581
OpStore %_0_ok %593
%594 = OpLoad %bool %_0_ok
OpSelectionMerge %596 None
OpBranchConditional %594 %595 %596
%595 = OpLabel
%597 = OpFunctionCall %bool %test_half_b
OpBranch %596
%596 = OpLabel
%598 = OpPhi %bool %false %582 %597 %595
OpSelectionMerge %603 None
OpBranchConditional %598 %601 %602
%601 = OpLabel
%604 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%608 = OpLoad %v4float %604
OpStore %599 %608
OpBranch %603
%602 = OpLabel
%609 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%611 = OpLoad %v4float %609
OpStore %599 %611
OpBranch %603
%603 = OpLabel
%612 = OpLoad %v4float %599
OpReturnValue %612
OpFunctionEnd
