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
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m23 "_1_m23"
OpName %_2_m24 "_2_m24"
OpName %_3_m32 "_3_m32"
OpName %_4_m34 "_4_m34"
OpName %_5_m42 "_5_m42"
OpName %_6_m43 "_6_m43"
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
OpDecorate %153 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
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
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%221 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%151 = OpCompositeConstruct %v2float %float_0 %float_0
%152 = OpCompositeConstruct %v2float %float_0 %float_0
%148 = OpCompositeConstruct %mat4v2float %149 %150 %151 %152
OpStore %m42 %148
%153 = OpLoad %bool %ok
OpSelectionMerge %155 None
OpBranchConditional %153 %154 %155
%154 = OpLabel
%156 = OpLoad %mat4v2float %m42
%158 = OpCompositeConstruct %v2float %float_6 %float_0
%159 = OpCompositeConstruct %v2float %float_0 %float_6
%160 = OpCompositeConstruct %v2float %float_0 %float_0
%161 = OpCompositeConstruct %v2float %float_0 %float_0
%157 = OpCompositeConstruct %mat4v2float %158 %159 %160 %161
%162 = OpCompositeExtract %v2float %156 0
%163 = OpCompositeExtract %v2float %157 0
%164 = OpFOrdEqual %v2bool %162 %163
%165 = OpAll %bool %164
%166 = OpCompositeExtract %v2float %156 1
%167 = OpCompositeExtract %v2float %157 1
%168 = OpFOrdEqual %v2bool %166 %167
%169 = OpAll %bool %168
%170 = OpLogicalAnd %bool %165 %169
%171 = OpCompositeExtract %v2float %156 2
%172 = OpCompositeExtract %v2float %157 2
%173 = OpFOrdEqual %v2bool %171 %172
%174 = OpAll %bool %173
%175 = OpLogicalAnd %bool %170 %174
%176 = OpCompositeExtract %v2float %156 3
%177 = OpCompositeExtract %v2float %157 3
%178 = OpFOrdEqual %v2bool %176 %177
%179 = OpAll %bool %178
%180 = OpLogicalAnd %bool %175 %179
OpBranch %155
%155 = OpLabel
%181 = OpPhi %bool %false %123 %180 %154
OpStore %ok %181
%187 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%188 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%189 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%190 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%186 = OpCompositeConstruct %mat4v3float %187 %188 %189 %190
OpStore %m43 %186
%191 = OpLoad %bool %ok
OpSelectionMerge %193 None
OpBranchConditional %191 %192 %193
%192 = OpLabel
%194 = OpLoad %mat4v3float %m43
%196 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%197 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%198 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%199 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%195 = OpCompositeConstruct %mat4v3float %196 %197 %198 %199
%200 = OpCompositeExtract %v3float %194 0
%201 = OpCompositeExtract %v3float %195 0
%202 = OpFOrdEqual %v3bool %200 %201
%203 = OpAll %bool %202
%204 = OpCompositeExtract %v3float %194 1
%205 = OpCompositeExtract %v3float %195 1
%206 = OpFOrdEqual %v3bool %204 %205
%207 = OpAll %bool %206
%208 = OpLogicalAnd %bool %203 %207
%209 = OpCompositeExtract %v3float %194 2
%210 = OpCompositeExtract %v3float %195 2
%211 = OpFOrdEqual %v3bool %209 %210
%212 = OpAll %bool %211
%213 = OpLogicalAnd %bool %208 %212
%214 = OpCompositeExtract %v3float %194 3
%215 = OpCompositeExtract %v3float %195 3
%216 = OpFOrdEqual %v3bool %214 %215
%217 = OpAll %bool %216
%218 = OpLogicalAnd %bool %213 %217
OpBranch %193
%193 = OpLabel
%219 = OpPhi %bool %false %155 %218 %192
OpStore %ok %219
%220 = OpLoad %bool %ok
OpReturnValue %220
OpFunctionEnd
%main = OpFunction %v4float None %221
%222 = OpFunctionParameter %_ptr_Function_v2float
%223 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%398 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%227 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%228 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%226 = OpCompositeConstruct %mat2v3float %227 %228
OpStore %_1_m23 %226
%229 = OpLoad %bool %_0_ok
OpSelectionMerge %231 None
OpBranchConditional %229 %230 %231
%230 = OpLabel
%232 = OpLoad %mat2v3float %_1_m23
%234 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%235 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%233 = OpCompositeConstruct %mat2v3float %234 %235
%236 = OpCompositeExtract %v3float %232 0
%237 = OpCompositeExtract %v3float %233 0
%238 = OpFOrdEqual %v3bool %236 %237
%239 = OpAll %bool %238
%240 = OpCompositeExtract %v3float %232 1
%241 = OpCompositeExtract %v3float %233 1
%242 = OpFOrdEqual %v3bool %240 %241
%243 = OpAll %bool %242
%244 = OpLogicalAnd %bool %239 %243
OpBranch %231
%231 = OpLabel
%245 = OpPhi %bool %false %223 %244 %230
OpStore %_0_ok %245
%248 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%249 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%247 = OpCompositeConstruct %mat2v4float %248 %249
OpStore %_2_m24 %247
%250 = OpLoad %bool %_0_ok
OpSelectionMerge %252 None
OpBranchConditional %250 %251 %252
%251 = OpLabel
%253 = OpLoad %mat2v4float %_2_m24
%255 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%256 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%254 = OpCompositeConstruct %mat2v4float %255 %256
%257 = OpCompositeExtract %v4float %253 0
%258 = OpCompositeExtract %v4float %254 0
%259 = OpFOrdEqual %v4bool %257 %258
%260 = OpAll %bool %259
%261 = OpCompositeExtract %v4float %253 1
%262 = OpCompositeExtract %v4float %254 1
%263 = OpFOrdEqual %v4bool %261 %262
%264 = OpAll %bool %263
%265 = OpLogicalAnd %bool %260 %264
OpBranch %252
%252 = OpLabel
%266 = OpPhi %bool %false %231 %265 %251
OpStore %_0_ok %266
%269 = OpCompositeConstruct %v2float %float_4 %float_0
%270 = OpCompositeConstruct %v2float %float_0 %float_4
%271 = OpCompositeConstruct %v2float %float_0 %float_0
%268 = OpCompositeConstruct %mat3v2float %269 %270 %271
OpStore %_3_m32 %268
%272 = OpLoad %bool %_0_ok
OpSelectionMerge %274 None
OpBranchConditional %272 %273 %274
%273 = OpLabel
%275 = OpLoad %mat3v2float %_3_m32
%277 = OpCompositeConstruct %v2float %float_4 %float_0
%278 = OpCompositeConstruct %v2float %float_0 %float_4
%279 = OpCompositeConstruct %v2float %float_0 %float_0
%276 = OpCompositeConstruct %mat3v2float %277 %278 %279
%280 = OpCompositeExtract %v2float %275 0
%281 = OpCompositeExtract %v2float %276 0
%282 = OpFOrdEqual %v2bool %280 %281
%283 = OpAll %bool %282
%284 = OpCompositeExtract %v2float %275 1
%285 = OpCompositeExtract %v2float %276 1
%286 = OpFOrdEqual %v2bool %284 %285
%287 = OpAll %bool %286
%288 = OpLogicalAnd %bool %283 %287
%289 = OpCompositeExtract %v2float %275 2
%290 = OpCompositeExtract %v2float %276 2
%291 = OpFOrdEqual %v2bool %289 %290
%292 = OpAll %bool %291
%293 = OpLogicalAnd %bool %288 %292
OpBranch %274
%274 = OpLabel
%294 = OpPhi %bool %false %252 %293 %273
OpStore %_0_ok %294
%297 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%298 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%299 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%296 = OpCompositeConstruct %mat3v4float %297 %298 %299
OpStore %_4_m34 %296
%300 = OpLoad %bool %_0_ok
OpSelectionMerge %302 None
OpBranchConditional %300 %301 %302
%301 = OpLabel
%303 = OpLoad %mat3v4float %_4_m34
%305 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%306 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%307 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%304 = OpCompositeConstruct %mat3v4float %305 %306 %307
%308 = OpCompositeExtract %v4float %303 0
%309 = OpCompositeExtract %v4float %304 0
%310 = OpFOrdEqual %v4bool %308 %309
%311 = OpAll %bool %310
%312 = OpCompositeExtract %v4float %303 1
%313 = OpCompositeExtract %v4float %304 1
%314 = OpFOrdEqual %v4bool %312 %313
%315 = OpAll %bool %314
%316 = OpLogicalAnd %bool %311 %315
%317 = OpCompositeExtract %v4float %303 2
%318 = OpCompositeExtract %v4float %304 2
%319 = OpFOrdEqual %v4bool %317 %318
%320 = OpAll %bool %319
%321 = OpLogicalAnd %bool %316 %320
OpBranch %302
%302 = OpLabel
%322 = OpPhi %bool %false %274 %321 %301
OpStore %_0_ok %322
%325 = OpCompositeConstruct %v2float %float_6 %float_0
%326 = OpCompositeConstruct %v2float %float_0 %float_6
%327 = OpCompositeConstruct %v2float %float_0 %float_0
%328 = OpCompositeConstruct %v2float %float_0 %float_0
%324 = OpCompositeConstruct %mat4v2float %325 %326 %327 %328
OpStore %_5_m42 %324
%329 = OpLoad %bool %_0_ok
OpSelectionMerge %331 None
OpBranchConditional %329 %330 %331
%330 = OpLabel
%332 = OpLoad %mat4v2float %_5_m42
%334 = OpCompositeConstruct %v2float %float_6 %float_0
%335 = OpCompositeConstruct %v2float %float_0 %float_6
%336 = OpCompositeConstruct %v2float %float_0 %float_0
%337 = OpCompositeConstruct %v2float %float_0 %float_0
%333 = OpCompositeConstruct %mat4v2float %334 %335 %336 %337
%338 = OpCompositeExtract %v2float %332 0
%339 = OpCompositeExtract %v2float %333 0
%340 = OpFOrdEqual %v2bool %338 %339
%341 = OpAll %bool %340
%342 = OpCompositeExtract %v2float %332 1
%343 = OpCompositeExtract %v2float %333 1
%344 = OpFOrdEqual %v2bool %342 %343
%345 = OpAll %bool %344
%346 = OpLogicalAnd %bool %341 %345
%347 = OpCompositeExtract %v2float %332 2
%348 = OpCompositeExtract %v2float %333 2
%349 = OpFOrdEqual %v2bool %347 %348
%350 = OpAll %bool %349
%351 = OpLogicalAnd %bool %346 %350
%352 = OpCompositeExtract %v2float %332 3
%353 = OpCompositeExtract %v2float %333 3
%354 = OpFOrdEqual %v2bool %352 %353
%355 = OpAll %bool %354
%356 = OpLogicalAnd %bool %351 %355
OpBranch %331
%331 = OpLabel
%357 = OpPhi %bool %false %302 %356 %330
OpStore %_0_ok %357
%360 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%361 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%362 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%363 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%359 = OpCompositeConstruct %mat4v3float %360 %361 %362 %363
OpStore %_6_m43 %359
%364 = OpLoad %bool %_0_ok
OpSelectionMerge %366 None
OpBranchConditional %364 %365 %366
%365 = OpLabel
%367 = OpLoad %mat4v3float %_6_m43
%369 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%370 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%371 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%372 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%368 = OpCompositeConstruct %mat4v3float %369 %370 %371 %372
%373 = OpCompositeExtract %v3float %367 0
%374 = OpCompositeExtract %v3float %368 0
%375 = OpFOrdEqual %v3bool %373 %374
%376 = OpAll %bool %375
%377 = OpCompositeExtract %v3float %367 1
%378 = OpCompositeExtract %v3float %368 1
%379 = OpFOrdEqual %v3bool %377 %378
%380 = OpAll %bool %379
%381 = OpLogicalAnd %bool %376 %380
%382 = OpCompositeExtract %v3float %367 2
%383 = OpCompositeExtract %v3float %368 2
%384 = OpFOrdEqual %v3bool %382 %383
%385 = OpAll %bool %384
%386 = OpLogicalAnd %bool %381 %385
%387 = OpCompositeExtract %v3float %367 3
%388 = OpCompositeExtract %v3float %368 3
%389 = OpFOrdEqual %v3bool %387 %388
%390 = OpAll %bool %389
%391 = OpLogicalAnd %bool %386 %390
OpBranch %366
%366 = OpLabel
%392 = OpPhi %bool %false %331 %391 %365
OpStore %_0_ok %392
%393 = OpLoad %bool %_0_ok
OpSelectionMerge %395 None
OpBranchConditional %393 %394 %395
%394 = OpLabel
%396 = OpFunctionCall %bool %test_half_b
OpBranch %395
%395 = OpLabel
%397 = OpPhi %bool %false %366 %396 %394
OpSelectionMerge %402 None
OpBranchConditional %397 %400 %401
%400 = OpLabel
%403 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%407 = OpLoad %v4float %403
OpStore %398 %407
OpBranch %402
%401 = OpLabel
%408 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%410 = OpLoad %v4float %408
OpStore %398 %410
OpBranch %402
%402 = OpLabel
%411 = OpLoad %v4float %398
OpReturnValue %411
OpFunctionEnd
