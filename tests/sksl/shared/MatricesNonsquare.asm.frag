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
OpName %m44 "m44"
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
OpName %_9_m44 "_9_m44"
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
OpDecorate %m22 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %m44 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %487 RelaxedPrecision
OpDecorate %508 RelaxedPrecision
OpDecorate %535 RelaxedPrecision
OpDecorate %564 RelaxedPrecision
OpDecorate %578 RelaxedPrecision
OpDecorate %581 RelaxedPrecision
OpDecorate %582 RelaxedPrecision
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
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_8 = OpConstant %float 8
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_18 = OpConstant %float 18
%311 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%m44 = OpVariable %_ptr_Function_mat4v4float Function
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
%223 = OpLoad %mat3v2float %m32
%224 = OpLoad %mat2v3float %m23
%225 = OpMatrixTimesMatrix %mat2v2float %223 %224
OpStore %m22 %225
%226 = OpLoad %bool %ok
OpSelectionMerge %228 None
OpBranchConditional %226 %227 %228
%227 = OpLabel
%229 = OpLoad %mat2v2float %m22
%232 = OpCompositeConstruct %v2float %float_8 %float_0
%233 = OpCompositeConstruct %v2float %float_0 %float_8
%231 = OpCompositeConstruct %mat2v2float %232 %233
%234 = OpCompositeExtract %v2float %229 0
%235 = OpCompositeExtract %v2float %231 0
%236 = OpFOrdEqual %v2bool %234 %235
%237 = OpAll %bool %236
%238 = OpCompositeExtract %v2float %229 1
%239 = OpCompositeExtract %v2float %231 1
%240 = OpFOrdEqual %v2bool %238 %239
%241 = OpAll %bool %240
%242 = OpLogicalAnd %bool %237 %241
OpBranch %228
%228 = OpLabel
%243 = OpPhi %bool %false %193 %242 %227
OpStore %ok %243
%247 = OpLoad %mat4v3float %m43
%248 = OpLoad %mat3v4float %m34
%249 = OpMatrixTimesMatrix %mat3v3float %247 %248
OpStore %m33 %249
%250 = OpLoad %bool %ok
OpSelectionMerge %252 None
OpBranchConditional %250 %251 %252
%251 = OpLabel
%253 = OpLoad %mat3v3float %m33
%256 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%257 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%258 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%255 = OpCompositeConstruct %mat3v3float %256 %257 %258
%259 = OpCompositeExtract %v3float %253 0
%260 = OpCompositeExtract %v3float %255 0
%261 = OpFOrdEqual %v3bool %259 %260
%262 = OpAll %bool %261
%263 = OpCompositeExtract %v3float %253 1
%264 = OpCompositeExtract %v3float %255 1
%265 = OpFOrdEqual %v3bool %263 %264
%266 = OpAll %bool %265
%267 = OpLogicalAnd %bool %262 %266
%268 = OpCompositeExtract %v3float %253 2
%269 = OpCompositeExtract %v3float %255 2
%270 = OpFOrdEqual %v3bool %268 %269
%271 = OpAll %bool %270
%272 = OpLogicalAnd %bool %267 %271
OpBranch %252
%252 = OpLabel
%273 = OpPhi %bool %false %228 %272 %251
OpStore %ok %273
%277 = OpLoad %mat2v4float %m24
%278 = OpLoad %mat4v2float %m42
%279 = OpMatrixTimesMatrix %mat4v4float %277 %278
OpStore %m44 %279
%280 = OpLoad %bool %ok
OpSelectionMerge %282 None
OpBranchConditional %280 %281 %282
%281 = OpLabel
%283 = OpLoad %mat4v4float %m44
%286 = OpCompositeConstruct %v4float %float_18 %float_0 %float_0 %float_0
%287 = OpCompositeConstruct %v4float %float_0 %float_18 %float_0 %float_0
%288 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%289 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%285 = OpCompositeConstruct %mat4v4float %286 %287 %288 %289
%290 = OpCompositeExtract %v4float %283 0
%291 = OpCompositeExtract %v4float %285 0
%292 = OpFOrdEqual %v4bool %290 %291
%293 = OpAll %bool %292
%294 = OpCompositeExtract %v4float %283 1
%295 = OpCompositeExtract %v4float %285 1
%296 = OpFOrdEqual %v4bool %294 %295
%297 = OpAll %bool %296
%298 = OpLogicalAnd %bool %293 %297
%299 = OpCompositeExtract %v4float %283 2
%300 = OpCompositeExtract %v4float %285 2
%301 = OpFOrdEqual %v4bool %299 %300
%302 = OpAll %bool %301
%303 = OpLogicalAnd %bool %298 %302
%304 = OpCompositeExtract %v4float %283 3
%305 = OpCompositeExtract %v4float %285 3
%306 = OpFOrdEqual %v4bool %304 %305
%307 = OpAll %bool %306
%308 = OpLogicalAnd %bool %303 %307
OpBranch %282
%282 = OpLabel
%309 = OpPhi %bool %false %252 %308 %281
OpStore %ok %309
%310 = OpLoad %bool %ok
OpReturnValue %310
OpFunctionEnd
%main = OpFunction %v4float None %311
%312 = OpFunctionParameter %_ptr_Function_v2float
%313 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%_9_m44 = OpVariable %_ptr_Function_mat4v4float Function
%569 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%317 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%318 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%316 = OpCompositeConstruct %mat2v3float %317 %318
OpStore %_1_m23 %316
%319 = OpLoad %bool %_0_ok
OpSelectionMerge %321 None
OpBranchConditional %319 %320 %321
%320 = OpLabel
%322 = OpLoad %mat2v3float %_1_m23
%324 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%325 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%323 = OpCompositeConstruct %mat2v3float %324 %325
%326 = OpCompositeExtract %v3float %322 0
%327 = OpCompositeExtract %v3float %323 0
%328 = OpFOrdEqual %v3bool %326 %327
%329 = OpAll %bool %328
%330 = OpCompositeExtract %v3float %322 1
%331 = OpCompositeExtract %v3float %323 1
%332 = OpFOrdEqual %v3bool %330 %331
%333 = OpAll %bool %332
%334 = OpLogicalAnd %bool %329 %333
OpBranch %321
%321 = OpLabel
%335 = OpPhi %bool %false %313 %334 %320
OpStore %_0_ok %335
%338 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%339 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%337 = OpCompositeConstruct %mat2v4float %338 %339
OpStore %_2_m24 %337
%340 = OpLoad %bool %_0_ok
OpSelectionMerge %342 None
OpBranchConditional %340 %341 %342
%341 = OpLabel
%343 = OpLoad %mat2v4float %_2_m24
%345 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%346 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%344 = OpCompositeConstruct %mat2v4float %345 %346
%347 = OpCompositeExtract %v4float %343 0
%348 = OpCompositeExtract %v4float %344 0
%349 = OpFOrdEqual %v4bool %347 %348
%350 = OpAll %bool %349
%351 = OpCompositeExtract %v4float %343 1
%352 = OpCompositeExtract %v4float %344 1
%353 = OpFOrdEqual %v4bool %351 %352
%354 = OpAll %bool %353
%355 = OpLogicalAnd %bool %350 %354
OpBranch %342
%342 = OpLabel
%356 = OpPhi %bool %false %321 %355 %341
OpStore %_0_ok %356
%359 = OpCompositeConstruct %v2float %float_4 %float_0
%360 = OpCompositeConstruct %v2float %float_0 %float_4
%361 = OpCompositeConstruct %v2float %float_0 %float_0
%358 = OpCompositeConstruct %mat3v2float %359 %360 %361
OpStore %_3_m32 %358
%362 = OpLoad %bool %_0_ok
OpSelectionMerge %364 None
OpBranchConditional %362 %363 %364
%363 = OpLabel
%365 = OpLoad %mat3v2float %_3_m32
%367 = OpCompositeConstruct %v2float %float_4 %float_0
%368 = OpCompositeConstruct %v2float %float_0 %float_4
%369 = OpCompositeConstruct %v2float %float_0 %float_0
%366 = OpCompositeConstruct %mat3v2float %367 %368 %369
%370 = OpCompositeExtract %v2float %365 0
%371 = OpCompositeExtract %v2float %366 0
%372 = OpFOrdEqual %v2bool %370 %371
%373 = OpAll %bool %372
%374 = OpCompositeExtract %v2float %365 1
%375 = OpCompositeExtract %v2float %366 1
%376 = OpFOrdEqual %v2bool %374 %375
%377 = OpAll %bool %376
%378 = OpLogicalAnd %bool %373 %377
%379 = OpCompositeExtract %v2float %365 2
%380 = OpCompositeExtract %v2float %366 2
%381 = OpFOrdEqual %v2bool %379 %380
%382 = OpAll %bool %381
%383 = OpLogicalAnd %bool %378 %382
OpBranch %364
%364 = OpLabel
%384 = OpPhi %bool %false %342 %383 %363
OpStore %_0_ok %384
%387 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%388 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%389 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%386 = OpCompositeConstruct %mat3v4float %387 %388 %389
OpStore %_4_m34 %386
%390 = OpLoad %bool %_0_ok
OpSelectionMerge %392 None
OpBranchConditional %390 %391 %392
%391 = OpLabel
%393 = OpLoad %mat3v4float %_4_m34
%395 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%396 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%397 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%394 = OpCompositeConstruct %mat3v4float %395 %396 %397
%398 = OpCompositeExtract %v4float %393 0
%399 = OpCompositeExtract %v4float %394 0
%400 = OpFOrdEqual %v4bool %398 %399
%401 = OpAll %bool %400
%402 = OpCompositeExtract %v4float %393 1
%403 = OpCompositeExtract %v4float %394 1
%404 = OpFOrdEqual %v4bool %402 %403
%405 = OpAll %bool %404
%406 = OpLogicalAnd %bool %401 %405
%407 = OpCompositeExtract %v4float %393 2
%408 = OpCompositeExtract %v4float %394 2
%409 = OpFOrdEqual %v4bool %407 %408
%410 = OpAll %bool %409
%411 = OpLogicalAnd %bool %406 %410
OpBranch %392
%392 = OpLabel
%412 = OpPhi %bool %false %364 %411 %391
OpStore %_0_ok %412
%415 = OpCompositeConstruct %v2float %float_6 %float_0
%416 = OpCompositeConstruct %v2float %float_0 %float_6
%417 = OpCompositeConstruct %v2float %float_0 %float_0
%418 = OpCompositeConstruct %v2float %float_0 %float_0
%414 = OpCompositeConstruct %mat4v2float %415 %416 %417 %418
OpStore %_5_m42 %414
%419 = OpLoad %bool %_0_ok
OpSelectionMerge %421 None
OpBranchConditional %419 %420 %421
%420 = OpLabel
%422 = OpLoad %mat4v2float %_5_m42
%424 = OpCompositeConstruct %v2float %float_6 %float_0
%425 = OpCompositeConstruct %v2float %float_0 %float_6
%426 = OpCompositeConstruct %v2float %float_0 %float_0
%427 = OpCompositeConstruct %v2float %float_0 %float_0
%423 = OpCompositeConstruct %mat4v2float %424 %425 %426 %427
%428 = OpCompositeExtract %v2float %422 0
%429 = OpCompositeExtract %v2float %423 0
%430 = OpFOrdEqual %v2bool %428 %429
%431 = OpAll %bool %430
%432 = OpCompositeExtract %v2float %422 1
%433 = OpCompositeExtract %v2float %423 1
%434 = OpFOrdEqual %v2bool %432 %433
%435 = OpAll %bool %434
%436 = OpLogicalAnd %bool %431 %435
%437 = OpCompositeExtract %v2float %422 2
%438 = OpCompositeExtract %v2float %423 2
%439 = OpFOrdEqual %v2bool %437 %438
%440 = OpAll %bool %439
%441 = OpLogicalAnd %bool %436 %440
%442 = OpCompositeExtract %v2float %422 3
%443 = OpCompositeExtract %v2float %423 3
%444 = OpFOrdEqual %v2bool %442 %443
%445 = OpAll %bool %444
%446 = OpLogicalAnd %bool %441 %445
OpBranch %421
%421 = OpLabel
%447 = OpPhi %bool %false %392 %446 %420
OpStore %_0_ok %447
%450 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%451 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%452 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%453 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%449 = OpCompositeConstruct %mat4v3float %450 %451 %452 %453
OpStore %_6_m43 %449
%454 = OpLoad %bool %_0_ok
OpSelectionMerge %456 None
OpBranchConditional %454 %455 %456
%455 = OpLabel
%457 = OpLoad %mat4v3float %_6_m43
%459 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%460 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%461 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%462 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%458 = OpCompositeConstruct %mat4v3float %459 %460 %461 %462
%463 = OpCompositeExtract %v3float %457 0
%464 = OpCompositeExtract %v3float %458 0
%465 = OpFOrdEqual %v3bool %463 %464
%466 = OpAll %bool %465
%467 = OpCompositeExtract %v3float %457 1
%468 = OpCompositeExtract %v3float %458 1
%469 = OpFOrdEqual %v3bool %467 %468
%470 = OpAll %bool %469
%471 = OpLogicalAnd %bool %466 %470
%472 = OpCompositeExtract %v3float %457 2
%473 = OpCompositeExtract %v3float %458 2
%474 = OpFOrdEqual %v3bool %472 %473
%475 = OpAll %bool %474
%476 = OpLogicalAnd %bool %471 %475
%477 = OpCompositeExtract %v3float %457 3
%478 = OpCompositeExtract %v3float %458 3
%479 = OpFOrdEqual %v3bool %477 %478
%480 = OpAll %bool %479
%481 = OpLogicalAnd %bool %476 %480
OpBranch %456
%456 = OpLabel
%482 = OpPhi %bool %false %421 %481 %455
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
%492 = OpCompositeConstruct %v2float %float_8 %float_0
%493 = OpCompositeConstruct %v2float %float_0 %float_8
%491 = OpCompositeConstruct %mat2v2float %492 %493
%494 = OpCompositeExtract %v2float %490 0
%495 = OpCompositeExtract %v2float %491 0
%496 = OpFOrdEqual %v2bool %494 %495
%497 = OpAll %bool %496
%498 = OpCompositeExtract %v2float %490 1
%499 = OpCompositeExtract %v2float %491 1
%500 = OpFOrdEqual %v2bool %498 %499
%501 = OpAll %bool %500
%502 = OpLogicalAnd %bool %497 %501
OpBranch %489
%489 = OpLabel
%503 = OpPhi %bool %false %456 %502 %488
OpStore %_0_ok %503
%505 = OpLoad %mat4v3float %_6_m43
%506 = OpLoad %mat3v4float %_4_m34
%507 = OpMatrixTimesMatrix %mat3v3float %505 %506
OpStore %_8_m33 %507
%508 = OpLoad %bool %_0_ok
OpSelectionMerge %510 None
OpBranchConditional %508 %509 %510
%509 = OpLabel
%511 = OpLoad %mat3v3float %_8_m33
%513 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%514 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%515 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%512 = OpCompositeConstruct %mat3v3float %513 %514 %515
%516 = OpCompositeExtract %v3float %511 0
%517 = OpCompositeExtract %v3float %512 0
%518 = OpFOrdEqual %v3bool %516 %517
%519 = OpAll %bool %518
%520 = OpCompositeExtract %v3float %511 1
%521 = OpCompositeExtract %v3float %512 1
%522 = OpFOrdEqual %v3bool %520 %521
%523 = OpAll %bool %522
%524 = OpLogicalAnd %bool %519 %523
%525 = OpCompositeExtract %v3float %511 2
%526 = OpCompositeExtract %v3float %512 2
%527 = OpFOrdEqual %v3bool %525 %526
%528 = OpAll %bool %527
%529 = OpLogicalAnd %bool %524 %528
OpBranch %510
%510 = OpLabel
%530 = OpPhi %bool %false %489 %529 %509
OpStore %_0_ok %530
%532 = OpLoad %mat2v4float %_2_m24
%533 = OpLoad %mat4v2float %_5_m42
%534 = OpMatrixTimesMatrix %mat4v4float %532 %533
OpStore %_9_m44 %534
%535 = OpLoad %bool %_0_ok
OpSelectionMerge %537 None
OpBranchConditional %535 %536 %537
%536 = OpLabel
%538 = OpLoad %mat4v4float %_9_m44
%540 = OpCompositeConstruct %v4float %float_18 %float_0 %float_0 %float_0
%541 = OpCompositeConstruct %v4float %float_0 %float_18 %float_0 %float_0
%542 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%543 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%539 = OpCompositeConstruct %mat4v4float %540 %541 %542 %543
%544 = OpCompositeExtract %v4float %538 0
%545 = OpCompositeExtract %v4float %539 0
%546 = OpFOrdEqual %v4bool %544 %545
%547 = OpAll %bool %546
%548 = OpCompositeExtract %v4float %538 1
%549 = OpCompositeExtract %v4float %539 1
%550 = OpFOrdEqual %v4bool %548 %549
%551 = OpAll %bool %550
%552 = OpLogicalAnd %bool %547 %551
%553 = OpCompositeExtract %v4float %538 2
%554 = OpCompositeExtract %v4float %539 2
%555 = OpFOrdEqual %v4bool %553 %554
%556 = OpAll %bool %555
%557 = OpLogicalAnd %bool %552 %556
%558 = OpCompositeExtract %v4float %538 3
%559 = OpCompositeExtract %v4float %539 3
%560 = OpFOrdEqual %v4bool %558 %559
%561 = OpAll %bool %560
%562 = OpLogicalAnd %bool %557 %561
OpBranch %537
%537 = OpLabel
%563 = OpPhi %bool %false %510 %562 %536
OpStore %_0_ok %563
%564 = OpLoad %bool %_0_ok
OpSelectionMerge %566 None
OpBranchConditional %564 %565 %566
%565 = OpLabel
%567 = OpFunctionCall %bool %test_half_b
OpBranch %566
%566 = OpLabel
%568 = OpPhi %bool %false %537 %567 %565
OpSelectionMerge %573 None
OpBranchConditional %568 %571 %572
%571 = OpLabel
%574 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%578 = OpLoad %v4float %574
OpStore %569 %578
OpBranch %573
%572 = OpLabel
%579 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%581 = OpLoad %v4float %579
OpStore %569 %581
OpBranch %573
%573 = OpLabel
%582 = OpLoad %v4float %569
OpReturnValue %582
OpFunctionEnd
