               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %30
               OpName %_UniformBuffer "_UniformBuffer"  ; id %35
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %37
               OpName %takes_void_b "takes_void_b"      ; id %6
               OpName %takes_float_bf "takes_float_bf"  ; id %7
               OpName %takes_float2_bf2 "takes_float2_bf2"  ; id %8
               OpName %takes_float3_bf3 "takes_float3_bf3"  ; id %9
               OpName %takes_float4_bf4 "takes_float4_bf4"  ; id %10
               OpName %takes_float2x2_bf22 "takes_float2x2_bf22"    ; id %11
               OpName %takes_float3x3_bf33 "takes_float3x3_bf33"    ; id %12
               OpName %takes_float4x4_bf44 "takes_float4x4_bf44"    ; id %13
               OpName %takes_half_bh "takes_half_bh"                ; id %14
               OpName %takes_half2_bh2 "takes_half2_bh2"            ; id %15
               OpName %takes_half3_bh3 "takes_half3_bh3"            ; id %16
               OpName %takes_half4_bh4 "takes_half4_bh4"            ; id %17
               OpName %takes_half2x2_bh22 "takes_half2x2_bh22"      ; id %18
               OpName %takes_half3x3_bh33 "takes_half3x3_bh33"      ; id %19
               OpName %takes_half4x4_bh44 "takes_half4x4_bh44"      ; id %20
               OpName %takes_bool_bb "takes_bool_bb"                ; id %21
               OpName %takes_bool2_bb2 "takes_bool2_bb2"            ; id %22
               OpName %takes_bool3_bb3 "takes_bool3_bb3"            ; id %23
               OpName %takes_bool4_bb4 "takes_bool4_bb4"            ; id %24
               OpName %takes_int_bi "takes_int_bi"                  ; id %25
               OpName %takes_int2_bi2 "takes_int2_bi2"              ; id %26
               OpName %takes_int3_bi3 "takes_int3_bi3"              ; id %27
               OpName %takes_int4_bi4 "takes_int4_bi4"              ; id %28
               OpName %main "main"                                  ; id %29

               ; Annotations
               OpDecorate %main RelaxedPrecision
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
               OpDecorate %82 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %288 RelaxedPrecision
               OpDecorate %290 RelaxedPrecision
               OpDecorate %291 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %34 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %39 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %43 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %48 = OpTypeFunction %bool
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
%_ptr_Function_int = OpTypePointer Function %int
        %116 = OpTypeFunction %bool %_ptr_Function_int
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
        %121 = OpTypeFunction %bool %_ptr_Function_v2int
      %v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
        %126 = OpTypeFunction %bool %_ptr_Function_v3int
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
        %131 = OpTypeFunction %bool %_ptr_Function_v4int
        %134 = OpTypeFunction %v4float %_ptr_Function_v2float
      %false = OpConstantFalse %bool
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
        %151 = OpConstantComposite %v2float %float_2 %float_2
    %float_3 = OpConstant %float 3
        %158 = OpConstantComposite %v3float %float_3 %float_3 %float_3
    %float_4 = OpConstant %float 4
        %165 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
        %171 = OpConstantComposite %v2float %float_2 %float_0
        %172 = OpConstantComposite %v2float %float_0 %float_2
        %173 = OpConstantComposite %mat2v2float %171 %172
        %179 = OpConstantComposite %v3float %float_3 %float_0 %float_0
        %180 = OpConstantComposite %v3float %float_0 %float_3 %float_0
        %181 = OpConstantComposite %v3float %float_0 %float_0 %float_3
        %182 = OpConstantComposite %mat3v3float %179 %180 %181
        %188 = OpConstantComposite %v4float %float_4 %float_0 %float_0 %float_0
        %189 = OpConstantComposite %v4float %float_0 %float_4 %float_0 %float_0
        %190 = OpConstantComposite %v4float %float_0 %float_0 %float_4 %float_0
        %191 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_4
        %192 = OpConstantComposite %mat4v4float %188 %189 %190 %191
        %238 = OpConstantComposite %v2bool %true %true
        %244 = OpConstantComposite %v3bool %true %true %true
        %250 = OpConstantComposite %v4bool %true %true %true %true
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
        %263 = OpConstantComposite %v2int %int_2 %int_2
      %int_3 = OpConstant %int 3
        %270 = OpConstantComposite %v3int %int_3 %int_3 %int_3
      %int_4 = OpConstant %int 4
        %277 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %39

         %40 = OpLabel
         %44 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %44 %43
         %46 =   OpFunctionCall %v4float %main %44
                 OpStore %sk_FragColor %46
                 OpReturn
               OpFunctionEnd


               ; Function takes_void_b
%takes_void_b = OpFunction %bool None %48

         %49 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_float_bf
%takes_float_bf = OpFunction %bool None %52
         %53 = OpFunctionParameter %_ptr_Function_float

         %54 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_float2_bf2
%takes_float2_bf2 = OpFunction %bool None %55
         %56 = OpFunctionParameter %_ptr_Function_v2float

         %57 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_float3_bf3
%takes_float3_bf3 = OpFunction %bool None %60
         %61 = OpFunctionParameter %_ptr_Function_v3float

         %62 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_float4_bf4
%takes_float4_bf4 = OpFunction %bool None %64
         %65 = OpFunctionParameter %_ptr_Function_v4float

         %66 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_float2x2_bf22
%takes_float2x2_bf22 = OpFunction %bool None %69
         %70 = OpFunctionParameter %_ptr_Function_mat2v2float

         %71 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_float3x3_bf33
%takes_float3x3_bf33 = OpFunction %bool None %74
         %75 = OpFunctionParameter %_ptr_Function_mat3v3float

         %76 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_float4x4_bf44
%takes_float4x4_bf44 = OpFunction %bool None %79
         %80 = OpFunctionParameter %_ptr_Function_mat4v4float

         %81 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_half_bh
%takes_half_bh = OpFunction %bool None %52
         %82 = OpFunctionParameter %_ptr_Function_float     ; RelaxedPrecision

         %83 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_half2_bh2
%takes_half2_bh2 = OpFunction %bool None %55
         %84 = OpFunctionParameter %_ptr_Function_v2float   ; RelaxedPrecision

         %85 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_half3_bh3
%takes_half3_bh3 = OpFunction %bool None %60
         %86 = OpFunctionParameter %_ptr_Function_v3float   ; RelaxedPrecision

         %87 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_half4_bh4
%takes_half4_bh4 = OpFunction %bool None %64
         %88 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %89 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_half2x2_bh22
%takes_half2x2_bh22 = OpFunction %bool None %69
         %90 = OpFunctionParameter %_ptr_Function_mat2v2float   ; RelaxedPrecision

         %91 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_half3x3_bh33
%takes_half3x3_bh33 = OpFunction %bool None %74
         %92 = OpFunctionParameter %_ptr_Function_mat3v3float   ; RelaxedPrecision

         %93 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_half4x4_bh44
%takes_half4x4_bh44 = OpFunction %bool None %79
         %94 = OpFunctionParameter %_ptr_Function_mat4v4float   ; RelaxedPrecision

         %95 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_bool_bb
%takes_bool_bb = OpFunction %bool None %97
         %98 = OpFunctionParameter %_ptr_Function_bool

         %99 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_bool2_bb2
%takes_bool2_bb2 = OpFunction %bool None %102
        %103 = OpFunctionParameter %_ptr_Function_v2bool

        %104 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_bool3_bb3
%takes_bool3_bb3 = OpFunction %bool None %107
        %108 = OpFunctionParameter %_ptr_Function_v3bool

        %109 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_bool4_bb4
%takes_bool4_bb4 = OpFunction %bool None %112
        %113 = OpFunctionParameter %_ptr_Function_v4bool

        %114 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_int_bi
%takes_int_bi = OpFunction %bool None %116
        %117 = OpFunctionParameter %_ptr_Function_int

        %118 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_int2_bi2
%takes_int2_bi2 = OpFunction %bool None %121
        %122 = OpFunctionParameter %_ptr_Function_v2int

        %123 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_int3_bi3
%takes_int3_bi3 = OpFunction %bool None %126
        %127 = OpFunctionParameter %_ptr_Function_v3int

        %128 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function takes_int4_bi4
%takes_int4_bi4 = OpFunction %bool None %131
        %132 = OpFunctionParameter %_ptr_Function_v4int

        %133 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %134        ; RelaxedPrecision
        %135 = OpFunctionParameter %_ptr_Function_v2float

        %136 = OpLabel
        %145 =   OpVariable %_ptr_Function_float Function
        %152 =   OpVariable %_ptr_Function_v2float Function
        %159 =   OpVariable %_ptr_Function_v3float Function
        %166 =   OpVariable %_ptr_Function_v4float Function
        %174 =   OpVariable %_ptr_Function_mat2v2float Function
        %183 =   OpVariable %_ptr_Function_mat3v3float Function
        %193 =   OpVariable %_ptr_Function_mat4v4float Function
        %198 =   OpVariable %_ptr_Function_float Function
        %203 =   OpVariable %_ptr_Function_v2float Function
        %208 =   OpVariable %_ptr_Function_v3float Function
        %213 =   OpVariable %_ptr_Function_v4float Function
        %218 =   OpVariable %_ptr_Function_mat2v2float Function
        %223 =   OpVariable %_ptr_Function_mat3v3float Function
        %228 =   OpVariable %_ptr_Function_mat4v4float Function
        %233 =   OpVariable %_ptr_Function_bool Function
        %239 =   OpVariable %_ptr_Function_v2bool Function
        %245 =   OpVariable %_ptr_Function_v3bool Function
        %251 =   OpVariable %_ptr_Function_v4bool Function
        %257 =   OpVariable %_ptr_Function_int Function
        %264 =   OpVariable %_ptr_Function_v2int Function
        %271 =   OpVariable %_ptr_Function_v3int Function
        %278 =   OpVariable %_ptr_Function_v4int Function
        %281 =   OpVariable %_ptr_Function_v4float Function
                 OpSelectionMerge %139 None
                 OpBranchConditional %true %138 %139

        %138 =     OpLabel
        %140 =       OpFunctionCall %bool %takes_void_b
                     OpBranch %139

        %139 = OpLabel
        %141 =   OpPhi %bool %false %136 %140 %138
                 OpSelectionMerge %143 None
                 OpBranchConditional %141 %142 %143

        %142 =     OpLabel
                     OpStore %145 %float_1
        %146 =       OpFunctionCall %bool %takes_float_bf %145
                     OpBranch %143

        %143 = OpLabel
        %147 =   OpPhi %bool %false %139 %146 %142
                 OpSelectionMerge %149 None
                 OpBranchConditional %147 %148 %149

        %148 =     OpLabel
                     OpStore %152 %151
        %153 =       OpFunctionCall %bool %takes_float2_bf2 %152
                     OpBranch %149

        %149 = OpLabel
        %154 =   OpPhi %bool %false %143 %153 %148
                 OpSelectionMerge %156 None
                 OpBranchConditional %154 %155 %156

        %155 =     OpLabel
                     OpStore %159 %158
        %160 =       OpFunctionCall %bool %takes_float3_bf3 %159
                     OpBranch %156

        %156 = OpLabel
        %161 =   OpPhi %bool %false %149 %160 %155
                 OpSelectionMerge %163 None
                 OpBranchConditional %161 %162 %163

        %162 =     OpLabel
                     OpStore %166 %165
        %167 =       OpFunctionCall %bool %takes_float4_bf4 %166
                     OpBranch %163

        %163 = OpLabel
        %168 =   OpPhi %bool %false %156 %167 %162
                 OpSelectionMerge %170 None
                 OpBranchConditional %168 %169 %170

        %169 =     OpLabel
                     OpStore %174 %173
        %175 =       OpFunctionCall %bool %takes_float2x2_bf22 %174
                     OpBranch %170

        %170 = OpLabel
        %176 =   OpPhi %bool %false %163 %175 %169
                 OpSelectionMerge %178 None
                 OpBranchConditional %176 %177 %178

        %177 =     OpLabel
                     OpStore %183 %182
        %184 =       OpFunctionCall %bool %takes_float3x3_bf33 %183
                     OpBranch %178

        %178 = OpLabel
        %185 =   OpPhi %bool %false %170 %184 %177
                 OpSelectionMerge %187 None
                 OpBranchConditional %185 %186 %187

        %186 =     OpLabel
                     OpStore %193 %192
        %194 =       OpFunctionCall %bool %takes_float4x4_bf44 %193
                     OpBranch %187

        %187 = OpLabel
        %195 =   OpPhi %bool %false %178 %194 %186
                 OpSelectionMerge %197 None
                 OpBranchConditional %195 %196 %197

        %196 =     OpLabel
                     OpStore %198 %float_1
        %199 =       OpFunctionCall %bool %takes_half_bh %198
                     OpBranch %197

        %197 = OpLabel
        %200 =   OpPhi %bool %false %187 %199 %196
                 OpSelectionMerge %202 None
                 OpBranchConditional %200 %201 %202

        %201 =     OpLabel
                     OpStore %203 %151
        %204 =       OpFunctionCall %bool %takes_half2_bh2 %203
                     OpBranch %202

        %202 = OpLabel
        %205 =   OpPhi %bool %false %197 %204 %201
                 OpSelectionMerge %207 None
                 OpBranchConditional %205 %206 %207

        %206 =     OpLabel
                     OpStore %208 %158
        %209 =       OpFunctionCall %bool %takes_half3_bh3 %208
                     OpBranch %207

        %207 = OpLabel
        %210 =   OpPhi %bool %false %202 %209 %206
                 OpSelectionMerge %212 None
                 OpBranchConditional %210 %211 %212

        %211 =     OpLabel
                     OpStore %213 %165
        %214 =       OpFunctionCall %bool %takes_half4_bh4 %213
                     OpBranch %212

        %212 = OpLabel
        %215 =   OpPhi %bool %false %207 %214 %211
                 OpSelectionMerge %217 None
                 OpBranchConditional %215 %216 %217

        %216 =     OpLabel
                     OpStore %218 %173
        %219 =       OpFunctionCall %bool %takes_half2x2_bh22 %218
                     OpBranch %217

        %217 = OpLabel
        %220 =   OpPhi %bool %false %212 %219 %216
                 OpSelectionMerge %222 None
                 OpBranchConditional %220 %221 %222

        %221 =     OpLabel
                     OpStore %223 %182
        %224 =       OpFunctionCall %bool %takes_half3x3_bh33 %223
                     OpBranch %222

        %222 = OpLabel
        %225 =   OpPhi %bool %false %217 %224 %221
                 OpSelectionMerge %227 None
                 OpBranchConditional %225 %226 %227

        %226 =     OpLabel
                     OpStore %228 %192
        %229 =       OpFunctionCall %bool %takes_half4x4_bh44 %228
                     OpBranch %227

        %227 = OpLabel
        %230 =   OpPhi %bool %false %222 %229 %226
                 OpSelectionMerge %232 None
                 OpBranchConditional %230 %231 %232

        %231 =     OpLabel
                     OpStore %233 %true
        %234 =       OpFunctionCall %bool %takes_bool_bb %233
                     OpBranch %232

        %232 = OpLabel
        %235 =   OpPhi %bool %false %227 %234 %231
                 OpSelectionMerge %237 None
                 OpBranchConditional %235 %236 %237

        %236 =     OpLabel
                     OpStore %239 %238
        %240 =       OpFunctionCall %bool %takes_bool2_bb2 %239
                     OpBranch %237

        %237 = OpLabel
        %241 =   OpPhi %bool %false %232 %240 %236
                 OpSelectionMerge %243 None
                 OpBranchConditional %241 %242 %243

        %242 =     OpLabel
                     OpStore %245 %244
        %246 =       OpFunctionCall %bool %takes_bool3_bb3 %245
                     OpBranch %243

        %243 = OpLabel
        %247 =   OpPhi %bool %false %237 %246 %242
                 OpSelectionMerge %249 None
                 OpBranchConditional %247 %248 %249

        %248 =     OpLabel
                     OpStore %251 %250
        %252 =       OpFunctionCall %bool %takes_bool4_bb4 %251
                     OpBranch %249

        %249 = OpLabel
        %253 =   OpPhi %bool %false %243 %252 %248
                 OpSelectionMerge %255 None
                 OpBranchConditional %253 %254 %255

        %254 =     OpLabel
                     OpStore %257 %int_1
        %258 =       OpFunctionCall %bool %takes_int_bi %257
                     OpBranch %255

        %255 = OpLabel
        %259 =   OpPhi %bool %false %249 %258 %254
                 OpSelectionMerge %261 None
                 OpBranchConditional %259 %260 %261

        %260 =     OpLabel
                     OpStore %264 %263
        %265 =       OpFunctionCall %bool %takes_int2_bi2 %264
                     OpBranch %261

        %261 = OpLabel
        %266 =   OpPhi %bool %false %255 %265 %260
                 OpSelectionMerge %268 None
                 OpBranchConditional %266 %267 %268

        %267 =     OpLabel
                     OpStore %271 %270
        %272 =       OpFunctionCall %bool %takes_int3_bi3 %271
                     OpBranch %268

        %268 = OpLabel
        %273 =   OpPhi %bool %false %261 %272 %267
                 OpSelectionMerge %275 None
                 OpBranchConditional %273 %274 %275

        %274 =     OpLabel
                     OpStore %278 %277
        %279 =       OpFunctionCall %bool %takes_int4_bi4 %278
                     OpBranch %275

        %275 = OpLabel
        %280 =   OpPhi %bool %false %268 %279 %274
                 OpSelectionMerge %284 None
                 OpBranchConditional %280 %282 %283

        %282 =     OpLabel
        %285 =       OpAccessChain %_ptr_Uniform_v4float %34 %int_0
        %288 =       OpLoad %v4float %285           ; RelaxedPrecision
                     OpStore %281 %288
                     OpBranch %284

        %283 =     OpLabel
        %289 =       OpAccessChain %_ptr_Uniform_v4float %34 %int_1
        %290 =       OpLoad %v4float %289           ; RelaxedPrecision
                     OpStore %281 %290
                     OpBranch %284

        %284 = OpLabel
        %291 =   OpLoad %v4float %281               ; RelaxedPrecision
                 OpReturnValue %291
               OpFunctionEnd
