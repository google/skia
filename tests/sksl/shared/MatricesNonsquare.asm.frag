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
OpDecorate %310 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %443 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %497 RelaxedPrecision
OpDecorate %527 RelaxedPrecision
OpDecorate %550 RelaxedPrecision
OpDecorate %564 RelaxedPrecision
OpDecorate %567 RelaxedPrecision
OpDecorate %568 RelaxedPrecision
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
%302 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%301 = OpLoad %bool %ok
OpReturnValue %301
OpFunctionEnd
%main = OpFunction %v4float None %302
%303 = OpFunctionParameter %_ptr_Function_v2float
%304 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%555 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%308 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%309 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%307 = OpCompositeConstruct %mat2v3float %308 %309
OpStore %_1_m23 %307
%310 = OpLoad %bool %_0_ok
OpSelectionMerge %312 None
OpBranchConditional %310 %311 %312
%311 = OpLabel
%313 = OpLoad %mat2v3float %_1_m23
%315 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%316 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%314 = OpCompositeConstruct %mat2v3float %315 %316
%317 = OpCompositeExtract %v3float %313 0
%318 = OpCompositeExtract %v3float %314 0
%319 = OpFOrdEqual %v3bool %317 %318
%320 = OpAll %bool %319
%321 = OpCompositeExtract %v3float %313 1
%322 = OpCompositeExtract %v3float %314 1
%323 = OpFOrdEqual %v3bool %321 %322
%324 = OpAll %bool %323
%325 = OpLogicalAnd %bool %320 %324
OpBranch %312
%312 = OpLabel
%326 = OpPhi %bool %false %304 %325 %311
OpStore %_0_ok %326
%329 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%330 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%328 = OpCompositeConstruct %mat2v4float %329 %330
OpStore %_2_m24 %328
%331 = OpLoad %bool %_0_ok
OpSelectionMerge %333 None
OpBranchConditional %331 %332 %333
%332 = OpLabel
%334 = OpLoad %mat2v4float %_2_m24
%336 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%337 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%335 = OpCompositeConstruct %mat2v4float %336 %337
%338 = OpCompositeExtract %v4float %334 0
%339 = OpCompositeExtract %v4float %335 0
%340 = OpFOrdEqual %v4bool %338 %339
%341 = OpAll %bool %340
%342 = OpCompositeExtract %v4float %334 1
%343 = OpCompositeExtract %v4float %335 1
%344 = OpFOrdEqual %v4bool %342 %343
%345 = OpAll %bool %344
%346 = OpLogicalAnd %bool %341 %345
OpBranch %333
%333 = OpLabel
%347 = OpPhi %bool %false %312 %346 %332
OpStore %_0_ok %347
%350 = OpCompositeConstruct %v2float %float_4 %float_0
%351 = OpCompositeConstruct %v2float %float_0 %float_4
%352 = OpCompositeConstruct %v2float %float_0 %float_0
%349 = OpCompositeConstruct %mat3v2float %350 %351 %352
OpStore %_3_m32 %349
%353 = OpLoad %bool %_0_ok
OpSelectionMerge %355 None
OpBranchConditional %353 %354 %355
%354 = OpLabel
%356 = OpLoad %mat3v2float %_3_m32
%358 = OpCompositeConstruct %v2float %float_4 %float_0
%359 = OpCompositeConstruct %v2float %float_0 %float_4
%360 = OpCompositeConstruct %v2float %float_0 %float_0
%357 = OpCompositeConstruct %mat3v2float %358 %359 %360
%361 = OpCompositeExtract %v2float %356 0
%362 = OpCompositeExtract %v2float %357 0
%363 = OpFOrdEqual %v2bool %361 %362
%364 = OpAll %bool %363
%365 = OpCompositeExtract %v2float %356 1
%366 = OpCompositeExtract %v2float %357 1
%367 = OpFOrdEqual %v2bool %365 %366
%368 = OpAll %bool %367
%369 = OpLogicalAnd %bool %364 %368
%370 = OpCompositeExtract %v2float %356 2
%371 = OpCompositeExtract %v2float %357 2
%372 = OpFOrdEqual %v2bool %370 %371
%373 = OpAll %bool %372
%374 = OpLogicalAnd %bool %369 %373
OpBranch %355
%355 = OpLabel
%375 = OpPhi %bool %false %333 %374 %354
OpStore %_0_ok %375
%378 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%379 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%380 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%377 = OpCompositeConstruct %mat3v4float %378 %379 %380
OpStore %_4_m34 %377
%381 = OpLoad %bool %_0_ok
OpSelectionMerge %383 None
OpBranchConditional %381 %382 %383
%382 = OpLabel
%384 = OpLoad %mat3v4float %_4_m34
%386 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%387 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%388 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%385 = OpCompositeConstruct %mat3v4float %386 %387 %388
%389 = OpCompositeExtract %v4float %384 0
%390 = OpCompositeExtract %v4float %385 0
%391 = OpFOrdEqual %v4bool %389 %390
%392 = OpAll %bool %391
%393 = OpCompositeExtract %v4float %384 1
%394 = OpCompositeExtract %v4float %385 1
%395 = OpFOrdEqual %v4bool %393 %394
%396 = OpAll %bool %395
%397 = OpLogicalAnd %bool %392 %396
%398 = OpCompositeExtract %v4float %384 2
%399 = OpCompositeExtract %v4float %385 2
%400 = OpFOrdEqual %v4bool %398 %399
%401 = OpAll %bool %400
%402 = OpLogicalAnd %bool %397 %401
OpBranch %383
%383 = OpLabel
%403 = OpPhi %bool %false %355 %402 %382
OpStore %_0_ok %403
%406 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%407 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%408 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%409 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%405 = OpCompositeConstruct %mat4v3float %406 %407 %408 %409
OpStore %_6_m43 %405
%410 = OpLoad %bool %_0_ok
OpSelectionMerge %412 None
OpBranchConditional %410 %411 %412
%411 = OpLabel
%413 = OpLoad %mat4v3float %_6_m43
%415 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%416 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%417 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%418 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%414 = OpCompositeConstruct %mat4v3float %415 %416 %417 %418
%419 = OpCompositeExtract %v3float %413 0
%420 = OpCompositeExtract %v3float %414 0
%421 = OpFOrdEqual %v3bool %419 %420
%422 = OpAll %bool %421
%423 = OpCompositeExtract %v3float %413 1
%424 = OpCompositeExtract %v3float %414 1
%425 = OpFOrdEqual %v3bool %423 %424
%426 = OpAll %bool %425
%427 = OpLogicalAnd %bool %422 %426
%428 = OpCompositeExtract %v3float %413 2
%429 = OpCompositeExtract %v3float %414 2
%430 = OpFOrdEqual %v3bool %428 %429
%431 = OpAll %bool %430
%432 = OpLogicalAnd %bool %427 %431
%433 = OpCompositeExtract %v3float %413 3
%434 = OpCompositeExtract %v3float %414 3
%435 = OpFOrdEqual %v3bool %433 %434
%436 = OpAll %bool %435
%437 = OpLogicalAnd %bool %432 %436
OpBranch %412
%412 = OpLabel
%438 = OpPhi %bool %false %383 %437 %411
OpStore %_0_ok %438
%440 = OpLoad %mat3v2float %_3_m32
%441 = OpLoad %mat2v3float %_1_m23
%442 = OpMatrixTimesMatrix %mat2v2float %440 %441
OpStore %_7_m22 %442
%443 = OpLoad %bool %_0_ok
OpSelectionMerge %445 None
OpBranchConditional %443 %444 %445
%444 = OpLabel
%446 = OpLoad %mat2v2float %_7_m22
%448 = OpCompositeConstruct %v2float %float_8 %float_0
%449 = OpCompositeConstruct %v2float %float_0 %float_8
%447 = OpCompositeConstruct %mat2v2float %448 %449
%450 = OpCompositeExtract %v2float %446 0
%451 = OpCompositeExtract %v2float %447 0
%452 = OpFOrdEqual %v2bool %450 %451
%453 = OpAll %bool %452
%454 = OpCompositeExtract %v2float %446 1
%455 = OpCompositeExtract %v2float %447 1
%456 = OpFOrdEqual %v2bool %454 %455
%457 = OpAll %bool %456
%458 = OpLogicalAnd %bool %453 %457
OpBranch %445
%445 = OpLabel
%459 = OpPhi %bool %false %412 %458 %444
OpStore %_0_ok %459
%461 = OpLoad %mat4v3float %_6_m43
%462 = OpLoad %mat3v4float %_4_m34
%463 = OpMatrixTimesMatrix %mat3v3float %461 %462
OpStore %_8_m33 %463
%464 = OpLoad %bool %_0_ok
OpSelectionMerge %466 None
OpBranchConditional %464 %465 %466
%465 = OpLabel
%467 = OpLoad %mat3v3float %_8_m33
%469 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%470 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%471 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%468 = OpCompositeConstruct %mat3v3float %469 %470 %471
%472 = OpCompositeExtract %v3float %467 0
%473 = OpCompositeExtract %v3float %468 0
%474 = OpFOrdEqual %v3bool %472 %473
%475 = OpAll %bool %474
%476 = OpCompositeExtract %v3float %467 1
%477 = OpCompositeExtract %v3float %468 1
%478 = OpFOrdEqual %v3bool %476 %477
%479 = OpAll %bool %478
%480 = OpLogicalAnd %bool %475 %479
%481 = OpCompositeExtract %v3float %467 2
%482 = OpCompositeExtract %v3float %468 2
%483 = OpFOrdEqual %v3bool %481 %482
%484 = OpAll %bool %483
%485 = OpLogicalAnd %bool %480 %484
OpBranch %466
%466 = OpLabel
%486 = OpPhi %bool %false %445 %485 %465
OpStore %_0_ok %486
%487 = OpLoad %mat2v3float %_1_m23
%488 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%489 = OpCompositeConstruct %mat2v3float %488 %488
%490 = OpCompositeExtract %v3float %487 0
%491 = OpCompositeExtract %v3float %489 0
%492 = OpFAdd %v3float %490 %491
%493 = OpCompositeExtract %v3float %487 1
%494 = OpCompositeExtract %v3float %489 1
%495 = OpFAdd %v3float %493 %494
%496 = OpCompositeConstruct %mat2v3float %492 %495
OpStore %_1_m23 %496
%497 = OpLoad %bool %_0_ok
OpSelectionMerge %499 None
OpBranchConditional %497 %498 %499
%498 = OpLabel
%500 = OpLoad %mat2v3float %_1_m23
%502 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%503 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%501 = OpCompositeConstruct %mat2v3float %502 %503
%504 = OpCompositeExtract %v3float %500 0
%505 = OpCompositeExtract %v3float %501 0
%506 = OpFOrdEqual %v3bool %504 %505
%507 = OpAll %bool %506
%508 = OpCompositeExtract %v3float %500 1
%509 = OpCompositeExtract %v3float %501 1
%510 = OpFOrdEqual %v3bool %508 %509
%511 = OpAll %bool %510
%512 = OpLogicalAnd %bool %507 %511
OpBranch %499
%499 = OpLabel
%513 = OpPhi %bool %false %466 %512 %498
OpStore %_0_ok %513
%514 = OpLoad %mat3v2float %_3_m32
%515 = OpCompositeConstruct %v2float %float_2 %float_2
%516 = OpCompositeConstruct %mat3v2float %515 %515 %515
%517 = OpCompositeExtract %v2float %514 0
%518 = OpCompositeExtract %v2float %516 0
%519 = OpFSub %v2float %517 %518
%520 = OpCompositeExtract %v2float %514 1
%521 = OpCompositeExtract %v2float %516 1
%522 = OpFSub %v2float %520 %521
%523 = OpCompositeExtract %v2float %514 2
%524 = OpCompositeExtract %v2float %516 2
%525 = OpFSub %v2float %523 %524
%526 = OpCompositeConstruct %mat3v2float %519 %522 %525
OpStore %_3_m32 %526
%527 = OpLoad %bool %_0_ok
OpSelectionMerge %529 None
OpBranchConditional %527 %528 %529
%528 = OpLabel
%530 = OpLoad %mat3v2float %_3_m32
%532 = OpCompositeConstruct %v2float %float_2 %float_n2
%533 = OpCompositeConstruct %v2float %float_n2 %float_2
%534 = OpCompositeConstruct %v2float %float_n2 %float_n2
%531 = OpCompositeConstruct %mat3v2float %532 %533 %534
%535 = OpCompositeExtract %v2float %530 0
%536 = OpCompositeExtract %v2float %531 0
%537 = OpFOrdEqual %v2bool %535 %536
%538 = OpAll %bool %537
%539 = OpCompositeExtract %v2float %530 1
%540 = OpCompositeExtract %v2float %531 1
%541 = OpFOrdEqual %v2bool %539 %540
%542 = OpAll %bool %541
%543 = OpLogicalAnd %bool %538 %542
%544 = OpCompositeExtract %v2float %530 2
%545 = OpCompositeExtract %v2float %531 2
%546 = OpFOrdEqual %v2bool %544 %545
%547 = OpAll %bool %546
%548 = OpLogicalAnd %bool %543 %547
OpBranch %529
%529 = OpLabel
%549 = OpPhi %bool %false %499 %548 %528
OpStore %_0_ok %549
%550 = OpLoad %bool %_0_ok
OpSelectionMerge %552 None
OpBranchConditional %550 %551 %552
%551 = OpLabel
%553 = OpFunctionCall %bool %test_half_b
OpBranch %552
%552 = OpLabel
%554 = OpPhi %bool %false %529 %553 %551
OpSelectionMerge %559 None
OpBranchConditional %554 %557 %558
%557 = OpLabel
%560 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%564 = OpLoad %v4float %560
OpStore %555 %564
OpBranch %559
%558 = OpLabel
%565 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%567 = OpLoad %v4float %565
OpStore %555 %567
OpBranch %559
%559 = OpLabel
%568 = OpLoad %v4float %555
OpReturnValue %568
OpFunctionEnd
