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
OpDecorate %56 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %m32 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %m34 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %m42 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %m22 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %476 RelaxedPrecision
OpDecorate %496 RelaxedPrecision
OpDecorate %518 RelaxedPrecision
OpDecorate %530 RelaxedPrecision
OpDecorate %544 RelaxedPrecision
OpDecorate %547 RelaxedPrecision
OpDecorate %548 RelaxedPrecision
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
%57 = OpConstantComposite %v4float %float_3 %float_0 %float_0 %float_0
%58 = OpConstantComposite %v4float %float_0 %float_3 %float_0 %float_0
%63 = OpConstantComposite %mat2v4float %57 %58
%v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%78 = OpConstantComposite %v2float %float_4 %float_0
%79 = OpConstantComposite %v2float %float_0 %float_4
%84 = OpConstantComposite %mat3v2float %78 %79 %20
%v2bool = OpTypeVector %bool 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%103 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
%104 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
%105 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
%110 = OpConstantComposite %mat3v4float %103 %104 %105
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%128 = OpConstantComposite %v2float %float_6 %float_0
%129 = OpConstantComposite %v2float %float_0 %float_6
%134 = OpConstantComposite %mat4v2float %128 %129 %20 %20
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%156 = OpConstantComposite %v3float %float_7 %float_0 %float_0
%157 = OpConstantComposite %v3float %float_0 %float_7 %float_0
%158 = OpConstantComposite %v3float %float_0 %float_0 %float_7
%159 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%164 = OpConstantComposite %mat4v3float %156 %157 %158 %159
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_8 = OpConstant %float 8
%193 = OpConstantComposite %v2float %float_8 %float_0
%194 = OpConstantComposite %v2float %float_0 %float_8
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%217 = OpConstantComposite %v3float %float_35 %float_0 %float_0
%218 = OpConstantComposite %v3float %float_0 %float_35 %float_0
%219 = OpConstantComposite %v3float %float_0 %float_0 %float_35
%float_1 = OpConstant %float 1
%237 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%238 = OpConstantComposite %mat2v3float %237 %237
%248 = OpConstantComposite %v3float %float_3 %float_1 %float_1
%249 = OpConstantComposite %v3float %float_1 %float_3 %float_1
%250 = OpConstantComposite %mat2v3float %248 %249
%260 = OpConstantComposite %v2float %float_2 %float_2
%261 = OpConstantComposite %mat3v2float %260 %260 %260
%float_n2 = OpConstant %float -2
%274 = OpConstantComposite %v2float %float_2 %float_n2
%275 = OpConstantComposite %v2float %float_n2 %float_2
%276 = OpConstantComposite %v2float %float_n2 %float_n2
%277 = OpConstantComposite %mat3v2float %274 %275 %276
%291 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%292 = OpConstantComposite %mat2v4float %291 %291
%float_0_75 = OpConstant %float 0.75
%303 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0 %float_0
%304 = OpConstantComposite %v4float %float_0 %float_0_75 %float_0 %float_0
%305 = OpConstantComposite %mat2v4float %303 %304
%315 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%45 = OpFOrdEqual %v3bool %44 %35
%46 = OpAll %bool %45
%47 = OpCompositeExtract %v3float %41 1
%48 = OpFOrdEqual %v3bool %47 %36
%49 = OpAll %bool %48
%50 = OpLogicalAnd %bool %46 %49
OpBranch %40
%40 = OpLabel
%51 = OpPhi %bool %false %25 %50 %39
OpStore %ok %51
%56 = OpCompositeConstruct %mat2v4float %57 %58
OpStore %m24 %56
%59 = OpLoad %bool %ok
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%62 = OpLoad %mat2v4float %m24
%65 = OpCompositeExtract %v4float %62 0
%66 = OpFOrdEqual %v4bool %65 %57
%67 = OpAll %bool %66
%68 = OpCompositeExtract %v4float %62 1
%69 = OpFOrdEqual %v4bool %68 %58
%70 = OpAll %bool %69
%71 = OpLogicalAnd %bool %67 %70
OpBranch %61
%61 = OpLabel
%72 = OpPhi %bool %false %40 %71 %60
OpStore %ok %72
%77 = OpCompositeConstruct %mat3v2float %78 %79 %20
OpStore %m32 %77
%80 = OpLoad %bool %ok
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%83 = OpLoad %mat3v2float %m32
%86 = OpCompositeExtract %v2float %83 0
%87 = OpFOrdEqual %v2bool %86 %78
%88 = OpAll %bool %87
%89 = OpCompositeExtract %v2float %83 1
%90 = OpFOrdEqual %v2bool %89 %79
%91 = OpAll %bool %90
%92 = OpLogicalAnd %bool %88 %91
%93 = OpCompositeExtract %v2float %83 2
%94 = OpFOrdEqual %v2bool %93 %20
%95 = OpAll %bool %94
%96 = OpLogicalAnd %bool %92 %95
OpBranch %82
%82 = OpLabel
%97 = OpPhi %bool %false %61 %96 %81
OpStore %ok %97
%102 = OpCompositeConstruct %mat3v4float %103 %104 %105
OpStore %m34 %102
%106 = OpLoad %bool %ok
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%109 = OpLoad %mat3v4float %m34
%111 = OpCompositeExtract %v4float %109 0
%112 = OpFOrdEqual %v4bool %111 %103
%113 = OpAll %bool %112
%114 = OpCompositeExtract %v4float %109 1
%115 = OpFOrdEqual %v4bool %114 %104
%116 = OpAll %bool %115
%117 = OpLogicalAnd %bool %113 %116
%118 = OpCompositeExtract %v4float %109 2
%119 = OpFOrdEqual %v4bool %118 %105
%120 = OpAll %bool %119
%121 = OpLogicalAnd %bool %117 %120
OpBranch %108
%108 = OpLabel
%122 = OpPhi %bool %false %82 %121 %107
OpStore %ok %122
%127 = OpCompositeConstruct %mat4v2float %128 %129 %20 %20
OpStore %m42 %127
%130 = OpLoad %bool %ok
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%133 = OpLoad %mat4v2float %m42
%135 = OpCompositeExtract %v2float %133 0
%136 = OpFOrdEqual %v2bool %135 %128
%137 = OpAll %bool %136
%138 = OpCompositeExtract %v2float %133 1
%139 = OpFOrdEqual %v2bool %138 %129
%140 = OpAll %bool %139
%141 = OpLogicalAnd %bool %137 %140
%142 = OpCompositeExtract %v2float %133 2
%143 = OpFOrdEqual %v2bool %142 %20
%144 = OpAll %bool %143
%145 = OpLogicalAnd %bool %141 %144
%146 = OpCompositeExtract %v2float %133 3
%147 = OpFOrdEqual %v2bool %146 %20
%148 = OpAll %bool %147
%149 = OpLogicalAnd %bool %145 %148
OpBranch %132
%132 = OpLabel
%150 = OpPhi %bool %false %108 %149 %131
OpStore %ok %150
%155 = OpCompositeConstruct %mat4v3float %156 %157 %158 %159
OpStore %m43 %155
%160 = OpLoad %bool %ok
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%163 = OpLoad %mat4v3float %m43
%165 = OpCompositeExtract %v3float %163 0
%166 = OpFOrdEqual %v3bool %165 %156
%167 = OpAll %bool %166
%168 = OpCompositeExtract %v3float %163 1
%169 = OpFOrdEqual %v3bool %168 %157
%170 = OpAll %bool %169
%171 = OpLogicalAnd %bool %167 %170
%172 = OpCompositeExtract %v3float %163 2
%173 = OpFOrdEqual %v3bool %172 %158
%174 = OpAll %bool %173
%175 = OpLogicalAnd %bool %171 %174
%176 = OpCompositeExtract %v3float %163 3
%177 = OpFOrdEqual %v3bool %176 %159
%178 = OpAll %bool %177
%179 = OpLogicalAnd %bool %175 %178
OpBranch %162
%162 = OpLabel
%180 = OpPhi %bool %false %132 %179 %161
OpStore %ok %180
%184 = OpLoad %mat3v2float %m32
%185 = OpLoad %mat2v3float %m23
%186 = OpMatrixTimesMatrix %mat2v2float %184 %185
OpStore %m22 %186
%187 = OpLoad %bool %ok
OpSelectionMerge %189 None
OpBranchConditional %187 %188 %189
%188 = OpLabel
%190 = OpLoad %mat2v2float %m22
%192 = OpCompositeConstruct %mat2v2float %193 %194
%195 = OpCompositeExtract %v2float %190 0
%196 = OpCompositeExtract %v2float %192 0
%197 = OpFOrdEqual %v2bool %195 %196
%198 = OpAll %bool %197
%199 = OpCompositeExtract %v2float %190 1
%200 = OpCompositeExtract %v2float %192 1
%201 = OpFOrdEqual %v2bool %199 %200
%202 = OpAll %bool %201
%203 = OpLogicalAnd %bool %198 %202
OpBranch %189
%189 = OpLabel
%204 = OpPhi %bool %false %162 %203 %188
OpStore %ok %204
%208 = OpLoad %mat4v3float %m43
%209 = OpLoad %mat3v4float %m34
%210 = OpMatrixTimesMatrix %mat3v3float %208 %209
OpStore %m33 %210
%211 = OpLoad %bool %ok
OpSelectionMerge %213 None
OpBranchConditional %211 %212 %213
%212 = OpLabel
%214 = OpLoad %mat3v3float %m33
%216 = OpCompositeConstruct %mat3v3float %217 %218 %219
%220 = OpCompositeExtract %v3float %214 0
%221 = OpCompositeExtract %v3float %216 0
%222 = OpFOrdEqual %v3bool %220 %221
%223 = OpAll %bool %222
%224 = OpCompositeExtract %v3float %214 1
%225 = OpCompositeExtract %v3float %216 1
%226 = OpFOrdEqual %v3bool %224 %225
%227 = OpAll %bool %226
%228 = OpLogicalAnd %bool %223 %227
%229 = OpCompositeExtract %v3float %214 2
%230 = OpCompositeExtract %v3float %216 2
%231 = OpFOrdEqual %v3bool %229 %230
%232 = OpAll %bool %231
%233 = OpLogicalAnd %bool %228 %232
OpBranch %213
%213 = OpLabel
%234 = OpPhi %bool %false %189 %233 %212
OpStore %ok %234
%235 = OpLoad %mat2v3float %m23
%239 = OpCompositeExtract %v3float %235 0
%240 = OpFAdd %v3float %239 %237
%241 = OpCompositeExtract %v3float %235 1
%242 = OpFAdd %v3float %241 %237
%243 = OpCompositeConstruct %mat2v3float %240 %242
OpStore %m23 %243
%244 = OpLoad %bool %ok
OpSelectionMerge %246 None
OpBranchConditional %244 %245 %246
%245 = OpLabel
%247 = OpLoad %mat2v3float %m23
%251 = OpCompositeExtract %v3float %247 0
%252 = OpFOrdEqual %v3bool %251 %248
%253 = OpAll %bool %252
%254 = OpCompositeExtract %v3float %247 1
%255 = OpFOrdEqual %v3bool %254 %249
%256 = OpAll %bool %255
%257 = OpLogicalAnd %bool %253 %256
OpBranch %246
%246 = OpLabel
%258 = OpPhi %bool %false %213 %257 %245
OpStore %ok %258
%259 = OpLoad %mat3v2float %m32
%262 = OpCompositeExtract %v2float %259 0
%263 = OpFSub %v2float %262 %260
%264 = OpCompositeExtract %v2float %259 1
%265 = OpFSub %v2float %264 %260
%266 = OpCompositeExtract %v2float %259 2
%267 = OpFSub %v2float %266 %260
%268 = OpCompositeConstruct %mat3v2float %263 %265 %267
OpStore %m32 %268
%269 = OpLoad %bool %ok
OpSelectionMerge %271 None
OpBranchConditional %269 %270 %271
%270 = OpLabel
%272 = OpLoad %mat3v2float %m32
%278 = OpCompositeExtract %v2float %272 0
%279 = OpFOrdEqual %v2bool %278 %274
%280 = OpAll %bool %279
%281 = OpCompositeExtract %v2float %272 1
%282 = OpFOrdEqual %v2bool %281 %275
%283 = OpAll %bool %282
%284 = OpLogicalAnd %bool %280 %283
%285 = OpCompositeExtract %v2float %272 2
%286 = OpFOrdEqual %v2bool %285 %276
%287 = OpAll %bool %286
%288 = OpLogicalAnd %bool %284 %287
OpBranch %271
%271 = OpLabel
%289 = OpPhi %bool %false %246 %288 %270
OpStore %ok %289
%290 = OpLoad %mat2v4float %m24
%293 = OpCompositeExtract %v4float %290 0
%294 = OpFDiv %v4float %293 %291
%295 = OpCompositeExtract %v4float %290 1
%296 = OpFDiv %v4float %295 %291
%297 = OpCompositeConstruct %mat2v4float %294 %296
OpStore %m24 %297
%298 = OpLoad %bool %ok
OpSelectionMerge %300 None
OpBranchConditional %298 %299 %300
%299 = OpLabel
%301 = OpLoad %mat2v4float %m24
%306 = OpCompositeExtract %v4float %301 0
%307 = OpFOrdEqual %v4bool %306 %303
%308 = OpAll %bool %307
%309 = OpCompositeExtract %v4float %301 1
%310 = OpFOrdEqual %v4bool %309 %304
%311 = OpAll %bool %310
%312 = OpLogicalAnd %bool %308 %311
OpBranch %300
%300 = OpLabel
%313 = OpPhi %bool %false %271 %312 %299
OpStore %ok %313
%314 = OpLoad %bool %ok
OpReturnValue %314
OpFunctionEnd
%main = OpFunction %v4float None %315
%316 = OpFunctionParameter %_ptr_Function_v2float
%317 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%535 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%320 = OpCompositeConstruct %mat2v3float %35 %36
OpStore %_1_m23 %320
%321 = OpLoad %bool %_0_ok
OpSelectionMerge %323 None
OpBranchConditional %321 %322 %323
%322 = OpLabel
%324 = OpLoad %mat2v3float %_1_m23
%325 = OpCompositeExtract %v3float %324 0
%326 = OpFOrdEqual %v3bool %325 %35
%327 = OpAll %bool %326
%328 = OpCompositeExtract %v3float %324 1
%329 = OpFOrdEqual %v3bool %328 %36
%330 = OpAll %bool %329
%331 = OpLogicalAnd %bool %327 %330
OpBranch %323
%323 = OpLabel
%332 = OpPhi %bool %false %317 %331 %322
OpStore %_0_ok %332
%334 = OpCompositeConstruct %mat2v4float %57 %58
OpStore %_2_m24 %334
%335 = OpLoad %bool %_0_ok
OpSelectionMerge %337 None
OpBranchConditional %335 %336 %337
%336 = OpLabel
%338 = OpLoad %mat2v4float %_2_m24
%339 = OpCompositeExtract %v4float %338 0
%340 = OpFOrdEqual %v4bool %339 %57
%341 = OpAll %bool %340
%342 = OpCompositeExtract %v4float %338 1
%343 = OpFOrdEqual %v4bool %342 %58
%344 = OpAll %bool %343
%345 = OpLogicalAnd %bool %341 %344
OpBranch %337
%337 = OpLabel
%346 = OpPhi %bool %false %323 %345 %336
OpStore %_0_ok %346
%348 = OpCompositeConstruct %mat3v2float %78 %79 %20
OpStore %_3_m32 %348
%349 = OpLoad %bool %_0_ok
OpSelectionMerge %351 None
OpBranchConditional %349 %350 %351
%350 = OpLabel
%352 = OpLoad %mat3v2float %_3_m32
%353 = OpCompositeExtract %v2float %352 0
%354 = OpFOrdEqual %v2bool %353 %78
%355 = OpAll %bool %354
%356 = OpCompositeExtract %v2float %352 1
%357 = OpFOrdEqual %v2bool %356 %79
%358 = OpAll %bool %357
%359 = OpLogicalAnd %bool %355 %358
%360 = OpCompositeExtract %v2float %352 2
%361 = OpFOrdEqual %v2bool %360 %20
%362 = OpAll %bool %361
%363 = OpLogicalAnd %bool %359 %362
OpBranch %351
%351 = OpLabel
%364 = OpPhi %bool %false %337 %363 %350
OpStore %_0_ok %364
%366 = OpCompositeConstruct %mat3v4float %103 %104 %105
OpStore %_4_m34 %366
%367 = OpLoad %bool %_0_ok
OpSelectionMerge %369 None
OpBranchConditional %367 %368 %369
%368 = OpLabel
%370 = OpLoad %mat3v4float %_4_m34
%371 = OpCompositeExtract %v4float %370 0
%372 = OpFOrdEqual %v4bool %371 %103
%373 = OpAll %bool %372
%374 = OpCompositeExtract %v4float %370 1
%375 = OpFOrdEqual %v4bool %374 %104
%376 = OpAll %bool %375
%377 = OpLogicalAnd %bool %373 %376
%378 = OpCompositeExtract %v4float %370 2
%379 = OpFOrdEqual %v4bool %378 %105
%380 = OpAll %bool %379
%381 = OpLogicalAnd %bool %377 %380
OpBranch %369
%369 = OpLabel
%382 = OpPhi %bool %false %351 %381 %368
OpStore %_0_ok %382
%384 = OpCompositeConstruct %mat4v2float %128 %129 %20 %20
OpStore %_5_m42 %384
%385 = OpLoad %bool %_0_ok
OpSelectionMerge %387 None
OpBranchConditional %385 %386 %387
%386 = OpLabel
%388 = OpLoad %mat4v2float %_5_m42
%389 = OpCompositeExtract %v2float %388 0
%390 = OpFOrdEqual %v2bool %389 %128
%391 = OpAll %bool %390
%392 = OpCompositeExtract %v2float %388 1
%393 = OpFOrdEqual %v2bool %392 %129
%394 = OpAll %bool %393
%395 = OpLogicalAnd %bool %391 %394
%396 = OpCompositeExtract %v2float %388 2
%397 = OpFOrdEqual %v2bool %396 %20
%398 = OpAll %bool %397
%399 = OpLogicalAnd %bool %395 %398
%400 = OpCompositeExtract %v2float %388 3
%401 = OpFOrdEqual %v2bool %400 %20
%402 = OpAll %bool %401
%403 = OpLogicalAnd %bool %399 %402
OpBranch %387
%387 = OpLabel
%404 = OpPhi %bool %false %369 %403 %386
OpStore %_0_ok %404
%406 = OpCompositeConstruct %mat4v3float %156 %157 %158 %159
OpStore %_6_m43 %406
%407 = OpLoad %bool %_0_ok
OpSelectionMerge %409 None
OpBranchConditional %407 %408 %409
%408 = OpLabel
%410 = OpLoad %mat4v3float %_6_m43
%411 = OpCompositeExtract %v3float %410 0
%412 = OpFOrdEqual %v3bool %411 %156
%413 = OpAll %bool %412
%414 = OpCompositeExtract %v3float %410 1
%415 = OpFOrdEqual %v3bool %414 %157
%416 = OpAll %bool %415
%417 = OpLogicalAnd %bool %413 %416
%418 = OpCompositeExtract %v3float %410 2
%419 = OpFOrdEqual %v3bool %418 %158
%420 = OpAll %bool %419
%421 = OpLogicalAnd %bool %417 %420
%422 = OpCompositeExtract %v3float %410 3
%423 = OpFOrdEqual %v3bool %422 %159
%424 = OpAll %bool %423
%425 = OpLogicalAnd %bool %421 %424
OpBranch %409
%409 = OpLabel
%426 = OpPhi %bool %false %387 %425 %408
OpStore %_0_ok %426
%428 = OpLoad %mat3v2float %_3_m32
%429 = OpLoad %mat2v3float %_1_m23
%430 = OpMatrixTimesMatrix %mat2v2float %428 %429
OpStore %_7_m22 %430
%431 = OpLoad %bool %_0_ok
OpSelectionMerge %433 None
OpBranchConditional %431 %432 %433
%432 = OpLabel
%434 = OpLoad %mat2v2float %_7_m22
%435 = OpCompositeConstruct %mat2v2float %193 %194
%436 = OpCompositeExtract %v2float %434 0
%437 = OpCompositeExtract %v2float %435 0
%438 = OpFOrdEqual %v2bool %436 %437
%439 = OpAll %bool %438
%440 = OpCompositeExtract %v2float %434 1
%441 = OpCompositeExtract %v2float %435 1
%442 = OpFOrdEqual %v2bool %440 %441
%443 = OpAll %bool %442
%444 = OpLogicalAnd %bool %439 %443
OpBranch %433
%433 = OpLabel
%445 = OpPhi %bool %false %409 %444 %432
OpStore %_0_ok %445
%447 = OpLoad %mat4v3float %_6_m43
%448 = OpLoad %mat3v4float %_4_m34
%449 = OpMatrixTimesMatrix %mat3v3float %447 %448
OpStore %_8_m33 %449
%450 = OpLoad %bool %_0_ok
OpSelectionMerge %452 None
OpBranchConditional %450 %451 %452
%451 = OpLabel
%453 = OpLoad %mat3v3float %_8_m33
%454 = OpCompositeConstruct %mat3v3float %217 %218 %219
%455 = OpCompositeExtract %v3float %453 0
%456 = OpCompositeExtract %v3float %454 0
%457 = OpFOrdEqual %v3bool %455 %456
%458 = OpAll %bool %457
%459 = OpCompositeExtract %v3float %453 1
%460 = OpCompositeExtract %v3float %454 1
%461 = OpFOrdEqual %v3bool %459 %460
%462 = OpAll %bool %461
%463 = OpLogicalAnd %bool %458 %462
%464 = OpCompositeExtract %v3float %453 2
%465 = OpCompositeExtract %v3float %454 2
%466 = OpFOrdEqual %v3bool %464 %465
%467 = OpAll %bool %466
%468 = OpLogicalAnd %bool %463 %467
OpBranch %452
%452 = OpLabel
%469 = OpPhi %bool %false %433 %468 %451
OpStore %_0_ok %469
%470 = OpLoad %mat2v3float %_1_m23
%471 = OpCompositeExtract %v3float %470 0
%472 = OpFAdd %v3float %471 %237
%473 = OpCompositeExtract %v3float %470 1
%474 = OpFAdd %v3float %473 %237
%475 = OpCompositeConstruct %mat2v3float %472 %474
OpStore %_1_m23 %475
%476 = OpLoad %bool %_0_ok
OpSelectionMerge %478 None
OpBranchConditional %476 %477 %478
%477 = OpLabel
%479 = OpLoad %mat2v3float %_1_m23
%480 = OpCompositeExtract %v3float %479 0
%481 = OpFOrdEqual %v3bool %480 %248
%482 = OpAll %bool %481
%483 = OpCompositeExtract %v3float %479 1
%484 = OpFOrdEqual %v3bool %483 %249
%485 = OpAll %bool %484
%486 = OpLogicalAnd %bool %482 %485
OpBranch %478
%478 = OpLabel
%487 = OpPhi %bool %false %452 %486 %477
OpStore %_0_ok %487
%488 = OpLoad %mat3v2float %_3_m32
%489 = OpCompositeExtract %v2float %488 0
%490 = OpFSub %v2float %489 %260
%491 = OpCompositeExtract %v2float %488 1
%492 = OpFSub %v2float %491 %260
%493 = OpCompositeExtract %v2float %488 2
%494 = OpFSub %v2float %493 %260
%495 = OpCompositeConstruct %mat3v2float %490 %492 %494
OpStore %_3_m32 %495
%496 = OpLoad %bool %_0_ok
OpSelectionMerge %498 None
OpBranchConditional %496 %497 %498
%497 = OpLabel
%499 = OpLoad %mat3v2float %_3_m32
%500 = OpCompositeExtract %v2float %499 0
%501 = OpFOrdEqual %v2bool %500 %274
%502 = OpAll %bool %501
%503 = OpCompositeExtract %v2float %499 1
%504 = OpFOrdEqual %v2bool %503 %275
%505 = OpAll %bool %504
%506 = OpLogicalAnd %bool %502 %505
%507 = OpCompositeExtract %v2float %499 2
%508 = OpFOrdEqual %v2bool %507 %276
%509 = OpAll %bool %508
%510 = OpLogicalAnd %bool %506 %509
OpBranch %498
%498 = OpLabel
%511 = OpPhi %bool %false %478 %510 %497
OpStore %_0_ok %511
%512 = OpLoad %mat2v4float %_2_m24
%513 = OpCompositeExtract %v4float %512 0
%514 = OpFDiv %v4float %513 %291
%515 = OpCompositeExtract %v4float %512 1
%516 = OpFDiv %v4float %515 %291
%517 = OpCompositeConstruct %mat2v4float %514 %516
OpStore %_2_m24 %517
%518 = OpLoad %bool %_0_ok
OpSelectionMerge %520 None
OpBranchConditional %518 %519 %520
%519 = OpLabel
%521 = OpLoad %mat2v4float %_2_m24
%522 = OpCompositeExtract %v4float %521 0
%523 = OpFOrdEqual %v4bool %522 %303
%524 = OpAll %bool %523
%525 = OpCompositeExtract %v4float %521 1
%526 = OpFOrdEqual %v4bool %525 %304
%527 = OpAll %bool %526
%528 = OpLogicalAnd %bool %524 %527
OpBranch %520
%520 = OpLabel
%529 = OpPhi %bool %false %498 %528 %519
OpStore %_0_ok %529
%530 = OpLoad %bool %_0_ok
OpSelectionMerge %532 None
OpBranchConditional %530 %531 %532
%531 = OpLabel
%533 = OpFunctionCall %bool %test_half_b
OpBranch %532
%532 = OpLabel
%534 = OpPhi %bool %false %520 %533 %531
OpSelectionMerge %539 None
OpBranchConditional %534 %537 %538
%537 = OpLabel
%540 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%544 = OpLoad %v4float %540
OpStore %535 %544
OpBranch %539
%538 = OpLabel
%545 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%547 = OpLoad %v4float %545
OpStore %535 %547
OpBranch %539
%539 = OpLabel
%548 = OpLoad %v4float %535
OpReturnValue %548
OpFunctionEnd
