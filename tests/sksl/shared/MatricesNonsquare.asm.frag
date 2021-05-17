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
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
OpDecorate %492 RelaxedPrecision
OpDecorate %525 RelaxedPrecision
OpDecorate %555 RelaxedPrecision
OpDecorate %588 RelaxedPrecision
OpDecorate %605 RelaxedPrecision
OpDecorate %619 RelaxedPrecision
OpDecorate %622 RelaxedPrecision
OpDecorate %623 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%float_n2 = OpConstant %float -2
%float_0_75 = OpConstant %float 0.75
%330 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%236 = OpLoad %mat2v3float %m23
%238 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%239 = OpCompositeConstruct %mat2v3float %238 %238
%240 = OpCompositeExtract %v3float %236 0
%241 = OpCompositeExtract %v3float %239 0
%242 = OpFAdd %v3float %240 %241
%243 = OpCompositeExtract %v3float %236 1
%244 = OpCompositeExtract %v3float %239 1
%245 = OpFAdd %v3float %243 %244
%246 = OpCompositeConstruct %mat2v3float %242 %245
OpStore %m23 %246
%247 = OpLoad %bool %ok
OpSelectionMerge %249 None
OpBranchConditional %247 %248 %249
%248 = OpLabel
%250 = OpLoad %mat2v3float %m23
%252 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%253 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%251 = OpCompositeConstruct %mat2v3float %252 %253
%254 = OpCompositeExtract %v3float %250 0
%255 = OpCompositeExtract %v3float %251 0
%256 = OpFOrdEqual %v3bool %254 %255
%257 = OpAll %bool %256
%258 = OpCompositeExtract %v3float %250 1
%259 = OpCompositeExtract %v3float %251 1
%260 = OpFOrdEqual %v3bool %258 %259
%261 = OpAll %bool %260
%262 = OpLogicalAnd %bool %257 %261
OpBranch %249
%249 = OpLabel
%263 = OpPhi %bool %false %214 %262 %248
OpStore %ok %263
%264 = OpLoad %mat3v2float %m32
%265 = OpCompositeConstruct %v2float %float_2 %float_2
%266 = OpCompositeConstruct %mat3v2float %265 %265 %265
%267 = OpCompositeExtract %v2float %264 0
%268 = OpCompositeExtract %v2float %266 0
%269 = OpFSub %v2float %267 %268
%270 = OpCompositeExtract %v2float %264 1
%271 = OpCompositeExtract %v2float %266 1
%272 = OpFSub %v2float %270 %271
%273 = OpCompositeExtract %v2float %264 2
%274 = OpCompositeExtract %v2float %266 2
%275 = OpFSub %v2float %273 %274
%276 = OpCompositeConstruct %mat3v2float %269 %272 %275
OpStore %m32 %276
%277 = OpLoad %bool %ok
OpSelectionMerge %279 None
OpBranchConditional %277 %278 %279
%278 = OpLabel
%280 = OpLoad %mat3v2float %m32
%283 = OpCompositeConstruct %v2float %float_2 %float_n2
%284 = OpCompositeConstruct %v2float %float_n2 %float_2
%285 = OpCompositeConstruct %v2float %float_n2 %float_n2
%282 = OpCompositeConstruct %mat3v2float %283 %284 %285
%286 = OpCompositeExtract %v2float %280 0
%287 = OpCompositeExtract %v2float %282 0
%288 = OpFOrdEqual %v2bool %286 %287
%289 = OpAll %bool %288
%290 = OpCompositeExtract %v2float %280 1
%291 = OpCompositeExtract %v2float %282 1
%292 = OpFOrdEqual %v2bool %290 %291
%293 = OpAll %bool %292
%294 = OpLogicalAnd %bool %289 %293
%295 = OpCompositeExtract %v2float %280 2
%296 = OpCompositeExtract %v2float %282 2
%297 = OpFOrdEqual %v2bool %295 %296
%298 = OpAll %bool %297
%299 = OpLogicalAnd %bool %294 %298
OpBranch %279
%279 = OpLabel
%300 = OpPhi %bool %false %249 %299 %278
OpStore %ok %300
%301 = OpLoad %mat2v4float %m24
%302 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%303 = OpCompositeConstruct %mat2v4float %302 %302
%304 = OpCompositeExtract %v4float %301 0
%305 = OpCompositeExtract %v4float %303 0
%306 = OpFDiv %v4float %304 %305
%307 = OpCompositeExtract %v4float %301 1
%308 = OpCompositeExtract %v4float %303 1
%309 = OpFDiv %v4float %307 %308
%310 = OpCompositeConstruct %mat2v4float %306 %309
OpStore %m24 %310
%311 = OpLoad %bool %ok
OpSelectionMerge %313 None
OpBranchConditional %311 %312 %313
%312 = OpLabel
%314 = OpLoad %mat2v4float %m24
%317 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%318 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%316 = OpCompositeConstruct %mat2v4float %317 %318
%319 = OpCompositeExtract %v4float %314 0
%320 = OpCompositeExtract %v4float %316 0
%321 = OpFOrdEqual %v4bool %319 %320
%322 = OpAll %bool %321
%323 = OpCompositeExtract %v4float %314 1
%324 = OpCompositeExtract %v4float %316 1
%325 = OpFOrdEqual %v4bool %323 %324
%326 = OpAll %bool %325
%327 = OpLogicalAnd %bool %322 %326
OpBranch %313
%313 = OpLabel
%328 = OpPhi %bool %false %279 %327 %312
OpStore %ok %328
%329 = OpLoad %bool %ok
OpReturnValue %329
OpFunctionEnd
%main = OpFunction %v4float None %330
%331 = OpFunctionParameter %_ptr_Function_v2float
%332 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%610 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%336 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%337 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%335 = OpCompositeConstruct %mat2v3float %336 %337
OpStore %_1_m23 %335
%338 = OpLoad %bool %_0_ok
OpSelectionMerge %340 None
OpBranchConditional %338 %339 %340
%339 = OpLabel
%341 = OpLoad %mat2v3float %_1_m23
%343 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%344 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%342 = OpCompositeConstruct %mat2v3float %343 %344
%345 = OpCompositeExtract %v3float %341 0
%346 = OpCompositeExtract %v3float %342 0
%347 = OpFOrdEqual %v3bool %345 %346
%348 = OpAll %bool %347
%349 = OpCompositeExtract %v3float %341 1
%350 = OpCompositeExtract %v3float %342 1
%351 = OpFOrdEqual %v3bool %349 %350
%352 = OpAll %bool %351
%353 = OpLogicalAnd %bool %348 %352
OpBranch %340
%340 = OpLabel
%354 = OpPhi %bool %false %332 %353 %339
OpStore %_0_ok %354
%357 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%358 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%356 = OpCompositeConstruct %mat2v4float %357 %358
OpStore %_2_m24 %356
%359 = OpLoad %bool %_0_ok
OpSelectionMerge %361 None
OpBranchConditional %359 %360 %361
%360 = OpLabel
%362 = OpLoad %mat2v4float %_2_m24
%364 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%365 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%363 = OpCompositeConstruct %mat2v4float %364 %365
%366 = OpCompositeExtract %v4float %362 0
%367 = OpCompositeExtract %v4float %363 0
%368 = OpFOrdEqual %v4bool %366 %367
%369 = OpAll %bool %368
%370 = OpCompositeExtract %v4float %362 1
%371 = OpCompositeExtract %v4float %363 1
%372 = OpFOrdEqual %v4bool %370 %371
%373 = OpAll %bool %372
%374 = OpLogicalAnd %bool %369 %373
OpBranch %361
%361 = OpLabel
%375 = OpPhi %bool %false %340 %374 %360
OpStore %_0_ok %375
%378 = OpCompositeConstruct %v2float %float_4 %float_0
%379 = OpCompositeConstruct %v2float %float_0 %float_4
%380 = OpCompositeConstruct %v2float %float_0 %float_0
%377 = OpCompositeConstruct %mat3v2float %378 %379 %380
OpStore %_3_m32 %377
%381 = OpLoad %bool %_0_ok
OpSelectionMerge %383 None
OpBranchConditional %381 %382 %383
%382 = OpLabel
%384 = OpLoad %mat3v2float %_3_m32
%386 = OpCompositeConstruct %v2float %float_4 %float_0
%387 = OpCompositeConstruct %v2float %float_0 %float_4
%388 = OpCompositeConstruct %v2float %float_0 %float_0
%385 = OpCompositeConstruct %mat3v2float %386 %387 %388
%389 = OpCompositeExtract %v2float %384 0
%390 = OpCompositeExtract %v2float %385 0
%391 = OpFOrdEqual %v2bool %389 %390
%392 = OpAll %bool %391
%393 = OpCompositeExtract %v2float %384 1
%394 = OpCompositeExtract %v2float %385 1
%395 = OpFOrdEqual %v2bool %393 %394
%396 = OpAll %bool %395
%397 = OpLogicalAnd %bool %392 %396
%398 = OpCompositeExtract %v2float %384 2
%399 = OpCompositeExtract %v2float %385 2
%400 = OpFOrdEqual %v2bool %398 %399
%401 = OpAll %bool %400
%402 = OpLogicalAnd %bool %397 %401
OpBranch %383
%383 = OpLabel
%403 = OpPhi %bool %false %361 %402 %382
OpStore %_0_ok %403
%406 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%407 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%408 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%405 = OpCompositeConstruct %mat3v4float %406 %407 %408
OpStore %_4_m34 %405
%409 = OpLoad %bool %_0_ok
OpSelectionMerge %411 None
OpBranchConditional %409 %410 %411
%410 = OpLabel
%412 = OpLoad %mat3v4float %_4_m34
%414 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%415 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%416 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%413 = OpCompositeConstruct %mat3v4float %414 %415 %416
%417 = OpCompositeExtract %v4float %412 0
%418 = OpCompositeExtract %v4float %413 0
%419 = OpFOrdEqual %v4bool %417 %418
%420 = OpAll %bool %419
%421 = OpCompositeExtract %v4float %412 1
%422 = OpCompositeExtract %v4float %413 1
%423 = OpFOrdEqual %v4bool %421 %422
%424 = OpAll %bool %423
%425 = OpLogicalAnd %bool %420 %424
%426 = OpCompositeExtract %v4float %412 2
%427 = OpCompositeExtract %v4float %413 2
%428 = OpFOrdEqual %v4bool %426 %427
%429 = OpAll %bool %428
%430 = OpLogicalAnd %bool %425 %429
OpBranch %411
%411 = OpLabel
%431 = OpPhi %bool %false %383 %430 %410
OpStore %_0_ok %431
%434 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%435 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%436 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%437 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%433 = OpCompositeConstruct %mat4v3float %434 %435 %436 %437
OpStore %_6_m43 %433
%438 = OpLoad %bool %_0_ok
OpSelectionMerge %440 None
OpBranchConditional %438 %439 %440
%439 = OpLabel
%441 = OpLoad %mat4v3float %_6_m43
%443 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%444 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%445 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%446 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%442 = OpCompositeConstruct %mat4v3float %443 %444 %445 %446
%447 = OpCompositeExtract %v3float %441 0
%448 = OpCompositeExtract %v3float %442 0
%449 = OpFOrdEqual %v3bool %447 %448
%450 = OpAll %bool %449
%451 = OpCompositeExtract %v3float %441 1
%452 = OpCompositeExtract %v3float %442 1
%453 = OpFOrdEqual %v3bool %451 %452
%454 = OpAll %bool %453
%455 = OpLogicalAnd %bool %450 %454
%456 = OpCompositeExtract %v3float %441 2
%457 = OpCompositeExtract %v3float %442 2
%458 = OpFOrdEqual %v3bool %456 %457
%459 = OpAll %bool %458
%460 = OpLogicalAnd %bool %455 %459
%461 = OpCompositeExtract %v3float %441 3
%462 = OpCompositeExtract %v3float %442 3
%463 = OpFOrdEqual %v3bool %461 %462
%464 = OpAll %bool %463
%465 = OpLogicalAnd %bool %460 %464
OpBranch %440
%440 = OpLabel
%466 = OpPhi %bool %false %411 %465 %439
OpStore %_0_ok %466
%468 = OpLoad %mat3v2float %_3_m32
%469 = OpLoad %mat2v3float %_1_m23
%470 = OpMatrixTimesMatrix %mat2v2float %468 %469
OpStore %_7_m22 %470
%471 = OpLoad %bool %_0_ok
OpSelectionMerge %473 None
OpBranchConditional %471 %472 %473
%472 = OpLabel
%474 = OpLoad %mat2v2float %_7_m22
%476 = OpCompositeConstruct %v2float %float_8 %float_0
%477 = OpCompositeConstruct %v2float %float_0 %float_8
%475 = OpCompositeConstruct %mat2v2float %476 %477
%478 = OpCompositeExtract %v2float %474 0
%479 = OpCompositeExtract %v2float %475 0
%480 = OpFOrdEqual %v2bool %478 %479
%481 = OpAll %bool %480
%482 = OpCompositeExtract %v2float %474 1
%483 = OpCompositeExtract %v2float %475 1
%484 = OpFOrdEqual %v2bool %482 %483
%485 = OpAll %bool %484
%486 = OpLogicalAnd %bool %481 %485
OpBranch %473
%473 = OpLabel
%487 = OpPhi %bool %false %440 %486 %472
OpStore %_0_ok %487
%489 = OpLoad %mat4v3float %_6_m43
%490 = OpLoad %mat3v4float %_4_m34
%491 = OpMatrixTimesMatrix %mat3v3float %489 %490
OpStore %_8_m33 %491
%492 = OpLoad %bool %_0_ok
OpSelectionMerge %494 None
OpBranchConditional %492 %493 %494
%493 = OpLabel
%495 = OpLoad %mat3v3float %_8_m33
%497 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%498 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%499 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%496 = OpCompositeConstruct %mat3v3float %497 %498 %499
%500 = OpCompositeExtract %v3float %495 0
%501 = OpCompositeExtract %v3float %496 0
%502 = OpFOrdEqual %v3bool %500 %501
%503 = OpAll %bool %502
%504 = OpCompositeExtract %v3float %495 1
%505 = OpCompositeExtract %v3float %496 1
%506 = OpFOrdEqual %v3bool %504 %505
%507 = OpAll %bool %506
%508 = OpLogicalAnd %bool %503 %507
%509 = OpCompositeExtract %v3float %495 2
%510 = OpCompositeExtract %v3float %496 2
%511 = OpFOrdEqual %v3bool %509 %510
%512 = OpAll %bool %511
%513 = OpLogicalAnd %bool %508 %512
OpBranch %494
%494 = OpLabel
%514 = OpPhi %bool %false %473 %513 %493
OpStore %_0_ok %514
%515 = OpLoad %mat2v3float %_1_m23
%516 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%517 = OpCompositeConstruct %mat2v3float %516 %516
%518 = OpCompositeExtract %v3float %515 0
%519 = OpCompositeExtract %v3float %517 0
%520 = OpFAdd %v3float %518 %519
%521 = OpCompositeExtract %v3float %515 1
%522 = OpCompositeExtract %v3float %517 1
%523 = OpFAdd %v3float %521 %522
%524 = OpCompositeConstruct %mat2v3float %520 %523
OpStore %_1_m23 %524
%525 = OpLoad %bool %_0_ok
OpSelectionMerge %527 None
OpBranchConditional %525 %526 %527
%526 = OpLabel
%528 = OpLoad %mat2v3float %_1_m23
%530 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%531 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%529 = OpCompositeConstruct %mat2v3float %530 %531
%532 = OpCompositeExtract %v3float %528 0
%533 = OpCompositeExtract %v3float %529 0
%534 = OpFOrdEqual %v3bool %532 %533
%535 = OpAll %bool %534
%536 = OpCompositeExtract %v3float %528 1
%537 = OpCompositeExtract %v3float %529 1
%538 = OpFOrdEqual %v3bool %536 %537
%539 = OpAll %bool %538
%540 = OpLogicalAnd %bool %535 %539
OpBranch %527
%527 = OpLabel
%541 = OpPhi %bool %false %494 %540 %526
OpStore %_0_ok %541
%542 = OpLoad %mat3v2float %_3_m32
%543 = OpCompositeConstruct %v2float %float_2 %float_2
%544 = OpCompositeConstruct %mat3v2float %543 %543 %543
%545 = OpCompositeExtract %v2float %542 0
%546 = OpCompositeExtract %v2float %544 0
%547 = OpFSub %v2float %545 %546
%548 = OpCompositeExtract %v2float %542 1
%549 = OpCompositeExtract %v2float %544 1
%550 = OpFSub %v2float %548 %549
%551 = OpCompositeExtract %v2float %542 2
%552 = OpCompositeExtract %v2float %544 2
%553 = OpFSub %v2float %551 %552
%554 = OpCompositeConstruct %mat3v2float %547 %550 %553
OpStore %_3_m32 %554
%555 = OpLoad %bool %_0_ok
OpSelectionMerge %557 None
OpBranchConditional %555 %556 %557
%556 = OpLabel
%558 = OpLoad %mat3v2float %_3_m32
%560 = OpCompositeConstruct %v2float %float_2 %float_n2
%561 = OpCompositeConstruct %v2float %float_n2 %float_2
%562 = OpCompositeConstruct %v2float %float_n2 %float_n2
%559 = OpCompositeConstruct %mat3v2float %560 %561 %562
%563 = OpCompositeExtract %v2float %558 0
%564 = OpCompositeExtract %v2float %559 0
%565 = OpFOrdEqual %v2bool %563 %564
%566 = OpAll %bool %565
%567 = OpCompositeExtract %v2float %558 1
%568 = OpCompositeExtract %v2float %559 1
%569 = OpFOrdEqual %v2bool %567 %568
%570 = OpAll %bool %569
%571 = OpLogicalAnd %bool %566 %570
%572 = OpCompositeExtract %v2float %558 2
%573 = OpCompositeExtract %v2float %559 2
%574 = OpFOrdEqual %v2bool %572 %573
%575 = OpAll %bool %574
%576 = OpLogicalAnd %bool %571 %575
OpBranch %557
%557 = OpLabel
%577 = OpPhi %bool %false %527 %576 %556
OpStore %_0_ok %577
%578 = OpLoad %mat2v4float %_2_m24
%579 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%580 = OpCompositeConstruct %mat2v4float %579 %579
%581 = OpCompositeExtract %v4float %578 0
%582 = OpCompositeExtract %v4float %580 0
%583 = OpFDiv %v4float %581 %582
%584 = OpCompositeExtract %v4float %578 1
%585 = OpCompositeExtract %v4float %580 1
%586 = OpFDiv %v4float %584 %585
%587 = OpCompositeConstruct %mat2v4float %583 %586
OpStore %_2_m24 %587
%588 = OpLoad %bool %_0_ok
OpSelectionMerge %590 None
OpBranchConditional %588 %589 %590
%589 = OpLabel
%591 = OpLoad %mat2v4float %_2_m24
%593 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%594 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%592 = OpCompositeConstruct %mat2v4float %593 %594
%595 = OpCompositeExtract %v4float %591 0
%596 = OpCompositeExtract %v4float %592 0
%597 = OpFOrdEqual %v4bool %595 %596
%598 = OpAll %bool %597
%599 = OpCompositeExtract %v4float %591 1
%600 = OpCompositeExtract %v4float %592 1
%601 = OpFOrdEqual %v4bool %599 %600
%602 = OpAll %bool %601
%603 = OpLogicalAnd %bool %598 %602
OpBranch %590
%590 = OpLabel
%604 = OpPhi %bool %false %557 %603 %589
OpStore %_0_ok %604
%605 = OpLoad %bool %_0_ok
OpSelectionMerge %607 None
OpBranchConditional %605 %606 %607
%606 = OpLabel
%608 = OpFunctionCall %bool %test_half_b
OpBranch %607
%607 = OpLabel
%609 = OpPhi %bool %false %590 %608 %606
OpSelectionMerge %614 None
OpBranchConditional %609 %612 %613
%612 = OpLabel
%615 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%619 = OpLoad %v4float %615
OpStore %610 %619
OpBranch %614
%613 = OpLabel
%620 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%622 = OpLoad %v4float %620
OpStore %610 %622
OpBranch %614
%614 = OpLabel
%623 = OpLoad %v4float %610
OpReturnValue %623
OpFunctionEnd
