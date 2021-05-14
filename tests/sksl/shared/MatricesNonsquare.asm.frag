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
OpName %m43 "m43"
OpName %m22 "m22"
OpName %m33 "m33"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m23 "_1_m23"
OpName %_2_m24 "_2_m24"
OpName %_3_m32 "_3_m32"
OpName %_4_m34 "_4_m34"
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
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %m24 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %m32 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %m34 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %m22 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %440 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_3 = OpConstant %float 3
%v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%v2bool = OpTypeVector %bool 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_8 = OpConstant %float 8
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%237 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%m43 = OpVariable %_ptr_Function_mat4v3float Function
%m22 = OpVariable %_ptr_Function_mat2v2float Function
%m33 = OpVariable %_ptr_Function_mat3v3float Function
OpStore %ok %true
%35 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%36 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%34 = OpCompositeConstruct %mat2v3float %35 %36
OpStore %m23 %34
%38 = OpLoad %bool %ok
OpSelectionMerge %40 None
OpBranchConditional %38 %39 %40
%39 = OpLabel
%41 = OpLoad %mat2v3float %m23
%43 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%44 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%42 = OpCompositeConstruct %mat2v3float %43 %44
%46 = OpCompositeExtract %v3float %41 0
%47 = OpCompositeExtract %v3float %42 0
%48 = OpFOrdEqual %v3bool %46 %47
%49 = OpAll %bool %48
%50 = OpCompositeExtract %v3float %41 1
%51 = OpCompositeExtract %v3float %42 1
%52 = OpFOrdEqual %v3bool %50 %51
%53 = OpAll %bool %52
%54 = OpLogicalAnd %bool %49 %53
OpBranch %40
%40 = OpLabel
%55 = OpPhi %bool %false %25 %54 %39
OpStore %ok %55
%61 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%62 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%60 = OpCompositeConstruct %mat2v4float %61 %62
OpStore %m24 %60
%63 = OpLoad %bool %ok
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%66 = OpLoad %mat2v4float %m24
%68 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%69 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%67 = OpCompositeConstruct %mat2v4float %68 %69
%71 = OpCompositeExtract %v4float %66 0
%72 = OpCompositeExtract %v4float %67 0
%73 = OpFOrdEqual %v4bool %71 %72
%74 = OpAll %bool %73
%75 = OpCompositeExtract %v4float %66 1
%76 = OpCompositeExtract %v4float %67 1
%77 = OpFOrdEqual %v4bool %75 %76
%78 = OpAll %bool %77
%79 = OpLogicalAnd %bool %74 %78
OpBranch %65
%65 = OpLabel
%80 = OpPhi %bool %false %40 %79 %64
OpStore %ok %80
%86 = OpCompositeConstruct %v2float %float_4 %float_0
%87 = OpCompositeConstruct %v2float %float_0 %float_4
%88 = OpCompositeConstruct %v2float %float_0 %float_0
%85 = OpCompositeConstruct %mat3v2float %86 %87 %88
OpStore %m32 %85
%89 = OpLoad %bool %ok
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpLoad %mat3v2float %m32
%94 = OpCompositeConstruct %v2float %float_4 %float_0
%95 = OpCompositeConstruct %v2float %float_0 %float_4
%96 = OpCompositeConstruct %v2float %float_0 %float_0
%93 = OpCompositeConstruct %mat3v2float %94 %95 %96
%98 = OpCompositeExtract %v2float %92 0
%99 = OpCompositeExtract %v2float %93 0
%100 = OpFOrdEqual %v2bool %98 %99
%101 = OpAll %bool %100
%102 = OpCompositeExtract %v2float %92 1
%103 = OpCompositeExtract %v2float %93 1
%104 = OpFOrdEqual %v2bool %102 %103
%105 = OpAll %bool %104
%106 = OpLogicalAnd %bool %101 %105
%107 = OpCompositeExtract %v2float %92 2
%108 = OpCompositeExtract %v2float %93 2
%109 = OpFOrdEqual %v2bool %107 %108
%110 = OpAll %bool %109
%111 = OpLogicalAnd %bool %106 %110
OpBranch %91
%91 = OpLabel
%112 = OpPhi %bool %false %65 %111 %90
OpStore %ok %112
%118 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%119 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%120 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%117 = OpCompositeConstruct %mat3v4float %118 %119 %120
OpStore %m34 %117
%121 = OpLoad %bool %ok
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%124 = OpLoad %mat3v4float %m34
%126 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%127 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%128 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%125 = OpCompositeConstruct %mat3v4float %126 %127 %128
%129 = OpCompositeExtract %v4float %124 0
%130 = OpCompositeExtract %v4float %125 0
%131 = OpFOrdEqual %v4bool %129 %130
%132 = OpAll %bool %131
%133 = OpCompositeExtract %v4float %124 1
%134 = OpCompositeExtract %v4float %125 1
%135 = OpFOrdEqual %v4bool %133 %134
%136 = OpAll %bool %135
%137 = OpLogicalAnd %bool %132 %136
%138 = OpCompositeExtract %v4float %124 2
%139 = OpCompositeExtract %v4float %125 2
%140 = OpFOrdEqual %v4bool %138 %139
%141 = OpAll %bool %140
%142 = OpLogicalAnd %bool %137 %141
OpBranch %123
%123 = OpLabel
%143 = OpPhi %bool %false %91 %142 %122
OpStore %ok %143
%149 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%150 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%151 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%152 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%148 = OpCompositeConstruct %mat4v3float %149 %150 %151 %152
OpStore %m43 %148
%153 = OpLoad %bool %ok
OpSelectionMerge %155 None
OpBranchConditional %153 %154 %155
%154 = OpLabel
%156 = OpLoad %mat4v3float %m43
%158 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%159 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%160 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%161 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%157 = OpCompositeConstruct %mat4v3float %158 %159 %160 %161
%162 = OpCompositeExtract %v3float %156 0
%163 = OpCompositeExtract %v3float %157 0
%164 = OpFOrdEqual %v3bool %162 %163
%165 = OpAll %bool %164
%166 = OpCompositeExtract %v3float %156 1
%167 = OpCompositeExtract %v3float %157 1
%168 = OpFOrdEqual %v3bool %166 %167
%169 = OpAll %bool %168
%170 = OpLogicalAnd %bool %165 %169
%171 = OpCompositeExtract %v3float %156 2
%172 = OpCompositeExtract %v3float %157 2
%173 = OpFOrdEqual %v3bool %171 %172
%174 = OpAll %bool %173
%175 = OpLogicalAnd %bool %170 %174
%176 = OpCompositeExtract %v3float %156 3
%177 = OpCompositeExtract %v3float %157 3
%178 = OpFOrdEqual %v3bool %176 %177
%179 = OpAll %bool %178
%180 = OpLogicalAnd %bool %175 %179
OpBranch %155
%155 = OpLabel
%181 = OpPhi %bool %false %123 %180 %154
OpStore %ok %181
%185 = OpLoad %mat3v2float %m32
%186 = OpLoad %mat2v3float %m23
%187 = OpMatrixTimesMatrix %mat2v2float %185 %186
OpStore %m22 %187
%188 = OpLoad %bool %ok
OpSelectionMerge %190 None
OpBranchConditional %188 %189 %190
%189 = OpLabel
%191 = OpLoad %mat2v2float %m22
%194 = OpCompositeConstruct %v2float %float_8 %float_0
%195 = OpCompositeConstruct %v2float %float_0 %float_8
%193 = OpCompositeConstruct %mat2v2float %194 %195
%196 = OpCompositeExtract %v2float %191 0
%197 = OpCompositeExtract %v2float %193 0
%198 = OpFOrdEqual %v2bool %196 %197
%199 = OpAll %bool %198
%200 = OpCompositeExtract %v2float %191 1
%201 = OpCompositeExtract %v2float %193 1
%202 = OpFOrdEqual %v2bool %200 %201
%203 = OpAll %bool %202
%204 = OpLogicalAnd %bool %199 %203
OpBranch %190
%190 = OpLabel
%205 = OpPhi %bool %false %155 %204 %189
OpStore %ok %205
%209 = OpLoad %mat4v3float %m43
%210 = OpLoad %mat3v4float %m34
%211 = OpMatrixTimesMatrix %mat3v3float %209 %210
OpStore %m33 %211
%212 = OpLoad %bool %ok
OpSelectionMerge %214 None
OpBranchConditional %212 %213 %214
%213 = OpLabel
%215 = OpLoad %mat3v3float %m33
%218 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%219 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%220 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%217 = OpCompositeConstruct %mat3v3float %218 %219 %220
%221 = OpCompositeExtract %v3float %215 0
%222 = OpCompositeExtract %v3float %217 0
%223 = OpFOrdEqual %v3bool %221 %222
%224 = OpAll %bool %223
%225 = OpCompositeExtract %v3float %215 1
%226 = OpCompositeExtract %v3float %217 1
%227 = OpFOrdEqual %v3bool %225 %226
%228 = OpAll %bool %227
%229 = OpLogicalAnd %bool %224 %228
%230 = OpCompositeExtract %v3float %215 2
%231 = OpCompositeExtract %v3float %217 2
%232 = OpFOrdEqual %v3bool %230 %231
%233 = OpAll %bool %232
%234 = OpLogicalAnd %bool %229 %233
OpBranch %214
%214 = OpLabel
%235 = OpPhi %bool %false %190 %234 %213
OpStore %ok %235
%236 = OpLoad %bool %ok
OpReturnValue %236
OpFunctionEnd
%main = OpFunction %v4float None %237
%238 = OpFunctionParameter %_ptr_Function_v2float
%239 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%427 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%243 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%244 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%242 = OpCompositeConstruct %mat2v3float %243 %244
OpStore %_1_m23 %242
%245 = OpLoad %bool %_0_ok
OpSelectionMerge %247 None
OpBranchConditional %245 %246 %247
%246 = OpLabel
%248 = OpLoad %mat2v3float %_1_m23
%250 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%251 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%249 = OpCompositeConstruct %mat2v3float %250 %251
%252 = OpCompositeExtract %v3float %248 0
%253 = OpCompositeExtract %v3float %249 0
%254 = OpFOrdEqual %v3bool %252 %253
%255 = OpAll %bool %254
%256 = OpCompositeExtract %v3float %248 1
%257 = OpCompositeExtract %v3float %249 1
%258 = OpFOrdEqual %v3bool %256 %257
%259 = OpAll %bool %258
%260 = OpLogicalAnd %bool %255 %259
OpBranch %247
%247 = OpLabel
%261 = OpPhi %bool %false %239 %260 %246
OpStore %_0_ok %261
%264 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%265 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%263 = OpCompositeConstruct %mat2v4float %264 %265
OpStore %_2_m24 %263
%266 = OpLoad %bool %_0_ok
OpSelectionMerge %268 None
OpBranchConditional %266 %267 %268
%267 = OpLabel
%269 = OpLoad %mat2v4float %_2_m24
%271 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%272 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%270 = OpCompositeConstruct %mat2v4float %271 %272
%273 = OpCompositeExtract %v4float %269 0
%274 = OpCompositeExtract %v4float %270 0
%275 = OpFOrdEqual %v4bool %273 %274
%276 = OpAll %bool %275
%277 = OpCompositeExtract %v4float %269 1
%278 = OpCompositeExtract %v4float %270 1
%279 = OpFOrdEqual %v4bool %277 %278
%280 = OpAll %bool %279
%281 = OpLogicalAnd %bool %276 %280
OpBranch %268
%268 = OpLabel
%282 = OpPhi %bool %false %247 %281 %267
OpStore %_0_ok %282
%285 = OpCompositeConstruct %v2float %float_4 %float_0
%286 = OpCompositeConstruct %v2float %float_0 %float_4
%287 = OpCompositeConstruct %v2float %float_0 %float_0
%284 = OpCompositeConstruct %mat3v2float %285 %286 %287
OpStore %_3_m32 %284
%288 = OpLoad %bool %_0_ok
OpSelectionMerge %290 None
OpBranchConditional %288 %289 %290
%289 = OpLabel
%291 = OpLoad %mat3v2float %_3_m32
%293 = OpCompositeConstruct %v2float %float_4 %float_0
%294 = OpCompositeConstruct %v2float %float_0 %float_4
%295 = OpCompositeConstruct %v2float %float_0 %float_0
%292 = OpCompositeConstruct %mat3v2float %293 %294 %295
%296 = OpCompositeExtract %v2float %291 0
%297 = OpCompositeExtract %v2float %292 0
%298 = OpFOrdEqual %v2bool %296 %297
%299 = OpAll %bool %298
%300 = OpCompositeExtract %v2float %291 1
%301 = OpCompositeExtract %v2float %292 1
%302 = OpFOrdEqual %v2bool %300 %301
%303 = OpAll %bool %302
%304 = OpLogicalAnd %bool %299 %303
%305 = OpCompositeExtract %v2float %291 2
%306 = OpCompositeExtract %v2float %292 2
%307 = OpFOrdEqual %v2bool %305 %306
%308 = OpAll %bool %307
%309 = OpLogicalAnd %bool %304 %308
OpBranch %290
%290 = OpLabel
%310 = OpPhi %bool %false %268 %309 %289
OpStore %_0_ok %310
%313 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%314 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%315 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%312 = OpCompositeConstruct %mat3v4float %313 %314 %315
OpStore %_4_m34 %312
%316 = OpLoad %bool %_0_ok
OpSelectionMerge %318 None
OpBranchConditional %316 %317 %318
%317 = OpLabel
%319 = OpLoad %mat3v4float %_4_m34
%321 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%322 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%323 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%320 = OpCompositeConstruct %mat3v4float %321 %322 %323
%324 = OpCompositeExtract %v4float %319 0
%325 = OpCompositeExtract %v4float %320 0
%326 = OpFOrdEqual %v4bool %324 %325
%327 = OpAll %bool %326
%328 = OpCompositeExtract %v4float %319 1
%329 = OpCompositeExtract %v4float %320 1
%330 = OpFOrdEqual %v4bool %328 %329
%331 = OpAll %bool %330
%332 = OpLogicalAnd %bool %327 %331
%333 = OpCompositeExtract %v4float %319 2
%334 = OpCompositeExtract %v4float %320 2
%335 = OpFOrdEqual %v4bool %333 %334
%336 = OpAll %bool %335
%337 = OpLogicalAnd %bool %332 %336
OpBranch %318
%318 = OpLabel
%338 = OpPhi %bool %false %290 %337 %317
OpStore %_0_ok %338
%341 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%342 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%343 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%344 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%340 = OpCompositeConstruct %mat4v3float %341 %342 %343 %344
OpStore %_6_m43 %340
%345 = OpLoad %bool %_0_ok
OpSelectionMerge %347 None
OpBranchConditional %345 %346 %347
%346 = OpLabel
%348 = OpLoad %mat4v3float %_6_m43
%350 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%351 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%352 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%353 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%349 = OpCompositeConstruct %mat4v3float %350 %351 %352 %353
%354 = OpCompositeExtract %v3float %348 0
%355 = OpCompositeExtract %v3float %349 0
%356 = OpFOrdEqual %v3bool %354 %355
%357 = OpAll %bool %356
%358 = OpCompositeExtract %v3float %348 1
%359 = OpCompositeExtract %v3float %349 1
%360 = OpFOrdEqual %v3bool %358 %359
%361 = OpAll %bool %360
%362 = OpLogicalAnd %bool %357 %361
%363 = OpCompositeExtract %v3float %348 2
%364 = OpCompositeExtract %v3float %349 2
%365 = OpFOrdEqual %v3bool %363 %364
%366 = OpAll %bool %365
%367 = OpLogicalAnd %bool %362 %366
%368 = OpCompositeExtract %v3float %348 3
%369 = OpCompositeExtract %v3float %349 3
%370 = OpFOrdEqual %v3bool %368 %369
%371 = OpAll %bool %370
%372 = OpLogicalAnd %bool %367 %371
OpBranch %347
%347 = OpLabel
%373 = OpPhi %bool %false %318 %372 %346
OpStore %_0_ok %373
%375 = OpLoad %mat3v2float %_3_m32
%376 = OpLoad %mat2v3float %_1_m23
%377 = OpMatrixTimesMatrix %mat2v2float %375 %376
OpStore %_7_m22 %377
%378 = OpLoad %bool %_0_ok
OpSelectionMerge %380 None
OpBranchConditional %378 %379 %380
%379 = OpLabel
%381 = OpLoad %mat2v2float %_7_m22
%383 = OpCompositeConstruct %v2float %float_8 %float_0
%384 = OpCompositeConstruct %v2float %float_0 %float_8
%382 = OpCompositeConstruct %mat2v2float %383 %384
%385 = OpCompositeExtract %v2float %381 0
%386 = OpCompositeExtract %v2float %382 0
%387 = OpFOrdEqual %v2bool %385 %386
%388 = OpAll %bool %387
%389 = OpCompositeExtract %v2float %381 1
%390 = OpCompositeExtract %v2float %382 1
%391 = OpFOrdEqual %v2bool %389 %390
%392 = OpAll %bool %391
%393 = OpLogicalAnd %bool %388 %392
OpBranch %380
%380 = OpLabel
%394 = OpPhi %bool %false %347 %393 %379
OpStore %_0_ok %394
%396 = OpLoad %mat4v3float %_6_m43
%397 = OpLoad %mat3v4float %_4_m34
%398 = OpMatrixTimesMatrix %mat3v3float %396 %397
OpStore %_8_m33 %398
%399 = OpLoad %bool %_0_ok
OpSelectionMerge %401 None
OpBranchConditional %399 %400 %401
%400 = OpLabel
%402 = OpLoad %mat3v3float %_8_m33
%404 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%405 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%406 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%403 = OpCompositeConstruct %mat3v3float %404 %405 %406
%407 = OpCompositeExtract %v3float %402 0
%408 = OpCompositeExtract %v3float %403 0
%409 = OpFOrdEqual %v3bool %407 %408
%410 = OpAll %bool %409
%411 = OpCompositeExtract %v3float %402 1
%412 = OpCompositeExtract %v3float %403 1
%413 = OpFOrdEqual %v3bool %411 %412
%414 = OpAll %bool %413
%415 = OpLogicalAnd %bool %410 %414
%416 = OpCompositeExtract %v3float %402 2
%417 = OpCompositeExtract %v3float %403 2
%418 = OpFOrdEqual %v3bool %416 %417
%419 = OpAll %bool %418
%420 = OpLogicalAnd %bool %415 %419
OpBranch %401
%401 = OpLabel
%421 = OpPhi %bool %false %380 %420 %400
OpStore %_0_ok %421
%422 = OpLoad %bool %_0_ok
OpSelectionMerge %424 None
OpBranchConditional %422 %423 %424
%423 = OpLabel
%425 = OpFunctionCall %bool %test_half_b
OpBranch %424
%424 = OpLabel
%426 = OpPhi %bool %false %401 %425 %423
OpSelectionMerge %431 None
OpBranchConditional %426 %429 %430
%429 = OpLabel
%432 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%436 = OpLoad %v4float %432
OpStore %427 %436
OpBranch %431
%430 = OpLabel
%437 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%439 = OpLoad %v4float %437
OpStore %427 %439
OpBranch %431
%431 = OpLabel
%440 = OpLoad %v4float %427
OpReturnValue %440
OpFunctionEnd
