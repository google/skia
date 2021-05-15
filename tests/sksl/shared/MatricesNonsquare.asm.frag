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
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %m22 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
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
OpDecorate %317 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %482 RelaxedPrecision
OpDecorate %517 RelaxedPrecision
OpDecorate %550 RelaxedPrecision
OpDecorate %571 RelaxedPrecision
OpDecorate %604 RelaxedPrecision
OpDecorate %634 RelaxedPrecision
OpDecorate %667 RelaxedPrecision
OpDecorate %684 RelaxedPrecision
OpDecorate %698 RelaxedPrecision
OpDecorate %701 RelaxedPrecision
OpDecorate %702 RelaxedPrecision
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
%float_6 = OpConstant %float 6
%mat2v2float = OpTypeMatrix %v2float 2
%float_1 = OpConstant %float 1
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_8 = OpConstant %float 8
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%float_n2 = OpConstant %float -2
%float_0_75 = OpConstant %float 0.75
%371 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%149 = OpCompositeConstruct %v2float %float_6 %float_0
%150 = OpCompositeConstruct %v2float %float_0 %float_6
%148 = OpCompositeConstruct %mat2v2float %149 %150
%154 = OpCompositeExtract %v2float %148 0
%155 = OpCompositeExtract %v2float %148 1
%156 = OpCompositeConstruct %v2float %float_0 %float_0
%157 = OpCompositeConstruct %v2float %float_0 %float_0
%152 = OpCompositeConstruct %mat4v2float %154 %155 %156 %157
OpStore %m42 %152
%158 = OpLoad %bool %ok
OpSelectionMerge %160 None
OpBranchConditional %158 %159 %160
%159 = OpLabel
%161 = OpLoad %mat4v2float %m42
%163 = OpCompositeConstruct %v2float %float_6 %float_0
%164 = OpCompositeConstruct %v2float %float_0 %float_6
%165 = OpCompositeConstruct %v2float %float_0 %float_0
%166 = OpCompositeConstruct %v2float %float_0 %float_0
%162 = OpCompositeConstruct %mat4v2float %163 %164 %165 %166
%167 = OpCompositeExtract %v2float %161 0
%168 = OpCompositeExtract %v2float %162 0
%169 = OpFOrdEqual %v2bool %167 %168
%170 = OpAll %bool %169
%171 = OpCompositeExtract %v2float %161 1
%172 = OpCompositeExtract %v2float %162 1
%173 = OpFOrdEqual %v2bool %171 %172
%174 = OpAll %bool %173
%175 = OpLogicalAnd %bool %170 %174
%176 = OpCompositeExtract %v2float %161 2
%177 = OpCompositeExtract %v2float %162 2
%178 = OpFOrdEqual %v2bool %176 %177
%179 = OpAll %bool %178
%180 = OpLogicalAnd %bool %175 %179
%181 = OpCompositeExtract %v2float %161 3
%182 = OpCompositeExtract %v2float %162 3
%183 = OpFOrdEqual %v2bool %181 %182
%184 = OpAll %bool %183
%185 = OpLogicalAnd %bool %180 %184
OpBranch %160
%160 = OpLabel
%186 = OpPhi %bool %false %123 %185 %159
OpStore %ok %186
%192 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%193 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%194 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%195 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%191 = OpCompositeConstruct %mat4v3float %192 %193 %194 %195
OpStore %m43 %191
%196 = OpLoad %bool %ok
OpSelectionMerge %198 None
OpBranchConditional %196 %197 %198
%197 = OpLabel
%199 = OpLoad %mat4v3float %m43
%201 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%202 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%203 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%204 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%200 = OpCompositeConstruct %mat4v3float %201 %202 %203 %204
%205 = OpCompositeExtract %v3float %199 0
%206 = OpCompositeExtract %v3float %200 0
%207 = OpFOrdEqual %v3bool %205 %206
%208 = OpAll %bool %207
%209 = OpCompositeExtract %v3float %199 1
%210 = OpCompositeExtract %v3float %200 1
%211 = OpFOrdEqual %v3bool %209 %210
%212 = OpAll %bool %211
%213 = OpLogicalAnd %bool %208 %212
%214 = OpCompositeExtract %v3float %199 2
%215 = OpCompositeExtract %v3float %200 2
%216 = OpFOrdEqual %v3bool %214 %215
%217 = OpAll %bool %216
%218 = OpLogicalAnd %bool %213 %217
%219 = OpCompositeExtract %v3float %199 3
%220 = OpCompositeExtract %v3float %200 3
%221 = OpFOrdEqual %v3bool %219 %220
%222 = OpAll %bool %221
%223 = OpLogicalAnd %bool %218 %222
OpBranch %198
%198 = OpLabel
%224 = OpPhi %bool %false %160 %223 %197
OpStore %ok %224
%227 = OpLoad %mat3v2float %m32
%228 = OpLoad %mat2v3float %m23
%229 = OpMatrixTimesMatrix %mat2v2float %227 %228
OpStore %m22 %229
%230 = OpLoad %bool %ok
OpSelectionMerge %232 None
OpBranchConditional %230 %231 %232
%231 = OpLabel
%233 = OpLoad %mat2v2float %m22
%236 = OpCompositeConstruct %v2float %float_8 %float_0
%237 = OpCompositeConstruct %v2float %float_0 %float_8
%235 = OpCompositeConstruct %mat2v2float %236 %237
%238 = OpCompositeExtract %v2float %233 0
%239 = OpCompositeExtract %v2float %235 0
%240 = OpFOrdEqual %v2bool %238 %239
%241 = OpAll %bool %240
%242 = OpCompositeExtract %v2float %233 1
%243 = OpCompositeExtract %v2float %235 1
%244 = OpFOrdEqual %v2bool %242 %243
%245 = OpAll %bool %244
%246 = OpLogicalAnd %bool %241 %245
OpBranch %232
%232 = OpLabel
%247 = OpPhi %bool %false %198 %246 %231
OpStore %ok %247
%251 = OpLoad %mat4v3float %m43
%252 = OpLoad %mat3v4float %m34
%253 = OpMatrixTimesMatrix %mat3v3float %251 %252
OpStore %m33 %253
%254 = OpLoad %bool %ok
OpSelectionMerge %256 None
OpBranchConditional %254 %255 %256
%255 = OpLabel
%257 = OpLoad %mat3v3float %m33
%260 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%261 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%262 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%259 = OpCompositeConstruct %mat3v3float %260 %261 %262
%263 = OpCompositeExtract %v3float %257 0
%264 = OpCompositeExtract %v3float %259 0
%265 = OpFOrdEqual %v3bool %263 %264
%266 = OpAll %bool %265
%267 = OpCompositeExtract %v3float %257 1
%268 = OpCompositeExtract %v3float %259 1
%269 = OpFOrdEqual %v3bool %267 %268
%270 = OpAll %bool %269
%271 = OpLogicalAnd %bool %266 %270
%272 = OpCompositeExtract %v3float %257 2
%273 = OpCompositeExtract %v3float %259 2
%274 = OpFOrdEqual %v3bool %272 %273
%275 = OpAll %bool %274
%276 = OpLogicalAnd %bool %271 %275
OpBranch %256
%256 = OpLabel
%277 = OpPhi %bool %false %232 %276 %255
OpStore %ok %277
%278 = OpLoad %mat2v3float %m23
%279 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%280 = OpCompositeConstruct %mat2v3float %279 %279
%281 = OpCompositeExtract %v3float %278 0
%282 = OpCompositeExtract %v3float %280 0
%283 = OpFAdd %v3float %281 %282
%284 = OpCompositeExtract %v3float %278 1
%285 = OpCompositeExtract %v3float %280 1
%286 = OpFAdd %v3float %284 %285
%287 = OpCompositeConstruct %mat2v3float %283 %286
OpStore %m23 %287
%288 = OpLoad %bool %ok
OpSelectionMerge %290 None
OpBranchConditional %288 %289 %290
%289 = OpLabel
%291 = OpLoad %mat2v3float %m23
%293 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%294 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%292 = OpCompositeConstruct %mat2v3float %293 %294
%295 = OpCompositeExtract %v3float %291 0
%296 = OpCompositeExtract %v3float %292 0
%297 = OpFOrdEqual %v3bool %295 %296
%298 = OpAll %bool %297
%299 = OpCompositeExtract %v3float %291 1
%300 = OpCompositeExtract %v3float %292 1
%301 = OpFOrdEqual %v3bool %299 %300
%302 = OpAll %bool %301
%303 = OpLogicalAnd %bool %298 %302
OpBranch %290
%290 = OpLabel
%304 = OpPhi %bool %false %256 %303 %289
OpStore %ok %304
%305 = OpLoad %mat3v2float %m32
%306 = OpCompositeConstruct %v2float %float_2 %float_2
%307 = OpCompositeConstruct %mat3v2float %306 %306 %306
%308 = OpCompositeExtract %v2float %305 0
%309 = OpCompositeExtract %v2float %307 0
%310 = OpFSub %v2float %308 %309
%311 = OpCompositeExtract %v2float %305 1
%312 = OpCompositeExtract %v2float %307 1
%313 = OpFSub %v2float %311 %312
%314 = OpCompositeExtract %v2float %305 2
%315 = OpCompositeExtract %v2float %307 2
%316 = OpFSub %v2float %314 %315
%317 = OpCompositeConstruct %mat3v2float %310 %313 %316
OpStore %m32 %317
%318 = OpLoad %bool %ok
OpSelectionMerge %320 None
OpBranchConditional %318 %319 %320
%319 = OpLabel
%321 = OpLoad %mat3v2float %m32
%324 = OpCompositeConstruct %v2float %float_2 %float_n2
%325 = OpCompositeConstruct %v2float %float_n2 %float_2
%326 = OpCompositeConstruct %v2float %float_n2 %float_n2
%323 = OpCompositeConstruct %mat3v2float %324 %325 %326
%327 = OpCompositeExtract %v2float %321 0
%328 = OpCompositeExtract %v2float %323 0
%329 = OpFOrdEqual %v2bool %327 %328
%330 = OpAll %bool %329
%331 = OpCompositeExtract %v2float %321 1
%332 = OpCompositeExtract %v2float %323 1
%333 = OpFOrdEqual %v2bool %331 %332
%334 = OpAll %bool %333
%335 = OpLogicalAnd %bool %330 %334
%336 = OpCompositeExtract %v2float %321 2
%337 = OpCompositeExtract %v2float %323 2
%338 = OpFOrdEqual %v2bool %336 %337
%339 = OpAll %bool %338
%340 = OpLogicalAnd %bool %335 %339
OpBranch %320
%320 = OpLabel
%341 = OpPhi %bool %false %290 %340 %319
OpStore %ok %341
%342 = OpLoad %mat2v4float %m24
%343 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%344 = OpCompositeConstruct %mat2v4float %343 %343
%345 = OpCompositeExtract %v4float %342 0
%346 = OpCompositeExtract %v4float %344 0
%347 = OpFDiv %v4float %345 %346
%348 = OpCompositeExtract %v4float %342 1
%349 = OpCompositeExtract %v4float %344 1
%350 = OpFDiv %v4float %348 %349
%351 = OpCompositeConstruct %mat2v4float %347 %350
OpStore %m24 %351
%352 = OpLoad %bool %ok
OpSelectionMerge %354 None
OpBranchConditional %352 %353 %354
%353 = OpLabel
%355 = OpLoad %mat2v4float %m24
%358 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%359 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%357 = OpCompositeConstruct %mat2v4float %358 %359
%360 = OpCompositeExtract %v4float %355 0
%361 = OpCompositeExtract %v4float %357 0
%362 = OpFOrdEqual %v4bool %360 %361
%363 = OpAll %bool %362
%364 = OpCompositeExtract %v4float %355 1
%365 = OpCompositeExtract %v4float %357 1
%366 = OpFOrdEqual %v4bool %364 %365
%367 = OpAll %bool %366
%368 = OpLogicalAnd %bool %363 %367
OpBranch %354
%354 = OpLabel
%369 = OpPhi %bool %false %320 %368 %353
OpStore %ok %369
%370 = OpLoad %bool %ok
OpReturnValue %370
OpFunctionEnd
%main = OpFunction %v4float None %371
%372 = OpFunctionParameter %_ptr_Function_v2float
%373 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%689 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%377 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%378 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%376 = OpCompositeConstruct %mat2v3float %377 %378
OpStore %_1_m23 %376
%379 = OpLoad %bool %_0_ok
OpSelectionMerge %381 None
OpBranchConditional %379 %380 %381
%380 = OpLabel
%382 = OpLoad %mat2v3float %_1_m23
%384 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%385 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%383 = OpCompositeConstruct %mat2v3float %384 %385
%386 = OpCompositeExtract %v3float %382 0
%387 = OpCompositeExtract %v3float %383 0
%388 = OpFOrdEqual %v3bool %386 %387
%389 = OpAll %bool %388
%390 = OpCompositeExtract %v3float %382 1
%391 = OpCompositeExtract %v3float %383 1
%392 = OpFOrdEqual %v3bool %390 %391
%393 = OpAll %bool %392
%394 = OpLogicalAnd %bool %389 %393
OpBranch %381
%381 = OpLabel
%395 = OpPhi %bool %false %373 %394 %380
OpStore %_0_ok %395
%398 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%399 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%397 = OpCompositeConstruct %mat2v4float %398 %399
OpStore %_2_m24 %397
%400 = OpLoad %bool %_0_ok
OpSelectionMerge %402 None
OpBranchConditional %400 %401 %402
%401 = OpLabel
%403 = OpLoad %mat2v4float %_2_m24
%405 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%406 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%404 = OpCompositeConstruct %mat2v4float %405 %406
%407 = OpCompositeExtract %v4float %403 0
%408 = OpCompositeExtract %v4float %404 0
%409 = OpFOrdEqual %v4bool %407 %408
%410 = OpAll %bool %409
%411 = OpCompositeExtract %v4float %403 1
%412 = OpCompositeExtract %v4float %404 1
%413 = OpFOrdEqual %v4bool %411 %412
%414 = OpAll %bool %413
%415 = OpLogicalAnd %bool %410 %414
OpBranch %402
%402 = OpLabel
%416 = OpPhi %bool %false %381 %415 %401
OpStore %_0_ok %416
%419 = OpCompositeConstruct %v2float %float_4 %float_0
%420 = OpCompositeConstruct %v2float %float_0 %float_4
%421 = OpCompositeConstruct %v2float %float_0 %float_0
%418 = OpCompositeConstruct %mat3v2float %419 %420 %421
OpStore %_3_m32 %418
%422 = OpLoad %bool %_0_ok
OpSelectionMerge %424 None
OpBranchConditional %422 %423 %424
%423 = OpLabel
%425 = OpLoad %mat3v2float %_3_m32
%427 = OpCompositeConstruct %v2float %float_4 %float_0
%428 = OpCompositeConstruct %v2float %float_0 %float_4
%429 = OpCompositeConstruct %v2float %float_0 %float_0
%426 = OpCompositeConstruct %mat3v2float %427 %428 %429
%430 = OpCompositeExtract %v2float %425 0
%431 = OpCompositeExtract %v2float %426 0
%432 = OpFOrdEqual %v2bool %430 %431
%433 = OpAll %bool %432
%434 = OpCompositeExtract %v2float %425 1
%435 = OpCompositeExtract %v2float %426 1
%436 = OpFOrdEqual %v2bool %434 %435
%437 = OpAll %bool %436
%438 = OpLogicalAnd %bool %433 %437
%439 = OpCompositeExtract %v2float %425 2
%440 = OpCompositeExtract %v2float %426 2
%441 = OpFOrdEqual %v2bool %439 %440
%442 = OpAll %bool %441
%443 = OpLogicalAnd %bool %438 %442
OpBranch %424
%424 = OpLabel
%444 = OpPhi %bool %false %402 %443 %423
OpStore %_0_ok %444
%447 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%448 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%449 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%446 = OpCompositeConstruct %mat3v4float %447 %448 %449
OpStore %_4_m34 %446
%450 = OpLoad %bool %_0_ok
OpSelectionMerge %452 None
OpBranchConditional %450 %451 %452
%451 = OpLabel
%453 = OpLoad %mat3v4float %_4_m34
%455 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%456 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%457 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%454 = OpCompositeConstruct %mat3v4float %455 %456 %457
%458 = OpCompositeExtract %v4float %453 0
%459 = OpCompositeExtract %v4float %454 0
%460 = OpFOrdEqual %v4bool %458 %459
%461 = OpAll %bool %460
%462 = OpCompositeExtract %v4float %453 1
%463 = OpCompositeExtract %v4float %454 1
%464 = OpFOrdEqual %v4bool %462 %463
%465 = OpAll %bool %464
%466 = OpLogicalAnd %bool %461 %465
%467 = OpCompositeExtract %v4float %453 2
%468 = OpCompositeExtract %v4float %454 2
%469 = OpFOrdEqual %v4bool %467 %468
%470 = OpAll %bool %469
%471 = OpLogicalAnd %bool %466 %470
OpBranch %452
%452 = OpLabel
%472 = OpPhi %bool %false %424 %471 %451
OpStore %_0_ok %472
%475 = OpCompositeConstruct %v2float %float_6 %float_0
%476 = OpCompositeConstruct %v2float %float_0 %float_6
%474 = OpCompositeConstruct %mat2v2float %475 %476
%478 = OpCompositeExtract %v2float %474 0
%479 = OpCompositeExtract %v2float %474 1
%480 = OpCompositeConstruct %v2float %float_0 %float_0
%481 = OpCompositeConstruct %v2float %float_0 %float_0
%477 = OpCompositeConstruct %mat4v2float %478 %479 %480 %481
OpStore %_5_m42 %477
%482 = OpLoad %bool %_0_ok
OpSelectionMerge %484 None
OpBranchConditional %482 %483 %484
%483 = OpLabel
%485 = OpLoad %mat4v2float %_5_m42
%487 = OpCompositeConstruct %v2float %float_6 %float_0
%488 = OpCompositeConstruct %v2float %float_0 %float_6
%489 = OpCompositeConstruct %v2float %float_0 %float_0
%490 = OpCompositeConstruct %v2float %float_0 %float_0
%486 = OpCompositeConstruct %mat4v2float %487 %488 %489 %490
%491 = OpCompositeExtract %v2float %485 0
%492 = OpCompositeExtract %v2float %486 0
%493 = OpFOrdEqual %v2bool %491 %492
%494 = OpAll %bool %493
%495 = OpCompositeExtract %v2float %485 1
%496 = OpCompositeExtract %v2float %486 1
%497 = OpFOrdEqual %v2bool %495 %496
%498 = OpAll %bool %497
%499 = OpLogicalAnd %bool %494 %498
%500 = OpCompositeExtract %v2float %485 2
%501 = OpCompositeExtract %v2float %486 2
%502 = OpFOrdEqual %v2bool %500 %501
%503 = OpAll %bool %502
%504 = OpLogicalAnd %bool %499 %503
%505 = OpCompositeExtract %v2float %485 3
%506 = OpCompositeExtract %v2float %486 3
%507 = OpFOrdEqual %v2bool %505 %506
%508 = OpAll %bool %507
%509 = OpLogicalAnd %bool %504 %508
OpBranch %484
%484 = OpLabel
%510 = OpPhi %bool %false %452 %509 %483
OpStore %_0_ok %510
%513 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%514 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%515 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%516 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%512 = OpCompositeConstruct %mat4v3float %513 %514 %515 %516
OpStore %_6_m43 %512
%517 = OpLoad %bool %_0_ok
OpSelectionMerge %519 None
OpBranchConditional %517 %518 %519
%518 = OpLabel
%520 = OpLoad %mat4v3float %_6_m43
%522 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%523 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%524 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%525 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%521 = OpCompositeConstruct %mat4v3float %522 %523 %524 %525
%526 = OpCompositeExtract %v3float %520 0
%527 = OpCompositeExtract %v3float %521 0
%528 = OpFOrdEqual %v3bool %526 %527
%529 = OpAll %bool %528
%530 = OpCompositeExtract %v3float %520 1
%531 = OpCompositeExtract %v3float %521 1
%532 = OpFOrdEqual %v3bool %530 %531
%533 = OpAll %bool %532
%534 = OpLogicalAnd %bool %529 %533
%535 = OpCompositeExtract %v3float %520 2
%536 = OpCompositeExtract %v3float %521 2
%537 = OpFOrdEqual %v3bool %535 %536
%538 = OpAll %bool %537
%539 = OpLogicalAnd %bool %534 %538
%540 = OpCompositeExtract %v3float %520 3
%541 = OpCompositeExtract %v3float %521 3
%542 = OpFOrdEqual %v3bool %540 %541
%543 = OpAll %bool %542
%544 = OpLogicalAnd %bool %539 %543
OpBranch %519
%519 = OpLabel
%545 = OpPhi %bool %false %484 %544 %518
OpStore %_0_ok %545
%547 = OpLoad %mat3v2float %_3_m32
%548 = OpLoad %mat2v3float %_1_m23
%549 = OpMatrixTimesMatrix %mat2v2float %547 %548
OpStore %_7_m22 %549
%550 = OpLoad %bool %_0_ok
OpSelectionMerge %552 None
OpBranchConditional %550 %551 %552
%551 = OpLabel
%553 = OpLoad %mat2v2float %_7_m22
%555 = OpCompositeConstruct %v2float %float_8 %float_0
%556 = OpCompositeConstruct %v2float %float_0 %float_8
%554 = OpCompositeConstruct %mat2v2float %555 %556
%557 = OpCompositeExtract %v2float %553 0
%558 = OpCompositeExtract %v2float %554 0
%559 = OpFOrdEqual %v2bool %557 %558
%560 = OpAll %bool %559
%561 = OpCompositeExtract %v2float %553 1
%562 = OpCompositeExtract %v2float %554 1
%563 = OpFOrdEqual %v2bool %561 %562
%564 = OpAll %bool %563
%565 = OpLogicalAnd %bool %560 %564
OpBranch %552
%552 = OpLabel
%566 = OpPhi %bool %false %519 %565 %551
OpStore %_0_ok %566
%568 = OpLoad %mat4v3float %_6_m43
%569 = OpLoad %mat3v4float %_4_m34
%570 = OpMatrixTimesMatrix %mat3v3float %568 %569
OpStore %_8_m33 %570
%571 = OpLoad %bool %_0_ok
OpSelectionMerge %573 None
OpBranchConditional %571 %572 %573
%572 = OpLabel
%574 = OpLoad %mat3v3float %_8_m33
%576 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%577 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%578 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%575 = OpCompositeConstruct %mat3v3float %576 %577 %578
%579 = OpCompositeExtract %v3float %574 0
%580 = OpCompositeExtract %v3float %575 0
%581 = OpFOrdEqual %v3bool %579 %580
%582 = OpAll %bool %581
%583 = OpCompositeExtract %v3float %574 1
%584 = OpCompositeExtract %v3float %575 1
%585 = OpFOrdEqual %v3bool %583 %584
%586 = OpAll %bool %585
%587 = OpLogicalAnd %bool %582 %586
%588 = OpCompositeExtract %v3float %574 2
%589 = OpCompositeExtract %v3float %575 2
%590 = OpFOrdEqual %v3bool %588 %589
%591 = OpAll %bool %590
%592 = OpLogicalAnd %bool %587 %591
OpBranch %573
%573 = OpLabel
%593 = OpPhi %bool %false %552 %592 %572
OpStore %_0_ok %593
%594 = OpLoad %mat2v3float %_1_m23
%595 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%596 = OpCompositeConstruct %mat2v3float %595 %595
%597 = OpCompositeExtract %v3float %594 0
%598 = OpCompositeExtract %v3float %596 0
%599 = OpFAdd %v3float %597 %598
%600 = OpCompositeExtract %v3float %594 1
%601 = OpCompositeExtract %v3float %596 1
%602 = OpFAdd %v3float %600 %601
%603 = OpCompositeConstruct %mat2v3float %599 %602
OpStore %_1_m23 %603
%604 = OpLoad %bool %_0_ok
OpSelectionMerge %606 None
OpBranchConditional %604 %605 %606
%605 = OpLabel
%607 = OpLoad %mat2v3float %_1_m23
%609 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%610 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%608 = OpCompositeConstruct %mat2v3float %609 %610
%611 = OpCompositeExtract %v3float %607 0
%612 = OpCompositeExtract %v3float %608 0
%613 = OpFOrdEqual %v3bool %611 %612
%614 = OpAll %bool %613
%615 = OpCompositeExtract %v3float %607 1
%616 = OpCompositeExtract %v3float %608 1
%617 = OpFOrdEqual %v3bool %615 %616
%618 = OpAll %bool %617
%619 = OpLogicalAnd %bool %614 %618
OpBranch %606
%606 = OpLabel
%620 = OpPhi %bool %false %573 %619 %605
OpStore %_0_ok %620
%621 = OpLoad %mat3v2float %_3_m32
%622 = OpCompositeConstruct %v2float %float_2 %float_2
%623 = OpCompositeConstruct %mat3v2float %622 %622 %622
%624 = OpCompositeExtract %v2float %621 0
%625 = OpCompositeExtract %v2float %623 0
%626 = OpFSub %v2float %624 %625
%627 = OpCompositeExtract %v2float %621 1
%628 = OpCompositeExtract %v2float %623 1
%629 = OpFSub %v2float %627 %628
%630 = OpCompositeExtract %v2float %621 2
%631 = OpCompositeExtract %v2float %623 2
%632 = OpFSub %v2float %630 %631
%633 = OpCompositeConstruct %mat3v2float %626 %629 %632
OpStore %_3_m32 %633
%634 = OpLoad %bool %_0_ok
OpSelectionMerge %636 None
OpBranchConditional %634 %635 %636
%635 = OpLabel
%637 = OpLoad %mat3v2float %_3_m32
%639 = OpCompositeConstruct %v2float %float_2 %float_n2
%640 = OpCompositeConstruct %v2float %float_n2 %float_2
%641 = OpCompositeConstruct %v2float %float_n2 %float_n2
%638 = OpCompositeConstruct %mat3v2float %639 %640 %641
%642 = OpCompositeExtract %v2float %637 0
%643 = OpCompositeExtract %v2float %638 0
%644 = OpFOrdEqual %v2bool %642 %643
%645 = OpAll %bool %644
%646 = OpCompositeExtract %v2float %637 1
%647 = OpCompositeExtract %v2float %638 1
%648 = OpFOrdEqual %v2bool %646 %647
%649 = OpAll %bool %648
%650 = OpLogicalAnd %bool %645 %649
%651 = OpCompositeExtract %v2float %637 2
%652 = OpCompositeExtract %v2float %638 2
%653 = OpFOrdEqual %v2bool %651 %652
%654 = OpAll %bool %653
%655 = OpLogicalAnd %bool %650 %654
OpBranch %636
%636 = OpLabel
%656 = OpPhi %bool %false %606 %655 %635
OpStore %_0_ok %656
%657 = OpLoad %mat2v4float %_2_m24
%658 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%659 = OpCompositeConstruct %mat2v4float %658 %658
%660 = OpCompositeExtract %v4float %657 0
%661 = OpCompositeExtract %v4float %659 0
%662 = OpFDiv %v4float %660 %661
%663 = OpCompositeExtract %v4float %657 1
%664 = OpCompositeExtract %v4float %659 1
%665 = OpFDiv %v4float %663 %664
%666 = OpCompositeConstruct %mat2v4float %662 %665
OpStore %_2_m24 %666
%667 = OpLoad %bool %_0_ok
OpSelectionMerge %669 None
OpBranchConditional %667 %668 %669
%668 = OpLabel
%670 = OpLoad %mat2v4float %_2_m24
%672 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%673 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%671 = OpCompositeConstruct %mat2v4float %672 %673
%674 = OpCompositeExtract %v4float %670 0
%675 = OpCompositeExtract %v4float %671 0
%676 = OpFOrdEqual %v4bool %674 %675
%677 = OpAll %bool %676
%678 = OpCompositeExtract %v4float %670 1
%679 = OpCompositeExtract %v4float %671 1
%680 = OpFOrdEqual %v4bool %678 %679
%681 = OpAll %bool %680
%682 = OpLogicalAnd %bool %677 %681
OpBranch %669
%669 = OpLabel
%683 = OpPhi %bool %false %636 %682 %668
OpStore %_0_ok %683
%684 = OpLoad %bool %_0_ok
OpSelectionMerge %686 None
OpBranchConditional %684 %685 %686
%685 = OpLabel
%687 = OpFunctionCall %bool %test_half_b
OpBranch %686
%686 = OpLabel
%688 = OpPhi %bool %false %669 %687 %685
OpSelectionMerge %693 None
OpBranchConditional %688 %691 %692
%691 = OpLabel
%694 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%698 = OpLoad %v4float %694
OpStore %689 %698
OpBranch %693
%692 = OpLabel
%699 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%701 = OpLoad %v4float %699
OpStore %689 %701
OpBranch %693
%693 = OpLabel
%702 = OpLoad %v4float %689
OpReturnValue %702
OpFunctionEnd
