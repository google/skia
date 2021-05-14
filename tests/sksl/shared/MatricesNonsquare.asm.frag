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
OpDecorate %384 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %484 RelaxedPrecision
OpDecorate %519 RelaxedPrecision
OpDecorate %552 RelaxedPrecision
OpDecorate %573 RelaxedPrecision
OpDecorate %600 RelaxedPrecision
OpDecorate %639 RelaxedPrecision
OpDecorate %669 RelaxedPrecision
OpDecorate %692 RelaxedPrecision
OpDecorate %706 RelaxedPrecision
OpDecorate %709 RelaxedPrecision
OpDecorate %710 RelaxedPrecision
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
%376 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%375 = OpLoad %bool %ok
OpReturnValue %375
OpFunctionEnd
%main = OpFunction %v4float None %376
%377 = OpFunctionParameter %_ptr_Function_v2float
%378 = OpLabel
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
%697 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%382 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%383 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%381 = OpCompositeConstruct %mat2v3float %382 %383
OpStore %_1_m23 %381
%384 = OpLoad %bool %_0_ok
OpSelectionMerge %386 None
OpBranchConditional %384 %385 %386
%385 = OpLabel
%387 = OpLoad %mat2v3float %_1_m23
%389 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%390 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%388 = OpCompositeConstruct %mat2v3float %389 %390
%391 = OpCompositeExtract %v3float %387 0
%392 = OpCompositeExtract %v3float %388 0
%393 = OpFOrdEqual %v3bool %391 %392
%394 = OpAll %bool %393
%395 = OpCompositeExtract %v3float %387 1
%396 = OpCompositeExtract %v3float %388 1
%397 = OpFOrdEqual %v3bool %395 %396
%398 = OpAll %bool %397
%399 = OpLogicalAnd %bool %394 %398
OpBranch %386
%386 = OpLabel
%400 = OpPhi %bool %false %378 %399 %385
OpStore %_0_ok %400
%403 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%404 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%402 = OpCompositeConstruct %mat2v4float %403 %404
OpStore %_2_m24 %402
%405 = OpLoad %bool %_0_ok
OpSelectionMerge %407 None
OpBranchConditional %405 %406 %407
%406 = OpLabel
%408 = OpLoad %mat2v4float %_2_m24
%410 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%411 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%409 = OpCompositeConstruct %mat2v4float %410 %411
%412 = OpCompositeExtract %v4float %408 0
%413 = OpCompositeExtract %v4float %409 0
%414 = OpFOrdEqual %v4bool %412 %413
%415 = OpAll %bool %414
%416 = OpCompositeExtract %v4float %408 1
%417 = OpCompositeExtract %v4float %409 1
%418 = OpFOrdEqual %v4bool %416 %417
%419 = OpAll %bool %418
%420 = OpLogicalAnd %bool %415 %419
OpBranch %407
%407 = OpLabel
%421 = OpPhi %bool %false %386 %420 %406
OpStore %_0_ok %421
%424 = OpCompositeConstruct %v2float %float_4 %float_0
%425 = OpCompositeConstruct %v2float %float_0 %float_4
%426 = OpCompositeConstruct %v2float %float_0 %float_0
%423 = OpCompositeConstruct %mat3v2float %424 %425 %426
OpStore %_3_m32 %423
%427 = OpLoad %bool %_0_ok
OpSelectionMerge %429 None
OpBranchConditional %427 %428 %429
%428 = OpLabel
%430 = OpLoad %mat3v2float %_3_m32
%432 = OpCompositeConstruct %v2float %float_4 %float_0
%433 = OpCompositeConstruct %v2float %float_0 %float_4
%434 = OpCompositeConstruct %v2float %float_0 %float_0
%431 = OpCompositeConstruct %mat3v2float %432 %433 %434
%435 = OpCompositeExtract %v2float %430 0
%436 = OpCompositeExtract %v2float %431 0
%437 = OpFOrdEqual %v2bool %435 %436
%438 = OpAll %bool %437
%439 = OpCompositeExtract %v2float %430 1
%440 = OpCompositeExtract %v2float %431 1
%441 = OpFOrdEqual %v2bool %439 %440
%442 = OpAll %bool %441
%443 = OpLogicalAnd %bool %438 %442
%444 = OpCompositeExtract %v2float %430 2
%445 = OpCompositeExtract %v2float %431 2
%446 = OpFOrdEqual %v2bool %444 %445
%447 = OpAll %bool %446
%448 = OpLogicalAnd %bool %443 %447
OpBranch %429
%429 = OpLabel
%449 = OpPhi %bool %false %407 %448 %428
OpStore %_0_ok %449
%452 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%453 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%454 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%451 = OpCompositeConstruct %mat3v4float %452 %453 %454
OpStore %_4_m34 %451
%455 = OpLoad %bool %_0_ok
OpSelectionMerge %457 None
OpBranchConditional %455 %456 %457
%456 = OpLabel
%458 = OpLoad %mat3v4float %_4_m34
%460 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%461 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%462 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%459 = OpCompositeConstruct %mat3v4float %460 %461 %462
%463 = OpCompositeExtract %v4float %458 0
%464 = OpCompositeExtract %v4float %459 0
%465 = OpFOrdEqual %v4bool %463 %464
%466 = OpAll %bool %465
%467 = OpCompositeExtract %v4float %458 1
%468 = OpCompositeExtract %v4float %459 1
%469 = OpFOrdEqual %v4bool %467 %468
%470 = OpAll %bool %469
%471 = OpLogicalAnd %bool %466 %470
%472 = OpCompositeExtract %v4float %458 2
%473 = OpCompositeExtract %v4float %459 2
%474 = OpFOrdEqual %v4bool %472 %473
%475 = OpAll %bool %474
%476 = OpLogicalAnd %bool %471 %475
OpBranch %457
%457 = OpLabel
%477 = OpPhi %bool %false %429 %476 %456
OpStore %_0_ok %477
%480 = OpCompositeConstruct %v2float %float_6 %float_0
%481 = OpCompositeConstruct %v2float %float_0 %float_6
%482 = OpCompositeConstruct %v2float %float_0 %float_0
%483 = OpCompositeConstruct %v2float %float_0 %float_0
%479 = OpCompositeConstruct %mat4v2float %480 %481 %482 %483
OpStore %_5_m42 %479
%484 = OpLoad %bool %_0_ok
OpSelectionMerge %486 None
OpBranchConditional %484 %485 %486
%485 = OpLabel
%487 = OpLoad %mat4v2float %_5_m42
%489 = OpCompositeConstruct %v2float %float_6 %float_0
%490 = OpCompositeConstruct %v2float %float_0 %float_6
%491 = OpCompositeConstruct %v2float %float_0 %float_0
%492 = OpCompositeConstruct %v2float %float_0 %float_0
%488 = OpCompositeConstruct %mat4v2float %489 %490 %491 %492
%493 = OpCompositeExtract %v2float %487 0
%494 = OpCompositeExtract %v2float %488 0
%495 = OpFOrdEqual %v2bool %493 %494
%496 = OpAll %bool %495
%497 = OpCompositeExtract %v2float %487 1
%498 = OpCompositeExtract %v2float %488 1
%499 = OpFOrdEqual %v2bool %497 %498
%500 = OpAll %bool %499
%501 = OpLogicalAnd %bool %496 %500
%502 = OpCompositeExtract %v2float %487 2
%503 = OpCompositeExtract %v2float %488 2
%504 = OpFOrdEqual %v2bool %502 %503
%505 = OpAll %bool %504
%506 = OpLogicalAnd %bool %501 %505
%507 = OpCompositeExtract %v2float %487 3
%508 = OpCompositeExtract %v2float %488 3
%509 = OpFOrdEqual %v2bool %507 %508
%510 = OpAll %bool %509
%511 = OpLogicalAnd %bool %506 %510
OpBranch %486
%486 = OpLabel
%512 = OpPhi %bool %false %457 %511 %485
OpStore %_0_ok %512
%515 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%516 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%517 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%518 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%514 = OpCompositeConstruct %mat4v3float %515 %516 %517 %518
OpStore %_6_m43 %514
%519 = OpLoad %bool %_0_ok
OpSelectionMerge %521 None
OpBranchConditional %519 %520 %521
%520 = OpLabel
%522 = OpLoad %mat4v3float %_6_m43
%524 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%525 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%526 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%527 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%523 = OpCompositeConstruct %mat4v3float %524 %525 %526 %527
%528 = OpCompositeExtract %v3float %522 0
%529 = OpCompositeExtract %v3float %523 0
%530 = OpFOrdEqual %v3bool %528 %529
%531 = OpAll %bool %530
%532 = OpCompositeExtract %v3float %522 1
%533 = OpCompositeExtract %v3float %523 1
%534 = OpFOrdEqual %v3bool %532 %533
%535 = OpAll %bool %534
%536 = OpLogicalAnd %bool %531 %535
%537 = OpCompositeExtract %v3float %522 2
%538 = OpCompositeExtract %v3float %523 2
%539 = OpFOrdEqual %v3bool %537 %538
%540 = OpAll %bool %539
%541 = OpLogicalAnd %bool %536 %540
%542 = OpCompositeExtract %v3float %522 3
%543 = OpCompositeExtract %v3float %523 3
%544 = OpFOrdEqual %v3bool %542 %543
%545 = OpAll %bool %544
%546 = OpLogicalAnd %bool %541 %545
OpBranch %521
%521 = OpLabel
%547 = OpPhi %bool %false %486 %546 %520
OpStore %_0_ok %547
%549 = OpLoad %mat3v2float %_3_m32
%550 = OpLoad %mat2v3float %_1_m23
%551 = OpMatrixTimesMatrix %mat2v2float %549 %550
OpStore %_7_m22 %551
%552 = OpLoad %bool %_0_ok
OpSelectionMerge %554 None
OpBranchConditional %552 %553 %554
%553 = OpLabel
%555 = OpLoad %mat2v2float %_7_m22
%557 = OpCompositeConstruct %v2float %float_8 %float_0
%558 = OpCompositeConstruct %v2float %float_0 %float_8
%556 = OpCompositeConstruct %mat2v2float %557 %558
%559 = OpCompositeExtract %v2float %555 0
%560 = OpCompositeExtract %v2float %556 0
%561 = OpFOrdEqual %v2bool %559 %560
%562 = OpAll %bool %561
%563 = OpCompositeExtract %v2float %555 1
%564 = OpCompositeExtract %v2float %556 1
%565 = OpFOrdEqual %v2bool %563 %564
%566 = OpAll %bool %565
%567 = OpLogicalAnd %bool %562 %566
OpBranch %554
%554 = OpLabel
%568 = OpPhi %bool %false %521 %567 %553
OpStore %_0_ok %568
%570 = OpLoad %mat4v3float %_6_m43
%571 = OpLoad %mat3v4float %_4_m34
%572 = OpMatrixTimesMatrix %mat3v3float %570 %571
OpStore %_8_m33 %572
%573 = OpLoad %bool %_0_ok
OpSelectionMerge %575 None
OpBranchConditional %573 %574 %575
%574 = OpLabel
%576 = OpLoad %mat3v3float %_8_m33
%578 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%579 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%580 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%577 = OpCompositeConstruct %mat3v3float %578 %579 %580
%581 = OpCompositeExtract %v3float %576 0
%582 = OpCompositeExtract %v3float %577 0
%583 = OpFOrdEqual %v3bool %581 %582
%584 = OpAll %bool %583
%585 = OpCompositeExtract %v3float %576 1
%586 = OpCompositeExtract %v3float %577 1
%587 = OpFOrdEqual %v3bool %585 %586
%588 = OpAll %bool %587
%589 = OpLogicalAnd %bool %584 %588
%590 = OpCompositeExtract %v3float %576 2
%591 = OpCompositeExtract %v3float %577 2
%592 = OpFOrdEqual %v3bool %590 %591
%593 = OpAll %bool %592
%594 = OpLogicalAnd %bool %589 %593
OpBranch %575
%575 = OpLabel
%595 = OpPhi %bool %false %554 %594 %574
OpStore %_0_ok %595
%597 = OpLoad %mat2v4float %_2_m24
%598 = OpLoad %mat4v2float %_5_m42
%599 = OpMatrixTimesMatrix %mat4v4float %597 %598
OpStore %_9_m44 %599
%600 = OpLoad %bool %_0_ok
OpSelectionMerge %602 None
OpBranchConditional %600 %601 %602
%601 = OpLabel
%603 = OpLoad %mat4v4float %_9_m44
%605 = OpCompositeConstruct %v4float %float_18 %float_0 %float_0 %float_0
%606 = OpCompositeConstruct %v4float %float_0 %float_18 %float_0 %float_0
%607 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%608 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%604 = OpCompositeConstruct %mat4v4float %605 %606 %607 %608
%609 = OpCompositeExtract %v4float %603 0
%610 = OpCompositeExtract %v4float %604 0
%611 = OpFOrdEqual %v4bool %609 %610
%612 = OpAll %bool %611
%613 = OpCompositeExtract %v4float %603 1
%614 = OpCompositeExtract %v4float %604 1
%615 = OpFOrdEqual %v4bool %613 %614
%616 = OpAll %bool %615
%617 = OpLogicalAnd %bool %612 %616
%618 = OpCompositeExtract %v4float %603 2
%619 = OpCompositeExtract %v4float %604 2
%620 = OpFOrdEqual %v4bool %618 %619
%621 = OpAll %bool %620
%622 = OpLogicalAnd %bool %617 %621
%623 = OpCompositeExtract %v4float %603 3
%624 = OpCompositeExtract %v4float %604 3
%625 = OpFOrdEqual %v4bool %623 %624
%626 = OpAll %bool %625
%627 = OpLogicalAnd %bool %622 %626
OpBranch %602
%602 = OpLabel
%628 = OpPhi %bool %false %575 %627 %601
OpStore %_0_ok %628
%629 = OpLoad %mat2v3float %_1_m23
%630 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%631 = OpCompositeConstruct %mat2v3float %630 %630
%632 = OpCompositeExtract %v3float %629 0
%633 = OpCompositeExtract %v3float %631 0
%634 = OpFAdd %v3float %632 %633
%635 = OpCompositeExtract %v3float %629 1
%636 = OpCompositeExtract %v3float %631 1
%637 = OpFAdd %v3float %635 %636
%638 = OpCompositeConstruct %mat2v3float %634 %637
OpStore %_1_m23 %638
%639 = OpLoad %bool %_0_ok
OpSelectionMerge %641 None
OpBranchConditional %639 %640 %641
%640 = OpLabel
%642 = OpLoad %mat2v3float %_1_m23
%644 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%645 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%643 = OpCompositeConstruct %mat2v3float %644 %645
%646 = OpCompositeExtract %v3float %642 0
%647 = OpCompositeExtract %v3float %643 0
%648 = OpFOrdEqual %v3bool %646 %647
%649 = OpAll %bool %648
%650 = OpCompositeExtract %v3float %642 1
%651 = OpCompositeExtract %v3float %643 1
%652 = OpFOrdEqual %v3bool %650 %651
%653 = OpAll %bool %652
%654 = OpLogicalAnd %bool %649 %653
OpBranch %641
%641 = OpLabel
%655 = OpPhi %bool %false %602 %654 %640
OpStore %_0_ok %655
%656 = OpLoad %mat3v2float %_3_m32
%657 = OpCompositeConstruct %v2float %float_2 %float_2
%658 = OpCompositeConstruct %mat3v2float %657 %657 %657
%659 = OpCompositeExtract %v2float %656 0
%660 = OpCompositeExtract %v2float %658 0
%661 = OpFSub %v2float %659 %660
%662 = OpCompositeExtract %v2float %656 1
%663 = OpCompositeExtract %v2float %658 1
%664 = OpFSub %v2float %662 %663
%665 = OpCompositeExtract %v2float %656 2
%666 = OpCompositeExtract %v2float %658 2
%667 = OpFSub %v2float %665 %666
%668 = OpCompositeConstruct %mat3v2float %661 %664 %667
OpStore %_3_m32 %668
%669 = OpLoad %bool %_0_ok
OpSelectionMerge %671 None
OpBranchConditional %669 %670 %671
%670 = OpLabel
%672 = OpLoad %mat3v2float %_3_m32
%674 = OpCompositeConstruct %v2float %float_2 %float_n2
%675 = OpCompositeConstruct %v2float %float_n2 %float_2
%676 = OpCompositeConstruct %v2float %float_n2 %float_n2
%673 = OpCompositeConstruct %mat3v2float %674 %675 %676
%677 = OpCompositeExtract %v2float %672 0
%678 = OpCompositeExtract %v2float %673 0
%679 = OpFOrdEqual %v2bool %677 %678
%680 = OpAll %bool %679
%681 = OpCompositeExtract %v2float %672 1
%682 = OpCompositeExtract %v2float %673 1
%683 = OpFOrdEqual %v2bool %681 %682
%684 = OpAll %bool %683
%685 = OpLogicalAnd %bool %680 %684
%686 = OpCompositeExtract %v2float %672 2
%687 = OpCompositeExtract %v2float %673 2
%688 = OpFOrdEqual %v2bool %686 %687
%689 = OpAll %bool %688
%690 = OpLogicalAnd %bool %685 %689
OpBranch %671
%671 = OpLabel
%691 = OpPhi %bool %false %641 %690 %670
OpStore %_0_ok %691
%692 = OpLoad %bool %_0_ok
OpSelectionMerge %694 None
OpBranchConditional %692 %693 %694
%693 = OpLabel
%695 = OpFunctionCall %bool %test_half_b
OpBranch %694
%694 = OpLabel
%696 = OpPhi %bool %false %671 %695 %693
OpSelectionMerge %701 None
OpBranchConditional %696 %699 %700
%699 = OpLabel
%702 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%706 = OpLoad %v4float %702
OpStore %697 %706
OpBranch %701
%700 = OpLabel
%707 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%709 = OpLoad %v4float %707
OpStore %697 %709
OpBranch %701
%701 = OpLabel
%710 = OpLoad %v4float %697
OpReturnValue %710
OpFunctionEnd
