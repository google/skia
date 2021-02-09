OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %takes_float "takes_float"
OpName %takes_float2 "takes_float2"
OpName %takes_float3 "takes_float3"
OpName %takes_float4 "takes_float4"
OpName %takes_float2x2 "takes_float2x2"
OpName %takes_float3x3 "takes_float3x3"
OpName %takes_float4x4 "takes_float4x4"
OpName %takes_half "takes_half"
OpName %takes_half2 "takes_half2"
OpName %takes_half3 "takes_half3"
OpName %takes_half4 "takes_half4"
OpName %takes_half2x2 "takes_half2x2"
OpName %takes_half3x3 "takes_half3x3"
OpName %takes_half4x4 "takes_half4x4"
OpName %takes_bool "takes_bool"
OpName %takes_bool2 "takes_bool2"
OpName %takes_bool3 "takes_bool3"
OpName %takes_bool4 "takes_bool4"
OpName %takes_int "takes_int"
OpName %takes_int2 "takes_int2"
OpName %takes_int3 "takes_int3"
OpName %takes_int4 "takes_int4"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %32 Binding 0
OpDecorate %32 DescriptorSet 0
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%32 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%37 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%40 = OpTypeFunction %bool %_ptr_Function_float
%true = OpConstantTrue %bool
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%46 = OpTypeFunction %bool %_ptr_Function_v2float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%51 = OpTypeFunction %bool %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%55 = OpTypeFunction %bool %_ptr_Function_v4float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%60 = OpTypeFunction %bool %_ptr_Function_mat2v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%65 = OpTypeFunction %bool %_ptr_Function_mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%70 = OpTypeFunction %bool %_ptr_Function_mat4v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%88 = OpTypeFunction %bool %_ptr_Function_bool
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%93 = OpTypeFunction %bool %_ptr_Function_v2bool
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%98 = OpTypeFunction %bool %_ptr_Function_v3bool
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%103 = OpTypeFunction %bool %_ptr_Function_v4bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%108 = OpTypeFunction %bool %_ptr_Function_int
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%113 = OpTypeFunction %bool %_ptr_Function_v2int
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%118 = OpTypeFunction %bool %_ptr_Function_v3int
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%123 = OpTypeFunction %bool %_ptr_Function_v4int
%127 = OpTypeFunction %v4float
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%136 = OpConstantComposite %v2float %float_2 %float_2
%float_3 = OpConstant %float 3
%143 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%float_4 = OpConstant %float 4
%150 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%float_0 = OpConstant %float 0
%236 = OpConstantComposite %v2bool %true %true
%242 = OpConstantComposite %v3bool %true %true %true
%248 = OpConstantComposite %v4bool %true %true %true %true
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%261 = OpConstantComposite %v2int %int_2 %int_2
%int_3 = OpConstant %int 3
%268 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%int_4 = OpConstant %int 4
%275 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint = OpFunction %void None %37
%38 = OpLabel
%39 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %39
OpReturn
OpFunctionEnd
%takes_float = OpFunction %bool None %40
%42 = OpFunctionParameter %_ptr_Function_float
%43 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float2 = OpFunction %bool None %46
%48 = OpFunctionParameter %_ptr_Function_v2float
%49 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float3 = OpFunction %bool None %51
%53 = OpFunctionParameter %_ptr_Function_v3float
%54 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float4 = OpFunction %bool None %55
%57 = OpFunctionParameter %_ptr_Function_v4float
%58 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float2x2 = OpFunction %bool None %60
%62 = OpFunctionParameter %_ptr_Function_mat2v2float
%63 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float3x3 = OpFunction %bool None %65
%67 = OpFunctionParameter %_ptr_Function_mat3v3float
%68 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_float4x4 = OpFunction %bool None %70
%72 = OpFunctionParameter %_ptr_Function_mat4v4float
%73 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half = OpFunction %bool None %40
%74 = OpFunctionParameter %_ptr_Function_float
%75 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half2 = OpFunction %bool None %46
%76 = OpFunctionParameter %_ptr_Function_v2float
%77 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half3 = OpFunction %bool None %51
%78 = OpFunctionParameter %_ptr_Function_v3float
%79 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half4 = OpFunction %bool None %55
%80 = OpFunctionParameter %_ptr_Function_v4float
%81 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half2x2 = OpFunction %bool None %60
%82 = OpFunctionParameter %_ptr_Function_mat2v2float
%83 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half3x3 = OpFunction %bool None %65
%84 = OpFunctionParameter %_ptr_Function_mat3v3float
%85 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_half4x4 = OpFunction %bool None %70
%86 = OpFunctionParameter %_ptr_Function_mat4v4float
%87 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool = OpFunction %bool None %88
%90 = OpFunctionParameter %_ptr_Function_bool
%91 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool2 = OpFunction %bool None %93
%95 = OpFunctionParameter %_ptr_Function_v2bool
%96 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool3 = OpFunction %bool None %98
%100 = OpFunctionParameter %_ptr_Function_v3bool
%101 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_bool4 = OpFunction %bool None %103
%105 = OpFunctionParameter %_ptr_Function_v4bool
%106 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int = OpFunction %bool None %108
%110 = OpFunctionParameter %_ptr_Function_int
%111 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int2 = OpFunction %bool None %113
%115 = OpFunctionParameter %_ptr_Function_v2int
%116 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int3 = OpFunction %bool None %118
%120 = OpFunctionParameter %_ptr_Function_v3int
%121 = OpLabel
OpReturnValue %true
OpFunctionEnd
%takes_int4 = OpFunction %bool None %123
%125 = OpFunctionParameter %_ptr_Function_v4int
%126 = OpLabel
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %127
%128 = OpLabel
%131 = OpVariable %_ptr_Function_float Function
%137 = OpVariable %_ptr_Function_v2float Function
%144 = OpVariable %_ptr_Function_v3float Function
%151 = OpVariable %_ptr_Function_v4float Function
%160 = OpVariable %_ptr_Function_mat2v2float Function
%169 = OpVariable %_ptr_Function_mat3v3float Function
%179 = OpVariable %_ptr_Function_mat4v4float Function
%184 = OpVariable %_ptr_Function_float Function
%189 = OpVariable %_ptr_Function_v2float Function
%194 = OpVariable %_ptr_Function_v3float Function
%199 = OpVariable %_ptr_Function_v4float Function
%207 = OpVariable %_ptr_Function_mat2v2float Function
%216 = OpVariable %_ptr_Function_mat3v3float Function
%226 = OpVariable %_ptr_Function_mat4v4float Function
%231 = OpVariable %_ptr_Function_bool Function
%237 = OpVariable %_ptr_Function_v2bool Function
%243 = OpVariable %_ptr_Function_v3bool Function
%249 = OpVariable %_ptr_Function_v4bool Function
%255 = OpVariable %_ptr_Function_int Function
%262 = OpVariable %_ptr_Function_v2int Function
%269 = OpVariable %_ptr_Function_v3int Function
%276 = OpVariable %_ptr_Function_v4int Function
%279 = OpVariable %_ptr_Function_v4float Function
OpStore %131 %float_1
%132 = OpFunctionCall %bool %takes_float %131
OpSelectionMerge %134 None
OpBranchConditional %132 %133 %134
%133 = OpLabel
OpStore %137 %136
%138 = OpFunctionCall %bool %takes_float2 %137
OpBranch %134
%134 = OpLabel
%139 = OpPhi %bool %false %128 %138 %133
OpSelectionMerge %141 None
OpBranchConditional %139 %140 %141
%140 = OpLabel
OpStore %144 %143
%145 = OpFunctionCall %bool %takes_float3 %144
OpBranch %141
%141 = OpLabel
%146 = OpPhi %bool %false %134 %145 %140
OpSelectionMerge %148 None
OpBranchConditional %146 %147 %148
%147 = OpLabel
OpStore %151 %150
%152 = OpFunctionCall %bool %takes_float4 %151
OpBranch %148
%148 = OpLabel
%153 = OpPhi %bool %false %141 %152 %147
OpSelectionMerge %155 None
OpBranchConditional %153 %154 %155
%154 = OpLabel
%158 = OpCompositeConstruct %v2float %float_2 %float_0
%159 = OpCompositeConstruct %v2float %float_0 %float_2
%156 = OpCompositeConstruct %mat2v2float %158 %159
OpStore %160 %156
%161 = OpFunctionCall %bool %takes_float2x2 %160
OpBranch %155
%155 = OpLabel
%162 = OpPhi %bool %false %148 %161 %154
OpSelectionMerge %164 None
OpBranchConditional %162 %163 %164
%163 = OpLabel
%166 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%167 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%168 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%165 = OpCompositeConstruct %mat3v3float %166 %167 %168
OpStore %169 %165
%170 = OpFunctionCall %bool %takes_float3x3 %169
OpBranch %164
%164 = OpLabel
%171 = OpPhi %bool %false %155 %170 %163
OpSelectionMerge %173 None
OpBranchConditional %171 %172 %173
%172 = OpLabel
%175 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%176 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%177 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%178 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%174 = OpCompositeConstruct %mat4v4float %175 %176 %177 %178
OpStore %179 %174
%180 = OpFunctionCall %bool %takes_float4x4 %179
OpBranch %173
%173 = OpLabel
%181 = OpPhi %bool %false %164 %180 %172
OpSelectionMerge %183 None
OpBranchConditional %181 %182 %183
%182 = OpLabel
OpStore %184 %float_1
%185 = OpFunctionCall %bool %takes_half %184
OpBranch %183
%183 = OpLabel
%186 = OpPhi %bool %false %173 %185 %182
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
OpStore %189 %136
%190 = OpFunctionCall %bool %takes_half2 %189
OpBranch %188
%188 = OpLabel
%191 = OpPhi %bool %false %183 %190 %187
OpSelectionMerge %193 None
OpBranchConditional %191 %192 %193
%192 = OpLabel
OpStore %194 %143
%195 = OpFunctionCall %bool %takes_half3 %194
OpBranch %193
%193 = OpLabel
%196 = OpPhi %bool %false %188 %195 %192
OpSelectionMerge %198 None
OpBranchConditional %196 %197 %198
%197 = OpLabel
OpStore %199 %150
%200 = OpFunctionCall %bool %takes_half4 %199
OpBranch %198
%198 = OpLabel
%201 = OpPhi %bool %false %193 %200 %197
OpSelectionMerge %203 None
OpBranchConditional %201 %202 %203
%202 = OpLabel
%205 = OpCompositeConstruct %v2float %float_2 %float_0
%206 = OpCompositeConstruct %v2float %float_0 %float_2
%204 = OpCompositeConstruct %mat2v2float %205 %206
OpStore %207 %204
%208 = OpFunctionCall %bool %takes_half2x2 %207
OpBranch %203
%203 = OpLabel
%209 = OpPhi %bool %false %198 %208 %202
OpSelectionMerge %211 None
OpBranchConditional %209 %210 %211
%210 = OpLabel
%213 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%214 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%215 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%212 = OpCompositeConstruct %mat3v3float %213 %214 %215
OpStore %216 %212
%217 = OpFunctionCall %bool %takes_half3x3 %216
OpBranch %211
%211 = OpLabel
%218 = OpPhi %bool %false %203 %217 %210
OpSelectionMerge %220 None
OpBranchConditional %218 %219 %220
%219 = OpLabel
%222 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%223 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%224 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%225 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%221 = OpCompositeConstruct %mat4v4float %222 %223 %224 %225
OpStore %226 %221
%227 = OpFunctionCall %bool %takes_half4x4 %226
OpBranch %220
%220 = OpLabel
%228 = OpPhi %bool %false %211 %227 %219
OpSelectionMerge %230 None
OpBranchConditional %228 %229 %230
%229 = OpLabel
OpStore %231 %true
%232 = OpFunctionCall %bool %takes_bool %231
OpBranch %230
%230 = OpLabel
%233 = OpPhi %bool %false %220 %232 %229
OpSelectionMerge %235 None
OpBranchConditional %233 %234 %235
%234 = OpLabel
OpStore %237 %236
%238 = OpFunctionCall %bool %takes_bool2 %237
OpBranch %235
%235 = OpLabel
%239 = OpPhi %bool %false %230 %238 %234
OpSelectionMerge %241 None
OpBranchConditional %239 %240 %241
%240 = OpLabel
OpStore %243 %242
%244 = OpFunctionCall %bool %takes_bool3 %243
OpBranch %241
%241 = OpLabel
%245 = OpPhi %bool %false %235 %244 %240
OpSelectionMerge %247 None
OpBranchConditional %245 %246 %247
%246 = OpLabel
OpStore %249 %248
%250 = OpFunctionCall %bool %takes_bool4 %249
OpBranch %247
%247 = OpLabel
%251 = OpPhi %bool %false %241 %250 %246
OpSelectionMerge %253 None
OpBranchConditional %251 %252 %253
%252 = OpLabel
OpStore %255 %int_1
%256 = OpFunctionCall %bool %takes_int %255
OpBranch %253
%253 = OpLabel
%257 = OpPhi %bool %false %247 %256 %252
OpSelectionMerge %259 None
OpBranchConditional %257 %258 %259
%258 = OpLabel
OpStore %262 %261
%263 = OpFunctionCall %bool %takes_int2 %262
OpBranch %259
%259 = OpLabel
%264 = OpPhi %bool %false %253 %263 %258
OpSelectionMerge %266 None
OpBranchConditional %264 %265 %266
%265 = OpLabel
OpStore %269 %268
%270 = OpFunctionCall %bool %takes_int3 %269
OpBranch %266
%266 = OpLabel
%271 = OpPhi %bool %false %259 %270 %265
OpSelectionMerge %273 None
OpBranchConditional %271 %272 %273
%272 = OpLabel
OpStore %276 %275
%277 = OpFunctionCall %bool %takes_int4 %276
OpBranch %273
%273 = OpLabel
%278 = OpPhi %bool %false %266 %277 %272
OpSelectionMerge %282 None
OpBranchConditional %278 %280 %281
%280 = OpLabel
%283 = OpAccessChain %_ptr_Uniform_v4float %32 %int_0
%286 = OpLoad %v4float %283
OpStore %279 %286
OpBranch %282
%281 = OpLabel
%287 = OpAccessChain %_ptr_Uniform_v4float %32 %int_1
%288 = OpLoad %v4float %287
OpStore %279 %288
OpBranch %282
%282 = OpLabel
%289 = OpLoad %v4float %279
OpReturnValue %289
OpFunctionEnd
