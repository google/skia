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
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
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
OpDecorate %295 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %457 RelaxedPrecision
OpDecorate %496 RelaxedPrecision
OpDecorate %526 RelaxedPrecision
OpDecorate %559 RelaxedPrecision
OpDecorate %576 RelaxedPrecision
OpDecorate %590 RelaxedPrecision
OpDecorate %593 RelaxedPrecision
OpDecorate %594 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%float_n2 = OpConstant %float -2
%float_0_75 = OpConstant %float 0.75
%314 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%220 = OpLoad %mat2v3float %m23
%222 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%223 = OpCompositeConstruct %mat2v3float %222 %222
%224 = OpCompositeExtract %v3float %220 0
%225 = OpCompositeExtract %v3float %223 0
%226 = OpFAdd %v3float %224 %225
%227 = OpCompositeExtract %v3float %220 1
%228 = OpCompositeExtract %v3float %223 1
%229 = OpFAdd %v3float %227 %228
%230 = OpCompositeConstruct %mat2v3float %226 %229
OpStore %m23 %230
%231 = OpLoad %bool %ok
OpSelectionMerge %233 None
OpBranchConditional %231 %232 %233
%232 = OpLabel
%234 = OpLoad %mat2v3float %m23
%236 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%237 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%235 = OpCompositeConstruct %mat2v3float %236 %237
%238 = OpCompositeExtract %v3float %234 0
%239 = OpCompositeExtract %v3float %235 0
%240 = OpFOrdEqual %v3bool %238 %239
%241 = OpAll %bool %240
%242 = OpCompositeExtract %v3float %234 1
%243 = OpCompositeExtract %v3float %235 1
%244 = OpFOrdEqual %v3bool %242 %243
%245 = OpAll %bool %244
%246 = OpLogicalAnd %bool %241 %245
OpBranch %233
%233 = OpLabel
%247 = OpPhi %bool %false %193 %246 %232
OpStore %ok %247
%248 = OpLoad %mat3v2float %m32
%249 = OpCompositeConstruct %v2float %float_2 %float_2
%250 = OpCompositeConstruct %mat3v2float %249 %249 %249
%251 = OpCompositeExtract %v2float %248 0
%252 = OpCompositeExtract %v2float %250 0
%253 = OpFSub %v2float %251 %252
%254 = OpCompositeExtract %v2float %248 1
%255 = OpCompositeExtract %v2float %250 1
%256 = OpFSub %v2float %254 %255
%257 = OpCompositeExtract %v2float %248 2
%258 = OpCompositeExtract %v2float %250 2
%259 = OpFSub %v2float %257 %258
%260 = OpCompositeConstruct %mat3v2float %253 %256 %259
OpStore %m32 %260
%261 = OpLoad %bool %ok
OpSelectionMerge %263 None
OpBranchConditional %261 %262 %263
%262 = OpLabel
%264 = OpLoad %mat3v2float %m32
%267 = OpCompositeConstruct %v2float %float_2 %float_n2
%268 = OpCompositeConstruct %v2float %float_n2 %float_2
%269 = OpCompositeConstruct %v2float %float_n2 %float_n2
%266 = OpCompositeConstruct %mat3v2float %267 %268 %269
%270 = OpCompositeExtract %v2float %264 0
%271 = OpCompositeExtract %v2float %266 0
%272 = OpFOrdEqual %v2bool %270 %271
%273 = OpAll %bool %272
%274 = OpCompositeExtract %v2float %264 1
%275 = OpCompositeExtract %v2float %266 1
%276 = OpFOrdEqual %v2bool %274 %275
%277 = OpAll %bool %276
%278 = OpLogicalAnd %bool %273 %277
%279 = OpCompositeExtract %v2float %264 2
%280 = OpCompositeExtract %v2float %266 2
%281 = OpFOrdEqual %v2bool %279 %280
%282 = OpAll %bool %281
%283 = OpLogicalAnd %bool %278 %282
OpBranch %263
%263 = OpLabel
%284 = OpPhi %bool %false %233 %283 %262
OpStore %ok %284
%285 = OpLoad %mat2v4float %m24
%286 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%287 = OpCompositeConstruct %mat2v4float %286 %286
%288 = OpCompositeExtract %v4float %285 0
%289 = OpCompositeExtract %v4float %287 0
%290 = OpFDiv %v4float %288 %289
%291 = OpCompositeExtract %v4float %285 1
%292 = OpCompositeExtract %v4float %287 1
%293 = OpFDiv %v4float %291 %292
%294 = OpCompositeConstruct %mat2v4float %290 %293
OpStore %m24 %294
%295 = OpLoad %bool %ok
OpSelectionMerge %297 None
OpBranchConditional %295 %296 %297
%296 = OpLabel
%298 = OpLoad %mat2v4float %m24
%301 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%302 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%300 = OpCompositeConstruct %mat2v4float %301 %302
%303 = OpCompositeExtract %v4float %298 0
%304 = OpCompositeExtract %v4float %300 0
%305 = OpFOrdEqual %v4bool %303 %304
%306 = OpAll %bool %305
%307 = OpCompositeExtract %v4float %298 1
%308 = OpCompositeExtract %v4float %300 1
%309 = OpFOrdEqual %v4bool %307 %308
%310 = OpAll %bool %309
%311 = OpLogicalAnd %bool %306 %310
OpBranch %297
%297 = OpLabel
%312 = OpPhi %bool %false %263 %311 %296
OpStore %ok %312
%313 = OpLoad %bool %ok
OpReturnValue %313
OpFunctionEnd
%main = OpFunction %v4float None %314
%315 = OpFunctionParameter %_ptr_Function_v2float
%316 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%581 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%320 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%321 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%319 = OpCompositeConstruct %mat2v3float %320 %321
OpStore %_1_m23 %319
%322 = OpLoad %bool %_0_ok
OpSelectionMerge %324 None
OpBranchConditional %322 %323 %324
%323 = OpLabel
%325 = OpLoad %mat2v3float %_1_m23
%327 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%328 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%326 = OpCompositeConstruct %mat2v3float %327 %328
%329 = OpCompositeExtract %v3float %325 0
%330 = OpCompositeExtract %v3float %326 0
%331 = OpFOrdEqual %v3bool %329 %330
%332 = OpAll %bool %331
%333 = OpCompositeExtract %v3float %325 1
%334 = OpCompositeExtract %v3float %326 1
%335 = OpFOrdEqual %v3bool %333 %334
%336 = OpAll %bool %335
%337 = OpLogicalAnd %bool %332 %336
OpBranch %324
%324 = OpLabel
%338 = OpPhi %bool %false %316 %337 %323
OpStore %_0_ok %338
%341 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%342 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%340 = OpCompositeConstruct %mat2v4float %341 %342
OpStore %_2_m24 %340
%343 = OpLoad %bool %_0_ok
OpSelectionMerge %345 None
OpBranchConditional %343 %344 %345
%344 = OpLabel
%346 = OpLoad %mat2v4float %_2_m24
%348 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%349 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%347 = OpCompositeConstruct %mat2v4float %348 %349
%350 = OpCompositeExtract %v4float %346 0
%351 = OpCompositeExtract %v4float %347 0
%352 = OpFOrdEqual %v4bool %350 %351
%353 = OpAll %bool %352
%354 = OpCompositeExtract %v4float %346 1
%355 = OpCompositeExtract %v4float %347 1
%356 = OpFOrdEqual %v4bool %354 %355
%357 = OpAll %bool %356
%358 = OpLogicalAnd %bool %353 %357
OpBranch %345
%345 = OpLabel
%359 = OpPhi %bool %false %324 %358 %344
OpStore %_0_ok %359
%362 = OpCompositeConstruct %v2float %float_4 %float_0
%363 = OpCompositeConstruct %v2float %float_0 %float_4
%364 = OpCompositeConstruct %v2float %float_0 %float_0
%361 = OpCompositeConstruct %mat3v2float %362 %363 %364
OpStore %_3_m32 %361
%365 = OpLoad %bool %_0_ok
OpSelectionMerge %367 None
OpBranchConditional %365 %366 %367
%366 = OpLabel
%368 = OpLoad %mat3v2float %_3_m32
%370 = OpCompositeConstruct %v2float %float_4 %float_0
%371 = OpCompositeConstruct %v2float %float_0 %float_4
%372 = OpCompositeConstruct %v2float %float_0 %float_0
%369 = OpCompositeConstruct %mat3v2float %370 %371 %372
%373 = OpCompositeExtract %v2float %368 0
%374 = OpCompositeExtract %v2float %369 0
%375 = OpFOrdEqual %v2bool %373 %374
%376 = OpAll %bool %375
%377 = OpCompositeExtract %v2float %368 1
%378 = OpCompositeExtract %v2float %369 1
%379 = OpFOrdEqual %v2bool %377 %378
%380 = OpAll %bool %379
%381 = OpLogicalAnd %bool %376 %380
%382 = OpCompositeExtract %v2float %368 2
%383 = OpCompositeExtract %v2float %369 2
%384 = OpFOrdEqual %v2bool %382 %383
%385 = OpAll %bool %384
%386 = OpLogicalAnd %bool %381 %385
OpBranch %367
%367 = OpLabel
%387 = OpPhi %bool %false %345 %386 %366
OpStore %_0_ok %387
%390 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%391 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%392 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%389 = OpCompositeConstruct %mat3v4float %390 %391 %392
OpStore %_4_m34 %389
%393 = OpLoad %bool %_0_ok
OpSelectionMerge %395 None
OpBranchConditional %393 %394 %395
%394 = OpLabel
%396 = OpLoad %mat3v4float %_4_m34
%398 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%399 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%400 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%397 = OpCompositeConstruct %mat3v4float %398 %399 %400
%401 = OpCompositeExtract %v4float %396 0
%402 = OpCompositeExtract %v4float %397 0
%403 = OpFOrdEqual %v4bool %401 %402
%404 = OpAll %bool %403
%405 = OpCompositeExtract %v4float %396 1
%406 = OpCompositeExtract %v4float %397 1
%407 = OpFOrdEqual %v4bool %405 %406
%408 = OpAll %bool %407
%409 = OpLogicalAnd %bool %404 %408
%410 = OpCompositeExtract %v4float %396 2
%411 = OpCompositeExtract %v4float %397 2
%412 = OpFOrdEqual %v4bool %410 %411
%413 = OpAll %bool %412
%414 = OpLogicalAnd %bool %409 %413
OpBranch %395
%395 = OpLabel
%415 = OpPhi %bool %false %367 %414 %394
OpStore %_0_ok %415
%418 = OpCompositeConstruct %v2float %float_6 %float_0
%419 = OpCompositeConstruct %v2float %float_0 %float_6
%420 = OpCompositeConstruct %v2float %float_0 %float_0
%421 = OpCompositeConstruct %v2float %float_0 %float_0
%417 = OpCompositeConstruct %mat4v2float %418 %419 %420 %421
OpStore %_5_m42 %417
%422 = OpLoad %bool %_0_ok
OpSelectionMerge %424 None
OpBranchConditional %422 %423 %424
%423 = OpLabel
%425 = OpLoad %mat4v2float %_5_m42
%427 = OpCompositeConstruct %v2float %float_6 %float_0
%428 = OpCompositeConstruct %v2float %float_0 %float_6
%429 = OpCompositeConstruct %v2float %float_0 %float_0
%430 = OpCompositeConstruct %v2float %float_0 %float_0
%426 = OpCompositeConstruct %mat4v2float %427 %428 %429 %430
%431 = OpCompositeExtract %v2float %425 0
%432 = OpCompositeExtract %v2float %426 0
%433 = OpFOrdEqual %v2bool %431 %432
%434 = OpAll %bool %433
%435 = OpCompositeExtract %v2float %425 1
%436 = OpCompositeExtract %v2float %426 1
%437 = OpFOrdEqual %v2bool %435 %436
%438 = OpAll %bool %437
%439 = OpLogicalAnd %bool %434 %438
%440 = OpCompositeExtract %v2float %425 2
%441 = OpCompositeExtract %v2float %426 2
%442 = OpFOrdEqual %v2bool %440 %441
%443 = OpAll %bool %442
%444 = OpLogicalAnd %bool %439 %443
%445 = OpCompositeExtract %v2float %425 3
%446 = OpCompositeExtract %v2float %426 3
%447 = OpFOrdEqual %v2bool %445 %446
%448 = OpAll %bool %447
%449 = OpLogicalAnd %bool %444 %448
OpBranch %424
%424 = OpLabel
%450 = OpPhi %bool %false %395 %449 %423
OpStore %_0_ok %450
%453 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%454 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%455 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%456 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%452 = OpCompositeConstruct %mat4v3float %453 %454 %455 %456
OpStore %_6_m43 %452
%457 = OpLoad %bool %_0_ok
OpSelectionMerge %459 None
OpBranchConditional %457 %458 %459
%458 = OpLabel
%460 = OpLoad %mat4v3float %_6_m43
%462 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%463 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%464 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%465 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%461 = OpCompositeConstruct %mat4v3float %462 %463 %464 %465
%466 = OpCompositeExtract %v3float %460 0
%467 = OpCompositeExtract %v3float %461 0
%468 = OpFOrdEqual %v3bool %466 %467
%469 = OpAll %bool %468
%470 = OpCompositeExtract %v3float %460 1
%471 = OpCompositeExtract %v3float %461 1
%472 = OpFOrdEqual %v3bool %470 %471
%473 = OpAll %bool %472
%474 = OpLogicalAnd %bool %469 %473
%475 = OpCompositeExtract %v3float %460 2
%476 = OpCompositeExtract %v3float %461 2
%477 = OpFOrdEqual %v3bool %475 %476
%478 = OpAll %bool %477
%479 = OpLogicalAnd %bool %474 %478
%480 = OpCompositeExtract %v3float %460 3
%481 = OpCompositeExtract %v3float %461 3
%482 = OpFOrdEqual %v3bool %480 %481
%483 = OpAll %bool %482
%484 = OpLogicalAnd %bool %479 %483
OpBranch %459
%459 = OpLabel
%485 = OpPhi %bool %false %424 %484 %458
OpStore %_0_ok %485
%486 = OpLoad %mat2v3float %_1_m23
%487 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%488 = OpCompositeConstruct %mat2v3float %487 %487
%489 = OpCompositeExtract %v3float %486 0
%490 = OpCompositeExtract %v3float %488 0
%491 = OpFAdd %v3float %489 %490
%492 = OpCompositeExtract %v3float %486 1
%493 = OpCompositeExtract %v3float %488 1
%494 = OpFAdd %v3float %492 %493
%495 = OpCompositeConstruct %mat2v3float %491 %494
OpStore %_1_m23 %495
%496 = OpLoad %bool %_0_ok
OpSelectionMerge %498 None
OpBranchConditional %496 %497 %498
%497 = OpLabel
%499 = OpLoad %mat2v3float %_1_m23
%501 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%502 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%500 = OpCompositeConstruct %mat2v3float %501 %502
%503 = OpCompositeExtract %v3float %499 0
%504 = OpCompositeExtract %v3float %500 0
%505 = OpFOrdEqual %v3bool %503 %504
%506 = OpAll %bool %505
%507 = OpCompositeExtract %v3float %499 1
%508 = OpCompositeExtract %v3float %500 1
%509 = OpFOrdEqual %v3bool %507 %508
%510 = OpAll %bool %509
%511 = OpLogicalAnd %bool %506 %510
OpBranch %498
%498 = OpLabel
%512 = OpPhi %bool %false %459 %511 %497
OpStore %_0_ok %512
%513 = OpLoad %mat3v2float %_3_m32
%514 = OpCompositeConstruct %v2float %float_2 %float_2
%515 = OpCompositeConstruct %mat3v2float %514 %514 %514
%516 = OpCompositeExtract %v2float %513 0
%517 = OpCompositeExtract %v2float %515 0
%518 = OpFSub %v2float %516 %517
%519 = OpCompositeExtract %v2float %513 1
%520 = OpCompositeExtract %v2float %515 1
%521 = OpFSub %v2float %519 %520
%522 = OpCompositeExtract %v2float %513 2
%523 = OpCompositeExtract %v2float %515 2
%524 = OpFSub %v2float %522 %523
%525 = OpCompositeConstruct %mat3v2float %518 %521 %524
OpStore %_3_m32 %525
%526 = OpLoad %bool %_0_ok
OpSelectionMerge %528 None
OpBranchConditional %526 %527 %528
%527 = OpLabel
%529 = OpLoad %mat3v2float %_3_m32
%531 = OpCompositeConstruct %v2float %float_2 %float_n2
%532 = OpCompositeConstruct %v2float %float_n2 %float_2
%533 = OpCompositeConstruct %v2float %float_n2 %float_n2
%530 = OpCompositeConstruct %mat3v2float %531 %532 %533
%534 = OpCompositeExtract %v2float %529 0
%535 = OpCompositeExtract %v2float %530 0
%536 = OpFOrdEqual %v2bool %534 %535
%537 = OpAll %bool %536
%538 = OpCompositeExtract %v2float %529 1
%539 = OpCompositeExtract %v2float %530 1
%540 = OpFOrdEqual %v2bool %538 %539
%541 = OpAll %bool %540
%542 = OpLogicalAnd %bool %537 %541
%543 = OpCompositeExtract %v2float %529 2
%544 = OpCompositeExtract %v2float %530 2
%545 = OpFOrdEqual %v2bool %543 %544
%546 = OpAll %bool %545
%547 = OpLogicalAnd %bool %542 %546
OpBranch %528
%528 = OpLabel
%548 = OpPhi %bool %false %498 %547 %527
OpStore %_0_ok %548
%549 = OpLoad %mat2v4float %_2_m24
%550 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%551 = OpCompositeConstruct %mat2v4float %550 %550
%552 = OpCompositeExtract %v4float %549 0
%553 = OpCompositeExtract %v4float %551 0
%554 = OpFDiv %v4float %552 %553
%555 = OpCompositeExtract %v4float %549 1
%556 = OpCompositeExtract %v4float %551 1
%557 = OpFDiv %v4float %555 %556
%558 = OpCompositeConstruct %mat2v4float %554 %557
OpStore %_2_m24 %558
%559 = OpLoad %bool %_0_ok
OpSelectionMerge %561 None
OpBranchConditional %559 %560 %561
%560 = OpLabel
%562 = OpLoad %mat2v4float %_2_m24
%564 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%565 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%563 = OpCompositeConstruct %mat2v4float %564 %565
%566 = OpCompositeExtract %v4float %562 0
%567 = OpCompositeExtract %v4float %563 0
%568 = OpFOrdEqual %v4bool %566 %567
%569 = OpAll %bool %568
%570 = OpCompositeExtract %v4float %562 1
%571 = OpCompositeExtract %v4float %563 1
%572 = OpFOrdEqual %v4bool %570 %571
%573 = OpAll %bool %572
%574 = OpLogicalAnd %bool %569 %573
OpBranch %561
%561 = OpLabel
%575 = OpPhi %bool %false %528 %574 %560
OpStore %_0_ok %575
%576 = OpLoad %bool %_0_ok
OpSelectionMerge %578 None
OpBranchConditional %576 %577 %578
%577 = OpLabel
%579 = OpFunctionCall %bool %test_half_b
OpBranch %578
%578 = OpLabel
%580 = OpPhi %bool %false %561 %579 %577
OpSelectionMerge %585 None
OpBranchConditional %580 %583 %584
%583 = OpLabel
%586 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%590 = OpLoad %v4float %586
OpStore %581 %590
OpBranch %585
%584 = OpLabel
%591 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%593 = OpLoad %v4float %591
OpStore %581 %593
OpBranch %585
%585 = OpLabel
%594 = OpLoad %v4float %581
OpReturnValue %594
OpFunctionEnd
