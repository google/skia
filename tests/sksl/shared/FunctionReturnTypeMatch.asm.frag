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
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %31 Binding 0
               OpDecorate %31 DescriptorSet 0
               OpDecorate %x8 RelaxedPrecision
               OpDecorate %x9 RelaxedPrecision
               OpDecorate %x10 RelaxedPrecision
               OpDecorate %x11 RelaxedPrecision
               OpDecorate %x12 RelaxedPrecision
               OpDecorate %x13 RelaxedPrecision
               OpDecorate %x14 RelaxedPrecision
               OpDecorate %250 RelaxedPrecision
               OpDecorate %251 RelaxedPrecision
               OpDecorate %253 RelaxedPrecision
               OpDecorate %254 RelaxedPrecision
               OpDecorate %261 RelaxedPrecision
               OpDecorate %262 RelaxedPrecision
               OpDecorate %264 RelaxedPrecision
               OpDecorate %265 RelaxedPrecision
               OpDecorate %268 RelaxedPrecision
               OpDecorate %269 RelaxedPrecision
               OpDecorate %276 RelaxedPrecision
               OpDecorate %277 RelaxedPrecision
               OpDecorate %279 RelaxedPrecision
               OpDecorate %280 RelaxedPrecision
               OpDecorate %283 RelaxedPrecision
               OpDecorate %284 RelaxedPrecision
               OpDecorate %287 RelaxedPrecision
               OpDecorate %288 RelaxedPrecision
               OpDecorate %345 RelaxedPrecision
               OpDecorate %347 RelaxedPrecision
               OpDecorate %348 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %31 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %36 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %40 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %44 = OpTypeFunction %v2float
    %float_2 = OpConstant %float 2
         %47 = OpConstantComposite %v2float %float_2 %float_2
    %v3float = OpTypeVector %float 3
         %49 = OpTypeFunction %v3float
    %float_3 = OpConstant %float 3
         %52 = OpConstantComposite %v3float %float_3 %float_3 %float_3
         %53 = OpTypeFunction %v4float
    %float_4 = OpConstant %float 4
         %56 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%mat2v2float = OpTypeMatrix %v2float 2
         %58 = OpTypeFunction %mat2v2float
         %60 = OpConstantComposite %v2float %float_2 %float_0
         %61 = OpConstantComposite %v2float %float_0 %float_2
         %62 = OpConstantComposite %mat2v2float %60 %61
%mat3v3float = OpTypeMatrix %v3float 3
         %64 = OpTypeFunction %mat3v3float
         %66 = OpConstantComposite %v3float %float_3 %float_0 %float_0
         %67 = OpConstantComposite %v3float %float_0 %float_3 %float_0
         %68 = OpConstantComposite %v3float %float_0 %float_0 %float_3
         %69 = OpConstantComposite %mat3v3float %66 %67 %68
%mat4v4float = OpTypeMatrix %v4float 4
         %71 = OpTypeFunction %mat4v4float
         %73 = OpConstantComposite %v4float %float_4 %float_0 %float_0 %float_0
         %74 = OpConstantComposite %v4float %float_0 %float_4 %float_0 %float_0
         %75 = OpConstantComposite %v4float %float_0 %float_0 %float_4 %float_0
         %76 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_4
         %77 = OpConstantComposite %mat4v4float %73 %74 %75 %76
         %78 = OpTypeFunction %float
    %float_1 = OpConstant %float 1
         %87 = OpTypeFunction %bool
       %true = OpConstantTrue %bool
     %v2bool = OpTypeVector %bool 2
         %91 = OpTypeFunction %v2bool
         %93 = OpConstantComposite %v2bool %true %true
     %v3bool = OpTypeVector %bool 3
         %95 = OpTypeFunction %v3bool
         %97 = OpConstantComposite %v3bool %true %true %true
     %v4bool = OpTypeVector %bool 4
         %99 = OpTypeFunction %v4bool
        %101 = OpConstantComposite %v4bool %true %true %true %true
        %int = OpTypeInt 32 1
        %103 = OpTypeFunction %int
      %int_1 = OpConstant %int 1
      %v2int = OpTypeVector %int 2
        %107 = OpTypeFunction %v2int
      %int_2 = OpConstant %int 2
        %110 = OpConstantComposite %v2int %int_2 %int_2
      %v3int = OpTypeVector %int 3
        %112 = OpTypeFunction %v3int
      %int_3 = OpConstant %int 3
        %115 = OpConstantComposite %v3int %int_3 %int_3 %int_3
      %v4int = OpTypeVector %int 4
        %117 = OpTypeFunction %v4int
      %int_4 = OpConstant %int 4
        %120 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
        %121 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%_entrypoint_v = OpFunction %void None %36
         %37 = OpLabel
         %41 = OpVariable %_ptr_Function_v2float Function
               OpStore %41 %40
         %43 = OpFunctionCall %v4float %main %41
               OpStore %sk_FragColor %43
               OpReturn
               OpFunctionEnd
%returns_float2_f2 = OpFunction %v2float None %44
         %45 = OpLabel
               OpReturnValue %47
               OpFunctionEnd
%returns_float3_f3 = OpFunction %v3float None %49
         %50 = OpLabel
               OpReturnValue %52
               OpFunctionEnd
%returns_float4_f4 = OpFunction %v4float None %53
         %54 = OpLabel
               OpReturnValue %56
               OpFunctionEnd
%returns_float2x2_f22 = OpFunction %mat2v2float None %58
         %59 = OpLabel
               OpReturnValue %62
               OpFunctionEnd
%returns_float3x3_f33 = OpFunction %mat3v3float None %64
         %65 = OpLabel
               OpReturnValue %69
               OpFunctionEnd
%returns_float4x4_f44 = OpFunction %mat4v4float None %71
         %72 = OpLabel
               OpReturnValue %77
               OpFunctionEnd
%returns_half_h = OpFunction %float None %78
         %79 = OpLabel
               OpReturnValue %float_1
               OpFunctionEnd
%returns_half2_h2 = OpFunction %v2float None %44
         %81 = OpLabel
               OpReturnValue %47
               OpFunctionEnd
%returns_half3_h3 = OpFunction %v3float None %49
         %82 = OpLabel
               OpReturnValue %52
               OpFunctionEnd
%returns_half4_h4 = OpFunction %v4float None %53
         %83 = OpLabel
               OpReturnValue %56
               OpFunctionEnd
%returns_half2x2_h22 = OpFunction %mat2v2float None %58
         %84 = OpLabel
               OpReturnValue %62
               OpFunctionEnd
%returns_half3x3_h33 = OpFunction %mat3v3float None %64
         %85 = OpLabel
               OpReturnValue %69
               OpFunctionEnd
%returns_half4x4_h44 = OpFunction %mat4v4float None %71
         %86 = OpLabel
               OpReturnValue %77
               OpFunctionEnd
%returns_bool_b = OpFunction %bool None %87
         %88 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%returns_bool2_b2 = OpFunction %v2bool None %91
         %92 = OpLabel
               OpReturnValue %93
               OpFunctionEnd
%returns_bool3_b3 = OpFunction %v3bool None %95
         %96 = OpLabel
               OpReturnValue %97
               OpFunctionEnd
%returns_bool4_b4 = OpFunction %v4bool None %99
        %100 = OpLabel
               OpReturnValue %101
               OpFunctionEnd
%returns_int_i = OpFunction %int None %103
        %104 = OpLabel
               OpReturnValue %int_1
               OpFunctionEnd
%returns_int2_i2 = OpFunction %v2int None %107
        %108 = OpLabel
               OpReturnValue %110
               OpFunctionEnd
%returns_int3_i3 = OpFunction %v3int None %112
        %113 = OpLabel
               OpReturnValue %115
               OpFunctionEnd
%returns_int4_i4 = OpFunction %v4int None %117
        %118 = OpLabel
               OpReturnValue %120
               OpFunctionEnd
       %main = OpFunction %v4float None %121
        %122 = OpFunctionParameter %_ptr_Function_v2float
        %123 = OpLabel
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
        %338 = OpVariable %_ptr_Function_v4float Function
               OpStore %x1 %float_1
               OpStore %x2 %47
               OpStore %x3 %52
               OpStore %x4 %56
               OpStore %x5 %62
               OpStore %x6 %69
               OpStore %x7 %77
               OpStore %x8 %float_1
               OpStore %x9 %47
               OpStore %x10 %52
               OpStore %x11 %56
               OpStore %x12 %62
               OpStore %x13 %69
               OpStore %x14 %77
               OpStore %x15 %true
               OpStore %x16 %93
               OpStore %x17 %97
               OpStore %x18 %101
               OpStore %x19 %int_1
               OpStore %x20 %110
               OpStore %x21 %115
               OpStore %x22 %120
               OpSelectionMerge %162 None
               OpBranchConditional %true %161 %162
        %161 = OpLabel
        %163 = OpFunctionCall %v2float %returns_float2_f2
        %164 = OpFOrdEqual %v2bool %47 %163
        %165 = OpAll %bool %164
               OpBranch %162
        %162 = OpLabel
        %166 = OpPhi %bool %false %123 %165 %161
               OpSelectionMerge %168 None
               OpBranchConditional %166 %167 %168
        %167 = OpLabel
        %169 = OpFunctionCall %v3float %returns_float3_f3
        %170 = OpFOrdEqual %v3bool %52 %169
        %171 = OpAll %bool %170
               OpBranch %168
        %168 = OpLabel
        %172 = OpPhi %bool %false %162 %171 %167
               OpSelectionMerge %174 None
               OpBranchConditional %172 %173 %174
        %173 = OpLabel
        %175 = OpFunctionCall %v4float %returns_float4_f4
        %176 = OpFOrdEqual %v4bool %56 %175
        %177 = OpAll %bool %176
               OpBranch %174
        %174 = OpLabel
        %178 = OpPhi %bool %false %168 %177 %173
               OpSelectionMerge %180 None
               OpBranchConditional %178 %179 %180
        %179 = OpLabel
        %181 = OpFunctionCall %mat2v2float %returns_float2x2_f22
        %182 = OpCompositeExtract %v2float %181 0
        %183 = OpFOrdEqual %v2bool %60 %182
        %184 = OpAll %bool %183
        %185 = OpCompositeExtract %v2float %181 1
        %186 = OpFOrdEqual %v2bool %61 %185
        %187 = OpAll %bool %186
        %188 = OpLogicalAnd %bool %184 %187
               OpBranch %180
        %180 = OpLabel
        %189 = OpPhi %bool %false %174 %188 %179
               OpSelectionMerge %191 None
               OpBranchConditional %189 %190 %191
        %190 = OpLabel
        %192 = OpFunctionCall %mat3v3float %returns_float3x3_f33
        %193 = OpCompositeExtract %v3float %192 0
        %194 = OpFOrdEqual %v3bool %66 %193
        %195 = OpAll %bool %194
        %196 = OpCompositeExtract %v3float %192 1
        %197 = OpFOrdEqual %v3bool %67 %196
        %198 = OpAll %bool %197
        %199 = OpLogicalAnd %bool %195 %198
        %200 = OpCompositeExtract %v3float %192 2
        %201 = OpFOrdEqual %v3bool %68 %200
        %202 = OpAll %bool %201
        %203 = OpLogicalAnd %bool %199 %202
               OpBranch %191
        %191 = OpLabel
        %204 = OpPhi %bool %false %180 %203 %190
               OpSelectionMerge %206 None
               OpBranchConditional %204 %205 %206
        %205 = OpLabel
        %207 = OpFunctionCall %mat4v4float %returns_float4x4_f44
        %208 = OpCompositeExtract %v4float %207 0
        %209 = OpFOrdEqual %v4bool %73 %208
        %210 = OpAll %bool %209
        %211 = OpCompositeExtract %v4float %207 1
        %212 = OpFOrdEqual %v4bool %74 %211
        %213 = OpAll %bool %212
        %214 = OpLogicalAnd %bool %210 %213
        %215 = OpCompositeExtract %v4float %207 2
        %216 = OpFOrdEqual %v4bool %75 %215
        %217 = OpAll %bool %216
        %218 = OpLogicalAnd %bool %214 %217
        %219 = OpCompositeExtract %v4float %207 3
        %220 = OpFOrdEqual %v4bool %76 %219
        %221 = OpAll %bool %220
        %222 = OpLogicalAnd %bool %218 %221
               OpBranch %206
        %206 = OpLabel
        %223 = OpPhi %bool %false %191 %222 %205
               OpSelectionMerge %225 None
               OpBranchConditional %223 %224 %225
        %224 = OpLabel
        %226 = OpFunctionCall %float %returns_half_h
        %227 = OpFOrdEqual %bool %float_1 %226
               OpBranch %225
        %225 = OpLabel
        %228 = OpPhi %bool %false %206 %227 %224
               OpSelectionMerge %230 None
               OpBranchConditional %228 %229 %230
        %229 = OpLabel
        %231 = OpFunctionCall %v2float %returns_half2_h2
        %232 = OpFOrdEqual %v2bool %47 %231
        %233 = OpAll %bool %232
               OpBranch %230
        %230 = OpLabel
        %234 = OpPhi %bool %false %225 %233 %229
               OpSelectionMerge %236 None
               OpBranchConditional %234 %235 %236
        %235 = OpLabel
        %237 = OpFunctionCall %v3float %returns_half3_h3
        %238 = OpFOrdEqual %v3bool %52 %237
        %239 = OpAll %bool %238
               OpBranch %236
        %236 = OpLabel
        %240 = OpPhi %bool %false %230 %239 %235
               OpSelectionMerge %242 None
               OpBranchConditional %240 %241 %242
        %241 = OpLabel
        %243 = OpFunctionCall %v4float %returns_half4_h4
        %244 = OpFOrdEqual %v4bool %56 %243
        %245 = OpAll %bool %244
               OpBranch %242
        %242 = OpLabel
        %246 = OpPhi %bool %false %236 %245 %241
               OpSelectionMerge %248 None
               OpBranchConditional %246 %247 %248
        %247 = OpLabel
        %249 = OpFunctionCall %mat2v2float %returns_half2x2_h22
        %250 = OpCompositeExtract %v2float %249 0
        %251 = OpFOrdEqual %v2bool %60 %250
        %252 = OpAll %bool %251
        %253 = OpCompositeExtract %v2float %249 1
        %254 = OpFOrdEqual %v2bool %61 %253
        %255 = OpAll %bool %254
        %256 = OpLogicalAnd %bool %252 %255
               OpBranch %248
        %248 = OpLabel
        %257 = OpPhi %bool %false %242 %256 %247
               OpSelectionMerge %259 None
               OpBranchConditional %257 %258 %259
        %258 = OpLabel
        %260 = OpFunctionCall %mat3v3float %returns_half3x3_h33
        %261 = OpCompositeExtract %v3float %260 0
        %262 = OpFOrdEqual %v3bool %66 %261
        %263 = OpAll %bool %262
        %264 = OpCompositeExtract %v3float %260 1
        %265 = OpFOrdEqual %v3bool %67 %264
        %266 = OpAll %bool %265
        %267 = OpLogicalAnd %bool %263 %266
        %268 = OpCompositeExtract %v3float %260 2
        %269 = OpFOrdEqual %v3bool %68 %268
        %270 = OpAll %bool %269
        %271 = OpLogicalAnd %bool %267 %270
               OpBranch %259
        %259 = OpLabel
        %272 = OpPhi %bool %false %248 %271 %258
               OpSelectionMerge %274 None
               OpBranchConditional %272 %273 %274
        %273 = OpLabel
        %275 = OpFunctionCall %mat4v4float %returns_half4x4_h44
        %276 = OpCompositeExtract %v4float %275 0
        %277 = OpFOrdEqual %v4bool %73 %276
        %278 = OpAll %bool %277
        %279 = OpCompositeExtract %v4float %275 1
        %280 = OpFOrdEqual %v4bool %74 %279
        %281 = OpAll %bool %280
        %282 = OpLogicalAnd %bool %278 %281
        %283 = OpCompositeExtract %v4float %275 2
        %284 = OpFOrdEqual %v4bool %75 %283
        %285 = OpAll %bool %284
        %286 = OpLogicalAnd %bool %282 %285
        %287 = OpCompositeExtract %v4float %275 3
        %288 = OpFOrdEqual %v4bool %76 %287
        %289 = OpAll %bool %288
        %290 = OpLogicalAnd %bool %286 %289
               OpBranch %274
        %274 = OpLabel
        %291 = OpPhi %bool %false %259 %290 %273
               OpSelectionMerge %293 None
               OpBranchConditional %291 %292 %293
        %292 = OpLabel
        %294 = OpFunctionCall %bool %returns_bool_b
        %295 = OpLogicalEqual %bool %true %294
               OpBranch %293
        %293 = OpLabel
        %296 = OpPhi %bool %false %274 %295 %292
               OpSelectionMerge %298 None
               OpBranchConditional %296 %297 %298
        %297 = OpLabel
        %299 = OpFunctionCall %v2bool %returns_bool2_b2
        %300 = OpLogicalEqual %v2bool %93 %299
        %301 = OpAll %bool %300
               OpBranch %298
        %298 = OpLabel
        %302 = OpPhi %bool %false %293 %301 %297
               OpSelectionMerge %304 None
               OpBranchConditional %302 %303 %304
        %303 = OpLabel
        %305 = OpFunctionCall %v3bool %returns_bool3_b3
        %306 = OpLogicalEqual %v3bool %97 %305
        %307 = OpAll %bool %306
               OpBranch %304
        %304 = OpLabel
        %308 = OpPhi %bool %false %298 %307 %303
               OpSelectionMerge %310 None
               OpBranchConditional %308 %309 %310
        %309 = OpLabel
        %311 = OpFunctionCall %v4bool %returns_bool4_b4
        %312 = OpLogicalEqual %v4bool %101 %311
        %313 = OpAll %bool %312
               OpBranch %310
        %310 = OpLabel
        %314 = OpPhi %bool %false %304 %313 %309
               OpSelectionMerge %316 None
               OpBranchConditional %314 %315 %316
        %315 = OpLabel
        %317 = OpFunctionCall %int %returns_int_i
        %318 = OpIEqual %bool %int_1 %317
               OpBranch %316
        %316 = OpLabel
        %319 = OpPhi %bool %false %310 %318 %315
               OpSelectionMerge %321 None
               OpBranchConditional %319 %320 %321
        %320 = OpLabel
        %322 = OpFunctionCall %v2int %returns_int2_i2
        %323 = OpIEqual %v2bool %110 %322
        %324 = OpAll %bool %323
               OpBranch %321
        %321 = OpLabel
        %325 = OpPhi %bool %false %316 %324 %320
               OpSelectionMerge %327 None
               OpBranchConditional %325 %326 %327
        %326 = OpLabel
        %328 = OpFunctionCall %v3int %returns_int3_i3
        %329 = OpIEqual %v3bool %115 %328
        %330 = OpAll %bool %329
               OpBranch %327
        %327 = OpLabel
        %331 = OpPhi %bool %false %321 %330 %326
               OpSelectionMerge %333 None
               OpBranchConditional %331 %332 %333
        %332 = OpLabel
        %334 = OpFunctionCall %v4int %returns_int4_i4
        %335 = OpIEqual %v4bool %120 %334
        %336 = OpAll %bool %335
               OpBranch %333
        %333 = OpLabel
        %337 = OpPhi %bool %false %327 %336 %332
               OpSelectionMerge %341 None
               OpBranchConditional %337 %339 %340
        %339 = OpLabel
        %342 = OpAccessChain %_ptr_Uniform_v4float %31 %int_0
        %345 = OpLoad %v4float %342
               OpStore %338 %345
               OpBranch %341
        %340 = OpLabel
        %346 = OpAccessChain %_ptr_Uniform_v4float %31 %int_1
        %347 = OpLoad %v4float %346
               OpStore %338 %347
               OpBranch %341
        %341 = OpLabel
        %348 = OpLoad %v4float %338
               OpReturnValue %348
               OpFunctionEnd
