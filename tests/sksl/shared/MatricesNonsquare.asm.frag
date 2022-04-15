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
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %m24 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %m32 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %m34 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %m42 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %m22 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %452 RelaxedPrecision
OpDecorate %472 RelaxedPrecision
OpDecorate %494 RelaxedPrecision
OpDecorate %506 RelaxedPrecision
OpDecorate %520 RelaxedPrecision
OpDecorate %523 RelaxedPrecision
OpDecorate %524 RelaxedPrecision
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
%55 = OpConstantComposite %v4float %float_3 %float_0 %float_0 %float_0
%56 = OpConstantComposite %v4float %float_0 %float_3 %float_0 %float_0
%57 = OpConstantComposite %mat2v4float %55 %56
%v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%75 = OpConstantComposite %v2float %float_4 %float_0
%76 = OpConstantComposite %v2float %float_0 %float_4
%77 = OpConstantComposite %mat3v2float %75 %76 %20
%v2bool = OpTypeVector %bool 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%99 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
%100 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
%101 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
%102 = OpConstantComposite %mat3v4float %99 %100 %101
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%123 = OpConstantComposite %v2float %float_6 %float_0
%124 = OpConstantComposite %v2float %float_0 %float_6
%125 = OpConstantComposite %mat4v2float %123 %124 %20 %20
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%150 = OpConstantComposite %v3float %float_7 %float_0 %float_0
%151 = OpConstantComposite %v3float %float_0 %float_7 %float_0
%152 = OpConstantComposite %v3float %float_0 %float_0 %float_7
%153 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%154 = OpConstantComposite %mat4v3float %150 %151 %152 %153
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_8 = OpConstant %float 8
%186 = OpConstantComposite %v2float %float_8 %float_0
%187 = OpConstantComposite %v2float %float_0 %float_8
%188 = OpConstantComposite %mat2v2float %186 %187
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%208 = OpConstantComposite %v3float %float_35 %float_0 %float_0
%209 = OpConstantComposite %v3float %float_0 %float_35 %float_0
%210 = OpConstantComposite %v3float %float_0 %float_0 %float_35
%211 = OpConstantComposite %mat3v3float %208 %209 %210
%float_1 = OpConstant %float 1
%226 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%227 = OpConstantComposite %mat2v3float %226 %226
%237 = OpConstantComposite %v3float %float_3 %float_1 %float_1
%238 = OpConstantComposite %v3float %float_1 %float_3 %float_1
%239 = OpConstantComposite %mat2v3float %237 %238
%249 = OpConstantComposite %v2float %float_2 %float_2
%250 = OpConstantComposite %mat3v2float %249 %249 %249
%float_n2 = OpConstant %float -2
%263 = OpConstantComposite %v2float %float_2 %float_n2
%264 = OpConstantComposite %v2float %float_n2 %float_2
%265 = OpConstantComposite %v2float %float_n2 %float_n2
%266 = OpConstantComposite %mat3v2float %263 %264 %265
%280 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%281 = OpConstantComposite %mat2v4float %280 %280
%float_0_75 = OpConstant %float 0.75
%292 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0 %float_0
%293 = OpConstantComposite %v4float %float_0 %float_0_75 %float_0 %float_0
%294 = OpConstantComposite %mat2v4float %292 %293
%304 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%38 = OpLoad %bool %ok
OpSelectionMerge %40 None
OpBranchConditional %38 %39 %40
%39 = OpLabel
%41 = OpLoad %mat2v3float %m23
%43 = OpCompositeExtract %v3float %41 0
%44 = OpFOrdEqual %v3bool %43 %34
%45 = OpAll %bool %44
%46 = OpCompositeExtract %v3float %41 1
%47 = OpFOrdEqual %v3bool %46 %35
%48 = OpAll %bool %47
%49 = OpLogicalAnd %bool %45 %48
OpBranch %40
%40 = OpLabel
%50 = OpPhi %bool %false %25 %49 %39
OpStore %ok %50
OpStore %m24 %57
%58 = OpLoad %bool %ok
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%61 = OpLoad %mat2v4float %m24
%63 = OpCompositeExtract %v4float %61 0
%64 = OpFOrdEqual %v4bool %63 %55
%65 = OpAll %bool %64
%66 = OpCompositeExtract %v4float %61 1
%67 = OpFOrdEqual %v4bool %66 %56
%68 = OpAll %bool %67
%69 = OpLogicalAnd %bool %65 %68
OpBranch %60
%60 = OpLabel
%70 = OpPhi %bool %false %40 %69 %59
OpStore %ok %70
OpStore %m32 %77
%78 = OpLoad %bool %ok
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%81 = OpLoad %mat3v2float %m32
%83 = OpCompositeExtract %v2float %81 0
%84 = OpFOrdEqual %v2bool %83 %75
%85 = OpAll %bool %84
%86 = OpCompositeExtract %v2float %81 1
%87 = OpFOrdEqual %v2bool %86 %76
%88 = OpAll %bool %87
%89 = OpLogicalAnd %bool %85 %88
%90 = OpCompositeExtract %v2float %81 2
%91 = OpFOrdEqual %v2bool %90 %20
%92 = OpAll %bool %91
%93 = OpLogicalAnd %bool %89 %92
OpBranch %80
%80 = OpLabel
%94 = OpPhi %bool %false %60 %93 %79
OpStore %ok %94
OpStore %m34 %102
%103 = OpLoad %bool %ok
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%106 = OpLoad %mat3v4float %m34
%107 = OpCompositeExtract %v4float %106 0
%108 = OpFOrdEqual %v4bool %107 %99
%109 = OpAll %bool %108
%110 = OpCompositeExtract %v4float %106 1
%111 = OpFOrdEqual %v4bool %110 %100
%112 = OpAll %bool %111
%113 = OpLogicalAnd %bool %109 %112
%114 = OpCompositeExtract %v4float %106 2
%115 = OpFOrdEqual %v4bool %114 %101
%116 = OpAll %bool %115
%117 = OpLogicalAnd %bool %113 %116
OpBranch %105
%105 = OpLabel
%118 = OpPhi %bool %false %80 %117 %104
OpStore %ok %118
OpStore %m42 %125
%126 = OpLoad %bool %ok
OpSelectionMerge %128 None
OpBranchConditional %126 %127 %128
%127 = OpLabel
%129 = OpLoad %mat4v2float %m42
%130 = OpCompositeExtract %v2float %129 0
%131 = OpFOrdEqual %v2bool %130 %123
%132 = OpAll %bool %131
%133 = OpCompositeExtract %v2float %129 1
%134 = OpFOrdEqual %v2bool %133 %124
%135 = OpAll %bool %134
%136 = OpLogicalAnd %bool %132 %135
%137 = OpCompositeExtract %v2float %129 2
%138 = OpFOrdEqual %v2bool %137 %20
%139 = OpAll %bool %138
%140 = OpLogicalAnd %bool %136 %139
%141 = OpCompositeExtract %v2float %129 3
%142 = OpFOrdEqual %v2bool %141 %20
%143 = OpAll %bool %142
%144 = OpLogicalAnd %bool %140 %143
OpBranch %128
%128 = OpLabel
%145 = OpPhi %bool %false %105 %144 %127
OpStore %ok %145
OpStore %m43 %154
%155 = OpLoad %bool %ok
OpSelectionMerge %157 None
OpBranchConditional %155 %156 %157
%156 = OpLabel
%158 = OpLoad %mat4v3float %m43
%159 = OpCompositeExtract %v3float %158 0
%160 = OpFOrdEqual %v3bool %159 %150
%161 = OpAll %bool %160
%162 = OpCompositeExtract %v3float %158 1
%163 = OpFOrdEqual %v3bool %162 %151
%164 = OpAll %bool %163
%165 = OpLogicalAnd %bool %161 %164
%166 = OpCompositeExtract %v3float %158 2
%167 = OpFOrdEqual %v3bool %166 %152
%168 = OpAll %bool %167
%169 = OpLogicalAnd %bool %165 %168
%170 = OpCompositeExtract %v3float %158 3
%171 = OpFOrdEqual %v3bool %170 %153
%172 = OpAll %bool %171
%173 = OpLogicalAnd %bool %169 %172
OpBranch %157
%157 = OpLabel
%174 = OpPhi %bool %false %128 %173 %156
OpStore %ok %174
%178 = OpLoad %mat3v2float %m32
%179 = OpLoad %mat2v3float %m23
%180 = OpMatrixTimesMatrix %mat2v2float %178 %179
OpStore %m22 %180
%181 = OpLoad %bool %ok
OpSelectionMerge %183 None
OpBranchConditional %181 %182 %183
%182 = OpLabel
%184 = OpLoad %mat2v2float %m22
%189 = OpCompositeExtract %v2float %184 0
%190 = OpFOrdEqual %v2bool %189 %186
%191 = OpAll %bool %190
%192 = OpCompositeExtract %v2float %184 1
%193 = OpFOrdEqual %v2bool %192 %187
%194 = OpAll %bool %193
%195 = OpLogicalAnd %bool %191 %194
OpBranch %183
%183 = OpLabel
%196 = OpPhi %bool %false %157 %195 %182
OpStore %ok %196
%200 = OpLoad %mat4v3float %m43
%201 = OpLoad %mat3v4float %m34
%202 = OpMatrixTimesMatrix %mat3v3float %200 %201
OpStore %m33 %202
%203 = OpLoad %bool %ok
OpSelectionMerge %205 None
OpBranchConditional %203 %204 %205
%204 = OpLabel
%206 = OpLoad %mat3v3float %m33
%212 = OpCompositeExtract %v3float %206 0
%213 = OpFOrdEqual %v3bool %212 %208
%214 = OpAll %bool %213
%215 = OpCompositeExtract %v3float %206 1
%216 = OpFOrdEqual %v3bool %215 %209
%217 = OpAll %bool %216
%218 = OpLogicalAnd %bool %214 %217
%219 = OpCompositeExtract %v3float %206 2
%220 = OpFOrdEqual %v3bool %219 %210
%221 = OpAll %bool %220
%222 = OpLogicalAnd %bool %218 %221
OpBranch %205
%205 = OpLabel
%223 = OpPhi %bool %false %183 %222 %204
OpStore %ok %223
%224 = OpLoad %mat2v3float %m23
%228 = OpCompositeExtract %v3float %224 0
%229 = OpFAdd %v3float %228 %226
%230 = OpCompositeExtract %v3float %224 1
%231 = OpFAdd %v3float %230 %226
%232 = OpCompositeConstruct %mat2v3float %229 %231
OpStore %m23 %232
%233 = OpLoad %bool %ok
OpSelectionMerge %235 None
OpBranchConditional %233 %234 %235
%234 = OpLabel
%236 = OpLoad %mat2v3float %m23
%240 = OpCompositeExtract %v3float %236 0
%241 = OpFOrdEqual %v3bool %240 %237
%242 = OpAll %bool %241
%243 = OpCompositeExtract %v3float %236 1
%244 = OpFOrdEqual %v3bool %243 %238
%245 = OpAll %bool %244
%246 = OpLogicalAnd %bool %242 %245
OpBranch %235
%235 = OpLabel
%247 = OpPhi %bool %false %205 %246 %234
OpStore %ok %247
%248 = OpLoad %mat3v2float %m32
%251 = OpCompositeExtract %v2float %248 0
%252 = OpFSub %v2float %251 %249
%253 = OpCompositeExtract %v2float %248 1
%254 = OpFSub %v2float %253 %249
%255 = OpCompositeExtract %v2float %248 2
%256 = OpFSub %v2float %255 %249
%257 = OpCompositeConstruct %mat3v2float %252 %254 %256
OpStore %m32 %257
%258 = OpLoad %bool %ok
OpSelectionMerge %260 None
OpBranchConditional %258 %259 %260
%259 = OpLabel
%261 = OpLoad %mat3v2float %m32
%267 = OpCompositeExtract %v2float %261 0
%268 = OpFOrdEqual %v2bool %267 %263
%269 = OpAll %bool %268
%270 = OpCompositeExtract %v2float %261 1
%271 = OpFOrdEqual %v2bool %270 %264
%272 = OpAll %bool %271
%273 = OpLogicalAnd %bool %269 %272
%274 = OpCompositeExtract %v2float %261 2
%275 = OpFOrdEqual %v2bool %274 %265
%276 = OpAll %bool %275
%277 = OpLogicalAnd %bool %273 %276
OpBranch %260
%260 = OpLabel
%278 = OpPhi %bool %false %235 %277 %259
OpStore %ok %278
%279 = OpLoad %mat2v4float %m24
%282 = OpCompositeExtract %v4float %279 0
%283 = OpFDiv %v4float %282 %280
%284 = OpCompositeExtract %v4float %279 1
%285 = OpFDiv %v4float %284 %280
%286 = OpCompositeConstruct %mat2v4float %283 %285
OpStore %m24 %286
%287 = OpLoad %bool %ok
OpSelectionMerge %289 None
OpBranchConditional %287 %288 %289
%288 = OpLabel
%290 = OpLoad %mat2v4float %m24
%295 = OpCompositeExtract %v4float %290 0
%296 = OpFOrdEqual %v4bool %295 %292
%297 = OpAll %bool %296
%298 = OpCompositeExtract %v4float %290 1
%299 = OpFOrdEqual %v4bool %298 %293
%300 = OpAll %bool %299
%301 = OpLogicalAnd %bool %297 %300
OpBranch %289
%289 = OpLabel
%302 = OpPhi %bool %false %260 %301 %288
OpStore %ok %302
%303 = OpLoad %bool %ok
OpReturnValue %303
OpFunctionEnd
%main = OpFunction %v4float None %304
%305 = OpFunctionParameter %_ptr_Function_v2float
%306 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%511 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
OpStore %_1_m23 %36
%309 = OpLoad %bool %_0_ok
OpSelectionMerge %311 None
OpBranchConditional %309 %310 %311
%310 = OpLabel
%312 = OpLoad %mat2v3float %_1_m23
%313 = OpCompositeExtract %v3float %312 0
%314 = OpFOrdEqual %v3bool %313 %34
%315 = OpAll %bool %314
%316 = OpCompositeExtract %v3float %312 1
%317 = OpFOrdEqual %v3bool %316 %35
%318 = OpAll %bool %317
%319 = OpLogicalAnd %bool %315 %318
OpBranch %311
%311 = OpLabel
%320 = OpPhi %bool %false %306 %319 %310
OpStore %_0_ok %320
OpStore %_2_m24 %57
%322 = OpLoad %bool %_0_ok
OpSelectionMerge %324 None
OpBranchConditional %322 %323 %324
%323 = OpLabel
%325 = OpLoad %mat2v4float %_2_m24
%326 = OpCompositeExtract %v4float %325 0
%327 = OpFOrdEqual %v4bool %326 %55
%328 = OpAll %bool %327
%329 = OpCompositeExtract %v4float %325 1
%330 = OpFOrdEqual %v4bool %329 %56
%331 = OpAll %bool %330
%332 = OpLogicalAnd %bool %328 %331
OpBranch %324
%324 = OpLabel
%333 = OpPhi %bool %false %311 %332 %323
OpStore %_0_ok %333
OpStore %_3_m32 %77
%335 = OpLoad %bool %_0_ok
OpSelectionMerge %337 None
OpBranchConditional %335 %336 %337
%336 = OpLabel
%338 = OpLoad %mat3v2float %_3_m32
%339 = OpCompositeExtract %v2float %338 0
%340 = OpFOrdEqual %v2bool %339 %75
%341 = OpAll %bool %340
%342 = OpCompositeExtract %v2float %338 1
%343 = OpFOrdEqual %v2bool %342 %76
%344 = OpAll %bool %343
%345 = OpLogicalAnd %bool %341 %344
%346 = OpCompositeExtract %v2float %338 2
%347 = OpFOrdEqual %v2bool %346 %20
%348 = OpAll %bool %347
%349 = OpLogicalAnd %bool %345 %348
OpBranch %337
%337 = OpLabel
%350 = OpPhi %bool %false %324 %349 %336
OpStore %_0_ok %350
OpStore %_4_m34 %102
%352 = OpLoad %bool %_0_ok
OpSelectionMerge %354 None
OpBranchConditional %352 %353 %354
%353 = OpLabel
%355 = OpLoad %mat3v4float %_4_m34
%356 = OpCompositeExtract %v4float %355 0
%357 = OpFOrdEqual %v4bool %356 %99
%358 = OpAll %bool %357
%359 = OpCompositeExtract %v4float %355 1
%360 = OpFOrdEqual %v4bool %359 %100
%361 = OpAll %bool %360
%362 = OpLogicalAnd %bool %358 %361
%363 = OpCompositeExtract %v4float %355 2
%364 = OpFOrdEqual %v4bool %363 %101
%365 = OpAll %bool %364
%366 = OpLogicalAnd %bool %362 %365
OpBranch %354
%354 = OpLabel
%367 = OpPhi %bool %false %337 %366 %353
OpStore %_0_ok %367
OpStore %_5_m42 %125
%369 = OpLoad %bool %_0_ok
OpSelectionMerge %371 None
OpBranchConditional %369 %370 %371
%370 = OpLabel
%372 = OpLoad %mat4v2float %_5_m42
%373 = OpCompositeExtract %v2float %372 0
%374 = OpFOrdEqual %v2bool %373 %123
%375 = OpAll %bool %374
%376 = OpCompositeExtract %v2float %372 1
%377 = OpFOrdEqual %v2bool %376 %124
%378 = OpAll %bool %377
%379 = OpLogicalAnd %bool %375 %378
%380 = OpCompositeExtract %v2float %372 2
%381 = OpFOrdEqual %v2bool %380 %20
%382 = OpAll %bool %381
%383 = OpLogicalAnd %bool %379 %382
%384 = OpCompositeExtract %v2float %372 3
%385 = OpFOrdEqual %v2bool %384 %20
%386 = OpAll %bool %385
%387 = OpLogicalAnd %bool %383 %386
OpBranch %371
%371 = OpLabel
%388 = OpPhi %bool %false %354 %387 %370
OpStore %_0_ok %388
OpStore %_6_m43 %154
%390 = OpLoad %bool %_0_ok
OpSelectionMerge %392 None
OpBranchConditional %390 %391 %392
%391 = OpLabel
%393 = OpLoad %mat4v3float %_6_m43
%394 = OpCompositeExtract %v3float %393 0
%395 = OpFOrdEqual %v3bool %394 %150
%396 = OpAll %bool %395
%397 = OpCompositeExtract %v3float %393 1
%398 = OpFOrdEqual %v3bool %397 %151
%399 = OpAll %bool %398
%400 = OpLogicalAnd %bool %396 %399
%401 = OpCompositeExtract %v3float %393 2
%402 = OpFOrdEqual %v3bool %401 %152
%403 = OpAll %bool %402
%404 = OpLogicalAnd %bool %400 %403
%405 = OpCompositeExtract %v3float %393 3
%406 = OpFOrdEqual %v3bool %405 %153
%407 = OpAll %bool %406
%408 = OpLogicalAnd %bool %404 %407
OpBranch %392
%392 = OpLabel
%409 = OpPhi %bool %false %371 %408 %391
OpStore %_0_ok %409
%411 = OpLoad %mat3v2float %_3_m32
%412 = OpLoad %mat2v3float %_1_m23
%413 = OpMatrixTimesMatrix %mat2v2float %411 %412
OpStore %_7_m22 %413
%414 = OpLoad %bool %_0_ok
OpSelectionMerge %416 None
OpBranchConditional %414 %415 %416
%415 = OpLabel
%417 = OpLoad %mat2v2float %_7_m22
%418 = OpCompositeExtract %v2float %417 0
%419 = OpFOrdEqual %v2bool %418 %186
%420 = OpAll %bool %419
%421 = OpCompositeExtract %v2float %417 1
%422 = OpFOrdEqual %v2bool %421 %187
%423 = OpAll %bool %422
%424 = OpLogicalAnd %bool %420 %423
OpBranch %416
%416 = OpLabel
%425 = OpPhi %bool %false %392 %424 %415
OpStore %_0_ok %425
%427 = OpLoad %mat4v3float %_6_m43
%428 = OpLoad %mat3v4float %_4_m34
%429 = OpMatrixTimesMatrix %mat3v3float %427 %428
OpStore %_8_m33 %429
%430 = OpLoad %bool %_0_ok
OpSelectionMerge %432 None
OpBranchConditional %430 %431 %432
%431 = OpLabel
%433 = OpLoad %mat3v3float %_8_m33
%434 = OpCompositeExtract %v3float %433 0
%435 = OpFOrdEqual %v3bool %434 %208
%436 = OpAll %bool %435
%437 = OpCompositeExtract %v3float %433 1
%438 = OpFOrdEqual %v3bool %437 %209
%439 = OpAll %bool %438
%440 = OpLogicalAnd %bool %436 %439
%441 = OpCompositeExtract %v3float %433 2
%442 = OpFOrdEqual %v3bool %441 %210
%443 = OpAll %bool %442
%444 = OpLogicalAnd %bool %440 %443
OpBranch %432
%432 = OpLabel
%445 = OpPhi %bool %false %416 %444 %431
OpStore %_0_ok %445
%446 = OpLoad %mat2v3float %_1_m23
%447 = OpCompositeExtract %v3float %446 0
%448 = OpFAdd %v3float %447 %226
%449 = OpCompositeExtract %v3float %446 1
%450 = OpFAdd %v3float %449 %226
%451 = OpCompositeConstruct %mat2v3float %448 %450
OpStore %_1_m23 %451
%452 = OpLoad %bool %_0_ok
OpSelectionMerge %454 None
OpBranchConditional %452 %453 %454
%453 = OpLabel
%455 = OpLoad %mat2v3float %_1_m23
%456 = OpCompositeExtract %v3float %455 0
%457 = OpFOrdEqual %v3bool %456 %237
%458 = OpAll %bool %457
%459 = OpCompositeExtract %v3float %455 1
%460 = OpFOrdEqual %v3bool %459 %238
%461 = OpAll %bool %460
%462 = OpLogicalAnd %bool %458 %461
OpBranch %454
%454 = OpLabel
%463 = OpPhi %bool %false %432 %462 %453
OpStore %_0_ok %463
%464 = OpLoad %mat3v2float %_3_m32
%465 = OpCompositeExtract %v2float %464 0
%466 = OpFSub %v2float %465 %249
%467 = OpCompositeExtract %v2float %464 1
%468 = OpFSub %v2float %467 %249
%469 = OpCompositeExtract %v2float %464 2
%470 = OpFSub %v2float %469 %249
%471 = OpCompositeConstruct %mat3v2float %466 %468 %470
OpStore %_3_m32 %471
%472 = OpLoad %bool %_0_ok
OpSelectionMerge %474 None
OpBranchConditional %472 %473 %474
%473 = OpLabel
%475 = OpLoad %mat3v2float %_3_m32
%476 = OpCompositeExtract %v2float %475 0
%477 = OpFOrdEqual %v2bool %476 %263
%478 = OpAll %bool %477
%479 = OpCompositeExtract %v2float %475 1
%480 = OpFOrdEqual %v2bool %479 %264
%481 = OpAll %bool %480
%482 = OpLogicalAnd %bool %478 %481
%483 = OpCompositeExtract %v2float %475 2
%484 = OpFOrdEqual %v2bool %483 %265
%485 = OpAll %bool %484
%486 = OpLogicalAnd %bool %482 %485
OpBranch %474
%474 = OpLabel
%487 = OpPhi %bool %false %454 %486 %473
OpStore %_0_ok %487
%488 = OpLoad %mat2v4float %_2_m24
%489 = OpCompositeExtract %v4float %488 0
%490 = OpFDiv %v4float %489 %280
%491 = OpCompositeExtract %v4float %488 1
%492 = OpFDiv %v4float %491 %280
%493 = OpCompositeConstruct %mat2v4float %490 %492
OpStore %_2_m24 %493
%494 = OpLoad %bool %_0_ok
OpSelectionMerge %496 None
OpBranchConditional %494 %495 %496
%495 = OpLabel
%497 = OpLoad %mat2v4float %_2_m24
%498 = OpCompositeExtract %v4float %497 0
%499 = OpFOrdEqual %v4bool %498 %292
%500 = OpAll %bool %499
%501 = OpCompositeExtract %v4float %497 1
%502 = OpFOrdEqual %v4bool %501 %293
%503 = OpAll %bool %502
%504 = OpLogicalAnd %bool %500 %503
OpBranch %496
%496 = OpLabel
%505 = OpPhi %bool %false %474 %504 %495
OpStore %_0_ok %505
%506 = OpLoad %bool %_0_ok
OpSelectionMerge %508 None
OpBranchConditional %506 %507 %508
%507 = OpLabel
%509 = OpFunctionCall %bool %test_half_b
OpBranch %508
%508 = OpLabel
%510 = OpPhi %bool %false %496 %509 %507
OpSelectionMerge %515 None
OpBranchConditional %510 %513 %514
%513 = OpLabel
%516 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%520 = OpLoad %v4float %516
OpStore %511 %520
OpBranch %515
%514 = OpLabel
%521 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%523 = OpLoad %v4float %521
OpStore %511 %523
OpBranch %515
%515 = OpLabel
%524 = OpLoad %v4float %511
OpReturnValue %524
OpFunctionEnd
