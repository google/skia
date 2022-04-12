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
OpDecorate %42 RelaxedPrecision
OpDecorate %m24 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %m32 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %m34 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %m42 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
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
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
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
OpDecorate %302 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %487 RelaxedPrecision
OpDecorate %506 RelaxedPrecision
OpDecorate %535 RelaxedPrecision
OpDecorate %562 RelaxedPrecision
OpDecorate %591 RelaxedPrecision
OpDecorate %606 RelaxedPrecision
OpDecorate %620 RelaxedPrecision
OpDecorate %623 RelaxedPrecision
OpDecorate %624 RelaxedPrecision
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
%v3bool = OpTypeVector %bool 3
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_3 = OpConstant %float 3
%59 = OpConstantComposite %v4float %float_3 %float_0 %float_0 %float_0
%60 = OpConstantComposite %v4float %float_0 %float_3 %float_0 %float_0
%v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%82 = OpConstantComposite %v2float %float_4 %float_0
%83 = OpConstantComposite %v2float %float_0 %float_4
%v2bool = OpTypeVector %bool 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%110 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
%111 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
%112 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%138 = OpConstantComposite %v2float %float_6 %float_0
%139 = OpConstantComposite %v2float %float_0 %float_6
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%170 = OpConstantComposite %v3float %float_7 %float_0 %float_0
%171 = OpConstantComposite %v3float %float_0 %float_7 %float_0
%172 = OpConstantComposite %v3float %float_0 %float_0 %float_7
%173 = OpConstantComposite %v3float %float_0 %float_0 %float_0
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
%268 = OpConstantComposite %v3float %float_3 %float_1 %float_1
%269 = OpConstantComposite %v3float %float_1 %float_3 %float_1
%282 = OpConstantComposite %v2float %float_2 %float_2
%float_n2 = OpConstant %float -2
%299 = OpConstantComposite %v2float %float_2 %float_n2
%300 = OpConstantComposite %v2float %float_n2 %float_2
%301 = OpConstantComposite %v2float %float_n2 %float_n2
%319 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%float_0_75 = OpConstant %float 0.75
%333 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0 %float_0
%334 = OpConstantComposite %v4float %float_0 %float_0_75 %float_0 %float_0
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
%42 = OpCompositeConstruct %mat2v3float %35 %36
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
%65 = OpCompositeConstruct %mat2v4float %59 %60
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
%88 = OpCompositeConstruct %mat3v2float %82 %83 %20
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
%117 = OpCompositeConstruct %mat3v4float %110 %111 %112
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
%144 = OpCompositeConstruct %mat4v2float %138 %139 %20 %20
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
%178 = OpCompositeConstruct %mat4v3float %170 %171 %172 %173
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
%256 = OpCompositeConstruct %mat2v3float %255 %255
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
%270 = OpCompositeConstruct %mat2v3float %268 %269
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
%283 = OpCompositeConstruct %mat3v2float %282 %282 %282
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
%302 = OpCompositeConstruct %mat3v2float %299 %300 %301
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
%320 = OpCompositeConstruct %mat2v4float %319 %319
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
%335 = OpCompositeConstruct %mat2v4float %333 %334
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
%611 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%352 = OpCompositeConstruct %mat2v3float %35 %36
OpStore %_1_m23 %352
%353 = OpLoad %bool %_0_ok
OpSelectionMerge %355 None
OpBranchConditional %353 %354 %355
%354 = OpLabel
%356 = OpLoad %mat2v3float %_1_m23
%357 = OpCompositeConstruct %mat2v3float %35 %36
%358 = OpCompositeExtract %v3float %356 0
%359 = OpCompositeExtract %v3float %357 0
%360 = OpFOrdEqual %v3bool %358 %359
%361 = OpAll %bool %360
%362 = OpCompositeExtract %v3float %356 1
%363 = OpCompositeExtract %v3float %357 1
%364 = OpFOrdEqual %v3bool %362 %363
%365 = OpAll %bool %364
%366 = OpLogicalAnd %bool %361 %365
OpBranch %355
%355 = OpLabel
%367 = OpPhi %bool %false %349 %366 %354
OpStore %_0_ok %367
%369 = OpCompositeConstruct %mat2v4float %59 %60
OpStore %_2_m24 %369
%370 = OpLoad %bool %_0_ok
OpSelectionMerge %372 None
OpBranchConditional %370 %371 %372
%371 = OpLabel
%373 = OpLoad %mat2v4float %_2_m24
%374 = OpCompositeConstruct %mat2v4float %59 %60
%375 = OpCompositeExtract %v4float %373 0
%376 = OpCompositeExtract %v4float %374 0
%377 = OpFOrdEqual %v4bool %375 %376
%378 = OpAll %bool %377
%379 = OpCompositeExtract %v4float %373 1
%380 = OpCompositeExtract %v4float %374 1
%381 = OpFOrdEqual %v4bool %379 %380
%382 = OpAll %bool %381
%383 = OpLogicalAnd %bool %378 %382
OpBranch %372
%372 = OpLabel
%384 = OpPhi %bool %false %355 %383 %371
OpStore %_0_ok %384
%386 = OpCompositeConstruct %mat3v2float %82 %83 %20
OpStore %_3_m32 %386
%387 = OpLoad %bool %_0_ok
OpSelectionMerge %389 None
OpBranchConditional %387 %388 %389
%388 = OpLabel
%390 = OpLoad %mat3v2float %_3_m32
%391 = OpCompositeConstruct %mat3v2float %82 %83 %20
%392 = OpCompositeExtract %v2float %390 0
%393 = OpCompositeExtract %v2float %391 0
%394 = OpFOrdEqual %v2bool %392 %393
%395 = OpAll %bool %394
%396 = OpCompositeExtract %v2float %390 1
%397 = OpCompositeExtract %v2float %391 1
%398 = OpFOrdEqual %v2bool %396 %397
%399 = OpAll %bool %398
%400 = OpLogicalAnd %bool %395 %399
%401 = OpCompositeExtract %v2float %390 2
%402 = OpCompositeExtract %v2float %391 2
%403 = OpFOrdEqual %v2bool %401 %402
%404 = OpAll %bool %403
%405 = OpLogicalAnd %bool %400 %404
OpBranch %389
%389 = OpLabel
%406 = OpPhi %bool %false %372 %405 %388
OpStore %_0_ok %406
%408 = OpCompositeConstruct %mat3v4float %110 %111 %112
OpStore %_4_m34 %408
%409 = OpLoad %bool %_0_ok
OpSelectionMerge %411 None
OpBranchConditional %409 %410 %411
%410 = OpLabel
%412 = OpLoad %mat3v4float %_4_m34
%413 = OpCompositeConstruct %mat3v4float %110 %111 %112
%414 = OpCompositeExtract %v4float %412 0
%415 = OpCompositeExtract %v4float %413 0
%416 = OpFOrdEqual %v4bool %414 %415
%417 = OpAll %bool %416
%418 = OpCompositeExtract %v4float %412 1
%419 = OpCompositeExtract %v4float %413 1
%420 = OpFOrdEqual %v4bool %418 %419
%421 = OpAll %bool %420
%422 = OpLogicalAnd %bool %417 %421
%423 = OpCompositeExtract %v4float %412 2
%424 = OpCompositeExtract %v4float %413 2
%425 = OpFOrdEqual %v4bool %423 %424
%426 = OpAll %bool %425
%427 = OpLogicalAnd %bool %422 %426
OpBranch %411
%411 = OpLabel
%428 = OpPhi %bool %false %389 %427 %410
OpStore %_0_ok %428
%430 = OpCompositeConstruct %mat4v2float %138 %139 %20 %20
OpStore %_5_m42 %430
%431 = OpLoad %bool %_0_ok
OpSelectionMerge %433 None
OpBranchConditional %431 %432 %433
%432 = OpLabel
%434 = OpLoad %mat4v2float %_5_m42
%435 = OpCompositeConstruct %mat4v2float %138 %139 %20 %20
%436 = OpCompositeExtract %v2float %434 0
%437 = OpCompositeExtract %v2float %435 0
%438 = OpFOrdEqual %v2bool %436 %437
%439 = OpAll %bool %438
%440 = OpCompositeExtract %v2float %434 1
%441 = OpCompositeExtract %v2float %435 1
%442 = OpFOrdEqual %v2bool %440 %441
%443 = OpAll %bool %442
%444 = OpLogicalAnd %bool %439 %443
%445 = OpCompositeExtract %v2float %434 2
%446 = OpCompositeExtract %v2float %435 2
%447 = OpFOrdEqual %v2bool %445 %446
%448 = OpAll %bool %447
%449 = OpLogicalAnd %bool %444 %448
%450 = OpCompositeExtract %v2float %434 3
%451 = OpCompositeExtract %v2float %435 3
%452 = OpFOrdEqual %v2bool %450 %451
%453 = OpAll %bool %452
%454 = OpLogicalAnd %bool %449 %453
OpBranch %433
%433 = OpLabel
%455 = OpPhi %bool %false %411 %454 %432
OpStore %_0_ok %455
%457 = OpCompositeConstruct %mat4v3float %170 %171 %172 %173
OpStore %_6_m43 %457
%458 = OpLoad %bool %_0_ok
OpSelectionMerge %460 None
OpBranchConditional %458 %459 %460
%459 = OpLabel
%461 = OpLoad %mat4v3float %_6_m43
%462 = OpCompositeConstruct %mat4v3float %170 %171 %172 %173
%463 = OpCompositeExtract %v3float %461 0
%464 = OpCompositeExtract %v3float %462 0
%465 = OpFOrdEqual %v3bool %463 %464
%466 = OpAll %bool %465
%467 = OpCompositeExtract %v3float %461 1
%468 = OpCompositeExtract %v3float %462 1
%469 = OpFOrdEqual %v3bool %467 %468
%470 = OpAll %bool %469
%471 = OpLogicalAnd %bool %466 %470
%472 = OpCompositeExtract %v3float %461 2
%473 = OpCompositeExtract %v3float %462 2
%474 = OpFOrdEqual %v3bool %472 %473
%475 = OpAll %bool %474
%476 = OpLogicalAnd %bool %471 %475
%477 = OpCompositeExtract %v3float %461 3
%478 = OpCompositeExtract %v3float %462 3
%479 = OpFOrdEqual %v3bool %477 %478
%480 = OpAll %bool %479
%481 = OpLogicalAnd %bool %476 %480
OpBranch %460
%460 = OpLabel
%482 = OpPhi %bool %false %433 %481 %459
OpStore %_0_ok %482
%484 = OpLoad %mat3v2float %_3_m32
%485 = OpLoad %mat2v3float %_1_m23
%486 = OpMatrixTimesMatrix %mat2v2float %484 %485
OpStore %_7_m22 %486
%487 = OpLoad %bool %_0_ok
OpSelectionMerge %489 None
OpBranchConditional %487 %488 %489
%488 = OpLabel
%490 = OpLoad %mat2v2float %_7_m22
%491 = OpCompositeConstruct %mat2v2float %211 %212
%492 = OpCompositeExtract %v2float %490 0
%493 = OpCompositeExtract %v2float %491 0
%494 = OpFOrdEqual %v2bool %492 %493
%495 = OpAll %bool %494
%496 = OpCompositeExtract %v2float %490 1
%497 = OpCompositeExtract %v2float %491 1
%498 = OpFOrdEqual %v2bool %496 %497
%499 = OpAll %bool %498
%500 = OpLogicalAnd %bool %495 %499
OpBranch %489
%489 = OpLabel
%501 = OpPhi %bool %false %460 %500 %488
OpStore %_0_ok %501
%503 = OpLoad %mat4v3float %_6_m43
%504 = OpLoad %mat3v4float %_4_m34
%505 = OpMatrixTimesMatrix %mat3v3float %503 %504
OpStore %_8_m33 %505
%506 = OpLoad %bool %_0_ok
OpSelectionMerge %508 None
OpBranchConditional %506 %507 %508
%507 = OpLabel
%509 = OpLoad %mat3v3float %_8_m33
%510 = OpCompositeConstruct %mat3v3float %235 %236 %237
%511 = OpCompositeExtract %v3float %509 0
%512 = OpCompositeExtract %v3float %510 0
%513 = OpFOrdEqual %v3bool %511 %512
%514 = OpAll %bool %513
%515 = OpCompositeExtract %v3float %509 1
%516 = OpCompositeExtract %v3float %510 1
%517 = OpFOrdEqual %v3bool %515 %516
%518 = OpAll %bool %517
%519 = OpLogicalAnd %bool %514 %518
%520 = OpCompositeExtract %v3float %509 2
%521 = OpCompositeExtract %v3float %510 2
%522 = OpFOrdEqual %v3bool %520 %521
%523 = OpAll %bool %522
%524 = OpLogicalAnd %bool %519 %523
OpBranch %508
%508 = OpLabel
%525 = OpPhi %bool %false %489 %524 %507
OpStore %_0_ok %525
%526 = OpLoad %mat2v3float %_1_m23
%527 = OpCompositeConstruct %mat2v3float %255 %255
%528 = OpCompositeExtract %v3float %526 0
%529 = OpCompositeExtract %v3float %527 0
%530 = OpFAdd %v3float %528 %529
%531 = OpCompositeExtract %v3float %526 1
%532 = OpCompositeExtract %v3float %527 1
%533 = OpFAdd %v3float %531 %532
%534 = OpCompositeConstruct %mat2v3float %530 %533
OpStore %_1_m23 %534
%535 = OpLoad %bool %_0_ok
OpSelectionMerge %537 None
OpBranchConditional %535 %536 %537
%536 = OpLabel
%538 = OpLoad %mat2v3float %_1_m23
%539 = OpCompositeConstruct %mat2v3float %268 %269
%540 = OpCompositeExtract %v3float %538 0
%541 = OpCompositeExtract %v3float %539 0
%542 = OpFOrdEqual %v3bool %540 %541
%543 = OpAll %bool %542
%544 = OpCompositeExtract %v3float %538 1
%545 = OpCompositeExtract %v3float %539 1
%546 = OpFOrdEqual %v3bool %544 %545
%547 = OpAll %bool %546
%548 = OpLogicalAnd %bool %543 %547
OpBranch %537
%537 = OpLabel
%549 = OpPhi %bool %false %508 %548 %536
OpStore %_0_ok %549
%550 = OpLoad %mat3v2float %_3_m32
%551 = OpCompositeConstruct %mat3v2float %282 %282 %282
%552 = OpCompositeExtract %v2float %550 0
%553 = OpCompositeExtract %v2float %551 0
%554 = OpFSub %v2float %552 %553
%555 = OpCompositeExtract %v2float %550 1
%556 = OpCompositeExtract %v2float %551 1
%557 = OpFSub %v2float %555 %556
%558 = OpCompositeExtract %v2float %550 2
%559 = OpCompositeExtract %v2float %551 2
%560 = OpFSub %v2float %558 %559
%561 = OpCompositeConstruct %mat3v2float %554 %557 %560
OpStore %_3_m32 %561
%562 = OpLoad %bool %_0_ok
OpSelectionMerge %564 None
OpBranchConditional %562 %563 %564
%563 = OpLabel
%565 = OpLoad %mat3v2float %_3_m32
%566 = OpCompositeConstruct %mat3v2float %299 %300 %301
%567 = OpCompositeExtract %v2float %565 0
%568 = OpCompositeExtract %v2float %566 0
%569 = OpFOrdEqual %v2bool %567 %568
%570 = OpAll %bool %569
%571 = OpCompositeExtract %v2float %565 1
%572 = OpCompositeExtract %v2float %566 1
%573 = OpFOrdEqual %v2bool %571 %572
%574 = OpAll %bool %573
%575 = OpLogicalAnd %bool %570 %574
%576 = OpCompositeExtract %v2float %565 2
%577 = OpCompositeExtract %v2float %566 2
%578 = OpFOrdEqual %v2bool %576 %577
%579 = OpAll %bool %578
%580 = OpLogicalAnd %bool %575 %579
OpBranch %564
%564 = OpLabel
%581 = OpPhi %bool %false %537 %580 %563
OpStore %_0_ok %581
%582 = OpLoad %mat2v4float %_2_m24
%583 = OpCompositeConstruct %mat2v4float %319 %319
%584 = OpCompositeExtract %v4float %582 0
%585 = OpCompositeExtract %v4float %583 0
%586 = OpFDiv %v4float %584 %585
%587 = OpCompositeExtract %v4float %582 1
%588 = OpCompositeExtract %v4float %583 1
%589 = OpFDiv %v4float %587 %588
%590 = OpCompositeConstruct %mat2v4float %586 %589
OpStore %_2_m24 %590
%591 = OpLoad %bool %_0_ok
OpSelectionMerge %593 None
OpBranchConditional %591 %592 %593
%592 = OpLabel
%594 = OpLoad %mat2v4float %_2_m24
%595 = OpCompositeConstruct %mat2v4float %333 %334
%596 = OpCompositeExtract %v4float %594 0
%597 = OpCompositeExtract %v4float %595 0
%598 = OpFOrdEqual %v4bool %596 %597
%599 = OpAll %bool %598
%600 = OpCompositeExtract %v4float %594 1
%601 = OpCompositeExtract %v4float %595 1
%602 = OpFOrdEqual %v4bool %600 %601
%603 = OpAll %bool %602
%604 = OpLogicalAnd %bool %599 %603
OpBranch %593
%593 = OpLabel
%605 = OpPhi %bool %false %564 %604 %592
OpStore %_0_ok %605
%606 = OpLoad %bool %_0_ok
OpSelectionMerge %608 None
OpBranchConditional %606 %607 %608
%607 = OpLabel
%609 = OpFunctionCall %bool %test_half_b
OpBranch %608
%608 = OpLabel
%610 = OpPhi %bool %false %593 %609 %607
OpSelectionMerge %615 None
OpBranchConditional %610 %613 %614
%613 = OpLabel
%616 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%620 = OpLoad %v4float %616
OpStore %611 %620
OpBranch %615
%614 = OpLabel
%621 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%623 = OpLoad %v4float %621
OpStore %611 %623
OpBranch %615
%615 = OpLabel
%624 = OpLoad %v4float %611
OpReturnValue %624
OpFunctionEnd
