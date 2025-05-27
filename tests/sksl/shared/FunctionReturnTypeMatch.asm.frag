               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %returns_float2_f2 "returns_float2_f2"
               OpName %returns_float3_f3 "returns_float3_f3"
               OpName %returns_float4_f4 "returns_float4_f4"
               OpName %returns_float2x2_f22 "returns_float2x2_f22"
               OpName %returns_float3x3_f33 "returns_float3x3_f33"
               OpName %returns_float4x4_f44 "returns_float4x4_f44"
               OpName %returns_half_h "returns_half_h"
               OpName %returns_half2_h2 "returns_half2_h2"
               OpName %returns_half3_h3 "returns_half3_h3"
               OpName %returns_half4_h4 "returns_half4_h4"
               OpName %returns_half2x2_h22 "returns_half2x2_h22"
               OpName %returns_half3x3_h33 "returns_half3x3_h33"
               OpName %returns_half4x4_h44 "returns_half4x4_h44"
               OpName %returns_bool_b "returns_bool_b"
               OpName %returns_bool2_b2 "returns_bool2_b2"
               OpName %returns_bool3_b3 "returns_bool3_b3"
               OpName %returns_bool4_b4 "returns_bool4_b4"
               OpName %returns_int_i "returns_int_i"
               OpName %returns_int2_i2 "returns_int2_i2"
               OpName %returns_int3_i3 "returns_int3_i3"
               OpName %returns_int4_i4 "returns_int4_i4"
               OpName %main "main"
               OpName %x1 "x1"
               OpName %x2 "x2"
               OpName %x3 "x3"
               OpName %x4 "x4"
               OpName %x5 "x5"
               OpName %x6 "x6"
               OpName %x7 "x7"
               OpName %x8 "x8"
               OpName %x9 "x9"
               OpName %x10 "x10"
               OpName %x11 "x11"
               OpName %x12 "x12"
               OpName %x13 "x13"
               OpName %x14 "x14"
               OpName %x15 "x15"
               OpName %x16 "x16"
               OpName %x17 "x17"
               OpName %x18 "x18"
               OpName %x19 "x19"
               OpName %x20 "x20"
               OpName %x21 "x21"
               OpName %x22 "x22"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %28 Binding 0
               OpDecorate %28 DescriptorSet 0
               OpDecorate %x8 RelaxedPrecision
               OpDecorate %x9 RelaxedPrecision
               OpDecorate %x10 RelaxedPrecision
               OpDecorate %x11 RelaxedPrecision
               OpDecorate %x12 RelaxedPrecision
               OpDecorate %x13 RelaxedPrecision
               OpDecorate %x14 RelaxedPrecision
               OpDecorate %248 RelaxedPrecision
               OpDecorate %249 RelaxedPrecision
               OpDecorate %251 RelaxedPrecision
               OpDecorate %252 RelaxedPrecision
               OpDecorate %259 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
               OpDecorate %262 RelaxedPrecision
               OpDecorate %263 RelaxedPrecision
               OpDecorate %266 RelaxedPrecision
               OpDecorate %267 RelaxedPrecision
               OpDecorate %274 RelaxedPrecision
               OpDecorate %275 RelaxedPrecision
               OpDecorate %277 RelaxedPrecision
               OpDecorate %278 RelaxedPrecision
               OpDecorate %281 RelaxedPrecision
               OpDecorate %282 RelaxedPrecision
               OpDecorate %285 RelaxedPrecision
               OpDecorate %286 RelaxedPrecision
               OpDecorate %343 RelaxedPrecision
               OpDecorate %345 RelaxedPrecision
               OpDecorate %346 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %28 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %33 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %37 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %41 = OpTypeFunction %v2float
    %float_2 = OpConstant %float 2
         %44 = OpConstantComposite %v2float %float_2 %float_2
    %v3float = OpTypeVector %float 3
         %46 = OpTypeFunction %v3float
    %float_3 = OpConstant %float 3
         %49 = OpConstantComposite %v3float %float_3 %float_3 %float_3
         %50 = OpTypeFunction %v4float
    %float_4 = OpConstant %float 4
         %53 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%mat2v2float = OpTypeMatrix %v2float 2
         %55 = OpTypeFunction %mat2v2float
         %57 = OpConstantComposite %v2float %float_2 %float_0
         %58 = OpConstantComposite %v2float %float_0 %float_2
         %59 = OpConstantComposite %mat2v2float %57 %58
%mat3v3float = OpTypeMatrix %v3float 3
         %61 = OpTypeFunction %mat3v3float
         %63 = OpConstantComposite %v3float %float_3 %float_0 %float_0
         %64 = OpConstantComposite %v3float %float_0 %float_3 %float_0
         %65 = OpConstantComposite %v3float %float_0 %float_0 %float_3
         %66 = OpConstantComposite %mat3v3float %63 %64 %65
%mat4v4float = OpTypeMatrix %v4float 4
         %68 = OpTypeFunction %mat4v4float
         %70 = OpConstantComposite %v4float %float_4 %float_0 %float_0 %float_0
         %71 = OpConstantComposite %v4float %float_0 %float_4 %float_0 %float_0
         %72 = OpConstantComposite %v4float %float_0 %float_0 %float_4 %float_0
         %73 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_4
         %74 = OpConstantComposite %mat4v4float %70 %71 %72 %73
         %75 = OpTypeFunction %float
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
         %85 = OpTypeFunction %bool
       %true = OpConstantTrue %bool
     %v2bool = OpTypeVector %bool 2
         %89 = OpTypeFunction %v2bool
         %91 = OpConstantComposite %v2bool %true %true
     %v3bool = OpTypeVector %bool 3
         %93 = OpTypeFunction %v3bool
         %95 = OpConstantComposite %v3bool %true %true %true
     %v4bool = OpTypeVector %bool 4
         %97 = OpTypeFunction %v4bool
         %99 = OpConstantComposite %v4bool %true %true %true %true
        %int = OpTypeInt 32 1
        %101 = OpTypeFunction %int
      %int_1 = OpConstant %int 1
      %v2int = OpTypeVector %int 2
        %105 = OpTypeFunction %v2int
      %int_2 = OpConstant %int 2
        %108 = OpConstantComposite %v2int %int_2 %int_2
      %v3int = OpTypeVector %int 3
        %110 = OpTypeFunction %v3int
      %int_3 = OpConstant %int 3
        %113 = OpConstantComposite %v3int %int_3 %int_3 %int_3
      %v4int = OpTypeVector %int 4
        %115 = OpTypeFunction %v4int
      %int_4 = OpConstant %int 4
        %118 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
        %119 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_v2int = OpTypePointer Function %v2int
%_ptr_Function_v3int = OpTypePointer Function %v3int
%_ptr_Function_v4int = OpTypePointer Function %v4int
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %33
         %34 = OpLabel
         %38 = OpVariable %_ptr_Function_v2float Function
               OpStore %38 %37
         %40 = OpFunctionCall %v4float %main %38
               OpStore %sk_FragColor %40
               OpReturn
               OpFunctionEnd
%returns_float2_f2 = OpFunction %v2float None %41
         %42 = OpLabel
               OpReturnValue %44
               OpFunctionEnd
%returns_float3_f3 = OpFunction %v3float None %46
         %47 = OpLabel
               OpReturnValue %49
               OpFunctionEnd
%returns_float4_f4 = OpFunction %v4float None %50
         %51 = OpLabel
               OpReturnValue %53
               OpFunctionEnd
%returns_float2x2_f22 = OpFunction %mat2v2float None %55
         %56 = OpLabel
               OpReturnValue %59
               OpFunctionEnd
%returns_float3x3_f33 = OpFunction %mat3v3float None %61
         %62 = OpLabel
               OpReturnValue %66
               OpFunctionEnd
%returns_float4x4_f44 = OpFunction %mat4v4float None %68
         %69 = OpLabel
               OpReturnValue %74
               OpFunctionEnd
%returns_half_h = OpFunction %float None %75
         %76 = OpLabel
               OpReturnValue %float_1
               OpFunctionEnd
%returns_half2_h2 = OpFunction %v2float None %41
         %78 = OpLabel
               OpReturnValue %44
               OpFunctionEnd
%returns_half3_h3 = OpFunction %v3float None %46
         %79 = OpLabel
               OpReturnValue %49
               OpFunctionEnd
%returns_half4_h4 = OpFunction %v4float None %50
         %80 = OpLabel
               OpReturnValue %53
               OpFunctionEnd
%returns_half2x2_h22 = OpFunction %mat2v2float None %55
         %81 = OpLabel
               OpReturnValue %59
               OpFunctionEnd
%returns_half3x3_h33 = OpFunction %mat3v3float None %61
         %82 = OpLabel
               OpReturnValue %66
               OpFunctionEnd
%returns_half4x4_h44 = OpFunction %mat4v4float None %68
         %83 = OpLabel
               OpReturnValue %74
               OpFunctionEnd
%returns_bool_b = OpFunction %bool None %85
         %86 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%returns_bool2_b2 = OpFunction %v2bool None %89
         %90 = OpLabel
               OpReturnValue %91
               OpFunctionEnd
%returns_bool3_b3 = OpFunction %v3bool None %93
         %94 = OpLabel
               OpReturnValue %95
               OpFunctionEnd
%returns_bool4_b4 = OpFunction %v4bool None %97
         %98 = OpLabel
               OpReturnValue %99
               OpFunctionEnd
%returns_int_i = OpFunction %int None %101
        %102 = OpLabel
               OpReturnValue %int_1
               OpFunctionEnd
%returns_int2_i2 = OpFunction %v2int None %105
        %106 = OpLabel
               OpReturnValue %108
               OpFunctionEnd
%returns_int3_i3 = OpFunction %v3int None %110
        %111 = OpLabel
               OpReturnValue %113
               OpFunctionEnd
%returns_int4_i4 = OpFunction %v4int None %115
        %116 = OpLabel
               OpReturnValue %118
               OpFunctionEnd
       %main = OpFunction %v4float None %119
        %120 = OpFunctionParameter %_ptr_Function_v2float
        %121 = OpLabel
         %x1 = OpVariable %_ptr_Function_float Function
         %x2 = OpVariable %_ptr_Function_v2float Function
         %x3 = OpVariable %_ptr_Function_v3float Function
         %x4 = OpVariable %_ptr_Function_v4float Function
         %x5 = OpVariable %_ptr_Function_mat2v2float Function
         %x6 = OpVariable %_ptr_Function_mat3v3float Function
         %x7 = OpVariable %_ptr_Function_mat4v4float Function
         %x8 = OpVariable %_ptr_Function_float Function
         %x9 = OpVariable %_ptr_Function_v2float Function
        %x10 = OpVariable %_ptr_Function_v3float Function
        %x11 = OpVariable %_ptr_Function_v4float Function
        %x12 = OpVariable %_ptr_Function_mat2v2float Function
        %x13 = OpVariable %_ptr_Function_mat3v3float Function
        %x14 = OpVariable %_ptr_Function_mat4v4float Function
        %x15 = OpVariable %_ptr_Function_bool Function
        %x16 = OpVariable %_ptr_Function_v2bool Function
        %x17 = OpVariable %_ptr_Function_v3bool Function
        %x18 = OpVariable %_ptr_Function_v4bool Function
        %x19 = OpVariable %_ptr_Function_int Function
        %x20 = OpVariable %_ptr_Function_v2int Function
        %x21 = OpVariable %_ptr_Function_v3int Function
        %x22 = OpVariable %_ptr_Function_v4int Function
        %336 = OpVariable %_ptr_Function_v4float Function
               OpStore %x1 %float_1
               OpStore %x2 %44
               OpStore %x3 %49
               OpStore %x4 %53
               OpStore %x5 %59
               OpStore %x6 %66
               OpStore %x7 %74
               OpStore %x8 %float_1
               OpStore %x9 %44
               OpStore %x10 %49
               OpStore %x11 %53
               OpStore %x12 %59
               OpStore %x13 %66
               OpStore %x14 %74
               OpStore %x15 %true
               OpStore %x16 %91
               OpStore %x17 %95
               OpStore %x18 %99
               OpStore %x19 %int_1
               OpStore %x20 %108
               OpStore %x21 %113
               OpStore %x22 %118
               OpSelectionMerge %160 None
               OpBranchConditional %true %159 %160
        %159 = OpLabel
        %161 = OpFunctionCall %v2float %returns_float2_f2
        %162 = OpFOrdEqual %v2bool %44 %161
        %163 = OpAll %bool %162
               OpBranch %160
        %160 = OpLabel
        %164 = OpPhi %bool %false %121 %163 %159
               OpSelectionMerge %166 None
               OpBranchConditional %164 %165 %166
        %165 = OpLabel
        %167 = OpFunctionCall %v3float %returns_float3_f3
        %168 = OpFOrdEqual %v3bool %49 %167
        %169 = OpAll %bool %168
               OpBranch %166
        %166 = OpLabel
        %170 = OpPhi %bool %false %160 %169 %165
               OpSelectionMerge %172 None
               OpBranchConditional %170 %171 %172
        %171 = OpLabel
        %173 = OpFunctionCall %v4float %returns_float4_f4
        %174 = OpFOrdEqual %v4bool %53 %173
        %175 = OpAll %bool %174
               OpBranch %172
        %172 = OpLabel
        %176 = OpPhi %bool %false %166 %175 %171
               OpSelectionMerge %178 None
               OpBranchConditional %176 %177 %178
        %177 = OpLabel
        %179 = OpFunctionCall %mat2v2float %returns_float2x2_f22
        %180 = OpCompositeExtract %v2float %179 0
        %181 = OpFOrdEqual %v2bool %57 %180
        %182 = OpAll %bool %181
        %183 = OpCompositeExtract %v2float %179 1
        %184 = OpFOrdEqual %v2bool %58 %183
        %185 = OpAll %bool %184
        %186 = OpLogicalAnd %bool %182 %185
               OpBranch %178
        %178 = OpLabel
        %187 = OpPhi %bool %false %172 %186 %177
               OpSelectionMerge %189 None
               OpBranchConditional %187 %188 %189
        %188 = OpLabel
        %190 = OpFunctionCall %mat3v3float %returns_float3x3_f33
        %191 = OpCompositeExtract %v3float %190 0
        %192 = OpFOrdEqual %v3bool %63 %191
        %193 = OpAll %bool %192
        %194 = OpCompositeExtract %v3float %190 1
        %195 = OpFOrdEqual %v3bool %64 %194
        %196 = OpAll %bool %195
        %197 = OpLogicalAnd %bool %193 %196
        %198 = OpCompositeExtract %v3float %190 2
        %199 = OpFOrdEqual %v3bool %65 %198
        %200 = OpAll %bool %199
        %201 = OpLogicalAnd %bool %197 %200
               OpBranch %189
        %189 = OpLabel
        %202 = OpPhi %bool %false %178 %201 %188
               OpSelectionMerge %204 None
               OpBranchConditional %202 %203 %204
        %203 = OpLabel
        %205 = OpFunctionCall %mat4v4float %returns_float4x4_f44
        %206 = OpCompositeExtract %v4float %205 0
        %207 = OpFOrdEqual %v4bool %70 %206
        %208 = OpAll %bool %207
        %209 = OpCompositeExtract %v4float %205 1
        %210 = OpFOrdEqual %v4bool %71 %209
        %211 = OpAll %bool %210
        %212 = OpLogicalAnd %bool %208 %211
        %213 = OpCompositeExtract %v4float %205 2
        %214 = OpFOrdEqual %v4bool %72 %213
        %215 = OpAll %bool %214
        %216 = OpLogicalAnd %bool %212 %215
        %217 = OpCompositeExtract %v4float %205 3
        %218 = OpFOrdEqual %v4bool %73 %217
        %219 = OpAll %bool %218
        %220 = OpLogicalAnd %bool %216 %219
               OpBranch %204
        %204 = OpLabel
        %221 = OpPhi %bool %false %189 %220 %203
               OpSelectionMerge %223 None
               OpBranchConditional %221 %222 %223
        %222 = OpLabel
        %224 = OpFunctionCall %float %returns_half_h
        %225 = OpFOrdEqual %bool %float_1 %224
               OpBranch %223
        %223 = OpLabel
        %226 = OpPhi %bool %false %204 %225 %222
               OpSelectionMerge %228 None
               OpBranchConditional %226 %227 %228
        %227 = OpLabel
        %229 = OpFunctionCall %v2float %returns_half2_h2
        %230 = OpFOrdEqual %v2bool %44 %229
        %231 = OpAll %bool %230
               OpBranch %228
        %228 = OpLabel
        %232 = OpPhi %bool %false %223 %231 %227
               OpSelectionMerge %234 None
               OpBranchConditional %232 %233 %234
        %233 = OpLabel
        %235 = OpFunctionCall %v3float %returns_half3_h3
        %236 = OpFOrdEqual %v3bool %49 %235
        %237 = OpAll %bool %236
               OpBranch %234
        %234 = OpLabel
        %238 = OpPhi %bool %false %228 %237 %233
               OpSelectionMerge %240 None
               OpBranchConditional %238 %239 %240
        %239 = OpLabel
        %241 = OpFunctionCall %v4float %returns_half4_h4
        %242 = OpFOrdEqual %v4bool %53 %241
        %243 = OpAll %bool %242
               OpBranch %240
        %240 = OpLabel
        %244 = OpPhi %bool %false %234 %243 %239
               OpSelectionMerge %246 None
               OpBranchConditional %244 %245 %246
        %245 = OpLabel
        %247 = OpFunctionCall %mat2v2float %returns_half2x2_h22
        %248 = OpCompositeExtract %v2float %247 0
        %249 = OpFOrdEqual %v2bool %57 %248
        %250 = OpAll %bool %249
        %251 = OpCompositeExtract %v2float %247 1
        %252 = OpFOrdEqual %v2bool %58 %251
        %253 = OpAll %bool %252
        %254 = OpLogicalAnd %bool %250 %253
               OpBranch %246
        %246 = OpLabel
        %255 = OpPhi %bool %false %240 %254 %245
               OpSelectionMerge %257 None
               OpBranchConditional %255 %256 %257
        %256 = OpLabel
        %258 = OpFunctionCall %mat3v3float %returns_half3x3_h33
        %259 = OpCompositeExtract %v3float %258 0
        %260 = OpFOrdEqual %v3bool %63 %259
        %261 = OpAll %bool %260
        %262 = OpCompositeExtract %v3float %258 1
        %263 = OpFOrdEqual %v3bool %64 %262
        %264 = OpAll %bool %263
        %265 = OpLogicalAnd %bool %261 %264
        %266 = OpCompositeExtract %v3float %258 2
        %267 = OpFOrdEqual %v3bool %65 %266
        %268 = OpAll %bool %267
        %269 = OpLogicalAnd %bool %265 %268
               OpBranch %257
        %257 = OpLabel
        %270 = OpPhi %bool %false %246 %269 %256
               OpSelectionMerge %272 None
               OpBranchConditional %270 %271 %272
        %271 = OpLabel
        %273 = OpFunctionCall %mat4v4float %returns_half4x4_h44
        %274 = OpCompositeExtract %v4float %273 0
        %275 = OpFOrdEqual %v4bool %70 %274
        %276 = OpAll %bool %275
        %277 = OpCompositeExtract %v4float %273 1
        %278 = OpFOrdEqual %v4bool %71 %277
        %279 = OpAll %bool %278
        %280 = OpLogicalAnd %bool %276 %279
        %281 = OpCompositeExtract %v4float %273 2
        %282 = OpFOrdEqual %v4bool %72 %281
        %283 = OpAll %bool %282
        %284 = OpLogicalAnd %bool %280 %283
        %285 = OpCompositeExtract %v4float %273 3
        %286 = OpFOrdEqual %v4bool %73 %285
        %287 = OpAll %bool %286
        %288 = OpLogicalAnd %bool %284 %287
               OpBranch %272
        %272 = OpLabel
        %289 = OpPhi %bool %false %257 %288 %271
               OpSelectionMerge %291 None
               OpBranchConditional %289 %290 %291
        %290 = OpLabel
        %292 = OpFunctionCall %bool %returns_bool_b
        %293 = OpLogicalEqual %bool %true %292
               OpBranch %291
        %291 = OpLabel
        %294 = OpPhi %bool %false %272 %293 %290
               OpSelectionMerge %296 None
               OpBranchConditional %294 %295 %296
        %295 = OpLabel
        %297 = OpFunctionCall %v2bool %returns_bool2_b2
        %298 = OpLogicalEqual %v2bool %91 %297
        %299 = OpAll %bool %298
               OpBranch %296
        %296 = OpLabel
        %300 = OpPhi %bool %false %291 %299 %295
               OpSelectionMerge %302 None
               OpBranchConditional %300 %301 %302
        %301 = OpLabel
        %303 = OpFunctionCall %v3bool %returns_bool3_b3
        %304 = OpLogicalEqual %v3bool %95 %303
        %305 = OpAll %bool %304
               OpBranch %302
        %302 = OpLabel
        %306 = OpPhi %bool %false %296 %305 %301
               OpSelectionMerge %308 None
               OpBranchConditional %306 %307 %308
        %307 = OpLabel
        %309 = OpFunctionCall %v4bool %returns_bool4_b4
        %310 = OpLogicalEqual %v4bool %99 %309
        %311 = OpAll %bool %310
               OpBranch %308
        %308 = OpLabel
        %312 = OpPhi %bool %false %302 %311 %307
               OpSelectionMerge %314 None
               OpBranchConditional %312 %313 %314
        %313 = OpLabel
        %315 = OpFunctionCall %int %returns_int_i
        %316 = OpIEqual %bool %int_1 %315
               OpBranch %314
        %314 = OpLabel
        %317 = OpPhi %bool %false %308 %316 %313
               OpSelectionMerge %319 None
               OpBranchConditional %317 %318 %319
        %318 = OpLabel
        %320 = OpFunctionCall %v2int %returns_int2_i2
        %321 = OpIEqual %v2bool %108 %320
        %322 = OpAll %bool %321
               OpBranch %319
        %319 = OpLabel
        %323 = OpPhi %bool %false %314 %322 %318
               OpSelectionMerge %325 None
               OpBranchConditional %323 %324 %325
        %324 = OpLabel
        %326 = OpFunctionCall %v3int %returns_int3_i3
        %327 = OpIEqual %v3bool %113 %326
        %328 = OpAll %bool %327
               OpBranch %325
        %325 = OpLabel
        %329 = OpPhi %bool %false %319 %328 %324
               OpSelectionMerge %331 None
               OpBranchConditional %329 %330 %331
        %330 = OpLabel
        %332 = OpFunctionCall %v4int %returns_int4_i4
        %333 = OpIEqual %v4bool %118 %332
        %334 = OpAll %bool %333
               OpBranch %331
        %331 = OpLabel
        %335 = OpPhi %bool %false %325 %334 %330
               OpSelectionMerge %339 None
               OpBranchConditional %335 %337 %338
        %337 = OpLabel
        %340 = OpAccessChain %_ptr_Uniform_v4float %28 %int_0
        %343 = OpLoad %v4float %340
               OpStore %336 %343
               OpBranch %339
        %338 = OpLabel
        %344 = OpAccessChain %_ptr_Uniform_v4float %28 %int_1
        %345 = OpLoad %v4float %344
               OpStore %336 %345
               OpBranch %339
        %339 = OpLabel
        %346 = OpLoad %v4float %336
               OpReturnValue %346
               OpFunctionEnd
