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
OpDecorate %31 Binding 0
OpDecorate %31 DescriptorSet 0
OpDecorate %184 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%31 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%36 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%40 = OpTypeFunction %bool %_ptr_Function_v2float
%true = OpConstantTrue %bool
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%46 = OpTypeFunction %bool %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%50 = OpTypeFunction %bool %_ptr_Function_v4float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%55 = OpTypeFunction %bool %_ptr_Function_mat2v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%60 = OpTypeFunction %bool %_ptr_Function_mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%65 = OpTypeFunction %bool %_ptr_Function_mat4v4float
%_ptr_Function_float = OpTypePointer Function %float
%69 = OpTypeFunction %bool %_ptr_Function_float
%_ptr_Function_bool = OpTypePointer Function %bool
%85 = OpTypeFunction %bool %_ptr_Function_bool
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%90 = OpTypeFunction %bool %_ptr_Function_v2bool
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%95 = OpTypeFunction %bool %_ptr_Function_v3bool
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%100 = OpTypeFunction %bool %_ptr_Function_v4bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%105 = OpTypeFunction %bool %_ptr_Function_int
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%110 = OpTypeFunction %bool %_ptr_Function_v2int
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%115 = OpTypeFunction %bool %_ptr_Function_v3int
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%120 = OpTypeFunction %bool %_ptr_Function_v4int
%124 = OpTypeFunction %v4float
%false = OpConstantFalse %bool
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %36
%37 = OpLabel
%38 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %38
OpReturn
OpFunctionEnd
%takes_float2_bf2 = OpFunction %bool None %40
%42 = OpFunctionParameter %_ptr_Function_v2float
%43 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float3_bf3 = OpFunction %bool None %46
%48 = OpFunctionParameter %_ptr_Function_v3float
%49 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float4_bf4 = OpFunction %bool None %50
%52 = OpFunctionParameter %_ptr_Function_v4float
%53 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float2x2_bf22 = OpFunction %bool None %55
%57 = OpFunctionParameter %_ptr_Function_mat2v2float
%58 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float3x3_bf33 = OpFunction %bool None %60
%62 = OpFunctionParameter %_ptr_Function_mat3v3float
%63 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float4x4_bf44 = OpFunction %bool None %65
%67 = OpFunctionParameter %_ptr_Function_mat4v4float
%68 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half_bh = OpFunction %bool None %69
%71 = OpFunctionParameter %_ptr_Function_float
%72 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half2_bh2 = OpFunction %bool None %40
%73 = OpFunctionParameter %_ptr_Function_v2float
%74 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half3_bh3 = OpFunction %bool None %46
%75 = OpFunctionParameter %_ptr_Function_v3float
%76 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half4_bh4 = OpFunction %bool None %50
%77 = OpFunctionParameter %_ptr_Function_v4float
%78 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half2x2_bh22 = OpFunction %bool None %55
%79 = OpFunctionParameter %_ptr_Function_mat2v2float
%80 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half3x3_bh33 = OpFunction %bool None %60
%81 = OpFunctionParameter %_ptr_Function_mat3v3float
%82 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half4x4_bh44 = OpFunction %bool None %65
%83 = OpFunctionParameter %_ptr_Function_mat4v4float
%84 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool_bb = OpFunction %bool None %85
%87 = OpFunctionParameter %_ptr_Function_bool
%88 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool2_bb2 = OpFunction %bool None %90
%92 = OpFunctionParameter %_ptr_Function_v2bool
%93 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool3_bb3 = OpFunction %bool None %95
%97 = OpFunctionParameter %_ptr_Function_v3bool
%98 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool4_bb4 = OpFunction %bool None %100
%102 = OpFunctionParameter %_ptr_Function_v4bool
%103 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int_bi = OpFunction %bool None %105
%107 = OpFunctionParameter %_ptr_Function_int
%108 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int2_bi2 = OpFunction %bool None %110
%112 = OpFunctionParameter %_ptr_Function_v2int
%113 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int3_bi3 = OpFunction %bool None %115
%117 = OpFunctionParameter %_ptr_Function_v3int
%118 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int4_bi4 = OpFunction %bool None %120
%122 = OpFunctionParameter %_ptr_Function_v4int
%123 = OpLabel
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %124
%125 = OpLabel
%131 = OpVariable %_ptr_Function_v2float Function
%138 = OpVariable %_ptr_Function_v3float Function
%145 = OpVariable %_ptr_Function_v4float Function
%154 = OpVariable %_ptr_Function_mat2v2float Function
%163 = OpVariable %_ptr_Function_mat3v3float Function
%173 = OpVariable %_ptr_Function_mat4v4float Function
%179 = OpVariable %_ptr_Function_float Function
%185 = OpVariable %_ptr_Function_v2float Function
%191 = OpVariable %_ptr_Function_v3float Function
%197 = OpVariable %_ptr_Function_v4float Function
%205 = OpVariable %_ptr_Function_mat2v2float Function
%214 = OpVariable %_ptr_Function_mat3v3float Function
%224 = OpVariable %_ptr_Function_mat4v4float Function
%229 = OpVariable %_ptr_Function_bool Function
%235 = OpVariable %_ptr_Function_v2bool Function
%241 = OpVariable %_ptr_Function_v3bool Function
%247 = OpVariable %_ptr_Function_v4bool Function
%253 = OpVariable %_ptr_Function_int Function
%260 = OpVariable %_ptr_Function_v2int Function
%267 = OpVariable %_ptr_Function_v3int Function
%274 = OpVariable %_ptr_Function_v4int Function
%277 = OpVariable %_ptr_Function_v4float Function
OpSelectionMerge %128 None
OpBranchConditional %true %127 %128
%127 = OpLabel
%130 = OpCompositeConstruct %v2float %float_2 %float_2
OpStore %131 %130
%132 = OpFunctionCall %bool %takes_float2_bf2 %131
OpBranch %128
%128 = OpLabel
%133 = OpPhi %bool %false %125 %132 %127
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%137 = OpCompositeConstruct %v3float %float_3 %float_3 %float_3
OpStore %138 %137
%139 = OpFunctionCall %bool %takes_float3_bf3 %138
OpBranch %135
%135 = OpLabel
%140 = OpPhi %bool %false %128 %139 %134
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%144 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
OpStore %145 %144
%146 = OpFunctionCall %bool %takes_float4_bf4 %145
OpBranch %142
%142 = OpLabel
%147 = OpPhi %bool %false %135 %146 %141
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%152 = OpCompositeConstruct %v2float %float_2 %float_0
%153 = OpCompositeConstruct %v2float %float_0 %float_2
%150 = OpCompositeConstruct %mat2v2float %152 %153
OpStore %154 %150
%155 = OpFunctionCall %bool %takes_float2x2_bf22 %154
OpBranch %149
%149 = OpLabel
%156 = OpPhi %bool %false %142 %155 %148
OpSelectionMerge %158 None
OpBranchConditional %156 %157 %158
%157 = OpLabel
%160 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%161 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%162 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%159 = OpCompositeConstruct %mat3v3float %160 %161 %162
OpStore %163 %159
%164 = OpFunctionCall %bool %takes_float3x3_bf33 %163
OpBranch %158
%158 = OpLabel
%165 = OpPhi %bool %false %149 %164 %157
OpSelectionMerge %167 None
OpBranchConditional %165 %166 %167
%166 = OpLabel
%169 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%170 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%171 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%172 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%168 = OpCompositeConstruct %mat4v4float %169 %170 %171 %172
OpStore %173 %168
%174 = OpFunctionCall %bool %takes_float4x4_bf44 %173
OpBranch %167
%167 = OpLabel
%175 = OpPhi %bool %false %158 %174 %166
OpSelectionMerge %177 None
OpBranchConditional %175 %176 %177
%176 = OpLabel
OpStore %179 %float_1
%180 = OpFunctionCall %bool %takes_half_bh %179
OpBranch %177
%177 = OpLabel
%181 = OpPhi %bool %false %167 %180 %176
OpSelectionMerge %183 None
OpBranchConditional %181 %182 %183
%182 = OpLabel
%184 = OpCompositeConstruct %v2float %float_2 %float_2
OpStore %185 %184
%186 = OpFunctionCall %bool %takes_half2_bh2 %185
OpBranch %183
%183 = OpLabel
%187 = OpPhi %bool %false %177 %186 %182
OpSelectionMerge %189 None
OpBranchConditional %187 %188 %189
%188 = OpLabel
%190 = OpCompositeConstruct %v3float %float_3 %float_3 %float_3
OpStore %191 %190
%192 = OpFunctionCall %bool %takes_half3_bh3 %191
OpBranch %189
%189 = OpLabel
%193 = OpPhi %bool %false %183 %192 %188
OpSelectionMerge %195 None
OpBranchConditional %193 %194 %195
%194 = OpLabel
%196 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
OpStore %197 %196
%198 = OpFunctionCall %bool %takes_half4_bh4 %197
OpBranch %195
%195 = OpLabel
%199 = OpPhi %bool %false %189 %198 %194
OpSelectionMerge %201 None
OpBranchConditional %199 %200 %201
%200 = OpLabel
%203 = OpCompositeConstruct %v2float %float_2 %float_0
%204 = OpCompositeConstruct %v2float %float_0 %float_2
%202 = OpCompositeConstruct %mat2v2float %203 %204
OpStore %205 %202
%206 = OpFunctionCall %bool %takes_half2x2_bh22 %205
OpBranch %201
%201 = OpLabel
%207 = OpPhi %bool %false %195 %206 %200
OpSelectionMerge %209 None
OpBranchConditional %207 %208 %209
%208 = OpLabel
%211 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%212 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%213 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%210 = OpCompositeConstruct %mat3v3float %211 %212 %213
OpStore %214 %210
%215 = OpFunctionCall %bool %takes_half3x3_bh33 %214
OpBranch %209
%209 = OpLabel
%216 = OpPhi %bool %false %201 %215 %208
OpSelectionMerge %218 None
OpBranchConditional %216 %217 %218
%217 = OpLabel
%220 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%221 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%222 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%223 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%219 = OpCompositeConstruct %mat4v4float %220 %221 %222 %223
OpStore %224 %219
%225 = OpFunctionCall %bool %takes_half4x4_bh44 %224
OpBranch %218
%218 = OpLabel
%226 = OpPhi %bool %false %209 %225 %217
OpSelectionMerge %228 None
OpBranchConditional %226 %227 %228
%227 = OpLabel
OpStore %229 %true
%230 = OpFunctionCall %bool %takes_bool_bb %229
OpBranch %228
%228 = OpLabel
%231 = OpPhi %bool %false %218 %230 %227
OpSelectionMerge %233 None
OpBranchConditional %231 %232 %233
%232 = OpLabel
%234 = OpCompositeConstruct %v2bool %true %true
OpStore %235 %234
%236 = OpFunctionCall %bool %takes_bool2_bb2 %235
OpBranch %233
%233 = OpLabel
%237 = OpPhi %bool %false %228 %236 %232
OpSelectionMerge %239 None
OpBranchConditional %237 %238 %239
%238 = OpLabel
%240 = OpCompositeConstruct %v3bool %true %true %true
OpStore %241 %240
%242 = OpFunctionCall %bool %takes_bool3_bb3 %241
OpBranch %239
%239 = OpLabel
%243 = OpPhi %bool %false %233 %242 %238
OpSelectionMerge %245 None
OpBranchConditional %243 %244 %245
%244 = OpLabel
%246 = OpCompositeConstruct %v4bool %true %true %true %true
OpStore %247 %246
%248 = OpFunctionCall %bool %takes_bool4_bb4 %247
OpBranch %245
%245 = OpLabel
%249 = OpPhi %bool %false %239 %248 %244
OpSelectionMerge %251 None
OpBranchConditional %249 %250 %251
%250 = OpLabel
OpStore %253 %int_1
%254 = OpFunctionCall %bool %takes_int_bi %253
OpBranch %251
%251 = OpLabel
%255 = OpPhi %bool %false %245 %254 %250
OpSelectionMerge %257 None
OpBranchConditional %255 %256 %257
%256 = OpLabel
%259 = OpCompositeConstruct %v2int %int_2 %int_2
OpStore %260 %259
%261 = OpFunctionCall %bool %takes_int2_bi2 %260
OpBranch %257
%257 = OpLabel
%262 = OpPhi %bool %false %251 %261 %256
OpSelectionMerge %264 None
OpBranchConditional %262 %263 %264
%263 = OpLabel
%266 = OpCompositeConstruct %v3int %int_3 %int_3 %int_3
OpStore %267 %266
%268 = OpFunctionCall %bool %takes_int3_bi3 %267
OpBranch %264
%264 = OpLabel
%269 = OpPhi %bool %false %257 %268 %263
OpSelectionMerge %271 None
OpBranchConditional %269 %270 %271
%270 = OpLabel
%273 = OpCompositeConstruct %v4int %int_4 %int_4 %int_4 %int_4
OpStore %274 %273
%275 = OpFunctionCall %bool %takes_int4_bi4 %274
OpBranch %271
%271 = OpLabel
%276 = OpPhi %bool %false %264 %275 %270
OpSelectionMerge %280 None
OpBranchConditional %276 %278 %279
%278 = OpLabel
%281 = OpAccessChain %_ptr_Uniform_v4float %31 %int_0
%284 = OpLoad %v4float %281
OpStore %277 %284
OpBranch %280
%279 = OpLabel
%285 = OpAccessChain %_ptr_Uniform_v4float %31 %int_1
%286 = OpLoad %v4float %285
OpStore %277 %286
OpBranch %280
%280 = OpLabel
%287 = OpLoad %v4float %277
OpReturnValue %287
OpFunctionEnd
