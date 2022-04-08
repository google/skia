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
OpDecorate %m34 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %m42 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %m22 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
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
OpDecorate %312 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %437 RelaxedPrecision
OpDecorate %463 RelaxedPrecision
OpDecorate %494 RelaxedPrecision
OpDecorate %523 RelaxedPrecision
OpDecorate %544 RelaxedPrecision
OpDecorate %576 RelaxedPrecision
OpDecorate %603 RelaxedPrecision
OpDecorate %632 RelaxedPrecision
OpDecorate %647 RelaxedPrecision
OpDecorate %661 RelaxedPrecision
OpDecorate %664 RelaxedPrecision
OpDecorate %665 RelaxedPrecision
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
%42 = OpConstantComposite %v3float %float_2 %float_0 %float_0
%43 = OpConstantComposite %v3float %float_0 %float_2 %float_0
%v3bool = OpTypeVector %bool 3
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_3 = OpConstant %float 3
%67 = OpConstantComposite %v4float %float_3 %float_0 %float_0 %float_0
%68 = OpConstantComposite %v4float %float_0 %float_3 %float_0 %float_0
%v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%93 = OpConstantComposite %v2float %float_4 %float_0
%94 = OpConstantComposite %v2float %float_0 %float_4
%v2bool = OpTypeVector %bool 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%124 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
%125 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
%126 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%156 = OpConstantComposite %v2float %float_6 %float_0
%157 = OpConstantComposite %v2float %float_0 %float_6
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%192 = OpConstantComposite %v3float %float_7 %float_0 %float_0
%193 = OpConstantComposite %v3float %float_0 %float_7 %float_0
%194 = OpConstantComposite %v3float %float_0 %float_0 %float_7
%195 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_8 = OpConstant %float 8
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%float_1 = OpConstant %float 1
%273 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%286 = OpConstantComposite %v3float %float_3 %float_1 %float_1
%287 = OpConstantComposite %v3float %float_1 %float_3 %float_1
%300 = OpConstantComposite %v2float %float_2 %float_2
%float_n2 = OpConstant %float -2
%317 = OpConstantComposite %v2float %float_2 %float_n2
%318 = OpConstantComposite %v2float %float_n2 %float_2
%319 = OpConstantComposite %v2float %float_n2 %float_n2
%337 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%float_0_75 = OpConstant %float 0.75
%351 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0 %float_0
%352 = OpConstantComposite %v4float %float_0 %float_0_75 %float_0 %float_0
%365 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%44 = OpCompositeConstruct %mat2v3float %42 %43
%46 = OpCompositeExtract %v3float %41 0
%47 = OpCompositeExtract %v3float %44 0
%48 = OpFOrdEqual %v3bool %46 %47
%49 = OpAll %bool %48
%50 = OpCompositeExtract %v3float %41 1
%51 = OpCompositeExtract %v3float %44 1
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
%69 = OpCompositeConstruct %mat2v4float %67 %68
%71 = OpCompositeExtract %v4float %66 0
%72 = OpCompositeExtract %v4float %69 0
%73 = OpFOrdEqual %v4bool %71 %72
%74 = OpAll %bool %73
%75 = OpCompositeExtract %v4float %66 1
%76 = OpCompositeExtract %v4float %69 1
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
%95 = OpCompositeConstruct %mat3v2float %93 %94 %20
%97 = OpCompositeExtract %v2float %92 0
%98 = OpCompositeExtract %v2float %95 0
%99 = OpFOrdEqual %v2bool %97 %98
%100 = OpAll %bool %99
%101 = OpCompositeExtract %v2float %92 1
%102 = OpCompositeExtract %v2float %95 1
%103 = OpFOrdEqual %v2bool %101 %102
%104 = OpAll %bool %103
%105 = OpLogicalAnd %bool %100 %104
%106 = OpCompositeExtract %v2float %92 2
%107 = OpCompositeExtract %v2float %95 2
%108 = OpFOrdEqual %v2bool %106 %107
%109 = OpAll %bool %108
%110 = OpLogicalAnd %bool %105 %109
OpBranch %91
%91 = OpLabel
%111 = OpPhi %bool %false %65 %110 %90
OpStore %ok %111
%117 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%118 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%119 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%116 = OpCompositeConstruct %mat3v4float %117 %118 %119
OpStore %m34 %116
%120 = OpLoad %bool %ok
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%123 = OpLoad %mat3v4float %m34
%127 = OpCompositeConstruct %mat3v4float %124 %125 %126
%128 = OpCompositeExtract %v4float %123 0
%129 = OpCompositeExtract %v4float %127 0
%130 = OpFOrdEqual %v4bool %128 %129
%131 = OpAll %bool %130
%132 = OpCompositeExtract %v4float %123 1
%133 = OpCompositeExtract %v4float %127 1
%134 = OpFOrdEqual %v4bool %132 %133
%135 = OpAll %bool %134
%136 = OpLogicalAnd %bool %131 %135
%137 = OpCompositeExtract %v4float %123 2
%138 = OpCompositeExtract %v4float %127 2
%139 = OpFOrdEqual %v4bool %137 %138
%140 = OpAll %bool %139
%141 = OpLogicalAnd %bool %136 %140
OpBranch %122
%122 = OpLabel
%142 = OpPhi %bool %false %91 %141 %121
OpStore %ok %142
%148 = OpCompositeConstruct %v2float %float_6 %float_0
%149 = OpCompositeConstruct %v2float %float_0 %float_6
%150 = OpCompositeConstruct %v2float %float_0 %float_0
%151 = OpCompositeConstruct %v2float %float_0 %float_0
%147 = OpCompositeConstruct %mat4v2float %148 %149 %150 %151
OpStore %m42 %147
%152 = OpLoad %bool %ok
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%155 = OpLoad %mat4v2float %m42
%158 = OpCompositeConstruct %mat4v2float %156 %157 %20 %20
%159 = OpCompositeExtract %v2float %155 0
%160 = OpCompositeExtract %v2float %158 0
%161 = OpFOrdEqual %v2bool %159 %160
%162 = OpAll %bool %161
%163 = OpCompositeExtract %v2float %155 1
%164 = OpCompositeExtract %v2float %158 1
%165 = OpFOrdEqual %v2bool %163 %164
%166 = OpAll %bool %165
%167 = OpLogicalAnd %bool %162 %166
%168 = OpCompositeExtract %v2float %155 2
%169 = OpCompositeExtract %v2float %158 2
%170 = OpFOrdEqual %v2bool %168 %169
%171 = OpAll %bool %170
%172 = OpLogicalAnd %bool %167 %171
%173 = OpCompositeExtract %v2float %155 3
%174 = OpCompositeExtract %v2float %158 3
%175 = OpFOrdEqual %v2bool %173 %174
%176 = OpAll %bool %175
%177 = OpLogicalAnd %bool %172 %176
OpBranch %154
%154 = OpLabel
%178 = OpPhi %bool %false %122 %177 %153
OpStore %ok %178
%184 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%185 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%186 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%187 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%183 = OpCompositeConstruct %mat4v3float %184 %185 %186 %187
OpStore %m43 %183
%188 = OpLoad %bool %ok
OpSelectionMerge %190 None
OpBranchConditional %188 %189 %190
%189 = OpLabel
%191 = OpLoad %mat4v3float %m43
%196 = OpCompositeConstruct %mat4v3float %192 %193 %194 %195
%197 = OpCompositeExtract %v3float %191 0
%198 = OpCompositeExtract %v3float %196 0
%199 = OpFOrdEqual %v3bool %197 %198
%200 = OpAll %bool %199
%201 = OpCompositeExtract %v3float %191 1
%202 = OpCompositeExtract %v3float %196 1
%203 = OpFOrdEqual %v3bool %201 %202
%204 = OpAll %bool %203
%205 = OpLogicalAnd %bool %200 %204
%206 = OpCompositeExtract %v3float %191 2
%207 = OpCompositeExtract %v3float %196 2
%208 = OpFOrdEqual %v3bool %206 %207
%209 = OpAll %bool %208
%210 = OpLogicalAnd %bool %205 %209
%211 = OpCompositeExtract %v3float %191 3
%212 = OpCompositeExtract %v3float %196 3
%213 = OpFOrdEqual %v3bool %211 %212
%214 = OpAll %bool %213
%215 = OpLogicalAnd %bool %210 %214
OpBranch %190
%190 = OpLabel
%216 = OpPhi %bool %false %154 %215 %189
OpStore %ok %216
%220 = OpLoad %mat3v2float %m32
%221 = OpLoad %mat2v3float %m23
%222 = OpMatrixTimesMatrix %mat2v2float %220 %221
OpStore %m22 %222
%223 = OpLoad %bool %ok
OpSelectionMerge %225 None
OpBranchConditional %223 %224 %225
%224 = OpLabel
%226 = OpLoad %mat2v2float %m22
%229 = OpCompositeConstruct %v2float %float_8 %float_0
%230 = OpCompositeConstruct %v2float %float_0 %float_8
%228 = OpCompositeConstruct %mat2v2float %229 %230
%231 = OpCompositeExtract %v2float %226 0
%232 = OpCompositeExtract %v2float %228 0
%233 = OpFOrdEqual %v2bool %231 %232
%234 = OpAll %bool %233
%235 = OpCompositeExtract %v2float %226 1
%236 = OpCompositeExtract %v2float %228 1
%237 = OpFOrdEqual %v2bool %235 %236
%238 = OpAll %bool %237
%239 = OpLogicalAnd %bool %234 %238
OpBranch %225
%225 = OpLabel
%240 = OpPhi %bool %false %190 %239 %224
OpStore %ok %240
%244 = OpLoad %mat4v3float %m43
%245 = OpLoad %mat3v4float %m34
%246 = OpMatrixTimesMatrix %mat3v3float %244 %245
OpStore %m33 %246
%247 = OpLoad %bool %ok
OpSelectionMerge %249 None
OpBranchConditional %247 %248 %249
%248 = OpLabel
%250 = OpLoad %mat3v3float %m33
%253 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%254 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%255 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%252 = OpCompositeConstruct %mat3v3float %253 %254 %255
%256 = OpCompositeExtract %v3float %250 0
%257 = OpCompositeExtract %v3float %252 0
%258 = OpFOrdEqual %v3bool %256 %257
%259 = OpAll %bool %258
%260 = OpCompositeExtract %v3float %250 1
%261 = OpCompositeExtract %v3float %252 1
%262 = OpFOrdEqual %v3bool %260 %261
%263 = OpAll %bool %262
%264 = OpLogicalAnd %bool %259 %263
%265 = OpCompositeExtract %v3float %250 2
%266 = OpCompositeExtract %v3float %252 2
%267 = OpFOrdEqual %v3bool %265 %266
%268 = OpAll %bool %267
%269 = OpLogicalAnd %bool %264 %268
OpBranch %249
%249 = OpLabel
%270 = OpPhi %bool %false %225 %269 %248
OpStore %ok %270
%271 = OpLoad %mat2v3float %m23
%274 = OpCompositeConstruct %mat2v3float %273 %273
%275 = OpCompositeExtract %v3float %271 0
%276 = OpCompositeExtract %v3float %274 0
%277 = OpFAdd %v3float %275 %276
%278 = OpCompositeExtract %v3float %271 1
%279 = OpCompositeExtract %v3float %274 1
%280 = OpFAdd %v3float %278 %279
%281 = OpCompositeConstruct %mat2v3float %277 %280
OpStore %m23 %281
%282 = OpLoad %bool %ok
OpSelectionMerge %284 None
OpBranchConditional %282 %283 %284
%283 = OpLabel
%285 = OpLoad %mat2v3float %m23
%288 = OpCompositeConstruct %mat2v3float %286 %287
%289 = OpCompositeExtract %v3float %285 0
%290 = OpCompositeExtract %v3float %288 0
%291 = OpFOrdEqual %v3bool %289 %290
%292 = OpAll %bool %291
%293 = OpCompositeExtract %v3float %285 1
%294 = OpCompositeExtract %v3float %288 1
%295 = OpFOrdEqual %v3bool %293 %294
%296 = OpAll %bool %295
%297 = OpLogicalAnd %bool %292 %296
OpBranch %284
%284 = OpLabel
%298 = OpPhi %bool %false %249 %297 %283
OpStore %ok %298
%299 = OpLoad %mat3v2float %m32
%301 = OpCompositeConstruct %mat3v2float %300 %300 %300
%302 = OpCompositeExtract %v2float %299 0
%303 = OpCompositeExtract %v2float %301 0
%304 = OpFSub %v2float %302 %303
%305 = OpCompositeExtract %v2float %299 1
%306 = OpCompositeExtract %v2float %301 1
%307 = OpFSub %v2float %305 %306
%308 = OpCompositeExtract %v2float %299 2
%309 = OpCompositeExtract %v2float %301 2
%310 = OpFSub %v2float %308 %309
%311 = OpCompositeConstruct %mat3v2float %304 %307 %310
OpStore %m32 %311
%312 = OpLoad %bool %ok
OpSelectionMerge %314 None
OpBranchConditional %312 %313 %314
%313 = OpLabel
%315 = OpLoad %mat3v2float %m32
%320 = OpCompositeConstruct %mat3v2float %317 %318 %319
%321 = OpCompositeExtract %v2float %315 0
%322 = OpCompositeExtract %v2float %320 0
%323 = OpFOrdEqual %v2bool %321 %322
%324 = OpAll %bool %323
%325 = OpCompositeExtract %v2float %315 1
%326 = OpCompositeExtract %v2float %320 1
%327 = OpFOrdEqual %v2bool %325 %326
%328 = OpAll %bool %327
%329 = OpLogicalAnd %bool %324 %328
%330 = OpCompositeExtract %v2float %315 2
%331 = OpCompositeExtract %v2float %320 2
%332 = OpFOrdEqual %v2bool %330 %331
%333 = OpAll %bool %332
%334 = OpLogicalAnd %bool %329 %333
OpBranch %314
%314 = OpLabel
%335 = OpPhi %bool %false %284 %334 %313
OpStore %ok %335
%336 = OpLoad %mat2v4float %m24
%338 = OpCompositeConstruct %mat2v4float %337 %337
%339 = OpCompositeExtract %v4float %336 0
%340 = OpCompositeExtract %v4float %338 0
%341 = OpFDiv %v4float %339 %340
%342 = OpCompositeExtract %v4float %336 1
%343 = OpCompositeExtract %v4float %338 1
%344 = OpFDiv %v4float %342 %343
%345 = OpCompositeConstruct %mat2v4float %341 %344
OpStore %m24 %345
%346 = OpLoad %bool %ok
OpSelectionMerge %348 None
OpBranchConditional %346 %347 %348
%347 = OpLabel
%349 = OpLoad %mat2v4float %m24
%353 = OpCompositeConstruct %mat2v4float %351 %352
%354 = OpCompositeExtract %v4float %349 0
%355 = OpCompositeExtract %v4float %353 0
%356 = OpFOrdEqual %v4bool %354 %355
%357 = OpAll %bool %356
%358 = OpCompositeExtract %v4float %349 1
%359 = OpCompositeExtract %v4float %353 1
%360 = OpFOrdEqual %v4bool %358 %359
%361 = OpAll %bool %360
%362 = OpLogicalAnd %bool %357 %361
OpBranch %348
%348 = OpLabel
%363 = OpPhi %bool %false %314 %362 %347
OpStore %ok %363
%364 = OpLoad %bool %ok
OpReturnValue %364
OpFunctionEnd
%main = OpFunction %v4float None %365
%366 = OpFunctionParameter %_ptr_Function_v2float
%367 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%652 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%371 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%372 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%370 = OpCompositeConstruct %mat2v3float %371 %372
OpStore %_1_m23 %370
%373 = OpLoad %bool %_0_ok
OpSelectionMerge %375 None
OpBranchConditional %373 %374 %375
%374 = OpLabel
%376 = OpLoad %mat2v3float %_1_m23
%377 = OpCompositeConstruct %mat2v3float %42 %43
%378 = OpCompositeExtract %v3float %376 0
%379 = OpCompositeExtract %v3float %377 0
%380 = OpFOrdEqual %v3bool %378 %379
%381 = OpAll %bool %380
%382 = OpCompositeExtract %v3float %376 1
%383 = OpCompositeExtract %v3float %377 1
%384 = OpFOrdEqual %v3bool %382 %383
%385 = OpAll %bool %384
%386 = OpLogicalAnd %bool %381 %385
OpBranch %375
%375 = OpLabel
%387 = OpPhi %bool %false %367 %386 %374
OpStore %_0_ok %387
%390 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%391 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%389 = OpCompositeConstruct %mat2v4float %390 %391
OpStore %_2_m24 %389
%392 = OpLoad %bool %_0_ok
OpSelectionMerge %394 None
OpBranchConditional %392 %393 %394
%393 = OpLabel
%395 = OpLoad %mat2v4float %_2_m24
%396 = OpCompositeConstruct %mat2v4float %67 %68
%397 = OpCompositeExtract %v4float %395 0
%398 = OpCompositeExtract %v4float %396 0
%399 = OpFOrdEqual %v4bool %397 %398
%400 = OpAll %bool %399
%401 = OpCompositeExtract %v4float %395 1
%402 = OpCompositeExtract %v4float %396 1
%403 = OpFOrdEqual %v4bool %401 %402
%404 = OpAll %bool %403
%405 = OpLogicalAnd %bool %400 %404
OpBranch %394
%394 = OpLabel
%406 = OpPhi %bool %false %375 %405 %393
OpStore %_0_ok %406
%409 = OpCompositeConstruct %v2float %float_4 %float_0
%410 = OpCompositeConstruct %v2float %float_0 %float_4
%411 = OpCompositeConstruct %v2float %float_0 %float_0
%408 = OpCompositeConstruct %mat3v2float %409 %410 %411
OpStore %_3_m32 %408
%412 = OpLoad %bool %_0_ok
OpSelectionMerge %414 None
OpBranchConditional %412 %413 %414
%413 = OpLabel
%415 = OpLoad %mat3v2float %_3_m32
%416 = OpCompositeConstruct %mat3v2float %93 %94 %20
%417 = OpCompositeExtract %v2float %415 0
%418 = OpCompositeExtract %v2float %416 0
%419 = OpFOrdEqual %v2bool %417 %418
%420 = OpAll %bool %419
%421 = OpCompositeExtract %v2float %415 1
%422 = OpCompositeExtract %v2float %416 1
%423 = OpFOrdEqual %v2bool %421 %422
%424 = OpAll %bool %423
%425 = OpLogicalAnd %bool %420 %424
%426 = OpCompositeExtract %v2float %415 2
%427 = OpCompositeExtract %v2float %416 2
%428 = OpFOrdEqual %v2bool %426 %427
%429 = OpAll %bool %428
%430 = OpLogicalAnd %bool %425 %429
OpBranch %414
%414 = OpLabel
%431 = OpPhi %bool %false %394 %430 %413
OpStore %_0_ok %431
%434 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%435 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%436 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%433 = OpCompositeConstruct %mat3v4float %434 %435 %436
OpStore %_4_m34 %433
%437 = OpLoad %bool %_0_ok
OpSelectionMerge %439 None
OpBranchConditional %437 %438 %439
%438 = OpLabel
%440 = OpLoad %mat3v4float %_4_m34
%441 = OpCompositeConstruct %mat3v4float %124 %125 %126
%442 = OpCompositeExtract %v4float %440 0
%443 = OpCompositeExtract %v4float %441 0
%444 = OpFOrdEqual %v4bool %442 %443
%445 = OpAll %bool %444
%446 = OpCompositeExtract %v4float %440 1
%447 = OpCompositeExtract %v4float %441 1
%448 = OpFOrdEqual %v4bool %446 %447
%449 = OpAll %bool %448
%450 = OpLogicalAnd %bool %445 %449
%451 = OpCompositeExtract %v4float %440 2
%452 = OpCompositeExtract %v4float %441 2
%453 = OpFOrdEqual %v4bool %451 %452
%454 = OpAll %bool %453
%455 = OpLogicalAnd %bool %450 %454
OpBranch %439
%439 = OpLabel
%456 = OpPhi %bool %false %414 %455 %438
OpStore %_0_ok %456
%459 = OpCompositeConstruct %v2float %float_6 %float_0
%460 = OpCompositeConstruct %v2float %float_0 %float_6
%461 = OpCompositeConstruct %v2float %float_0 %float_0
%462 = OpCompositeConstruct %v2float %float_0 %float_0
%458 = OpCompositeConstruct %mat4v2float %459 %460 %461 %462
OpStore %_5_m42 %458
%463 = OpLoad %bool %_0_ok
OpSelectionMerge %465 None
OpBranchConditional %463 %464 %465
%464 = OpLabel
%466 = OpLoad %mat4v2float %_5_m42
%467 = OpCompositeConstruct %mat4v2float %156 %157 %20 %20
%468 = OpCompositeExtract %v2float %466 0
%469 = OpCompositeExtract %v2float %467 0
%470 = OpFOrdEqual %v2bool %468 %469
%471 = OpAll %bool %470
%472 = OpCompositeExtract %v2float %466 1
%473 = OpCompositeExtract %v2float %467 1
%474 = OpFOrdEqual %v2bool %472 %473
%475 = OpAll %bool %474
%476 = OpLogicalAnd %bool %471 %475
%477 = OpCompositeExtract %v2float %466 2
%478 = OpCompositeExtract %v2float %467 2
%479 = OpFOrdEqual %v2bool %477 %478
%480 = OpAll %bool %479
%481 = OpLogicalAnd %bool %476 %480
%482 = OpCompositeExtract %v2float %466 3
%483 = OpCompositeExtract %v2float %467 3
%484 = OpFOrdEqual %v2bool %482 %483
%485 = OpAll %bool %484
%486 = OpLogicalAnd %bool %481 %485
OpBranch %465
%465 = OpLabel
%487 = OpPhi %bool %false %439 %486 %464
OpStore %_0_ok %487
%490 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%491 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%492 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%493 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%489 = OpCompositeConstruct %mat4v3float %490 %491 %492 %493
OpStore %_6_m43 %489
%494 = OpLoad %bool %_0_ok
OpSelectionMerge %496 None
OpBranchConditional %494 %495 %496
%495 = OpLabel
%497 = OpLoad %mat4v3float %_6_m43
%498 = OpCompositeConstruct %mat4v3float %192 %193 %194 %195
%499 = OpCompositeExtract %v3float %497 0
%500 = OpCompositeExtract %v3float %498 0
%501 = OpFOrdEqual %v3bool %499 %500
%502 = OpAll %bool %501
%503 = OpCompositeExtract %v3float %497 1
%504 = OpCompositeExtract %v3float %498 1
%505 = OpFOrdEqual %v3bool %503 %504
%506 = OpAll %bool %505
%507 = OpLogicalAnd %bool %502 %506
%508 = OpCompositeExtract %v3float %497 2
%509 = OpCompositeExtract %v3float %498 2
%510 = OpFOrdEqual %v3bool %508 %509
%511 = OpAll %bool %510
%512 = OpLogicalAnd %bool %507 %511
%513 = OpCompositeExtract %v3float %497 3
%514 = OpCompositeExtract %v3float %498 3
%515 = OpFOrdEqual %v3bool %513 %514
%516 = OpAll %bool %515
%517 = OpLogicalAnd %bool %512 %516
OpBranch %496
%496 = OpLabel
%518 = OpPhi %bool %false %465 %517 %495
OpStore %_0_ok %518
%520 = OpLoad %mat3v2float %_3_m32
%521 = OpLoad %mat2v3float %_1_m23
%522 = OpMatrixTimesMatrix %mat2v2float %520 %521
OpStore %_7_m22 %522
%523 = OpLoad %bool %_0_ok
OpSelectionMerge %525 None
OpBranchConditional %523 %524 %525
%524 = OpLabel
%526 = OpLoad %mat2v2float %_7_m22
%528 = OpCompositeConstruct %v2float %float_8 %float_0
%529 = OpCompositeConstruct %v2float %float_0 %float_8
%527 = OpCompositeConstruct %mat2v2float %528 %529
%530 = OpCompositeExtract %v2float %526 0
%531 = OpCompositeExtract %v2float %527 0
%532 = OpFOrdEqual %v2bool %530 %531
%533 = OpAll %bool %532
%534 = OpCompositeExtract %v2float %526 1
%535 = OpCompositeExtract %v2float %527 1
%536 = OpFOrdEqual %v2bool %534 %535
%537 = OpAll %bool %536
%538 = OpLogicalAnd %bool %533 %537
OpBranch %525
%525 = OpLabel
%539 = OpPhi %bool %false %496 %538 %524
OpStore %_0_ok %539
%541 = OpLoad %mat4v3float %_6_m43
%542 = OpLoad %mat3v4float %_4_m34
%543 = OpMatrixTimesMatrix %mat3v3float %541 %542
OpStore %_8_m33 %543
%544 = OpLoad %bool %_0_ok
OpSelectionMerge %546 None
OpBranchConditional %544 %545 %546
%545 = OpLabel
%547 = OpLoad %mat3v3float %_8_m33
%549 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%550 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%551 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%548 = OpCompositeConstruct %mat3v3float %549 %550 %551
%552 = OpCompositeExtract %v3float %547 0
%553 = OpCompositeExtract %v3float %548 0
%554 = OpFOrdEqual %v3bool %552 %553
%555 = OpAll %bool %554
%556 = OpCompositeExtract %v3float %547 1
%557 = OpCompositeExtract %v3float %548 1
%558 = OpFOrdEqual %v3bool %556 %557
%559 = OpAll %bool %558
%560 = OpLogicalAnd %bool %555 %559
%561 = OpCompositeExtract %v3float %547 2
%562 = OpCompositeExtract %v3float %548 2
%563 = OpFOrdEqual %v3bool %561 %562
%564 = OpAll %bool %563
%565 = OpLogicalAnd %bool %560 %564
OpBranch %546
%546 = OpLabel
%566 = OpPhi %bool %false %525 %565 %545
OpStore %_0_ok %566
%567 = OpLoad %mat2v3float %_1_m23
%568 = OpCompositeConstruct %mat2v3float %273 %273
%569 = OpCompositeExtract %v3float %567 0
%570 = OpCompositeExtract %v3float %568 0
%571 = OpFAdd %v3float %569 %570
%572 = OpCompositeExtract %v3float %567 1
%573 = OpCompositeExtract %v3float %568 1
%574 = OpFAdd %v3float %572 %573
%575 = OpCompositeConstruct %mat2v3float %571 %574
OpStore %_1_m23 %575
%576 = OpLoad %bool %_0_ok
OpSelectionMerge %578 None
OpBranchConditional %576 %577 %578
%577 = OpLabel
%579 = OpLoad %mat2v3float %_1_m23
%580 = OpCompositeConstruct %mat2v3float %286 %287
%581 = OpCompositeExtract %v3float %579 0
%582 = OpCompositeExtract %v3float %580 0
%583 = OpFOrdEqual %v3bool %581 %582
%584 = OpAll %bool %583
%585 = OpCompositeExtract %v3float %579 1
%586 = OpCompositeExtract %v3float %580 1
%587 = OpFOrdEqual %v3bool %585 %586
%588 = OpAll %bool %587
%589 = OpLogicalAnd %bool %584 %588
OpBranch %578
%578 = OpLabel
%590 = OpPhi %bool %false %546 %589 %577
OpStore %_0_ok %590
%591 = OpLoad %mat3v2float %_3_m32
%592 = OpCompositeConstruct %mat3v2float %300 %300 %300
%593 = OpCompositeExtract %v2float %591 0
%594 = OpCompositeExtract %v2float %592 0
%595 = OpFSub %v2float %593 %594
%596 = OpCompositeExtract %v2float %591 1
%597 = OpCompositeExtract %v2float %592 1
%598 = OpFSub %v2float %596 %597
%599 = OpCompositeExtract %v2float %591 2
%600 = OpCompositeExtract %v2float %592 2
%601 = OpFSub %v2float %599 %600
%602 = OpCompositeConstruct %mat3v2float %595 %598 %601
OpStore %_3_m32 %602
%603 = OpLoad %bool %_0_ok
OpSelectionMerge %605 None
OpBranchConditional %603 %604 %605
%604 = OpLabel
%606 = OpLoad %mat3v2float %_3_m32
%607 = OpCompositeConstruct %mat3v2float %317 %318 %319
%608 = OpCompositeExtract %v2float %606 0
%609 = OpCompositeExtract %v2float %607 0
%610 = OpFOrdEqual %v2bool %608 %609
%611 = OpAll %bool %610
%612 = OpCompositeExtract %v2float %606 1
%613 = OpCompositeExtract %v2float %607 1
%614 = OpFOrdEqual %v2bool %612 %613
%615 = OpAll %bool %614
%616 = OpLogicalAnd %bool %611 %615
%617 = OpCompositeExtract %v2float %606 2
%618 = OpCompositeExtract %v2float %607 2
%619 = OpFOrdEqual %v2bool %617 %618
%620 = OpAll %bool %619
%621 = OpLogicalAnd %bool %616 %620
OpBranch %605
%605 = OpLabel
%622 = OpPhi %bool %false %578 %621 %604
OpStore %_0_ok %622
%623 = OpLoad %mat2v4float %_2_m24
%624 = OpCompositeConstruct %mat2v4float %337 %337
%625 = OpCompositeExtract %v4float %623 0
%626 = OpCompositeExtract %v4float %624 0
%627 = OpFDiv %v4float %625 %626
%628 = OpCompositeExtract %v4float %623 1
%629 = OpCompositeExtract %v4float %624 1
%630 = OpFDiv %v4float %628 %629
%631 = OpCompositeConstruct %mat2v4float %627 %630
OpStore %_2_m24 %631
%632 = OpLoad %bool %_0_ok
OpSelectionMerge %634 None
OpBranchConditional %632 %633 %634
%633 = OpLabel
%635 = OpLoad %mat2v4float %_2_m24
%636 = OpCompositeConstruct %mat2v4float %351 %352
%637 = OpCompositeExtract %v4float %635 0
%638 = OpCompositeExtract %v4float %636 0
%639 = OpFOrdEqual %v4bool %637 %638
%640 = OpAll %bool %639
%641 = OpCompositeExtract %v4float %635 1
%642 = OpCompositeExtract %v4float %636 1
%643 = OpFOrdEqual %v4bool %641 %642
%644 = OpAll %bool %643
%645 = OpLogicalAnd %bool %640 %644
OpBranch %634
%634 = OpLabel
%646 = OpPhi %bool %false %605 %645 %633
OpStore %_0_ok %646
%647 = OpLoad %bool %_0_ok
OpSelectionMerge %649 None
OpBranchConditional %647 %648 %649
%648 = OpLabel
%650 = OpFunctionCall %bool %test_half_b
OpBranch %649
%649 = OpLabel
%651 = OpPhi %bool %false %634 %650 %648
OpSelectionMerge %656 None
OpBranchConditional %651 %654 %655
%654 = OpLabel
%657 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%661 = OpLoad %v4float %657
OpStore %652 %661
OpBranch %656
%655 = OpLabel
%662 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%664 = OpLoad %v4float %662
OpStore %652 %664
OpBranch %656
%656 = OpLabel
%665 = OpLoad %v4float %652
OpReturnValue %665
OpFunctionEnd
