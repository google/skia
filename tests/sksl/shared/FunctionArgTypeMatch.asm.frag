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
               OpDecorate %33 Binding 0
               OpDecorate %33 DescriptorSet 0
               OpDecorate %287 RelaxedPrecision
               OpDecorate %289 RelaxedPrecision
               OpDecorate %290 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
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
         %50 = OpTypeFunction %bool %_ptr_Function_float
         %53 = OpTypeFunction %bool %_ptr_Function_v2float
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %58 = OpTypeFunction %bool %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %62 = OpTypeFunction %bool %_ptr_Function_v4float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
         %67 = OpTypeFunction %bool %_ptr_Function_mat2v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
         %72 = OpTypeFunction %bool %_ptr_Function_mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
         %77 = OpTypeFunction %bool %_ptr_Function_mat4v4float
%_ptr_Function_bool = OpTypePointer Function %bool
         %95 = OpTypeFunction %bool %_ptr_Function_bool
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
        %100 = OpTypeFunction %bool %_ptr_Function_v2bool
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
        %105 = OpTypeFunction %bool %_ptr_Function_v3bool
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
        %110 = OpTypeFunction %bool %_ptr_Function_v4bool
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
        %115 = OpTypeFunction %bool %_ptr_Function_int
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
        %120 = OpTypeFunction %bool %_ptr_Function_v2int
      %v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
        %125 = OpTypeFunction %bool %_ptr_Function_v3int
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
        %130 = OpTypeFunction %bool %_ptr_Function_v4int
        %133 = OpTypeFunction %v4float %_ptr_Function_v2float
      %false = OpConstantFalse %bool
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
        %150 = OpConstantComposite %v2float %float_2 %float_2
    %float_3 = OpConstant %float 3
        %157 = OpConstantComposite %v3float %float_3 %float_3 %float_3
    %float_4 = OpConstant %float 4
        %164 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
        %170 = OpConstantComposite %v2float %float_2 %float_0
        %171 = OpConstantComposite %v2float %float_0 %float_2
        %172 = OpConstantComposite %mat2v2float %170 %171
        %178 = OpConstantComposite %v3float %float_3 %float_0 %float_0
        %179 = OpConstantComposite %v3float %float_0 %float_3 %float_0
        %180 = OpConstantComposite %v3float %float_0 %float_0 %float_3
        %181 = OpConstantComposite %mat3v3float %178 %179 %180
        %187 = OpConstantComposite %v4float %float_4 %float_0 %float_0 %float_0
        %188 = OpConstantComposite %v4float %float_0 %float_4 %float_0 %float_0
        %189 = OpConstantComposite %v4float %float_0 %float_0 %float_4 %float_0
        %190 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_4
        %191 = OpConstantComposite %mat4v4float %187 %188 %189 %190
        %237 = OpConstantComposite %v2bool %true %true
        %243 = OpConstantComposite %v3bool %true %true %true
        %249 = OpConstantComposite %v4bool %true %true %true %true
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
        %262 = OpConstantComposite %v2int %int_2 %int_2
      %int_3 = OpConstant %int 3
        %269 = OpConstantComposite %v3int %int_3 %int_3 %int_3
      %int_4 = OpConstant %int 4
        %276 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
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
%takes_float_bf = OpFunction %bool None %50
         %51 = OpFunctionParameter %_ptr_Function_float
         %52 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_float2_bf2 = OpFunction %bool None %53
         %54 = OpFunctionParameter %_ptr_Function_v2float
         %55 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_float3_bf3 = OpFunction %bool None %58
         %59 = OpFunctionParameter %_ptr_Function_v3float
         %60 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_float4_bf4 = OpFunction %bool None %62
         %63 = OpFunctionParameter %_ptr_Function_v4float
         %64 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_float2x2_bf22 = OpFunction %bool None %67
         %68 = OpFunctionParameter %_ptr_Function_mat2v2float
         %69 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_float3x3_bf33 = OpFunction %bool None %72
         %73 = OpFunctionParameter %_ptr_Function_mat3v3float
         %74 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_float4x4_bf44 = OpFunction %bool None %77
         %78 = OpFunctionParameter %_ptr_Function_mat4v4float
         %79 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_half_bh = OpFunction %bool None %50
         %80 = OpFunctionParameter %_ptr_Function_float
         %81 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_half2_bh2 = OpFunction %bool None %53
         %82 = OpFunctionParameter %_ptr_Function_v2float
         %83 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_half3_bh3 = OpFunction %bool None %58
         %84 = OpFunctionParameter %_ptr_Function_v3float
         %85 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_half4_bh4 = OpFunction %bool None %62
         %86 = OpFunctionParameter %_ptr_Function_v4float
         %87 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_half2x2_bh22 = OpFunction %bool None %67
         %88 = OpFunctionParameter %_ptr_Function_mat2v2float
         %89 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_half3x3_bh33 = OpFunction %bool None %72
         %90 = OpFunctionParameter %_ptr_Function_mat3v3float
         %91 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_half4x4_bh44 = OpFunction %bool None %77
         %92 = OpFunctionParameter %_ptr_Function_mat4v4float
         %93 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_bool_bb = OpFunction %bool None %95
         %96 = OpFunctionParameter %_ptr_Function_bool
         %97 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_bool2_bb2 = OpFunction %bool None %100
        %101 = OpFunctionParameter %_ptr_Function_v2bool
        %102 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_bool3_bb3 = OpFunction %bool None %105
        %106 = OpFunctionParameter %_ptr_Function_v3bool
        %107 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_bool4_bb4 = OpFunction %bool None %110
        %111 = OpFunctionParameter %_ptr_Function_v4bool
        %112 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_int_bi = OpFunction %bool None %115
        %116 = OpFunctionParameter %_ptr_Function_int
        %117 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_int2_bi2 = OpFunction %bool None %120
        %121 = OpFunctionParameter %_ptr_Function_v2int
        %122 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_int3_bi3 = OpFunction %bool None %125
        %126 = OpFunctionParameter %_ptr_Function_v3int
        %127 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%takes_int4_bi4 = OpFunction %bool None %130
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
        %217 = OpVariable %_ptr_Function_mat2v2float Function
        %222 = OpVariable %_ptr_Function_mat3v3float Function
        %227 = OpVariable %_ptr_Function_mat4v4float Function
        %232 = OpVariable %_ptr_Function_bool Function
        %238 = OpVariable %_ptr_Function_v2bool Function
        %244 = OpVariable %_ptr_Function_v3bool Function
        %250 = OpVariable %_ptr_Function_v4bool Function
        %256 = OpVariable %_ptr_Function_int Function
        %263 = OpVariable %_ptr_Function_v2int Function
        %270 = OpVariable %_ptr_Function_v3int Function
        %277 = OpVariable %_ptr_Function_v4int Function
        %280 = OpVariable %_ptr_Function_v4float Function
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
               OpStore %173 %172
        %174 = OpFunctionCall %bool %takes_float2x2_bf22 %173
               OpBranch %169
        %169 = OpLabel
        %175 = OpPhi %bool %false %162 %174 %168
               OpSelectionMerge %177 None
               OpBranchConditional %175 %176 %177
        %176 = OpLabel
               OpStore %182 %181
        %183 = OpFunctionCall %bool %takes_float3x3_bf33 %182
               OpBranch %177
        %177 = OpLabel
        %184 = OpPhi %bool %false %169 %183 %176
               OpSelectionMerge %186 None
               OpBranchConditional %184 %185 %186
        %185 = OpLabel
               OpStore %192 %191
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
               OpStore %217 %172
        %218 = OpFunctionCall %bool %takes_half2x2_bh22 %217
               OpBranch %216
        %216 = OpLabel
        %219 = OpPhi %bool %false %211 %218 %215
               OpSelectionMerge %221 None
               OpBranchConditional %219 %220 %221
        %220 = OpLabel
               OpStore %222 %181
        %223 = OpFunctionCall %bool %takes_half3x3_bh33 %222
               OpBranch %221
        %221 = OpLabel
        %224 = OpPhi %bool %false %216 %223 %220
               OpSelectionMerge %226 None
               OpBranchConditional %224 %225 %226
        %225 = OpLabel
               OpStore %227 %191
        %228 = OpFunctionCall %bool %takes_half4x4_bh44 %227
               OpBranch %226
        %226 = OpLabel
        %229 = OpPhi %bool %false %221 %228 %225
               OpSelectionMerge %231 None
               OpBranchConditional %229 %230 %231
        %230 = OpLabel
               OpStore %232 %true
        %233 = OpFunctionCall %bool %takes_bool_bb %232
               OpBranch %231
        %231 = OpLabel
        %234 = OpPhi %bool %false %226 %233 %230
               OpSelectionMerge %236 None
               OpBranchConditional %234 %235 %236
        %235 = OpLabel
               OpStore %238 %237
        %239 = OpFunctionCall %bool %takes_bool2_bb2 %238
               OpBranch %236
        %236 = OpLabel
        %240 = OpPhi %bool %false %231 %239 %235
               OpSelectionMerge %242 None
               OpBranchConditional %240 %241 %242
        %241 = OpLabel
               OpStore %244 %243
        %245 = OpFunctionCall %bool %takes_bool3_bb3 %244
               OpBranch %242
        %242 = OpLabel
        %246 = OpPhi %bool %false %236 %245 %241
               OpSelectionMerge %248 None
               OpBranchConditional %246 %247 %248
        %247 = OpLabel
               OpStore %250 %249
        %251 = OpFunctionCall %bool %takes_bool4_bb4 %250
               OpBranch %248
        %248 = OpLabel
        %252 = OpPhi %bool %false %242 %251 %247
               OpSelectionMerge %254 None
               OpBranchConditional %252 %253 %254
        %253 = OpLabel
               OpStore %256 %int_1
        %257 = OpFunctionCall %bool %takes_int_bi %256
               OpBranch %254
        %254 = OpLabel
        %258 = OpPhi %bool %false %248 %257 %253
               OpSelectionMerge %260 None
               OpBranchConditional %258 %259 %260
        %259 = OpLabel
               OpStore %263 %262
        %264 = OpFunctionCall %bool %takes_int2_bi2 %263
               OpBranch %260
        %260 = OpLabel
        %265 = OpPhi %bool %false %254 %264 %259
               OpSelectionMerge %267 None
               OpBranchConditional %265 %266 %267
        %266 = OpLabel
               OpStore %270 %269
        %271 = OpFunctionCall %bool %takes_int3_bi3 %270
               OpBranch %267
        %267 = OpLabel
        %272 = OpPhi %bool %false %260 %271 %266
               OpSelectionMerge %274 None
               OpBranchConditional %272 %273 %274
        %273 = OpLabel
               OpStore %277 %276
        %278 = OpFunctionCall %bool %takes_int4_bi4 %277
               OpBranch %274
        %274 = OpLabel
        %279 = OpPhi %bool %false %267 %278 %273
               OpSelectionMerge %283 None
               OpBranchConditional %279 %281 %282
        %281 = OpLabel
        %284 = OpAccessChain %_ptr_Uniform_v4float %33 %int_0
        %287 = OpLoad %v4float %284
               OpStore %280 %287
               OpBranch %283
        %282 = OpLabel
        %288 = OpAccessChain %_ptr_Uniform_v4float %33 %int_1
        %289 = OpLoad %v4float %288
               OpStore %280 %289
               OpBranch %283
        %283 = OpLabel
        %290 = OpLoad %v4float %280
               OpReturnValue %290
               OpFunctionEnd
