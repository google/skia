### Compilation failed:

error: SPIR-V validation error: Expected floating scalar or vector type as Result Type: FDiv
  %378 = OpFDiv %mat2v4float %375 %377

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
OpDecorate %382 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %477 RelaxedPrecision
OpDecorate %506 RelaxedPrecision
OpDecorate %541 RelaxedPrecision
OpDecorate %574 RelaxedPrecision
OpDecorate %595 RelaxedPrecision
OpDecorate %622 RelaxedPrecision
OpDecorate %661 RelaxedPrecision
OpDecorate %691 RelaxedPrecision
OpDecorate %718 RelaxedPrecision
OpDecorate %735 RelaxedPrecision
OpDecorate %749 RelaxedPrecision
OpDecorate %752 RelaxedPrecision
OpDecorate %753 RelaxedPrecision
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
%398 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%378 = OpFDiv %mat2v4float %375 %377
OpStore %m24 %378
%379 = OpLoad %bool %ok
OpSelectionMerge %381 None
OpBranchConditional %379 %380 %381
%380 = OpLabel
%382 = OpLoad %mat2v4float %m24
%385 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%386 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%384 = OpCompositeConstruct %mat2v4float %385 %386
%387 = OpCompositeExtract %v4float %382 0
%388 = OpCompositeExtract %v4float %384 0
%389 = OpFOrdEqual %v4bool %387 %388
%390 = OpAll %bool %389
%391 = OpCompositeExtract %v4float %382 1
%392 = OpCompositeExtract %v4float %384 1
%393 = OpFOrdEqual %v4bool %391 %392
%394 = OpAll %bool %393
%395 = OpLogicalAnd %bool %390 %394
OpBranch %381
%381 = OpLabel
%396 = OpPhi %bool %false %353 %395 %380
OpStore %ok %396
%397 = OpLoad %bool %ok
OpReturnValue %397
OpFunctionEnd
%main = OpFunction %v4float None %398
%399 = OpFunctionParameter %_ptr_Function_v2float
%400 = OpLabel
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
%740 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%404 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%405 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%403 = OpCompositeConstruct %mat2v3float %404 %405
OpStore %_1_m23 %403
%406 = OpLoad %bool %_0_ok
OpSelectionMerge %408 None
OpBranchConditional %406 %407 %408
%407 = OpLabel
%409 = OpLoad %mat2v3float %_1_m23
%411 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%412 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%410 = OpCompositeConstruct %mat2v3float %411 %412
%413 = OpCompositeExtract %v3float %409 0
%414 = OpCompositeExtract %v3float %410 0
%415 = OpFOrdEqual %v3bool %413 %414
%416 = OpAll %bool %415
%417 = OpCompositeExtract %v3float %409 1
%418 = OpCompositeExtract %v3float %410 1
%419 = OpFOrdEqual %v3bool %417 %418
%420 = OpAll %bool %419
%421 = OpLogicalAnd %bool %416 %420
OpBranch %408
%408 = OpLabel
%422 = OpPhi %bool %false %400 %421 %407
OpStore %_0_ok %422
%425 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%426 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%424 = OpCompositeConstruct %mat2v4float %425 %426
OpStore %_2_m24 %424
%427 = OpLoad %bool %_0_ok
OpSelectionMerge %429 None
OpBranchConditional %427 %428 %429
%428 = OpLabel
%430 = OpLoad %mat2v4float %_2_m24
%432 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%433 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%431 = OpCompositeConstruct %mat2v4float %432 %433
%434 = OpCompositeExtract %v4float %430 0
%435 = OpCompositeExtract %v4float %431 0
%436 = OpFOrdEqual %v4bool %434 %435
%437 = OpAll %bool %436
%438 = OpCompositeExtract %v4float %430 1
%439 = OpCompositeExtract %v4float %431 1
%440 = OpFOrdEqual %v4bool %438 %439
%441 = OpAll %bool %440
%442 = OpLogicalAnd %bool %437 %441
OpBranch %429
%429 = OpLabel
%443 = OpPhi %bool %false %408 %442 %428
OpStore %_0_ok %443
%446 = OpCompositeConstruct %v2float %float_4 %float_0
%447 = OpCompositeConstruct %v2float %float_0 %float_4
%448 = OpCompositeConstruct %v2float %float_0 %float_0
%445 = OpCompositeConstruct %mat3v2float %446 %447 %448
OpStore %_3_m32 %445
%449 = OpLoad %bool %_0_ok
OpSelectionMerge %451 None
OpBranchConditional %449 %450 %451
%450 = OpLabel
%452 = OpLoad %mat3v2float %_3_m32
%454 = OpCompositeConstruct %v2float %float_4 %float_0
%455 = OpCompositeConstruct %v2float %float_0 %float_4
%456 = OpCompositeConstruct %v2float %float_0 %float_0
%453 = OpCompositeConstruct %mat3v2float %454 %455 %456
%457 = OpCompositeExtract %v2float %452 0
%458 = OpCompositeExtract %v2float %453 0
%459 = OpFOrdEqual %v2bool %457 %458
%460 = OpAll %bool %459
%461 = OpCompositeExtract %v2float %452 1
%462 = OpCompositeExtract %v2float %453 1
%463 = OpFOrdEqual %v2bool %461 %462
%464 = OpAll %bool %463
%465 = OpLogicalAnd %bool %460 %464
%466 = OpCompositeExtract %v2float %452 2
%467 = OpCompositeExtract %v2float %453 2
%468 = OpFOrdEqual %v2bool %466 %467
%469 = OpAll %bool %468
%470 = OpLogicalAnd %bool %465 %469
OpBranch %451
%451 = OpLabel
%471 = OpPhi %bool %false %429 %470 %450
OpStore %_0_ok %471
%474 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%475 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%476 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%473 = OpCompositeConstruct %mat3v4float %474 %475 %476
OpStore %_4_m34 %473
%477 = OpLoad %bool %_0_ok
OpSelectionMerge %479 None
OpBranchConditional %477 %478 %479
%478 = OpLabel
%480 = OpLoad %mat3v4float %_4_m34
%482 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%483 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%484 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%481 = OpCompositeConstruct %mat3v4float %482 %483 %484
%485 = OpCompositeExtract %v4float %480 0
%486 = OpCompositeExtract %v4float %481 0
%487 = OpFOrdEqual %v4bool %485 %486
%488 = OpAll %bool %487
%489 = OpCompositeExtract %v4float %480 1
%490 = OpCompositeExtract %v4float %481 1
%491 = OpFOrdEqual %v4bool %489 %490
%492 = OpAll %bool %491
%493 = OpLogicalAnd %bool %488 %492
%494 = OpCompositeExtract %v4float %480 2
%495 = OpCompositeExtract %v4float %481 2
%496 = OpFOrdEqual %v4bool %494 %495
%497 = OpAll %bool %496
%498 = OpLogicalAnd %bool %493 %497
OpBranch %479
%479 = OpLabel
%499 = OpPhi %bool %false %451 %498 %478
OpStore %_0_ok %499
%502 = OpCompositeConstruct %v2float %float_6 %float_0
%503 = OpCompositeConstruct %v2float %float_0 %float_6
%504 = OpCompositeConstruct %v2float %float_0 %float_0
%505 = OpCompositeConstruct %v2float %float_0 %float_0
%501 = OpCompositeConstruct %mat4v2float %502 %503 %504 %505
OpStore %_5_m42 %501
%506 = OpLoad %bool %_0_ok
OpSelectionMerge %508 None
OpBranchConditional %506 %507 %508
%507 = OpLabel
%509 = OpLoad %mat4v2float %_5_m42
%511 = OpCompositeConstruct %v2float %float_6 %float_0
%512 = OpCompositeConstruct %v2float %float_0 %float_6
%513 = OpCompositeConstruct %v2float %float_0 %float_0
%514 = OpCompositeConstruct %v2float %float_0 %float_0
%510 = OpCompositeConstruct %mat4v2float %511 %512 %513 %514
%515 = OpCompositeExtract %v2float %509 0
%516 = OpCompositeExtract %v2float %510 0
%517 = OpFOrdEqual %v2bool %515 %516
%518 = OpAll %bool %517
%519 = OpCompositeExtract %v2float %509 1
%520 = OpCompositeExtract %v2float %510 1
%521 = OpFOrdEqual %v2bool %519 %520
%522 = OpAll %bool %521
%523 = OpLogicalAnd %bool %518 %522
%524 = OpCompositeExtract %v2float %509 2
%525 = OpCompositeExtract %v2float %510 2
%526 = OpFOrdEqual %v2bool %524 %525
%527 = OpAll %bool %526
%528 = OpLogicalAnd %bool %523 %527
%529 = OpCompositeExtract %v2float %509 3
%530 = OpCompositeExtract %v2float %510 3
%531 = OpFOrdEqual %v2bool %529 %530
%532 = OpAll %bool %531
%533 = OpLogicalAnd %bool %528 %532
OpBranch %508
%508 = OpLabel
%534 = OpPhi %bool %false %479 %533 %507
OpStore %_0_ok %534
%537 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%538 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%539 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%540 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%536 = OpCompositeConstruct %mat4v3float %537 %538 %539 %540
OpStore %_6_m43 %536
%541 = OpLoad %bool %_0_ok
OpSelectionMerge %543 None
OpBranchConditional %541 %542 %543
%542 = OpLabel
%544 = OpLoad %mat4v3float %_6_m43
%546 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%547 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%548 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%549 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%545 = OpCompositeConstruct %mat4v3float %546 %547 %548 %549
%550 = OpCompositeExtract %v3float %544 0
%551 = OpCompositeExtract %v3float %545 0
%552 = OpFOrdEqual %v3bool %550 %551
%553 = OpAll %bool %552
%554 = OpCompositeExtract %v3float %544 1
%555 = OpCompositeExtract %v3float %545 1
%556 = OpFOrdEqual %v3bool %554 %555
%557 = OpAll %bool %556
%558 = OpLogicalAnd %bool %553 %557
%559 = OpCompositeExtract %v3float %544 2
%560 = OpCompositeExtract %v3float %545 2
%561 = OpFOrdEqual %v3bool %559 %560
%562 = OpAll %bool %561
%563 = OpLogicalAnd %bool %558 %562
%564 = OpCompositeExtract %v3float %544 3
%565 = OpCompositeExtract %v3float %545 3
%566 = OpFOrdEqual %v3bool %564 %565
%567 = OpAll %bool %566
%568 = OpLogicalAnd %bool %563 %567
OpBranch %543
%543 = OpLabel
%569 = OpPhi %bool %false %508 %568 %542
OpStore %_0_ok %569
%571 = OpLoad %mat3v2float %_3_m32
%572 = OpLoad %mat2v3float %_1_m23
%573 = OpMatrixTimesMatrix %mat2v2float %571 %572
OpStore %_7_m22 %573
%574 = OpLoad %bool %_0_ok
OpSelectionMerge %576 None
OpBranchConditional %574 %575 %576
%575 = OpLabel
%577 = OpLoad %mat2v2float %_7_m22
%579 = OpCompositeConstruct %v2float %float_8 %float_0
%580 = OpCompositeConstruct %v2float %float_0 %float_8
%578 = OpCompositeConstruct %mat2v2float %579 %580
%581 = OpCompositeExtract %v2float %577 0
%582 = OpCompositeExtract %v2float %578 0
%583 = OpFOrdEqual %v2bool %581 %582
%584 = OpAll %bool %583
%585 = OpCompositeExtract %v2float %577 1
%586 = OpCompositeExtract %v2float %578 1
%587 = OpFOrdEqual %v2bool %585 %586
%588 = OpAll %bool %587
%589 = OpLogicalAnd %bool %584 %588
OpBranch %576
%576 = OpLabel
%590 = OpPhi %bool %false %543 %589 %575
OpStore %_0_ok %590
%592 = OpLoad %mat4v3float %_6_m43
%593 = OpLoad %mat3v4float %_4_m34
%594 = OpMatrixTimesMatrix %mat3v3float %592 %593
OpStore %_8_m33 %594
%595 = OpLoad %bool %_0_ok
OpSelectionMerge %597 None
OpBranchConditional %595 %596 %597
%596 = OpLabel
%598 = OpLoad %mat3v3float %_8_m33
%600 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%601 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%602 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%599 = OpCompositeConstruct %mat3v3float %600 %601 %602
%603 = OpCompositeExtract %v3float %598 0
%604 = OpCompositeExtract %v3float %599 0
%605 = OpFOrdEqual %v3bool %603 %604
%606 = OpAll %bool %605
%607 = OpCompositeExtract %v3float %598 1
%608 = OpCompositeExtract %v3float %599 1
%609 = OpFOrdEqual %v3bool %607 %608
%610 = OpAll %bool %609
%611 = OpLogicalAnd %bool %606 %610
%612 = OpCompositeExtract %v3float %598 2
%613 = OpCompositeExtract %v3float %599 2
%614 = OpFOrdEqual %v3bool %612 %613
%615 = OpAll %bool %614
%616 = OpLogicalAnd %bool %611 %615
OpBranch %597
%597 = OpLabel
%617 = OpPhi %bool %false %576 %616 %596
OpStore %_0_ok %617
%619 = OpLoad %mat2v4float %_2_m24
%620 = OpLoad %mat4v2float %_5_m42
%621 = OpMatrixTimesMatrix %mat4v4float %619 %620
OpStore %_9_m44 %621
%622 = OpLoad %bool %_0_ok
OpSelectionMerge %624 None
OpBranchConditional %622 %623 %624
%623 = OpLabel
%625 = OpLoad %mat4v4float %_9_m44
%627 = OpCompositeConstruct %v4float %float_18 %float_0 %float_0 %float_0
%628 = OpCompositeConstruct %v4float %float_0 %float_18 %float_0 %float_0
%629 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%630 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%626 = OpCompositeConstruct %mat4v4float %627 %628 %629 %630
%631 = OpCompositeExtract %v4float %625 0
%632 = OpCompositeExtract %v4float %626 0
%633 = OpFOrdEqual %v4bool %631 %632
%634 = OpAll %bool %633
%635 = OpCompositeExtract %v4float %625 1
%636 = OpCompositeExtract %v4float %626 1
%637 = OpFOrdEqual %v4bool %635 %636
%638 = OpAll %bool %637
%639 = OpLogicalAnd %bool %634 %638
%640 = OpCompositeExtract %v4float %625 2
%641 = OpCompositeExtract %v4float %626 2
%642 = OpFOrdEqual %v4bool %640 %641
%643 = OpAll %bool %642
%644 = OpLogicalAnd %bool %639 %643
%645 = OpCompositeExtract %v4float %625 3
%646 = OpCompositeExtract %v4float %626 3
%647 = OpFOrdEqual %v4bool %645 %646
%648 = OpAll %bool %647
%649 = OpLogicalAnd %bool %644 %648
OpBranch %624
%624 = OpLabel
%650 = OpPhi %bool %false %597 %649 %623
OpStore %_0_ok %650
%651 = OpLoad %mat2v3float %_1_m23
%652 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%653 = OpCompositeConstruct %mat2v3float %652 %652
%654 = OpCompositeExtract %v3float %651 0
%655 = OpCompositeExtract %v3float %653 0
%656 = OpFAdd %v3float %654 %655
%657 = OpCompositeExtract %v3float %651 1
%658 = OpCompositeExtract %v3float %653 1
%659 = OpFAdd %v3float %657 %658
%660 = OpCompositeConstruct %mat2v3float %656 %659
OpStore %_1_m23 %660
%661 = OpLoad %bool %_0_ok
OpSelectionMerge %663 None
OpBranchConditional %661 %662 %663
%662 = OpLabel
%664 = OpLoad %mat2v3float %_1_m23
%666 = OpCompositeConstruct %v3float %float_3 %float_1 %float_1
%667 = OpCompositeConstruct %v3float %float_1 %float_3 %float_1
%665 = OpCompositeConstruct %mat2v3float %666 %667
%668 = OpCompositeExtract %v3float %664 0
%669 = OpCompositeExtract %v3float %665 0
%670 = OpFOrdEqual %v3bool %668 %669
%671 = OpAll %bool %670
%672 = OpCompositeExtract %v3float %664 1
%673 = OpCompositeExtract %v3float %665 1
%674 = OpFOrdEqual %v3bool %672 %673
%675 = OpAll %bool %674
%676 = OpLogicalAnd %bool %671 %675
OpBranch %663
%663 = OpLabel
%677 = OpPhi %bool %false %624 %676 %662
OpStore %_0_ok %677
%678 = OpLoad %mat3v2float %_3_m32
%679 = OpCompositeConstruct %v2float %float_2 %float_2
%680 = OpCompositeConstruct %mat3v2float %679 %679 %679
%681 = OpCompositeExtract %v2float %678 0
%682 = OpCompositeExtract %v2float %680 0
%683 = OpFSub %v2float %681 %682
%684 = OpCompositeExtract %v2float %678 1
%685 = OpCompositeExtract %v2float %680 1
%686 = OpFSub %v2float %684 %685
%687 = OpCompositeExtract %v2float %678 2
%688 = OpCompositeExtract %v2float %680 2
%689 = OpFSub %v2float %687 %688
%690 = OpCompositeConstruct %mat3v2float %683 %686 %689
OpStore %_3_m32 %690
%691 = OpLoad %bool %_0_ok
OpSelectionMerge %693 None
OpBranchConditional %691 %692 %693
%692 = OpLabel
%694 = OpLoad %mat3v2float %_3_m32
%696 = OpCompositeConstruct %v2float %float_2 %float_n2
%697 = OpCompositeConstruct %v2float %float_n2 %float_2
%698 = OpCompositeConstruct %v2float %float_n2 %float_n2
%695 = OpCompositeConstruct %mat3v2float %696 %697 %698
%699 = OpCompositeExtract %v2float %694 0
%700 = OpCompositeExtract %v2float %695 0
%701 = OpFOrdEqual %v2bool %699 %700
%702 = OpAll %bool %701
%703 = OpCompositeExtract %v2float %694 1
%704 = OpCompositeExtract %v2float %695 1
%705 = OpFOrdEqual %v2bool %703 %704
%706 = OpAll %bool %705
%707 = OpLogicalAnd %bool %702 %706
%708 = OpCompositeExtract %v2float %694 2
%709 = OpCompositeExtract %v2float %695 2
%710 = OpFOrdEqual %v2bool %708 %709
%711 = OpAll %bool %710
%712 = OpLogicalAnd %bool %707 %711
OpBranch %693
%693 = OpLabel
%713 = OpPhi %bool %false %663 %712 %692
OpStore %_0_ok %713
%714 = OpLoad %mat2v4float %_2_m24
%715 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%716 = OpCompositeConstruct %mat2v4float %715 %715
%717 = OpFDiv %mat2v4float %714 %716
OpStore %_2_m24 %717
%718 = OpLoad %bool %_0_ok
OpSelectionMerge %720 None
OpBranchConditional %718 %719 %720
%719 = OpLabel
%721 = OpLoad %mat2v4float %_2_m24
%723 = OpCompositeConstruct %v4float %float_0_75 %float_0 %float_0 %float_0
%724 = OpCompositeConstruct %v4float %float_0 %float_0_75 %float_0 %float_0
%722 = OpCompositeConstruct %mat2v4float %723 %724
%725 = OpCompositeExtract %v4float %721 0
%726 = OpCompositeExtract %v4float %722 0
%727 = OpFOrdEqual %v4bool %725 %726
%728 = OpAll %bool %727
%729 = OpCompositeExtract %v4float %721 1
%730 = OpCompositeExtract %v4float %722 1
%731 = OpFOrdEqual %v4bool %729 %730
%732 = OpAll %bool %731
%733 = OpLogicalAnd %bool %728 %732
OpBranch %720
%720 = OpLabel
%734 = OpPhi %bool %false %693 %733 %719
OpStore %_0_ok %734
%735 = OpLoad %bool %_0_ok
OpSelectionMerge %737 None
OpBranchConditional %735 %736 %737
%736 = OpLabel
%738 = OpFunctionCall %bool %test_half_b
OpBranch %737
%737 = OpLabel
%739 = OpPhi %bool %false %720 %738 %736
OpSelectionMerge %744 None
OpBranchConditional %739 %742 %743
%742 = OpLabel
%745 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%749 = OpLoad %v4float %745
OpStore %740 %749
OpBranch %744
%743 = OpLabel
%750 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%752 = OpLoad %v4float %750
OpStore %740 %752
OpBranch %744
%744 = OpLabel
%753 = OpLoad %v4float %740
OpReturnValue %753
OpFunctionEnd

1 error
