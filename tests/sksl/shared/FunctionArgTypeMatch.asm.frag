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
OpName %takes_void_b "takes_void_b"
OpName %takes_float_bf "takes_float_bf"
OpName %takes_float2_bf2 "takes_float2_bf2"
OpName %takes_float3_bf3 "takes_float3_bf3"
OpName %takes_float4_bf4 "takes_float4_bf4"
OpName %takes_float2x2_bf22 "takes_float2x2_bf22"
OpName %takes_float3x3_bf33 "takes_float3x3_bf33"
OpName %takes_float4x4_bf44 "takes_float4x4_bf44"
OpName %takes_half_bh "takes_half_bh"
OpName %takes_half2_bh2 "takes_half2_bh2"
OpName %takes_half3_bh3 "takes_half3_bh3"
OpName %takes_half4_bh4 "takes_half4_bh4"
OpName %takes_half2x2_bh22 "takes_half2x2_bh22"
OpName %takes_half3x3_bh33 "takes_half3x3_bh33"
OpName %takes_half4x4_bh44 "takes_half4x4_bh44"
OpName %takes_bool_bb "takes_bool_bb"
OpName %takes_bool2_bb2 "takes_bool2_bb2"
OpName %takes_bool3_bb3 "takes_bool3_bb3"
OpName %takes_bool4_bb4 "takes_bool4_bb4"
OpName %takes_int_bi "takes_int_bi"
OpName %takes_int2_bi2 "takes_int2_bi2"
OpName %takes_int3_bi3 "takes_int3_bi3"
OpName %takes_int4_bi4 "takes_int4_bi4"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %33 Binding 0
OpDecorate %33 DescriptorSet 0
OpDecorate %217 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%33 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%38 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%42 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%46 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%_ptr_Function_float = OpTypePointer Function %float
%49 = OpTypeFunction %bool %_ptr_Function_float
%53 = OpTypeFunction %bool %_ptr_Function_v2float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%57 = OpTypeFunction %bool %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%61 = OpTypeFunction %bool %_ptr_Function_v4float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%66 = OpTypeFunction %bool %_ptr_Function_mat2v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%71 = OpTypeFunction %bool %_ptr_Function_mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%76 = OpTypeFunction %bool %_ptr_Function_mat4v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%94 = OpTypeFunction %bool %_ptr_Function_bool
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%99 = OpTypeFunction %bool %_ptr_Function_v2bool
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%104 = OpTypeFunction %bool %_ptr_Function_v3bool
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%109 = OpTypeFunction %bool %_ptr_Function_v4bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%114 = OpTypeFunction %bool %_ptr_Function_int
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%119 = OpTypeFunction %bool %_ptr_Function_v2int
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%124 = OpTypeFunction %bool %_ptr_Function_v3int
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%129 = OpTypeFunction %bool %_ptr_Function_v4int
%133 = OpTypeFunction %v4float %_ptr_Function_v2float
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%150 = OpConstantComposite %v2float %float_2 %float_2
%float_3 = OpConstant %float 3
%157 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%float_4 = OpConstant %float 4
%164 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%171 = OpConstantComposite %v2float %float_2 %float_0
%172 = OpConstantComposite %v2float %float_0 %float_2
%179 = OpConstantComposite %v3float %float_3 %float_0 %float_0
%180 = OpConstantComposite %v3float %float_0 %float_3 %float_0
%181 = OpConstantComposite %v3float %float_0 %float_0 %float_3
%188 = OpConstantComposite %v4float %float_4 %float_0 %float_0 %float_0
%189 = OpConstantComposite %v4float %float_0 %float_4 %float_0 %float_0
%190 = OpConstantComposite %v4float %float_0 %float_0 %float_4 %float_0
%191 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_4
%240 = OpConstantComposite %v2bool %true %true
%246 = OpConstantComposite %v3bool %true %true %true
%252 = OpConstantComposite %v4bool %true %true %true %true
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%265 = OpConstantComposite %v2int %int_2 %int_2
%int_3 = OpConstant %int 3
%272 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%int_4 = OpConstant %int 4
%279 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %38
%39 = OpLabel
%43 = OpVariable %_ptr_Function_v2float Function
OpStore %43 %42
%45 = OpFunctionCall %v4float %main %43
OpStore %sk_FragColor %45
OpReturn
OpFunctionEnd
%takes_void_b = OpFunction %bool None %46
%47 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float_bf = OpFunction %bool None %49
%51 = OpFunctionParameter %_ptr_Function_float
%52 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float2_bf2 = OpFunction %bool None %53
%54 = OpFunctionParameter %_ptr_Function_v2float
%55 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float3_bf3 = OpFunction %bool None %57
%59 = OpFunctionParameter %_ptr_Function_v3float
%60 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float4_bf4 = OpFunction %bool None %61
%63 = OpFunctionParameter %_ptr_Function_v4float
%64 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float2x2_bf22 = OpFunction %bool None %66
%68 = OpFunctionParameter %_ptr_Function_mat2v2float
%69 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float3x3_bf33 = OpFunction %bool None %71
%73 = OpFunctionParameter %_ptr_Function_mat3v3float
%74 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float4x4_bf44 = OpFunction %bool None %76
%78 = OpFunctionParameter %_ptr_Function_mat4v4float
%79 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half_bh = OpFunction %bool None %49
%80 = OpFunctionParameter %_ptr_Function_float
%81 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half2_bh2 = OpFunction %bool None %53
%82 = OpFunctionParameter %_ptr_Function_v2float
%83 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half3_bh3 = OpFunction %bool None %57
%84 = OpFunctionParameter %_ptr_Function_v3float
%85 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half4_bh4 = OpFunction %bool None %61
%86 = OpFunctionParameter %_ptr_Function_v4float
%87 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half2x2_bh22 = OpFunction %bool None %66
%88 = OpFunctionParameter %_ptr_Function_mat2v2float
%89 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half3x3_bh33 = OpFunction %bool None %71
%90 = OpFunctionParameter %_ptr_Function_mat3v3float
%91 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half4x4_bh44 = OpFunction %bool None %76
%92 = OpFunctionParameter %_ptr_Function_mat4v4float
%93 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool_bb = OpFunction %bool None %94
%96 = OpFunctionParameter %_ptr_Function_bool
%97 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool2_bb2 = OpFunction %bool None %99
%101 = OpFunctionParameter %_ptr_Function_v2bool
%102 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool3_bb3 = OpFunction %bool None %104
%106 = OpFunctionParameter %_ptr_Function_v3bool
%107 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool4_bb4 = OpFunction %bool None %109
%111 = OpFunctionParameter %_ptr_Function_v4bool
%112 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int_bi = OpFunction %bool None %114
%116 = OpFunctionParameter %_ptr_Function_int
%117 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int2_bi2 = OpFunction %bool None %119
%121 = OpFunctionParameter %_ptr_Function_v2int
%122 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int3_bi3 = OpFunction %bool None %124
%126 = OpFunctionParameter %_ptr_Function_v3int
%127 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int4_bi4 = OpFunction %bool None %129
%131 = OpFunctionParameter %_ptr_Function_v4int
%132 = OpLabel
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %133
%134 = OpFunctionParameter %_ptr_Function_v2float
%135 = OpLabel
%144 = OpVariable %_ptr_Function_float Function
%151 = OpVariable %_ptr_Function_v2float Function
%158 = OpVariable %_ptr_Function_v3float Function
%165 = OpVariable %_ptr_Function_v4float Function
%173 = OpVariable %_ptr_Function_mat2v2float Function
%182 = OpVariable %_ptr_Function_mat3v3float Function
%192 = OpVariable %_ptr_Function_mat4v4float Function
%197 = OpVariable %_ptr_Function_float Function
%202 = OpVariable %_ptr_Function_v2float Function
%207 = OpVariable %_ptr_Function_v3float Function
%212 = OpVariable %_ptr_Function_v4float Function
%218 = OpVariable %_ptr_Function_mat2v2float Function
%224 = OpVariable %_ptr_Function_mat3v3float Function
%230 = OpVariable %_ptr_Function_mat4v4float Function
%235 = OpVariable %_ptr_Function_bool Function
%241 = OpVariable %_ptr_Function_v2bool Function
%247 = OpVariable %_ptr_Function_v3bool Function
%253 = OpVariable %_ptr_Function_v4bool Function
%259 = OpVariable %_ptr_Function_int Function
%266 = OpVariable %_ptr_Function_v2int Function
%273 = OpVariable %_ptr_Function_v3int Function
%280 = OpVariable %_ptr_Function_v4int Function
%283 = OpVariable %_ptr_Function_v4float Function
OpSelectionMerge %138 None
OpBranchConditional %true %137 %138
%137 = OpLabel
%139 = OpFunctionCall %bool %takes_void_b
OpBranch %138
%138 = OpLabel
%140 = OpPhi %bool %false %135 %139 %137
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
OpStore %144 %float_1
%145 = OpFunctionCall %bool %takes_float_bf %144
OpBranch %142
%142 = OpLabel
%146 = OpPhi %bool %false %138 %145 %141
OpSelectionMerge %148 None
OpBranchConditional %146 %147 %148
%147 = OpLabel
OpStore %151 %150
%152 = OpFunctionCall %bool %takes_float2_bf2 %151
OpBranch %148
%148 = OpLabel
%153 = OpPhi %bool %false %142 %152 %147
OpSelectionMerge %155 None
OpBranchConditional %153 %154 %155
%154 = OpLabel
OpStore %158 %157
%159 = OpFunctionCall %bool %takes_float3_bf3 %158
OpBranch %155
%155 = OpLabel
%160 = OpPhi %bool %false %148 %159 %154
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
OpStore %165 %164
%166 = OpFunctionCall %bool %takes_float4_bf4 %165
OpBranch %162
%162 = OpLabel
%167 = OpPhi %bool %false %155 %166 %161
OpSelectionMerge %169 None
OpBranchConditional %167 %168 %169
%168 = OpLabel
%170 = OpCompositeConstruct %mat2v2float %171 %172
OpStore %173 %170
%174 = OpFunctionCall %bool %takes_float2x2_bf22 %173
OpBranch %169
%169 = OpLabel
%175 = OpPhi %bool %false %162 %174 %168
OpSelectionMerge %177 None
OpBranchConditional %175 %176 %177
%176 = OpLabel
%178 = OpCompositeConstruct %mat3v3float %179 %180 %181
OpStore %182 %178
%183 = OpFunctionCall %bool %takes_float3x3_bf33 %182
OpBranch %177
%177 = OpLabel
%184 = OpPhi %bool %false %169 %183 %176
OpSelectionMerge %186 None
OpBranchConditional %184 %185 %186
%185 = OpLabel
%187 = OpCompositeConstruct %mat4v4float %188 %189 %190 %191
OpStore %192 %187
%193 = OpFunctionCall %bool %takes_float4x4_bf44 %192
OpBranch %186
%186 = OpLabel
%194 = OpPhi %bool %false %177 %193 %185
OpSelectionMerge %196 None
OpBranchConditional %194 %195 %196
%195 = OpLabel
OpStore %197 %float_1
%198 = OpFunctionCall %bool %takes_half_bh %197
OpBranch %196
%196 = OpLabel
%199 = OpPhi %bool %false %186 %198 %195
OpSelectionMerge %201 None
OpBranchConditional %199 %200 %201
%200 = OpLabel
OpStore %202 %150
%203 = OpFunctionCall %bool %takes_half2_bh2 %202
OpBranch %201
%201 = OpLabel
%204 = OpPhi %bool %false %196 %203 %200
OpSelectionMerge %206 None
OpBranchConditional %204 %205 %206
%205 = OpLabel
OpStore %207 %157
%208 = OpFunctionCall %bool %takes_half3_bh3 %207
OpBranch %206
%206 = OpLabel
%209 = OpPhi %bool %false %201 %208 %205
OpSelectionMerge %211 None
OpBranchConditional %209 %210 %211
%210 = OpLabel
OpStore %212 %164
%213 = OpFunctionCall %bool %takes_half4_bh4 %212
OpBranch %211
%211 = OpLabel
%214 = OpPhi %bool %false %206 %213 %210
OpSelectionMerge %216 None
OpBranchConditional %214 %215 %216
%215 = OpLabel
%217 = OpCompositeConstruct %mat2v2float %171 %172
OpStore %218 %217
%219 = OpFunctionCall %bool %takes_half2x2_bh22 %218
OpBranch %216
%216 = OpLabel
%220 = OpPhi %bool %false %211 %219 %215
OpSelectionMerge %222 None
OpBranchConditional %220 %221 %222
%221 = OpLabel
%223 = OpCompositeConstruct %mat3v3float %179 %180 %181
OpStore %224 %223
%225 = OpFunctionCall %bool %takes_half3x3_bh33 %224
OpBranch %222
%222 = OpLabel
%226 = OpPhi %bool %false %216 %225 %221
OpSelectionMerge %228 None
OpBranchConditional %226 %227 %228
%227 = OpLabel
%229 = OpCompositeConstruct %mat4v4float %188 %189 %190 %191
OpStore %230 %229
%231 = OpFunctionCall %bool %takes_half4x4_bh44 %230
OpBranch %228
%228 = OpLabel
%232 = OpPhi %bool %false %222 %231 %227
OpSelectionMerge %234 None
OpBranchConditional %232 %233 %234
%233 = OpLabel
OpStore %235 %true
%236 = OpFunctionCall %bool %takes_bool_bb %235
OpBranch %234
%234 = OpLabel
%237 = OpPhi %bool %false %228 %236 %233
OpSelectionMerge %239 None
OpBranchConditional %237 %238 %239
%238 = OpLabel
OpStore %241 %240
%242 = OpFunctionCall %bool %takes_bool2_bb2 %241
OpBranch %239
%239 = OpLabel
%243 = OpPhi %bool %false %234 %242 %238
OpSelectionMerge %245 None
OpBranchConditional %243 %244 %245
%244 = OpLabel
OpStore %247 %246
%248 = OpFunctionCall %bool %takes_bool3_bb3 %247
OpBranch %245
%245 = OpLabel
%249 = OpPhi %bool %false %239 %248 %244
OpSelectionMerge %251 None
OpBranchConditional %249 %250 %251
%250 = OpLabel
OpStore %253 %252
%254 = OpFunctionCall %bool %takes_bool4_bb4 %253
OpBranch %251
%251 = OpLabel
%255 = OpPhi %bool %false %245 %254 %250
OpSelectionMerge %257 None
OpBranchConditional %255 %256 %257
%256 = OpLabel
OpStore %259 %int_1
%260 = OpFunctionCall %bool %takes_int_bi %259
OpBranch %257
%257 = OpLabel
%261 = OpPhi %bool %false %251 %260 %256
OpSelectionMerge %263 None
OpBranchConditional %261 %262 %263
%262 = OpLabel
OpStore %266 %265
%267 = OpFunctionCall %bool %takes_int2_bi2 %266
OpBranch %263
%263 = OpLabel
%268 = OpPhi %bool %false %257 %267 %262
OpSelectionMerge %270 None
OpBranchConditional %268 %269 %270
%269 = OpLabel
OpStore %273 %272
%274 = OpFunctionCall %bool %takes_int3_bi3 %273
OpBranch %270
%270 = OpLabel
%275 = OpPhi %bool %false %263 %274 %269
OpSelectionMerge %277 None
OpBranchConditional %275 %276 %277
%276 = OpLabel
OpStore %280 %279
%281 = OpFunctionCall %bool %takes_int4_bi4 %280
OpBranch %277
%277 = OpLabel
%282 = OpPhi %bool %false %270 %281 %276
OpSelectionMerge %286 None
OpBranchConditional %282 %284 %285
%284 = OpLabel
%287 = OpAccessChain %_ptr_Uniform_v4float %33 %int_0
%290 = OpLoad %v4float %287
OpStore %283 %290
OpBranch %286
%285 = OpLabel
%291 = OpAccessChain %_ptr_Uniform_v4float %33 %int_1
%292 = OpLoad %v4float %291
OpStore %283 %292
OpBranch %286
%286 = OpLabel
%293 = OpLoad %v4float %283
OpReturnValue %293
OpFunctionEnd
