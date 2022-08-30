OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %takes_nothing_b "takes_nothing_b"
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
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %34 Binding 0
OpDecorate %34 DescriptorSet 0
OpDecorate %290 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%34 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%39 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%43 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%47 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%_ptr_Function_float = OpTypePointer Function %float
%52 = OpTypeFunction %bool %_ptr_Function_float
%55 = OpTypeFunction %bool %_ptr_Function_v2float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%60 = OpTypeFunction %bool %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%64 = OpTypeFunction %bool %_ptr_Function_v4float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%69 = OpTypeFunction %bool %_ptr_Function_mat2v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%74 = OpTypeFunction %bool %_ptr_Function_mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%79 = OpTypeFunction %bool %_ptr_Function_mat4v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%97 = OpTypeFunction %bool %_ptr_Function_bool
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%102 = OpTypeFunction %bool %_ptr_Function_v2bool
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%107 = OpTypeFunction %bool %_ptr_Function_v3bool
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%112 = OpTypeFunction %bool %_ptr_Function_v4bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%117 = OpTypeFunction %bool %_ptr_Function_int
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%122 = OpTypeFunction %bool %_ptr_Function_v2int
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%127 = OpTypeFunction %bool %_ptr_Function_v3int
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%132 = OpTypeFunction %bool %_ptr_Function_v4int
%135 = OpTypeFunction %v4float %_ptr_Function_v2float
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%153 = OpConstantComposite %v2float %float_2 %float_2
%float_3 = OpConstant %float 3
%160 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%float_4 = OpConstant %float 4
%167 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%173 = OpConstantComposite %v2float %float_2 %float_0
%174 = OpConstantComposite %v2float %float_0 %float_2
%175 = OpConstantComposite %mat2v2float %173 %174
%181 = OpConstantComposite %v3float %float_3 %float_0 %float_0
%182 = OpConstantComposite %v3float %float_0 %float_3 %float_0
%183 = OpConstantComposite %v3float %float_0 %float_0 %float_3
%184 = OpConstantComposite %mat3v3float %181 %182 %183
%190 = OpConstantComposite %v4float %float_4 %float_0 %float_0 %float_0
%191 = OpConstantComposite %v4float %float_0 %float_4 %float_0 %float_0
%192 = OpConstantComposite %v4float %float_0 %float_0 %float_4 %float_0
%193 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_4
%194 = OpConstantComposite %mat4v4float %190 %191 %192 %193
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
%_entrypoint_v = OpFunction %void None %39
%40 = OpLabel
%44 = OpVariable %_ptr_Function_v2float Function
OpStore %44 %43
%46 = OpFunctionCall %v4float %main %44
OpStore %sk_FragColor %46
OpReturn
OpFunctionEnd
%takes_nothing_b = OpFunction %bool None %47
%48 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_void_b = OpFunction %bool None %47
%50 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float_bf = OpFunction %bool None %52
%53 = OpFunctionParameter %_ptr_Function_float
%54 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float2_bf2 = OpFunction %bool None %55
%56 = OpFunctionParameter %_ptr_Function_v2float
%57 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float3_bf3 = OpFunction %bool None %60
%61 = OpFunctionParameter %_ptr_Function_v3float
%62 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float4_bf4 = OpFunction %bool None %64
%65 = OpFunctionParameter %_ptr_Function_v4float
%66 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float2x2_bf22 = OpFunction %bool None %69
%70 = OpFunctionParameter %_ptr_Function_mat2v2float
%71 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float3x3_bf33 = OpFunction %bool None %74
%75 = OpFunctionParameter %_ptr_Function_mat3v3float
%76 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float4x4_bf44 = OpFunction %bool None %79
%80 = OpFunctionParameter %_ptr_Function_mat4v4float
%81 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half_bh = OpFunction %bool None %52
%82 = OpFunctionParameter %_ptr_Function_float
%83 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half2_bh2 = OpFunction %bool None %55
%84 = OpFunctionParameter %_ptr_Function_v2float
%85 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half3_bh3 = OpFunction %bool None %60
%86 = OpFunctionParameter %_ptr_Function_v3float
%87 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half4_bh4 = OpFunction %bool None %64
%88 = OpFunctionParameter %_ptr_Function_v4float
%89 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half2x2_bh22 = OpFunction %bool None %69
%90 = OpFunctionParameter %_ptr_Function_mat2v2float
%91 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half3x3_bh33 = OpFunction %bool None %74
%92 = OpFunctionParameter %_ptr_Function_mat3v3float
%93 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half4x4_bh44 = OpFunction %bool None %79
%94 = OpFunctionParameter %_ptr_Function_mat4v4float
%95 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool_bb = OpFunction %bool None %97
%98 = OpFunctionParameter %_ptr_Function_bool
%99 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool2_bb2 = OpFunction %bool None %102
%103 = OpFunctionParameter %_ptr_Function_v2bool
%104 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool3_bb3 = OpFunction %bool None %107
%108 = OpFunctionParameter %_ptr_Function_v3bool
%109 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool4_bb4 = OpFunction %bool None %112
%113 = OpFunctionParameter %_ptr_Function_v4bool
%114 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int_bi = OpFunction %bool None %117
%118 = OpFunctionParameter %_ptr_Function_int
%119 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int2_bi2 = OpFunction %bool None %122
%123 = OpFunctionParameter %_ptr_Function_v2int
%124 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int3_bi3 = OpFunction %bool None %127
%128 = OpFunctionParameter %_ptr_Function_v3int
%129 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int4_bi4 = OpFunction %bool None %132
%133 = OpFunctionParameter %_ptr_Function_v4int
%134 = OpLabel
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %135
%136 = OpFunctionParameter %_ptr_Function_v2float
%137 = OpLabel
%147 = OpVariable %_ptr_Function_float Function
%154 = OpVariable %_ptr_Function_v2float Function
%161 = OpVariable %_ptr_Function_v3float Function
%168 = OpVariable %_ptr_Function_v4float Function
%176 = OpVariable %_ptr_Function_mat2v2float Function
%185 = OpVariable %_ptr_Function_mat3v3float Function
%195 = OpVariable %_ptr_Function_mat4v4float Function
%200 = OpVariable %_ptr_Function_float Function
%205 = OpVariable %_ptr_Function_v2float Function
%210 = OpVariable %_ptr_Function_v3float Function
%215 = OpVariable %_ptr_Function_v4float Function
%220 = OpVariable %_ptr_Function_mat2v2float Function
%225 = OpVariable %_ptr_Function_mat3v3float Function
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
%139 = OpFunctionCall %bool %takes_nothing_b
OpSelectionMerge %141 None
OpBranchConditional %139 %140 %141
%140 = OpLabel
%142 = OpFunctionCall %bool %takes_void_b
OpBranch %141
%141 = OpLabel
%143 = OpPhi %bool %false %137 %142 %140
OpSelectionMerge %145 None
OpBranchConditional %143 %144 %145
%144 = OpLabel
OpStore %147 %float_1
%148 = OpFunctionCall %bool %takes_float_bf %147
OpBranch %145
%145 = OpLabel
%149 = OpPhi %bool %false %141 %148 %144
OpSelectionMerge %151 None
OpBranchConditional %149 %150 %151
%150 = OpLabel
OpStore %154 %153
%155 = OpFunctionCall %bool %takes_float2_bf2 %154
OpBranch %151
%151 = OpLabel
%156 = OpPhi %bool %false %145 %155 %150
OpSelectionMerge %158 None
OpBranchConditional %156 %157 %158
%157 = OpLabel
OpStore %161 %160
%162 = OpFunctionCall %bool %takes_float3_bf3 %161
OpBranch %158
%158 = OpLabel
%163 = OpPhi %bool %false %151 %162 %157
OpSelectionMerge %165 None
OpBranchConditional %163 %164 %165
%164 = OpLabel
OpStore %168 %167
%169 = OpFunctionCall %bool %takes_float4_bf4 %168
OpBranch %165
%165 = OpLabel
%170 = OpPhi %bool %false %158 %169 %164
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
OpStore %176 %175
%177 = OpFunctionCall %bool %takes_float2x2_bf22 %176
OpBranch %172
%172 = OpLabel
%178 = OpPhi %bool %false %165 %177 %171
OpSelectionMerge %180 None
OpBranchConditional %178 %179 %180
%179 = OpLabel
OpStore %185 %184
%186 = OpFunctionCall %bool %takes_float3x3_bf33 %185
OpBranch %180
%180 = OpLabel
%187 = OpPhi %bool %false %172 %186 %179
OpSelectionMerge %189 None
OpBranchConditional %187 %188 %189
%188 = OpLabel
OpStore %195 %194
%196 = OpFunctionCall %bool %takes_float4x4_bf44 %195
OpBranch %189
%189 = OpLabel
%197 = OpPhi %bool %false %180 %196 %188
OpSelectionMerge %199 None
OpBranchConditional %197 %198 %199
%198 = OpLabel
OpStore %200 %float_1
%201 = OpFunctionCall %bool %takes_half_bh %200
OpBranch %199
%199 = OpLabel
%202 = OpPhi %bool %false %189 %201 %198
OpSelectionMerge %204 None
OpBranchConditional %202 %203 %204
%203 = OpLabel
OpStore %205 %153
%206 = OpFunctionCall %bool %takes_half2_bh2 %205
OpBranch %204
%204 = OpLabel
%207 = OpPhi %bool %false %199 %206 %203
OpSelectionMerge %209 None
OpBranchConditional %207 %208 %209
%208 = OpLabel
OpStore %210 %160
%211 = OpFunctionCall %bool %takes_half3_bh3 %210
OpBranch %209
%209 = OpLabel
%212 = OpPhi %bool %false %204 %211 %208
OpSelectionMerge %214 None
OpBranchConditional %212 %213 %214
%213 = OpLabel
OpStore %215 %167
%216 = OpFunctionCall %bool %takes_half4_bh4 %215
OpBranch %214
%214 = OpLabel
%217 = OpPhi %bool %false %209 %216 %213
OpSelectionMerge %219 None
OpBranchConditional %217 %218 %219
%218 = OpLabel
OpStore %220 %175
%221 = OpFunctionCall %bool %takes_half2x2_bh22 %220
OpBranch %219
%219 = OpLabel
%222 = OpPhi %bool %false %214 %221 %218
OpSelectionMerge %224 None
OpBranchConditional %222 %223 %224
%223 = OpLabel
OpStore %225 %184
%226 = OpFunctionCall %bool %takes_half3x3_bh33 %225
OpBranch %224
%224 = OpLabel
%227 = OpPhi %bool %false %219 %226 %223
OpSelectionMerge %229 None
OpBranchConditional %227 %228 %229
%228 = OpLabel
OpStore %230 %194
%231 = OpFunctionCall %bool %takes_half4x4_bh44 %230
OpBranch %229
%229 = OpLabel
%232 = OpPhi %bool %false %224 %231 %228
OpSelectionMerge %234 None
OpBranchConditional %232 %233 %234
%233 = OpLabel
OpStore %235 %true
%236 = OpFunctionCall %bool %takes_bool_bb %235
OpBranch %234
%234 = OpLabel
%237 = OpPhi %bool %false %229 %236 %233
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
%287 = OpAccessChain %_ptr_Uniform_v4float %34 %int_0
%290 = OpLoad %v4float %287
OpStore %283 %290
OpBranch %286
%285 = OpLabel
%291 = OpAccessChain %_ptr_Uniform_v4float %34 %int_1
%292 = OpLoad %v4float %291
OpStore %283 %292
OpBranch %286
%286 = OpLabel
%293 = OpLoad %v4float %283
OpReturnValue %293
OpFunctionEnd
