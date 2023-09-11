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
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %30 Binding 0
               OpDecorate %30 DescriptorSet 0
               OpDecorate %285 RelaxedPrecision
               OpDecorate %287 RelaxedPrecision
               OpDecorate %288 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %30 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %35 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %39 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %44 = OpTypeFunction %bool
       %true = OpConstantTrue %bool
%_ptr_Function_float = OpTypePointer Function %float
         %48 = OpTypeFunction %bool %_ptr_Function_float
         %51 = OpTypeFunction %bool %_ptr_Function_v2float
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %56 = OpTypeFunction %bool %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %60 = OpTypeFunction %bool %_ptr_Function_v4float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
         %65 = OpTypeFunction %bool %_ptr_Function_mat2v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
         %70 = OpTypeFunction %bool %_ptr_Function_mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
         %75 = OpTypeFunction %bool %_ptr_Function_mat4v4float
%_ptr_Function_bool = OpTypePointer Function %bool
         %93 = OpTypeFunction %bool %_ptr_Function_bool
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
         %98 = OpTypeFunction %bool %_ptr_Function_v2bool
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
        %103 = OpTypeFunction %bool %_ptr_Function_v3bool
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
        %108 = OpTypeFunction %bool %_ptr_Function_v4bool
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
        %113 = OpTypeFunction %bool %_ptr_Function_int
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
        %118 = OpTypeFunction %bool %_ptr_Function_v2int
      %v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
        %123 = OpTypeFunction %bool %_ptr_Function_v3int
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
        %128 = OpTypeFunction %bool %_ptr_Function_v4int
        %131 = OpTypeFunction %v4float %_ptr_Function_v2float
      %false = OpConstantFalse %bool
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
        %148 = OpConstantComposite %v2float %float_2 %float_2
    %float_3 = OpConstant %float 3
        %155 = OpConstantComposite %v3float %float_3 %float_3 %float_3
    %float_4 = OpConstant %float 4
        %162 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
        %168 = OpConstantComposite %v2float %float_2 %float_0
        %169 = OpConstantComposite %v2float %float_0 %float_2
        %170 = OpConstantComposite %mat2v2float %168 %169
        %176 = OpConstantComposite %v3float %float_3 %float_0 %float_0
        %177 = OpConstantComposite %v3float %float_0 %float_3 %float_0
        %178 = OpConstantComposite %v3float %float_0 %float_0 %float_3
        %179 = OpConstantComposite %mat3v3float %176 %177 %178
        %185 = OpConstantComposite %v4float %float_4 %float_0 %float_0 %float_0
        %186 = OpConstantComposite %v4float %float_0 %float_4 %float_0 %float_0
        %187 = OpConstantComposite %v4float %float_0 %float_0 %float_4 %float_0
        %188 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_4
        %189 = OpConstantComposite %mat4v4float %185 %186 %187 %188
        %235 = OpConstantComposite %v2bool %true %true
        %241 = OpConstantComposite %v3bool %true %true %true
        %247 = OpConstantComposite %v4bool %true %true %true %true
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
        %260 = OpConstantComposite %v2int %int_2 %int_2
      %int_3 = OpConstant %int 3
        %267 = OpConstantComposite %v3int %int_3 %int_3 %int_3
      %int_4 = OpConstant %int 4
        %274 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %35
         %36 = OpLabel
         %40 = OpVariable %_ptr_Function_v2float Function
               OpStore %40 %39
         %42 = OpFunctionCall %v4float %main %40
               OpStore %sk_FragColor %42
               OpReturn
               OpFunctionEnd
%takes_void_b = OpFunction %bool None %44
         %45 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_float_bf = OpFunction %bool None %48
         %49 = OpFunctionParameter %_ptr_Function_float
         %50 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_float2_bf2 = OpFunction %bool None %51
         %52 = OpFunctionParameter %_ptr_Function_v2float
         %53 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_float3_bf3 = OpFunction %bool None %56
         %57 = OpFunctionParameter %_ptr_Function_v3float
         %58 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_float4_bf4 = OpFunction %bool None %60
         %61 = OpFunctionParameter %_ptr_Function_v4float
         %62 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_float2x2_bf22 = OpFunction %bool None %65
         %66 = OpFunctionParameter %_ptr_Function_mat2v2float
         %67 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_float3x3_bf33 = OpFunction %bool None %70
         %71 = OpFunctionParameter %_ptr_Function_mat3v3float
         %72 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_float4x4_bf44 = OpFunction %bool None %75
         %76 = OpFunctionParameter %_ptr_Function_mat4v4float
         %77 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_half_bh = OpFunction %bool None %48
         %78 = OpFunctionParameter %_ptr_Function_float
         %79 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_half2_bh2 = OpFunction %bool None %51
         %80 = OpFunctionParameter %_ptr_Function_v2float
         %81 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_half3_bh3 = OpFunction %bool None %56
         %82 = OpFunctionParameter %_ptr_Function_v3float
         %83 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_half4_bh4 = OpFunction %bool None %60
         %84 = OpFunctionParameter %_ptr_Function_v4float
         %85 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_half2x2_bh22 = OpFunction %bool None %65
         %86 = OpFunctionParameter %_ptr_Function_mat2v2float
         %87 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_half3x3_bh33 = OpFunction %bool None %70
         %88 = OpFunctionParameter %_ptr_Function_mat3v3float
         %89 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_half4x4_bh44 = OpFunction %bool None %75
         %90 = OpFunctionParameter %_ptr_Function_mat4v4float
         %91 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_bool_bb = OpFunction %bool None %93
         %94 = OpFunctionParameter %_ptr_Function_bool
         %95 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_bool2_bb2 = OpFunction %bool None %98
         %99 = OpFunctionParameter %_ptr_Function_v2bool
        %100 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_bool3_bb3 = OpFunction %bool None %103
        %104 = OpFunctionParameter %_ptr_Function_v3bool
        %105 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_bool4_bb4 = OpFunction %bool None %108
        %109 = OpFunctionParameter %_ptr_Function_v4bool
        %110 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_int_bi = OpFunction %bool None %113
        %114 = OpFunctionParameter %_ptr_Function_int
        %115 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_int2_bi2 = OpFunction %bool None %118
        %119 = OpFunctionParameter %_ptr_Function_v2int
        %120 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_int3_bi3 = OpFunction %bool None %123
        %124 = OpFunctionParameter %_ptr_Function_v3int
        %125 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_int4_bi4 = OpFunction %bool None %128
        %129 = OpFunctionParameter %_ptr_Function_v4int
        %130 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
       %main = OpFunction %v4float None %131
        %132 = OpFunctionParameter %_ptr_Function_v2float
        %133 = OpLabel
        %142 = OpVariable %_ptr_Function_float Function
        %149 = OpVariable %_ptr_Function_v2float Function
        %156 = OpVariable %_ptr_Function_v3float Function
        %163 = OpVariable %_ptr_Function_v4float Function
        %171 = OpVariable %_ptr_Function_mat2v2float Function
        %180 = OpVariable %_ptr_Function_mat3v3float Function
        %190 = OpVariable %_ptr_Function_mat4v4float Function
        %195 = OpVariable %_ptr_Function_float Function
        %200 = OpVariable %_ptr_Function_v2float Function
        %205 = OpVariable %_ptr_Function_v3float Function
        %210 = OpVariable %_ptr_Function_v4float Function
        %215 = OpVariable %_ptr_Function_mat2v2float Function
        %220 = OpVariable %_ptr_Function_mat3v3float Function
        %225 = OpVariable %_ptr_Function_mat4v4float Function
        %230 = OpVariable %_ptr_Function_bool Function
        %236 = OpVariable %_ptr_Function_v2bool Function
        %242 = OpVariable %_ptr_Function_v3bool Function
        %248 = OpVariable %_ptr_Function_v4bool Function
        %254 = OpVariable %_ptr_Function_int Function
        %261 = OpVariable %_ptr_Function_v2int Function
        %268 = OpVariable %_ptr_Function_v3int Function
        %275 = OpVariable %_ptr_Function_v4int Function
        %278 = OpVariable %_ptr_Function_v4float Function
               OpSelectionMerge %136 None
               OpBranchConditional %true %135 %136
        %135 = OpLabel
        %137 = OpFunctionCall %bool %takes_void_b
               OpBranch %136
        %136 = OpLabel
        %138 = OpPhi %bool %false %133 %137 %135
               OpSelectionMerge %140 None
               OpBranchConditional %138 %139 %140
        %139 = OpLabel
               OpStore %142 %float_1
        %143 = OpFunctionCall %bool %takes_float_bf %142
               OpBranch %140
        %140 = OpLabel
        %144 = OpPhi %bool %false %136 %143 %139
               OpSelectionMerge %146 None
               OpBranchConditional %144 %145 %146
        %145 = OpLabel
               OpStore %149 %148
        %150 = OpFunctionCall %bool %takes_float2_bf2 %149
               OpBranch %146
        %146 = OpLabel
        %151 = OpPhi %bool %false %140 %150 %145
               OpSelectionMerge %153 None
               OpBranchConditional %151 %152 %153
        %152 = OpLabel
               OpStore %156 %155
        %157 = OpFunctionCall %bool %takes_float3_bf3 %156
               OpBranch %153
        %153 = OpLabel
        %158 = OpPhi %bool %false %146 %157 %152
               OpSelectionMerge %160 None
               OpBranchConditional %158 %159 %160
        %159 = OpLabel
               OpStore %163 %162
        %164 = OpFunctionCall %bool %takes_float4_bf4 %163
               OpBranch %160
        %160 = OpLabel
        %165 = OpPhi %bool %false %153 %164 %159
               OpSelectionMerge %167 None
               OpBranchConditional %165 %166 %167
        %166 = OpLabel
               OpStore %171 %170
        %172 = OpFunctionCall %bool %takes_float2x2_bf22 %171
               OpBranch %167
        %167 = OpLabel
        %173 = OpPhi %bool %false %160 %172 %166
               OpSelectionMerge %175 None
               OpBranchConditional %173 %174 %175
        %174 = OpLabel
               OpStore %180 %179
        %181 = OpFunctionCall %bool %takes_float3x3_bf33 %180
               OpBranch %175
        %175 = OpLabel
        %182 = OpPhi %bool %false %167 %181 %174
               OpSelectionMerge %184 None
               OpBranchConditional %182 %183 %184
        %183 = OpLabel
               OpStore %190 %189
        %191 = OpFunctionCall %bool %takes_float4x4_bf44 %190
               OpBranch %184
        %184 = OpLabel
        %192 = OpPhi %bool %false %175 %191 %183
               OpSelectionMerge %194 None
               OpBranchConditional %192 %193 %194
        %193 = OpLabel
               OpStore %195 %float_1
        %196 = OpFunctionCall %bool %takes_half_bh %195
               OpBranch %194
        %194 = OpLabel
        %197 = OpPhi %bool %false %184 %196 %193
               OpSelectionMerge %199 None
               OpBranchConditional %197 %198 %199
        %198 = OpLabel
               OpStore %200 %148
        %201 = OpFunctionCall %bool %takes_half2_bh2 %200
               OpBranch %199
        %199 = OpLabel
        %202 = OpPhi %bool %false %194 %201 %198
               OpSelectionMerge %204 None
               OpBranchConditional %202 %203 %204
        %203 = OpLabel
               OpStore %205 %155
        %206 = OpFunctionCall %bool %takes_half3_bh3 %205
               OpBranch %204
        %204 = OpLabel
        %207 = OpPhi %bool %false %199 %206 %203
               OpSelectionMerge %209 None
               OpBranchConditional %207 %208 %209
        %208 = OpLabel
               OpStore %210 %162
        %211 = OpFunctionCall %bool %takes_half4_bh4 %210
               OpBranch %209
        %209 = OpLabel
        %212 = OpPhi %bool %false %204 %211 %208
               OpSelectionMerge %214 None
               OpBranchConditional %212 %213 %214
        %213 = OpLabel
               OpStore %215 %170
        %216 = OpFunctionCall %bool %takes_half2x2_bh22 %215
               OpBranch %214
        %214 = OpLabel
        %217 = OpPhi %bool %false %209 %216 %213
               OpSelectionMerge %219 None
               OpBranchConditional %217 %218 %219
        %218 = OpLabel
               OpStore %220 %179
        %221 = OpFunctionCall %bool %takes_half3x3_bh33 %220
               OpBranch %219
        %219 = OpLabel
        %222 = OpPhi %bool %false %214 %221 %218
               OpSelectionMerge %224 None
               OpBranchConditional %222 %223 %224
        %223 = OpLabel
               OpStore %225 %189
        %226 = OpFunctionCall %bool %takes_half4x4_bh44 %225
               OpBranch %224
        %224 = OpLabel
        %227 = OpPhi %bool %false %219 %226 %223
               OpSelectionMerge %229 None
               OpBranchConditional %227 %228 %229
        %228 = OpLabel
               OpStore %230 %true
        %231 = OpFunctionCall %bool %takes_bool_bb %230
               OpBranch %229
        %229 = OpLabel
        %232 = OpPhi %bool %false %224 %231 %228
               OpSelectionMerge %234 None
               OpBranchConditional %232 %233 %234
        %233 = OpLabel
               OpStore %236 %235
        %237 = OpFunctionCall %bool %takes_bool2_bb2 %236
               OpBranch %234
        %234 = OpLabel
        %238 = OpPhi %bool %false %229 %237 %233
               OpSelectionMerge %240 None
               OpBranchConditional %238 %239 %240
        %239 = OpLabel
               OpStore %242 %241
        %243 = OpFunctionCall %bool %takes_bool3_bb3 %242
               OpBranch %240
        %240 = OpLabel
        %244 = OpPhi %bool %false %234 %243 %239
               OpSelectionMerge %246 None
               OpBranchConditional %244 %245 %246
        %245 = OpLabel
               OpStore %248 %247
        %249 = OpFunctionCall %bool %takes_bool4_bb4 %248
               OpBranch %246
        %246 = OpLabel
        %250 = OpPhi %bool %false %240 %249 %245
               OpSelectionMerge %252 None
               OpBranchConditional %250 %251 %252
        %251 = OpLabel
               OpStore %254 %int_1
        %255 = OpFunctionCall %bool %takes_int_bi %254
               OpBranch %252
        %252 = OpLabel
        %256 = OpPhi %bool %false %246 %255 %251
               OpSelectionMerge %258 None
               OpBranchConditional %256 %257 %258
        %257 = OpLabel
               OpStore %261 %260
        %262 = OpFunctionCall %bool %takes_int2_bi2 %261
               OpBranch %258
        %258 = OpLabel
        %263 = OpPhi %bool %false %252 %262 %257
               OpSelectionMerge %265 None
               OpBranchConditional %263 %264 %265
        %264 = OpLabel
               OpStore %268 %267
        %269 = OpFunctionCall %bool %takes_int3_bi3 %268
               OpBranch %265
        %265 = OpLabel
        %270 = OpPhi %bool %false %258 %269 %264
               OpSelectionMerge %272 None
               OpBranchConditional %270 %271 %272
        %271 = OpLabel
               OpStore %275 %274
        %276 = OpFunctionCall %bool %takes_int4_bi4 %275
               OpBranch %272
        %272 = OpLabel
        %277 = OpPhi %bool %false %265 %276 %271
               OpSelectionMerge %281 None
               OpBranchConditional %277 %279 %280
        %279 = OpLabel
        %282 = OpAccessChain %_ptr_Uniform_v4float %30 %int_0
        %285 = OpLoad %v4float %282
               OpStore %278 %285
               OpBranch %281
        %280 = OpLabel
        %286 = OpAccessChain %_ptr_Uniform_v4float %30 %int_1
        %287 = OpLoad %v4float %286
               OpStore %278 %287
               OpBranch %281
        %281 = OpLabel
        %288 = OpLoad %v4float %278
               OpReturnValue %288
               OpFunctionEnd
