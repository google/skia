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
OpDecorate %m42 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %m22 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %448 RelaxedPrecision
OpDecorate %478 RelaxedPrecision
OpDecorate %513 RelaxedPrecision
OpDecorate %546 RelaxedPrecision
OpDecorate %567 RelaxedPrecision
OpDecorate %600 RelaxedPrecision
OpDecorate %630 RelaxedPrecision
OpDecorate %663 RelaxedPrecision
OpDecorate %680 RelaxedPrecision
OpDecorate %694 RelaxedPrecision
OpDecorate %697 RelaxedPrecision
OpDecorate %698 RelaxedPrecision
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
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_1 = OpConstant %float 1
%float_6 = OpConstant %float 6
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_8 = OpConstant %float 8
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%float_n2 = OpConstant %float -2
%float_0_75 = OpConstant %float 0.75
%369 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%149 = OpCompositeConstruct %v2float %float_1 %float_0
%150 = OpCompositeConstruct %v2float %float_0 %float_1
%151 = OpCompositeConstruct %v2float %float_0 %float_0
%152 = OpCompositeConstruct %v2float %float_0 %float_0
%148 = OpCompositeConstruct %mat4v2float %149 %150 %151 %152
%154 = OpMatrixTimesScalar %mat4v2float %148 %float_6
OpStore %m42 %154
%155 = OpLoad %bool %ok
OpSelectionMerge %157 None
OpBranchConditional %155 %156 %157
%156 = OpLabel
%158 = OpLoad %mat4v2float %m42
%160 = OpCompositeConstruct %v2float %float_6 %float_0
%161 = OpCompositeConstruct %v2float %float_0 %float_6
%162 = OpCompositeConstruct %v2float %float_0 %float_0
%163 = OpCompositeConstruct %v2float %float_0 %float_0
%159 = OpCompositeConstruct %mat4v2float %160 %161 %162 %163
%164 = OpCompositeExtract %v2float %158 0
%165 = OpCompositeExtract %v2float %159 0
%166 = OpFOrdEqual %v2bool %164 %165
%167 = OpAll %bool %166
%168 = OpCompositeExtract %v2float %158 1
%169 = OpCompositeExtract %v2float %159 1
%170 = OpFOrdEqual %v2bool %168 %169
%171 = OpAll %bool %170
%172 = OpLogicalAnd %bool %167 %171
%173 = OpCompositeExtract %v2float %158 2
%174 = OpCompositeExtract %v2float %159 2
%175 = OpFOrdEqual %v2bool %173 %174
%176 = OpAll %bool %175
%177 = OpLogicalAnd %bool %172 %176
%178 = OpCompositeExtract %v2float %158 3
%179 = OpCompositeExtract %v2float %159 3
%180 = OpFOrdEqual %v2bool %178 %179
%181 = OpAll %bool %180
%182 = OpLogicalAnd %bool %177 %181
OpBranch %157
%157 = OpLabel
%183 = OpPhi %bool %false %123 %182 %156
OpStore %ok %183
%189 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%190 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%191 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%192 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%188 = OpCompositeConstruct %mat4v3float %189 %190 %191 %192
OpStore %m43 %188
%193 = OpLoad %bool %ok
OpSelectionMerge %195 None
OpBranchConditional %193 %194 %195
%194 = OpLabel
%196 = OpLoad %mat4v3float %m43
%198 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%199 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%200 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%201 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%197 = OpCompositeConstruct %mat4v3float %198 %199 %200 %201
%202 = OpCompositeExtract %v3float %196 0
%203 = OpCompositeExtract %v3float %197 0
%204 = OpFOrdEqual %v3bool %202 %203
%205 = OpAll %bool %204
%206 = OpCompositeExtract %v3float %196 1
%207 = OpCompositeExtract %v3float %197 1
%208 = OpFOrdEqual %v3bool %206 %207
%209 = OpAll %bool %208
%210 = OpLogicalAnd %bool %205 %209
%211 = OpCompositeExtract %v3float %196 2
%212 = OpCompositeExtract %v3float %197 2
%213 = OpFOrdEqual %v3bool %211 %212
%214 = OpAll %bool %213
%215 = OpLogicalAnd %bool %210 %214
%216 = OpCompositeExtract %v3float %196 3
%217 = OpCompositeExtract %v3float %197 3
%218 = OpFOrdEqual %v3bool %216 %217
%219 = OpAll %bool %218
%220 = OpLogicalAnd %bool %215 %219
OpBranch %195
%195 = OpLabel
%221 = OpPhi %bool %false %157 %220 %194
OpStore %ok %221
%225 = OpLoad %mat3v2float %m32
%226 = OpLoad %mat2v3float %m23
%227 = OpMatrixTimesMatrix %mat2v2float %225 %226
OpStore %m22 %227
%228 = OpLoad %bool %ok
OpSelectionMerge %230 None
OpBranchConditional %228 %229 %230
%229 = OpLabel
%231 = OpLoad %mat2v2float %m22
%234 = OpCompositeConstruct %v2float %float_8 %float_0
%235 = OpCompositeConstruct %v2float %float_0 %float_8
%233 = OpCompositeConstruct %mat2v2float %234 %235
%236 = OpCompositeExtract %v2float %231 0
%237 = OpCompositeExtract %v2float %233 0
%238 = OpFOrdEqual %v2bool %236 %237
%239 = OpAll %bool %238
%240 = OpCompositeExtract %v2float %231 1
%241 = OpCompositeExtract %v2float %233 1
%242 = OpFOrdEqual %v2bool %240 %241
%243 = OpAll %bool %242
%244 = OpLogicalAnd %bool %239 %243
OpBranch %230
%230 = OpLabel
%245 = OpPhi %bool %false %195 %244 %229
OpStore %ok %245
%249 = OpLoad %mat4v3float %m43
%250 = OpLoad %mat3v4float %m34
%251 = OpMatrixTimesMatrix %mat3v3float %249 %250
OpStore %m33 %251
%252 = OpLoad %bool %ok
OpSelectionMerge %254 None
OpBranchConditional %252 %253 %254
%253 = OpLabel
%255 = OpLoad %mat3v3float %m33
%258 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%259 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%260 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%257 = OpCompositeConstruct %mat3v3float %258 %259 %260
%261 = OpCompositeExtract %v3float %255 0
%262 = OpCompositeExtract %v3float %257 0
%263 = OpFOrdEqual %v3bool %261 %262
%264 = OpAll %bool %263
%265 = OpCompositeExtract %v3float %255 1
%266 = OpCompositeExtract %v3float %257 1
%267 = OpFOrdEqual %v3bool %265 %266
%268 = OpAll %bool %267
%269 = OpLogicalAnd %bool %264 %268
%270 = OpCompositeExtract %v3float %255 2
%271 = OpCompositeExtract %v3float %257 2
%272 = OpFOrdEqual %v3bool %270 %271
%273 = OpAll %bool %272
%274 = OpLogicalAnd %bool %269 %273
OpBranch %254
%254 = OpLabel
%275 = OpPhi %bool %false %230 %274 %253
OpStore %ok %275
%276 = OpLoad %mat2v3float %m23
%277 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%278 = OpCompositeConstruct %mat2v3float %277 %277
%279 = OpCompositeExtract %v3float %276 0
%280 = OpCompositeExtract %v3float %278 0
%281 = OpFAdd %v3float %279 %280
%282 = OpCompositeExtract %v3float %276 1
%283 = OpCompositeExtract %v3float %278 1
%284 = OpFAdd %v3float %282 %283
%285 = OpCompositeConstruct %mat2v3float %281 %284
OpStore %m23 %285
%286 = OpLoad %bool %ok
OpSelectionMerge %288 None
OpBranchConditional %286 %287 %288
%287 = OpLabel
%289 = OpLoad %mat2v3float %m23
%291 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%292 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%290 = OpCompositeConstruct %mat2v3float %291 %292
%293 = OpCompositeExtract %v3float %289 0
%294 = OpCompositeExtract %v3float %290 0
%295 = OpFOrdEqual %v3bool %293 %294
%296 = OpAll %bool %295
%297 = OpCompositeExtract %v3float %289 1
%298 = OpCompositeExtract %v3float %290 1
%299 = OpFOrdEqual %v3bool %297 %298
%300 = OpAll %bool %299
%301 = OpLogicalAnd %bool %296 %300
OpBranch %288
%288 = OpLabel
%302 = OpPhi %bool %false %254 %301 %287
OpStore %ok %302
%303 = OpLoad %mat3v2float %m32
%304 = OpCompositeConstruct %v2float %float_2 %float_2
%305 = OpCompositeConstruct %mat3v2float %304 %304 %304
%306 = OpCompositeExtract %v2float %303 0
%307 = OpCompositeExtract %v2float %305 0
%308 = OpFSub %v2float %306 %307
%309 = OpCompositeExtract %v2float %303 1
%310 = OpCompositeExtract %v2float %305 1
%311 = OpFSub %v2float %309 %310
%312 = OpCompositeExtract %v2float %303 2
%313 = OpCompositeExtract %v2float %305 2
%314 = OpFSub %v2float %312 %313
%315 = OpCompositeConstruct %mat3v2float %308 %311 %314
OpStore %m32 %315
%316 = OpLoad %bool %ok
OpSelectionMerge %318 None
OpBranchConditional %316 %317 %318
%317 = OpLabel
%319 = OpLoad %mat3v2float %m32
%322 = OpCompositeConstruct %v2float %float_2 %float_n2
%323 = OpCompositeConstruct %v2float %float_n2 %float_2
%324 = OpCompositeConstruct %v2float %float_n2 %float_n2
%321 = OpCompositeConstruct %mat3v2float %322 %323 %324
%325 = OpCompositeExtract %v2float %319 0
%326 = OpCompositeExtract %v2float %321 0
%327 = OpFOrdEqual %v2bool %325 %326
%328 = OpAll %bool %327
%329 = OpCompositeExtract %v2float %319 1
%330 = OpCompositeExtract %v2float %321 1
%331 = OpFOrdEqual %v2bool %329 %330
%332 = OpAll %bool %331
%333 = OpLogicalAnd %bool %328 %332
%334 = OpCompositeExtract %v2float %319 2
%335 = OpCompositeExtract %v2float %321 2
%336 = OpFOrdEqual %v2bool %334 %335
%337 = OpAll %bool %336
%338 = OpLogicalAnd %bool %333 %337
OpBranch %318
%318 = OpLabel
%339 = OpPhi %bool %false %288 %338 %317
OpStore %ok %339
%340 = OpLoad %mat2v4float %m24
%341 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%342 = OpCompositeConstruct %mat2v4float %341 %341
%343 = OpCompositeExtract %v4float %340 0
%344 = OpCompositeExtract %v4float %342 0
%345 = OpFDiv %v4float %343 %344
%346 = OpCompositeExtract %v4float %340 1
%347 = OpCompositeExtract %v4float %342 1
%348 = OpFDiv %v4float %346 %347
%349 = OpCompositeConstruct %mat2v4float %345 %348
OpStore %m24 %349
%350 = OpLoad %bool %ok
OpSelectionMerge %352 None
OpBranchConditional %350 %351 %352
%351 = OpLabel
%353 = OpLoad %mat2v4float %m24
%356 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%357 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%355 = OpCompositeConstruct %mat2v4float %356 %357
%358 = OpCompositeExtract %v4float %353 0
%359 = OpCompositeExtract %v4float %355 0
%360 = OpFOrdEqual %v4bool %358 %359
%361 = OpAll %bool %360
%362 = OpCompositeExtract %v4float %353 1
%363 = OpCompositeExtract %v4float %355 1
%364 = OpFOrdEqual %v4bool %362 %363
%365 = OpAll %bool %364
%366 = OpLogicalAnd %bool %361 %365
OpBranch %352
%352 = OpLabel
%367 = OpPhi %bool %false %318 %366 %351
OpStore %ok %367
%368 = OpLoad %bool %ok
OpReturnValue %368
OpFunctionEnd
%main = OpFunction %v4float None %369
%370 = OpFunctionParameter %_ptr_Function_v2float
%371 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%685 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%375 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%376 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%374 = OpCompositeConstruct %mat2v3float %375 %376
OpStore %_1_m23 %374
%377 = OpLoad %bool %_0_ok
OpSelectionMerge %379 None
OpBranchConditional %377 %378 %379
%378 = OpLabel
%380 = OpLoad %mat2v3float %_1_m23
%382 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%383 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%381 = OpCompositeConstruct %mat2v3float %382 %383
%384 = OpCompositeExtract %v3float %380 0
%385 = OpCompositeExtract %v3float %381 0
%386 = OpFOrdEqual %v3bool %384 %385
%387 = OpAll %bool %386
%388 = OpCompositeExtract %v3float %380 1
%389 = OpCompositeExtract %v3float %381 1
%390 = OpFOrdEqual %v3bool %388 %389
%391 = OpAll %bool %390
%392 = OpLogicalAnd %bool %387 %391
OpBranch %379
%379 = OpLabel
%393 = OpPhi %bool %false %371 %392 %378
OpStore %_0_ok %393
%396 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%397 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%395 = OpCompositeConstruct %mat2v4float %396 %397
OpStore %_2_m24 %395
%398 = OpLoad %bool %_0_ok
OpSelectionMerge %400 None
OpBranchConditional %398 %399 %400
%399 = OpLabel
%401 = OpLoad %mat2v4float %_2_m24
%403 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%404 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%402 = OpCompositeConstruct %mat2v4float %403 %404
%405 = OpCompositeExtract %v4float %401 0
%406 = OpCompositeExtract %v4float %402 0
%407 = OpFOrdEqual %v4bool %405 %406
%408 = OpAll %bool %407
%409 = OpCompositeExtract %v4float %401 1
%410 = OpCompositeExtract %v4float %402 1
%411 = OpFOrdEqual %v4bool %409 %410
%412 = OpAll %bool %411
%413 = OpLogicalAnd %bool %408 %412
OpBranch %400
%400 = OpLabel
%414 = OpPhi %bool %false %379 %413 %399
OpStore %_0_ok %414
%417 = OpCompositeConstruct %v2float %float_4 %float_0
%418 = OpCompositeConstruct %v2float %float_0 %float_4
%419 = OpCompositeConstruct %v2float %float_0 %float_0
%416 = OpCompositeConstruct %mat3v2float %417 %418 %419
OpStore %_3_m32 %416
%420 = OpLoad %bool %_0_ok
OpSelectionMerge %422 None
OpBranchConditional %420 %421 %422
%421 = OpLabel
%423 = OpLoad %mat3v2float %_3_m32
%425 = OpCompositeConstruct %v2float %float_4 %float_0
%426 = OpCompositeConstruct %v2float %float_0 %float_4
%427 = OpCompositeConstruct %v2float %float_0 %float_0
%424 = OpCompositeConstruct %mat3v2float %425 %426 %427
%428 = OpCompositeExtract %v2float %423 0
%429 = OpCompositeExtract %v2float %424 0
%430 = OpFOrdEqual %v2bool %428 %429
%431 = OpAll %bool %430
%432 = OpCompositeExtract %v2float %423 1
%433 = OpCompositeExtract %v2float %424 1
%434 = OpFOrdEqual %v2bool %432 %433
%435 = OpAll %bool %434
%436 = OpLogicalAnd %bool %431 %435
%437 = OpCompositeExtract %v2float %423 2
%438 = OpCompositeExtract %v2float %424 2
%439 = OpFOrdEqual %v2bool %437 %438
%440 = OpAll %bool %439
%441 = OpLogicalAnd %bool %436 %440
OpBranch %422
%422 = OpLabel
%442 = OpPhi %bool %false %400 %441 %421
OpStore %_0_ok %442
%445 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%446 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%447 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%444 = OpCompositeConstruct %mat3v4float %445 %446 %447
OpStore %_4_m34 %444
%448 = OpLoad %bool %_0_ok
OpSelectionMerge %450 None
OpBranchConditional %448 %449 %450
%449 = OpLabel
%451 = OpLoad %mat3v4float %_4_m34
%453 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%454 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%455 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%452 = OpCompositeConstruct %mat3v4float %453 %454 %455
%456 = OpCompositeExtract %v4float %451 0
%457 = OpCompositeExtract %v4float %452 0
%458 = OpFOrdEqual %v4bool %456 %457
%459 = OpAll %bool %458
%460 = OpCompositeExtract %v4float %451 1
%461 = OpCompositeExtract %v4float %452 1
%462 = OpFOrdEqual %v4bool %460 %461
%463 = OpAll %bool %462
%464 = OpLogicalAnd %bool %459 %463
%465 = OpCompositeExtract %v4float %451 2
%466 = OpCompositeExtract %v4float %452 2
%467 = OpFOrdEqual %v4bool %465 %466
%468 = OpAll %bool %467
%469 = OpLogicalAnd %bool %464 %468
OpBranch %450
%450 = OpLabel
%470 = OpPhi %bool %false %422 %469 %449
OpStore %_0_ok %470
%473 = OpCompositeConstruct %v2float %float_1 %float_0
%474 = OpCompositeConstruct %v2float %float_0 %float_1
%475 = OpCompositeConstruct %v2float %float_0 %float_0
%476 = OpCompositeConstruct %v2float %float_0 %float_0
%472 = OpCompositeConstruct %mat4v2float %473 %474 %475 %476
%477 = OpMatrixTimesScalar %mat4v2float %472 %float_6
OpStore %_5_m42 %477
%478 = OpLoad %bool %_0_ok
OpSelectionMerge %480 None
OpBranchConditional %478 %479 %480
%479 = OpLabel
%481 = OpLoad %mat4v2float %_5_m42
%483 = OpCompositeConstruct %v2float %float_6 %float_0
%484 = OpCompositeConstruct %v2float %float_0 %float_6
%485 = OpCompositeConstruct %v2float %float_0 %float_0
%486 = OpCompositeConstruct %v2float %float_0 %float_0
%482 = OpCompositeConstruct %mat4v2float %483 %484 %485 %486
%487 = OpCompositeExtract %v2float %481 0
%488 = OpCompositeExtract %v2float %482 0
%489 = OpFOrdEqual %v2bool %487 %488
%490 = OpAll %bool %489
%491 = OpCompositeExtract %v2float %481 1
%492 = OpCompositeExtract %v2float %482 1
%493 = OpFOrdEqual %v2bool %491 %492
%494 = OpAll %bool %493
%495 = OpLogicalAnd %bool %490 %494
%496 = OpCompositeExtract %v2float %481 2
%497 = OpCompositeExtract %v2float %482 2
%498 = OpFOrdEqual %v2bool %496 %497
%499 = OpAll %bool %498
%500 = OpLogicalAnd %bool %495 %499
%501 = OpCompositeExtract %v2float %481 3
%502 = OpCompositeExtract %v2float %482 3
%503 = OpFOrdEqual %v2bool %501 %502
%504 = OpAll %bool %503
%505 = OpLogicalAnd %bool %500 %504
OpBranch %480
%480 = OpLabel
%506 = OpPhi %bool %false %450 %505 %479
OpStore %_0_ok %506
%509 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%510 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%511 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%512 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%508 = OpCompositeConstruct %mat4v3float %509 %510 %511 %512
OpStore %_6_m43 %508
%513 = OpLoad %bool %_0_ok
OpSelectionMerge %515 None
OpBranchConditional %513 %514 %515
%514 = OpLabel
%516 = OpLoad %mat4v3float %_6_m43
%518 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%519 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%520 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%521 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%517 = OpCompositeConstruct %mat4v3float %518 %519 %520 %521
%522 = OpCompositeExtract %v3float %516 0
%523 = OpCompositeExtract %v3float %517 0
%524 = OpFOrdEqual %v3bool %522 %523
%525 = OpAll %bool %524
%526 = OpCompositeExtract %v3float %516 1
%527 = OpCompositeExtract %v3float %517 1
%528 = OpFOrdEqual %v3bool %526 %527
%529 = OpAll %bool %528
%530 = OpLogicalAnd %bool %525 %529
%531 = OpCompositeExtract %v3float %516 2
%532 = OpCompositeExtract %v3float %517 2
%533 = OpFOrdEqual %v3bool %531 %532
%534 = OpAll %bool %533
%535 = OpLogicalAnd %bool %530 %534
%536 = OpCompositeExtract %v3float %516 3
%537 = OpCompositeExtract %v3float %517 3
%538 = OpFOrdEqual %v3bool %536 %537
%539 = OpAll %bool %538
%540 = OpLogicalAnd %bool %535 %539
OpBranch %515
%515 = OpLabel
%541 = OpPhi %bool %false %480 %540 %514
OpStore %_0_ok %541
%543 = OpLoad %mat3v2float %_3_m32
%544 = OpLoad %mat2v3float %_1_m23
%545 = OpMatrixTimesMatrix %mat2v2float %543 %544
OpStore %_7_m22 %545
%546 = OpLoad %bool %_0_ok
OpSelectionMerge %548 None
OpBranchConditional %546 %547 %548
%547 = OpLabel
%549 = OpLoad %mat2v2float %_7_m22
%551 = OpCompositeConstruct %v2float %float_8 %float_0
%552 = OpCompositeConstruct %v2float %float_0 %float_8
%550 = OpCompositeConstruct %mat2v2float %551 %552
%553 = OpCompositeExtract %v2float %549 0
%554 = OpCompositeExtract %v2float %550 0
%555 = OpFOrdEqual %v2bool %553 %554
%556 = OpAll %bool %555
%557 = OpCompositeExtract %v2float %549 1
%558 = OpCompositeExtract %v2float %550 1
%559 = OpFOrdEqual %v2bool %557 %558
%560 = OpAll %bool %559
%561 = OpLogicalAnd %bool %556 %560
OpBranch %548
%548 = OpLabel
%562 = OpPhi %bool %false %515 %561 %547
OpStore %_0_ok %562
%564 = OpLoad %mat4v3float %_6_m43
%565 = OpLoad %mat3v4float %_4_m34
%566 = OpMatrixTimesMatrix %mat3v3float %564 %565
OpStore %_8_m33 %566
%567 = OpLoad %bool %_0_ok
OpSelectionMerge %569 None
OpBranchConditional %567 %568 %569
%568 = OpLabel
%570 = OpLoad %mat3v3float %_8_m33
%572 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%573 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%574 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%571 = OpCompositeConstruct %mat3v3float %572 %573 %574
%575 = OpCompositeExtract %v3float %570 0
%576 = OpCompositeExtract %v3float %571 0
%577 = OpFOrdEqual %v3bool %575 %576
%578 = OpAll %bool %577
%579 = OpCompositeExtract %v3float %570 1
%580 = OpCompositeExtract %v3float %571 1
%581 = OpFOrdEqual %v3bool %579 %580
%582 = OpAll %bool %581
%583 = OpLogicalAnd %bool %578 %582
%584 = OpCompositeExtract %v3float %570 2
%585 = OpCompositeExtract %v3float %571 2
%586 = OpFOrdEqual %v3bool %584 %585
%587 = OpAll %bool %586
%588 = OpLogicalAnd %bool %583 %587
OpBranch %569
%569 = OpLabel
%589 = OpPhi %bool %false %548 %588 %568
OpStore %_0_ok %589
%590 = OpLoad %mat2v3float %_1_m23
%591 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%592 = OpCompositeConstruct %mat2v3float %591 %591
%593 = OpCompositeExtract %v3float %590 0
%594 = OpCompositeExtract %v3float %592 0
%595 = OpFAdd %v3float %593 %594
%596 = OpCompositeExtract %v3float %590 1
%597 = OpCompositeExtract %v3float %592 1
%598 = OpFAdd %v3float %596 %597
%599 = OpCompositeConstruct %mat2v3float %595 %598
OpStore %_1_m23 %599
%600 = OpLoad %bool %_0_ok
OpSelectionMerge %602 None
OpBranchConditional %600 %601 %602
%601 = OpLabel
%603 = OpLoad %mat2v3float %_1_m23
%605 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%606 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%604 = OpCompositeConstruct %mat2v3float %605 %606
%607 = OpCompositeExtract %v3float %603 0
%608 = OpCompositeExtract %v3float %604 0
%609 = OpFOrdEqual %v3bool %607 %608
%610 = OpAll %bool %609
%611 = OpCompositeExtract %v3float %603 1
%612 = OpCompositeExtract %v3float %604 1
%613 = OpFOrdEqual %v3bool %611 %612
%614 = OpAll %bool %613
%615 = OpLogicalAnd %bool %610 %614
OpBranch %602
%602 = OpLabel
%616 = OpPhi %bool %false %569 %615 %601
OpStore %_0_ok %616
%617 = OpLoad %mat3v2float %_3_m32
%618 = OpCompositeConstruct %v2float %float_2 %float_2
%619 = OpCompositeConstruct %mat3v2float %618 %618 %618
%620 = OpCompositeExtract %v2float %617 0
%621 = OpCompositeExtract %v2float %619 0
%622 = OpFSub %v2float %620 %621
%623 = OpCompositeExtract %v2float %617 1
%624 = OpCompositeExtract %v2float %619 1
%625 = OpFSub %v2float %623 %624
%626 = OpCompositeExtract %v2float %617 2
%627 = OpCompositeExtract %v2float %619 2
%628 = OpFSub %v2float %626 %627
%629 = OpCompositeConstruct %mat3v2float %622 %625 %628
OpStore %_3_m32 %629
%630 = OpLoad %bool %_0_ok
OpSelectionMerge %632 None
OpBranchConditional %630 %631 %632
%631 = OpLabel
%633 = OpLoad %mat3v2float %_3_m32
%635 = OpCompositeConstruct %v2float %float_2 %float_n2
%636 = OpCompositeConstruct %v2float %float_n2 %float_2
%637 = OpCompositeConstruct %v2float %float_n2 %float_n2
%634 = OpCompositeConstruct %mat3v2float %635 %636 %637
%638 = OpCompositeExtract %v2float %633 0
%639 = OpCompositeExtract %v2float %634 0
%640 = OpFOrdEqual %v2bool %638 %639
%641 = OpAll %bool %640
%642 = OpCompositeExtract %v2float %633 1
%643 = OpCompositeExtract %v2float %634 1
%644 = OpFOrdEqual %v2bool %642 %643
%645 = OpAll %bool %644
%646 = OpLogicalAnd %bool %641 %645
%647 = OpCompositeExtract %v2float %633 2
%648 = OpCompositeExtract %v2float %634 2
%649 = OpFOrdEqual %v2bool %647 %648
%650 = OpAll %bool %649
%651 = OpLogicalAnd %bool %646 %650
OpBranch %632
%632 = OpLabel
%652 = OpPhi %bool %false %602 %651 %631
OpStore %_0_ok %652
%653 = OpLoad %mat2v4float %_2_m24
%654 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%655 = OpCompositeConstruct %mat2v4float %654 %654
%656 = OpCompositeExtract %v4float %653 0
%657 = OpCompositeExtract %v4float %655 0
%658 = OpFDiv %v4float %656 %657
%659 = OpCompositeExtract %v4float %653 1
%660 = OpCompositeExtract %v4float %655 1
%661 = OpFDiv %v4float %659 %660
%662 = OpCompositeConstruct %mat2v4float %658 %661
OpStore %_2_m24 %662
%663 = OpLoad %bool %_0_ok
OpSelectionMerge %665 None
OpBranchConditional %663 %664 %665
%664 = OpLabel
%666 = OpLoad %mat2v4float %_2_m24
%668 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%669 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%667 = OpCompositeConstruct %mat2v4float %668 %669
%670 = OpCompositeExtract %v4float %666 0
%671 = OpCompositeExtract %v4float %667 0
%672 = OpFOrdEqual %v4bool %670 %671
%673 = OpAll %bool %672
%674 = OpCompositeExtract %v4float %666 1
%675 = OpCompositeExtract %v4float %667 1
%676 = OpFOrdEqual %v4bool %674 %675
%677 = OpAll %bool %676
%678 = OpLogicalAnd %bool %673 %677
OpBranch %665
%665 = OpLabel
%679 = OpPhi %bool %false %632 %678 %664
OpStore %_0_ok %679
%680 = OpLoad %bool %_0_ok
OpSelectionMerge %682 None
OpBranchConditional %680 %681 %682
%681 = OpLabel
%683 = OpFunctionCall %bool %test_half_b
OpBranch %682
%682 = OpLabel
%684 = OpPhi %bool %false %665 %683 %681
OpSelectionMerge %689 None
OpBranchConditional %684 %687 %688
%687 = OpLabel
%690 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%694 = OpLoad %v4float %690
OpStore %685 %694
OpBranch %689
%688 = OpLabel
%695 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%697 = OpLoad %v4float %695
OpStore %685 %697
OpBranch %689
%689 = OpLabel
%698 = OpLoad %v4float %685
OpReturnValue %698
OpFunctionEnd
