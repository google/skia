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
OpDecorate %312 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
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
OpDecorate %351 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %403 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %483 RelaxedPrecision
OpDecorate %512 RelaxedPrecision
OpDecorate %547 RelaxedPrecision
OpDecorate %580 RelaxedPrecision
OpDecorate %601 RelaxedPrecision
OpDecorate %628 RelaxedPrecision
OpDecorate %667 RelaxedPrecision
OpDecorate %697 RelaxedPrecision
OpDecorate %730 RelaxedPrecision
OpDecorate %747 RelaxedPrecision
OpDecorate %761 RelaxedPrecision
OpDecorate %764 RelaxedPrecision
OpDecorate %765 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%float_n2 = OpConstant %float -2
%float_0_75 = OpConstant %float 0.75
%404 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%310 = OpLoad %mat2v3float %m23
%312 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%313 = OpCompositeConstruct %mat2v3float %312 %312
%314 = OpCompositeExtract %v3float %310 0
%315 = OpCompositeExtract %v3float %313 0
%316 = OpFAdd %v3float %314 %315
%317 = OpCompositeExtract %v3float %310 1
%318 = OpCompositeExtract %v3float %313 1
%319 = OpFAdd %v3float %317 %318
%320 = OpCompositeConstruct %mat2v3float %316 %319
OpStore %m23 %320
%321 = OpLoad %bool %ok
OpSelectionMerge %323 None
OpBranchConditional %321 %322 %323
%322 = OpLabel
%324 = OpLoad %mat2v3float %m23
%326 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%327 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%325 = OpCompositeConstruct %mat2v3float %326 %327
%328 = OpCompositeExtract %v3float %324 0
%329 = OpCompositeExtract %v3float %325 0
%330 = OpFOrdEqual %v3bool %328 %329
%331 = OpAll %bool %330
%332 = OpCompositeExtract %v3float %324 1
%333 = OpCompositeExtract %v3float %325 1
%334 = OpFOrdEqual %v3bool %332 %333
%335 = OpAll %bool %334
%336 = OpLogicalAnd %bool %331 %335
OpBranch %323
%323 = OpLabel
%337 = OpPhi %bool %false %282 %336 %322
OpStore %ok %337
%338 = OpLoad %mat3v2float %m32
%339 = OpCompositeConstruct %v2float %float_2 %float_2
%340 = OpCompositeConstruct %mat3v2float %339 %339 %339
%341 = OpCompositeExtract %v2float %338 0
%342 = OpCompositeExtract %v2float %340 0
%343 = OpFSub %v2float %341 %342
%344 = OpCompositeExtract %v2float %338 1
%345 = OpCompositeExtract %v2float %340 1
%346 = OpFSub %v2float %344 %345
%347 = OpCompositeExtract %v2float %338 2
%348 = OpCompositeExtract %v2float %340 2
%349 = OpFSub %v2float %347 %348
%350 = OpCompositeConstruct %mat3v2float %343 %346 %349
OpStore %m32 %350
%351 = OpLoad %bool %ok
OpSelectionMerge %353 None
OpBranchConditional %351 %352 %353
%352 = OpLabel
%354 = OpLoad %mat3v2float %m32
%357 = OpCompositeConstruct %v2float %float_2 %float_n2
%358 = OpCompositeConstruct %v2float %float_n2 %float_2
%359 = OpCompositeConstruct %v2float %float_n2 %float_n2
%356 = OpCompositeConstruct %mat3v2float %357 %358 %359
%360 = OpCompositeExtract %v2float %354 0
%361 = OpCompositeExtract %v2float %356 0
%362 = OpFOrdEqual %v2bool %360 %361
%363 = OpAll %bool %362
%364 = OpCompositeExtract %v2float %354 1
%365 = OpCompositeExtract %v2float %356 1
%366 = OpFOrdEqual %v2bool %364 %365
%367 = OpAll %bool %366
%368 = OpLogicalAnd %bool %363 %367
%369 = OpCompositeExtract %v2float %354 2
%370 = OpCompositeExtract %v2float %356 2
%371 = OpFOrdEqual %v2bool %369 %370
%372 = OpAll %bool %371
%373 = OpLogicalAnd %bool %368 %372
OpBranch %353
%353 = OpLabel
%374 = OpPhi %bool %false %323 %373 %352
OpStore %ok %374
%375 = OpLoad %mat2v4float %m24
%376 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%377 = OpCompositeConstruct %mat2v4float %376 %376
%378 = OpCompositeExtract %v4float %375 0
%379 = OpCompositeExtract %v4float %377 0
%380 = OpFDiv %v4float %378 %379
%381 = OpCompositeExtract %v4float %375 1
%382 = OpCompositeExtract %v4float %377 1
%383 = OpFDiv %v4float %381 %382
%384 = OpCompositeConstruct %mat2v4float %380 %383
OpStore %m24 %384
%385 = OpLoad %bool %ok
OpSelectionMerge %387 None
OpBranchConditional %385 %386 %387
%386 = OpLabel
%388 = OpLoad %mat2v4float %m24
%391 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%392 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%390 = OpCompositeConstruct %mat2v4float %391 %392
%393 = OpCompositeExtract %v4float %388 0
%394 = OpCompositeExtract %v4float %390 0
%395 = OpFOrdEqual %v4bool %393 %394
%396 = OpAll %bool %395
%397 = OpCompositeExtract %v4float %388 1
%398 = OpCompositeExtract %v4float %390 1
%399 = OpFOrdEqual %v4bool %397 %398
%400 = OpAll %bool %399
%401 = OpLogicalAnd %bool %396 %400
OpBranch %387
%387 = OpLabel
%402 = OpPhi %bool %false %353 %401 %386
OpStore %ok %402
%403 = OpLoad %bool %ok
OpReturnValue %403
OpFunctionEnd
%main = OpFunction %v4float None %404
%405 = OpFunctionParameter %_ptr_Function_v2float
%406 = OpLabel
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
%752 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%410 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%411 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%409 = OpCompositeConstruct %mat2v3float %410 %411
OpStore %_1_m23 %409
%412 = OpLoad %bool %_0_ok
OpSelectionMerge %414 None
OpBranchConditional %412 %413 %414
%413 = OpLabel
%415 = OpLoad %mat2v3float %_1_m23
%417 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%418 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%416 = OpCompositeConstruct %mat2v3float %417 %418
%419 = OpCompositeExtract %v3float %415 0
%420 = OpCompositeExtract %v3float %416 0
%421 = OpFOrdEqual %v3bool %419 %420
%422 = OpAll %bool %421
%423 = OpCompositeExtract %v3float %415 1
%424 = OpCompositeExtract %v3float %416 1
%425 = OpFOrdEqual %v3bool %423 %424
%426 = OpAll %bool %425
%427 = OpLogicalAnd %bool %422 %426
OpBranch %414
%414 = OpLabel
%428 = OpPhi %bool %false %406 %427 %413
OpStore %_0_ok %428
%431 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%432 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%430 = OpCompositeConstruct %mat2v4float %431 %432
OpStore %_2_m24 %430
%433 = OpLoad %bool %_0_ok
OpSelectionMerge %435 None
OpBranchConditional %433 %434 %435
%434 = OpLabel
%436 = OpLoad %mat2v4float %_2_m24
%438 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%439 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%437 = OpCompositeConstruct %mat2v4float %438 %439
%440 = OpCompositeExtract %v4float %436 0
%441 = OpCompositeExtract %v4float %437 0
%442 = OpFOrdEqual %v4bool %440 %441
%443 = OpAll %bool %442
%444 = OpCompositeExtract %v4float %436 1
%445 = OpCompositeExtract %v4float %437 1
%446 = OpFOrdEqual %v4bool %444 %445
%447 = OpAll %bool %446
%448 = OpLogicalAnd %bool %443 %447
OpBranch %435
%435 = OpLabel
%449 = OpPhi %bool %false %414 %448 %434
OpStore %_0_ok %449
%452 = OpCompositeConstruct %v2float %float_4 %float_0
%453 = OpCompositeConstruct %v2float %float_0 %float_4
%454 = OpCompositeConstruct %v2float %float_0 %float_0
%451 = OpCompositeConstruct %mat3v2float %452 %453 %454
OpStore %_3_m32 %451
%455 = OpLoad %bool %_0_ok
OpSelectionMerge %457 None
OpBranchConditional %455 %456 %457
%456 = OpLabel
%458 = OpLoad %mat3v2float %_3_m32
%460 = OpCompositeConstruct %v2float %float_4 %float_0
%461 = OpCompositeConstruct %v2float %float_0 %float_4
%462 = OpCompositeConstruct %v2float %float_0 %float_0
%459 = OpCompositeConstruct %mat3v2float %460 %461 %462
%463 = OpCompositeExtract %v2float %458 0
%464 = OpCompositeExtract %v2float %459 0
%465 = OpFOrdEqual %v2bool %463 %464
%466 = OpAll %bool %465
%467 = OpCompositeExtract %v2float %458 1
%468 = OpCompositeExtract %v2float %459 1
%469 = OpFOrdEqual %v2bool %467 %468
%470 = OpAll %bool %469
%471 = OpLogicalAnd %bool %466 %470
%472 = OpCompositeExtract %v2float %458 2
%473 = OpCompositeExtract %v2float %459 2
%474 = OpFOrdEqual %v2bool %472 %473
%475 = OpAll %bool %474
%476 = OpLogicalAnd %bool %471 %475
OpBranch %457
%457 = OpLabel
%477 = OpPhi %bool %false %435 %476 %456
OpStore %_0_ok %477
%480 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%481 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%482 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%479 = OpCompositeConstruct %mat3v4float %480 %481 %482
OpStore %_4_m34 %479
%483 = OpLoad %bool %_0_ok
OpSelectionMerge %485 None
OpBranchConditional %483 %484 %485
%484 = OpLabel
%486 = OpLoad %mat3v4float %_4_m34
%488 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%489 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%490 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%487 = OpCompositeConstruct %mat3v4float %488 %489 %490
%491 = OpCompositeExtract %v4float %486 0
%492 = OpCompositeExtract %v4float %487 0
%493 = OpFOrdEqual %v4bool %491 %492
%494 = OpAll %bool %493
%495 = OpCompositeExtract %v4float %486 1
%496 = OpCompositeExtract %v4float %487 1
%497 = OpFOrdEqual %v4bool %495 %496
%498 = OpAll %bool %497
%499 = OpLogicalAnd %bool %494 %498
%500 = OpCompositeExtract %v4float %486 2
%501 = OpCompositeExtract %v4float %487 2
%502 = OpFOrdEqual %v4bool %500 %501
%503 = OpAll %bool %502
%504 = OpLogicalAnd %bool %499 %503
OpBranch %485
%485 = OpLabel
%505 = OpPhi %bool %false %457 %504 %484
OpStore %_0_ok %505
%508 = OpCompositeConstruct %v2float %float_6 %float_0
%509 = OpCompositeConstruct %v2float %float_0 %float_6
%510 = OpCompositeConstruct %v2float %float_0 %float_0
%511 = OpCompositeConstruct %v2float %float_0 %float_0
%507 = OpCompositeConstruct %mat4v2float %508 %509 %510 %511
OpStore %_5_m42 %507
%512 = OpLoad %bool %_0_ok
OpSelectionMerge %514 None
OpBranchConditional %512 %513 %514
%513 = OpLabel
%515 = OpLoad %mat4v2float %_5_m42
%517 = OpCompositeConstruct %v2float %float_6 %float_0
%518 = OpCompositeConstruct %v2float %float_0 %float_6
%519 = OpCompositeConstruct %v2float %float_0 %float_0
%520 = OpCompositeConstruct %v2float %float_0 %float_0
%516 = OpCompositeConstruct %mat4v2float %517 %518 %519 %520
%521 = OpCompositeExtract %v2float %515 0
%522 = OpCompositeExtract %v2float %516 0
%523 = OpFOrdEqual %v2bool %521 %522
%524 = OpAll %bool %523
%525 = OpCompositeExtract %v2float %515 1
%526 = OpCompositeExtract %v2float %516 1
%527 = OpFOrdEqual %v2bool %525 %526
%528 = OpAll %bool %527
%529 = OpLogicalAnd %bool %524 %528
%530 = OpCompositeExtract %v2float %515 2
%531 = OpCompositeExtract %v2float %516 2
%532 = OpFOrdEqual %v2bool %530 %531
%533 = OpAll %bool %532
%534 = OpLogicalAnd %bool %529 %533
%535 = OpCompositeExtract %v2float %515 3
%536 = OpCompositeExtract %v2float %516 3
%537 = OpFOrdEqual %v2bool %535 %536
%538 = OpAll %bool %537
%539 = OpLogicalAnd %bool %534 %538
OpBranch %514
%514 = OpLabel
%540 = OpPhi %bool %false %485 %539 %513
OpStore %_0_ok %540
%543 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%544 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%545 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%546 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%542 = OpCompositeConstruct %mat4v3float %543 %544 %545 %546
OpStore %_6_m43 %542
%547 = OpLoad %bool %_0_ok
OpSelectionMerge %549 None
OpBranchConditional %547 %548 %549
%548 = OpLabel
%550 = OpLoad %mat4v3float %_6_m43
%552 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%553 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%554 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%555 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%551 = OpCompositeConstruct %mat4v3float %552 %553 %554 %555
%556 = OpCompositeExtract %v3float %550 0
%557 = OpCompositeExtract %v3float %551 0
%558 = OpFOrdEqual %v3bool %556 %557
%559 = OpAll %bool %558
%560 = OpCompositeExtract %v3float %550 1
%561 = OpCompositeExtract %v3float %551 1
%562 = OpFOrdEqual %v3bool %560 %561
%563 = OpAll %bool %562
%564 = OpLogicalAnd %bool %559 %563
%565 = OpCompositeExtract %v3float %550 2
%566 = OpCompositeExtract %v3float %551 2
%567 = OpFOrdEqual %v3bool %565 %566
%568 = OpAll %bool %567
%569 = OpLogicalAnd %bool %564 %568
%570 = OpCompositeExtract %v3float %550 3
%571 = OpCompositeExtract %v3float %551 3
%572 = OpFOrdEqual %v3bool %570 %571
%573 = OpAll %bool %572
%574 = OpLogicalAnd %bool %569 %573
OpBranch %549
%549 = OpLabel
%575 = OpPhi %bool %false %514 %574 %548
OpStore %_0_ok %575
%577 = OpLoad %mat3v2float %_3_m32
%578 = OpLoad %mat2v3float %_1_m23
%579 = OpMatrixTimesMatrix %mat2v2float %577 %578
OpStore %_7_m22 %579
%580 = OpLoad %bool %_0_ok
OpSelectionMerge %582 None
OpBranchConditional %580 %581 %582
%581 = OpLabel
%583 = OpLoad %mat2v2float %_7_m22
%585 = OpCompositeConstruct %v2float %float_8 %float_0
%586 = OpCompositeConstruct %v2float %float_0 %float_8
%584 = OpCompositeConstruct %mat2v2float %585 %586
%587 = OpCompositeExtract %v2float %583 0
%588 = OpCompositeExtract %v2float %584 0
%589 = OpFOrdEqual %v2bool %587 %588
%590 = OpAll %bool %589
%591 = OpCompositeExtract %v2float %583 1
%592 = OpCompositeExtract %v2float %584 1
%593 = OpFOrdEqual %v2bool %591 %592
%594 = OpAll %bool %593
%595 = OpLogicalAnd %bool %590 %594
OpBranch %582
%582 = OpLabel
%596 = OpPhi %bool %false %549 %595 %581
OpStore %_0_ok %596
%598 = OpLoad %mat4v3float %_6_m43
%599 = OpLoad %mat3v4float %_4_m34
%600 = OpMatrixTimesMatrix %mat3v3float %598 %599
OpStore %_8_m33 %600
%601 = OpLoad %bool %_0_ok
OpSelectionMerge %603 None
OpBranchConditional %601 %602 %603
%602 = OpLabel
%604 = OpLoad %mat3v3float %_8_m33
%606 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%607 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%608 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%605 = OpCompositeConstruct %mat3v3float %606 %607 %608
%609 = OpCompositeExtract %v3float %604 0
%610 = OpCompositeExtract %v3float %605 0
%611 = OpFOrdEqual %v3bool %609 %610
%612 = OpAll %bool %611
%613 = OpCompositeExtract %v3float %604 1
%614 = OpCompositeExtract %v3float %605 1
%615 = OpFOrdEqual %v3bool %613 %614
%616 = OpAll %bool %615
%617 = OpLogicalAnd %bool %612 %616
%618 = OpCompositeExtract %v3float %604 2
%619 = OpCompositeExtract %v3float %605 2
%620 = OpFOrdEqual %v3bool %618 %619
%621 = OpAll %bool %620
%622 = OpLogicalAnd %bool %617 %621
OpBranch %603
%603 = OpLabel
%623 = OpPhi %bool %false %582 %622 %602
OpStore %_0_ok %623
%625 = OpLoad %mat2v4float %_2_m24
%626 = OpLoad %mat4v2float %_5_m42
%627 = OpMatrixTimesMatrix %mat4v4float %625 %626
OpStore %_9_m44 %627
%628 = OpLoad %bool %_0_ok
OpSelectionMerge %630 None
OpBranchConditional %628 %629 %630
%629 = OpLabel
%631 = OpLoad %mat4v4float %_9_m44
%633 = OpCompositeConstruct %v4float %float_18 %float_0 %float_0 %float_0
%634 = OpCompositeConstruct %v4float %float_0 %float_18 %float_0 %float_0
%635 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%636 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%632 = OpCompositeConstruct %mat4v4float %633 %634 %635 %636
%637 = OpCompositeExtract %v4float %631 0
%638 = OpCompositeExtract %v4float %632 0
%639 = OpFOrdEqual %v4bool %637 %638
%640 = OpAll %bool %639
%641 = OpCompositeExtract %v4float %631 1
%642 = OpCompositeExtract %v4float %632 1
%643 = OpFOrdEqual %v4bool %641 %642
%644 = OpAll %bool %643
%645 = OpLogicalAnd %bool %640 %644
%646 = OpCompositeExtract %v4float %631 2
%647 = OpCompositeExtract %v4float %632 2
%648 = OpFOrdEqual %v4bool %646 %647
%649 = OpAll %bool %648
%650 = OpLogicalAnd %bool %645 %649
%651 = OpCompositeExtract %v4float %631 3
%652 = OpCompositeExtract %v4float %632 3
%653 = OpFOrdEqual %v4bool %651 %652
%654 = OpAll %bool %653
%655 = OpLogicalAnd %bool %650 %654
OpBranch %630
%630 = OpLabel
%656 = OpPhi %bool %false %603 %655 %629
OpStore %_0_ok %656
%657 = OpLoad %mat2v3float %_1_m23
%658 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%659 = OpCompositeConstruct %mat2v3float %658 %658
%660 = OpCompositeExtract %v3float %657 0
%661 = OpCompositeExtract %v3float %659 0
%662 = OpFAdd %v3float %660 %661
%663 = OpCompositeExtract %v3float %657 1
%664 = OpCompositeExtract %v3float %659 1
%665 = OpFAdd %v3float %663 %664
%666 = OpCompositeConstruct %mat2v3float %662 %665
OpStore %_1_m23 %666
%667 = OpLoad %bool %_0_ok
OpSelectionMerge %669 None
OpBranchConditional %667 %668 %669
%668 = OpLabel
%670 = OpLoad %mat2v3float %_1_m23
%672 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%673 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%671 = OpCompositeConstruct %mat2v3float %672 %673
%674 = OpCompositeExtract %v3float %670 0
%675 = OpCompositeExtract %v3float %671 0
%676 = OpFOrdEqual %v3bool %674 %675
%677 = OpAll %bool %676
%678 = OpCompositeExtract %v3float %670 1
%679 = OpCompositeExtract %v3float %671 1
%680 = OpFOrdEqual %v3bool %678 %679
%681 = OpAll %bool %680
%682 = OpLogicalAnd %bool %677 %681
OpBranch %669
%669 = OpLabel
%683 = OpPhi %bool %false %630 %682 %668
OpStore %_0_ok %683
%684 = OpLoad %mat3v2float %_3_m32
%685 = OpCompositeConstruct %v2float %float_2 %float_2
%686 = OpCompositeConstruct %mat3v2float %685 %685 %685
%687 = OpCompositeExtract %v2float %684 0
%688 = OpCompositeExtract %v2float %686 0
%689 = OpFSub %v2float %687 %688
%690 = OpCompositeExtract %v2float %684 1
%691 = OpCompositeExtract %v2float %686 1
%692 = OpFSub %v2float %690 %691
%693 = OpCompositeExtract %v2float %684 2
%694 = OpCompositeExtract %v2float %686 2
%695 = OpFSub %v2float %693 %694
%696 = OpCompositeConstruct %mat3v2float %689 %692 %695
OpStore %_3_m32 %696
%697 = OpLoad %bool %_0_ok
OpSelectionMerge %699 None
OpBranchConditional %697 %698 %699
%698 = OpLabel
%700 = OpLoad %mat3v2float %_3_m32
%702 = OpCompositeConstruct %v2float %float_2 %float_n2
%703 = OpCompositeConstruct %v2float %float_n2 %float_2
%704 = OpCompositeConstruct %v2float %float_n2 %float_n2
%701 = OpCompositeConstruct %mat3v2float %702 %703 %704
%705 = OpCompositeExtract %v2float %700 0
%706 = OpCompositeExtract %v2float %701 0
%707 = OpFOrdEqual %v2bool %705 %706
%708 = OpAll %bool %707
%709 = OpCompositeExtract %v2float %700 1
%710 = OpCompositeExtract %v2float %701 1
%711 = OpFOrdEqual %v2bool %709 %710
%712 = OpAll %bool %711
%713 = OpLogicalAnd %bool %708 %712
%714 = OpCompositeExtract %v2float %700 2
%715 = OpCompositeExtract %v2float %701 2
%716 = OpFOrdEqual %v2bool %714 %715
%717 = OpAll %bool %716
%718 = OpLogicalAnd %bool %713 %717
OpBranch %699
%699 = OpLabel
%719 = OpPhi %bool %false %669 %718 %698
OpStore %_0_ok %719
%720 = OpLoad %mat2v4float %_2_m24
%721 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%722 = OpCompositeConstruct %mat2v4float %721 %721
%723 = OpCompositeExtract %v4float %720 0
%724 = OpCompositeExtract %v4float %722 0
%725 = OpFDiv %v4float %723 %724
%726 = OpCompositeExtract %v4float %720 1
%727 = OpCompositeExtract %v4float %722 1
%728 = OpFDiv %v4float %726 %727
%729 = OpCompositeConstruct %mat2v4float %725 %728
OpStore %_2_m24 %729
%730 = OpLoad %bool %_0_ok
OpSelectionMerge %732 None
OpBranchConditional %730 %731 %732
%731 = OpLabel
%733 = OpLoad %mat2v4float %_2_m24
%735 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%736 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%734 = OpCompositeConstruct %mat2v4float %735 %736
%737 = OpCompositeExtract %v4float %733 0
%738 = OpCompositeExtract %v4float %734 0
%739 = OpFOrdEqual %v4bool %737 %738
%740 = OpAll %bool %739
%741 = OpCompositeExtract %v4float %733 1
%742 = OpCompositeExtract %v4float %734 1
%743 = OpFOrdEqual %v4bool %741 %742
%744 = OpAll %bool %743
%745 = OpLogicalAnd %bool %740 %744
OpBranch %732
%732 = OpLabel
%746 = OpPhi %bool %false %699 %745 %731
OpStore %_0_ok %746
%747 = OpLoad %bool %_0_ok
OpSelectionMerge %749 None
OpBranchConditional %747 %748 %749
%748 = OpLabel
%750 = OpFunctionCall %bool %test_half_b
OpBranch %749
%749 = OpLabel
%751 = OpPhi %bool %false %732 %750 %748
OpSelectionMerge %756 None
OpBranchConditional %751 %754 %755
%754 = OpLabel
%757 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%761 = OpLoad %v4float %757
OpStore %752 %761
OpBranch %756
%755 = OpLabel
%762 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%764 = OpLoad %v4float %762
OpStore %752 %764
OpBranch %756
%756 = OpLabel
%765 = OpLoad %v4float %752
OpReturnValue %765
OpFunctionEnd
