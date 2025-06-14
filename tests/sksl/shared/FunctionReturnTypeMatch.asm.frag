               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %28
               OpName %_UniformBuffer "_UniformBuffer"  ; id %33
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %35
               OpName %returns_float2_f2 "returns_float2_f2"    ; id %6
               OpName %returns_float3_f3 "returns_float3_f3"    ; id %7
               OpName %returns_float4_f4 "returns_float4_f4"    ; id %8
               OpName %returns_float2x2_f22 "returns_float2x2_f22"  ; id %9
               OpName %returns_float3x3_f33 "returns_float3x3_f33"  ; id %10
               OpName %returns_float4x4_f44 "returns_float4x4_f44"  ; id %11
               OpName %returns_half_h "returns_half_h"              ; id %12
               OpName %returns_half2_h2 "returns_half2_h2"          ; id %13
               OpName %returns_half3_h3 "returns_half3_h3"          ; id %14
               OpName %returns_half4_h4 "returns_half4_h4"          ; id %15
               OpName %returns_half2x2_h22 "returns_half2x2_h22"    ; id %16
               OpName %returns_half3x3_h33 "returns_half3x3_h33"    ; id %17
               OpName %returns_half4x4_h44 "returns_half4x4_h44"    ; id %18
               OpName %returns_bool_b "returns_bool_b"              ; id %19
               OpName %returns_bool2_b2 "returns_bool2_b2"          ; id %20
               OpName %returns_bool3_b3 "returns_bool3_b3"          ; id %21
               OpName %returns_bool4_b4 "returns_bool4_b4"          ; id %22
               OpName %returns_int_i "returns_int_i"                ; id %23
               OpName %returns_int2_i2 "returns_int2_i2"            ; id %24
               OpName %returns_int3_i3 "returns_int3_i3"            ; id %25
               OpName %returns_int4_i4 "returns_int4_i4"            ; id %26
               OpName %main "main"                                  ; id %27
               OpName %x1 "x1"                                      ; id %125
               OpName %x2 "x2"                                      ; id %127
               OpName %x3 "x3"                                      ; id %128
               OpName %x4 "x4"                                      ; id %130
               OpName %x5 "x5"                                      ; id %132
               OpName %x6 "x6"                                      ; id %134
               OpName %x7 "x7"                                      ; id %136
               OpName %x8 "x8"                                      ; id %138
               OpName %x9 "x9"                                      ; id %139
               OpName %x10 "x10"                                    ; id %140
               OpName %x11 "x11"                                    ; id %141
               OpName %x12 "x12"                                    ; id %142
               OpName %x13 "x13"                                    ; id %143
               OpName %x14 "x14"                                    ; id %144
               OpName %x15 "x15"                                    ; id %145
               OpName %x16 "x16"                                    ; id %147
               OpName %x17 "x17"                                    ; id %149
               OpName %x18 "x18"                                    ; id %151
               OpName %x19 "x19"                                    ; id %153
               OpName %x20 "x20"                                    ; id %155
               OpName %x21 "x21"                                    ; id %157
               OpName %x22 "x22"                                    ; id %159

               ; Annotations
               OpDecorate %returns_half_h RelaxedPrecision
               OpDecorate %returns_half2_h2 RelaxedPrecision
               OpDecorate %returns_half3_h3 RelaxedPrecision
               OpDecorate %returns_half4_h4 RelaxedPrecision
               OpDecorate %returns_half2x2_h22 RelaxedPrecision
               OpDecorate %returns_half3x3_h33 RelaxedPrecision
               OpDecorate %returns_half4x4_h44 RelaxedPrecision
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %32 Binding 0
               OpDecorate %32 DescriptorSet 0
               OpDecorate %x8 RelaxedPrecision
               OpDecorate %x9 RelaxedPrecision
               OpDecorate %x10 RelaxedPrecision
               OpDecorate %x11 RelaxedPrecision
               OpDecorate %x12 RelaxedPrecision
               OpDecorate %x13 RelaxedPrecision
               OpDecorate %x14 RelaxedPrecision
               OpDecorate %251 RelaxedPrecision
               OpDecorate %252 RelaxedPrecision
               OpDecorate %254 RelaxedPrecision
               OpDecorate %255 RelaxedPrecision
               OpDecorate %262 RelaxedPrecision
               OpDecorate %263 RelaxedPrecision
               OpDecorate %265 RelaxedPrecision
               OpDecorate %266 RelaxedPrecision
               OpDecorate %269 RelaxedPrecision
               OpDecorate %270 RelaxedPrecision
               OpDecorate %277 RelaxedPrecision
               OpDecorate %278 RelaxedPrecision
               OpDecorate %280 RelaxedPrecision
               OpDecorate %281 RelaxedPrecision
               OpDecorate %284 RelaxedPrecision
               OpDecorate %285 RelaxedPrecision
               OpDecorate %288 RelaxedPrecision
               OpDecorate %289 RelaxedPrecision
               OpDecorate %346 RelaxedPrecision
               OpDecorate %348 RelaxedPrecision
               OpDecorate %349 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %32 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %37 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %41 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %45 = OpTypeFunction %v2float
    %float_2 = OpConstant %float 2
         %48 = OpConstantComposite %v2float %float_2 %float_2
    %v3float = OpTypeVector %float 3
         %50 = OpTypeFunction %v3float
    %float_3 = OpConstant %float 3
         %53 = OpConstantComposite %v3float %float_3 %float_3 %float_3
         %54 = OpTypeFunction %v4float
    %float_4 = OpConstant %float 4
         %57 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%mat2v2float = OpTypeMatrix %v2float 2
         %59 = OpTypeFunction %mat2v2float
         %61 = OpConstantComposite %v2float %float_2 %float_0
         %62 = OpConstantComposite %v2float %float_0 %float_2
         %63 = OpConstantComposite %mat2v2float %61 %62
%mat3v3float = OpTypeMatrix %v3float 3
         %65 = OpTypeFunction %mat3v3float
         %67 = OpConstantComposite %v3float %float_3 %float_0 %float_0
         %68 = OpConstantComposite %v3float %float_0 %float_3 %float_0
         %69 = OpConstantComposite %v3float %float_0 %float_0 %float_3
         %70 = OpConstantComposite %mat3v3float %67 %68 %69
%mat4v4float = OpTypeMatrix %v4float 4
         %72 = OpTypeFunction %mat4v4float
         %74 = OpConstantComposite %v4float %float_4 %float_0 %float_0 %float_0
         %75 = OpConstantComposite %v4float %float_0 %float_4 %float_0 %float_0
         %76 = OpConstantComposite %v4float %float_0 %float_0 %float_4 %float_0
         %77 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_4
         %78 = OpConstantComposite %mat4v4float %74 %75 %76 %77
         %79 = OpTypeFunction %float
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
         %89 = OpTypeFunction %bool
       %true = OpConstantTrue %bool
     %v2bool = OpTypeVector %bool 2
         %93 = OpTypeFunction %v2bool
         %95 = OpConstantComposite %v2bool %true %true
     %v3bool = OpTypeVector %bool 3
         %97 = OpTypeFunction %v3bool
         %99 = OpConstantComposite %v3bool %true %true %true
     %v4bool = OpTypeVector %bool 4
        %101 = OpTypeFunction %v4bool
        %103 = OpConstantComposite %v4bool %true %true %true %true
        %104 = OpTypeFunction %int
      %int_1 = OpConstant %int 1
      %v2int = OpTypeVector %int 2
        %108 = OpTypeFunction %v2int
      %int_2 = OpConstant %int 2
        %111 = OpConstantComposite %v2int %int_2 %int_2
      %v3int = OpTypeVector %int 3
        %113 = OpTypeFunction %v3int
      %int_3 = OpConstant %int 3
        %116 = OpConstantComposite %v3int %int_3 %int_3 %int_3
      %v4int = OpTypeVector %int 4
        %118 = OpTypeFunction %v4int
      %int_4 = OpConstant %int 4
        %121 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
        %122 = OpTypeFunction %v4float %_ptr_Function_v2float
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


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %37

         %38 = OpLabel
         %42 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %42 %41
         %44 =   OpFunctionCall %v4float %main %42
                 OpStore %sk_FragColor %44
                 OpReturn
               OpFunctionEnd


               ; Function returns_float2_f2
%returns_float2_f2 = OpFunction %v2float None %45

         %46 = OpLabel
                 OpReturnValue %48
               OpFunctionEnd


               ; Function returns_float3_f3
%returns_float3_f3 = OpFunction %v3float None %50

         %51 = OpLabel
                 OpReturnValue %53
               OpFunctionEnd


               ; Function returns_float4_f4
%returns_float4_f4 = OpFunction %v4float None %54

         %55 = OpLabel
                 OpReturnValue %57
               OpFunctionEnd


               ; Function returns_float2x2_f22
%returns_float2x2_f22 = OpFunction %mat2v2float None %59

         %60 = OpLabel
                 OpReturnValue %63
               OpFunctionEnd


               ; Function returns_float3x3_f33
%returns_float3x3_f33 = OpFunction %mat3v3float None %65

         %66 = OpLabel
                 OpReturnValue %70
               OpFunctionEnd


               ; Function returns_float4x4_f44
%returns_float4x4_f44 = OpFunction %mat4v4float None %72

         %73 = OpLabel
                 OpReturnValue %78
               OpFunctionEnd


               ; Function returns_half_h
%returns_half_h = OpFunction %float None %79        ; RelaxedPrecision

         %80 = OpLabel
                 OpReturnValue %float_1
               OpFunctionEnd


               ; Function returns_half2_h2
%returns_half2_h2 = OpFunction %v2float None %45    ; RelaxedPrecision

         %82 = OpLabel
                 OpReturnValue %48
               OpFunctionEnd


               ; Function returns_half3_h3
%returns_half3_h3 = OpFunction %v3float None %50    ; RelaxedPrecision

         %83 = OpLabel
                 OpReturnValue %53
               OpFunctionEnd


               ; Function returns_half4_h4
%returns_half4_h4 = OpFunction %v4float None %54    ; RelaxedPrecision

         %84 = OpLabel
                 OpReturnValue %57
               OpFunctionEnd


               ; Function returns_half2x2_h22
%returns_half2x2_h22 = OpFunction %mat2v2float None %59     ; RelaxedPrecision

         %85 = OpLabel
                 OpReturnValue %63
               OpFunctionEnd


               ; Function returns_half3x3_h33
%returns_half3x3_h33 = OpFunction %mat3v3float None %65     ; RelaxedPrecision

         %86 = OpLabel
                 OpReturnValue %70
               OpFunctionEnd


               ; Function returns_half4x4_h44
%returns_half4x4_h44 = OpFunction %mat4v4float None %72     ; RelaxedPrecision

         %87 = OpLabel
                 OpReturnValue %78
               OpFunctionEnd


               ; Function returns_bool_b
%returns_bool_b = OpFunction %bool None %89

         %90 = OpLabel
                 OpReturnValue %true
               OpFunctionEnd


               ; Function returns_bool2_b2
%returns_bool2_b2 = OpFunction %v2bool None %93

         %94 = OpLabel
                 OpReturnValue %95
               OpFunctionEnd


               ; Function returns_bool3_b3
%returns_bool3_b3 = OpFunction %v3bool None %97

         %98 = OpLabel
                 OpReturnValue %99
               OpFunctionEnd


               ; Function returns_bool4_b4
%returns_bool4_b4 = OpFunction %v4bool None %101

        %102 = OpLabel
                 OpReturnValue %103
               OpFunctionEnd


               ; Function returns_int_i
%returns_int_i = OpFunction %int None %104

        %105 = OpLabel
                 OpReturnValue %int_1
               OpFunctionEnd


               ; Function returns_int2_i2
%returns_int2_i2 = OpFunction %v2int None %108

        %109 = OpLabel
                 OpReturnValue %111
               OpFunctionEnd


               ; Function returns_int3_i3
%returns_int3_i3 = OpFunction %v3int None %113

        %114 = OpLabel
                 OpReturnValue %116
               OpFunctionEnd


               ; Function returns_int4_i4
%returns_int4_i4 = OpFunction %v4int None %118

        %119 = OpLabel
                 OpReturnValue %121
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %122        ; RelaxedPrecision
        %123 = OpFunctionParameter %_ptr_Function_v2float

        %124 = OpLabel
         %x1 =   OpVariable %_ptr_Function_float Function
         %x2 =   OpVariable %_ptr_Function_v2float Function
         %x3 =   OpVariable %_ptr_Function_v3float Function
         %x4 =   OpVariable %_ptr_Function_v4float Function
         %x5 =   OpVariable %_ptr_Function_mat2v2float Function
         %x6 =   OpVariable %_ptr_Function_mat3v3float Function
         %x7 =   OpVariable %_ptr_Function_mat4v4float Function
         %x8 =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
         %x9 =   OpVariable %_ptr_Function_v2float Function     ; RelaxedPrecision
        %x10 =   OpVariable %_ptr_Function_v3float Function     ; RelaxedPrecision
        %x11 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
        %x12 =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
        %x13 =   OpVariable %_ptr_Function_mat3v3float Function     ; RelaxedPrecision
        %x14 =   OpVariable %_ptr_Function_mat4v4float Function     ; RelaxedPrecision
        %x15 =   OpVariable %_ptr_Function_bool Function
        %x16 =   OpVariable %_ptr_Function_v2bool Function
        %x17 =   OpVariable %_ptr_Function_v3bool Function
        %x18 =   OpVariable %_ptr_Function_v4bool Function
        %x19 =   OpVariable %_ptr_Function_int Function
        %x20 =   OpVariable %_ptr_Function_v2int Function
        %x21 =   OpVariable %_ptr_Function_v3int Function
        %x22 =   OpVariable %_ptr_Function_v4int Function
        %339 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %x1 %float_1
                 OpStore %x2 %48
                 OpStore %x3 %53
                 OpStore %x4 %57
                 OpStore %x5 %63
                 OpStore %x6 %70
                 OpStore %x7 %78
                 OpStore %x8 %float_1
                 OpStore %x9 %48
                 OpStore %x10 %53
                 OpStore %x11 %57
                 OpStore %x12 %63
                 OpStore %x13 %70
                 OpStore %x14 %78
                 OpStore %x15 %true
                 OpStore %x16 %95
                 OpStore %x17 %99
                 OpStore %x18 %103
                 OpStore %x19 %int_1
                 OpStore %x20 %111
                 OpStore %x21 %116
                 OpStore %x22 %121
                 OpSelectionMerge %163 None
                 OpBranchConditional %true %162 %163

        %162 =     OpLabel
        %164 =       OpFunctionCall %v2float %returns_float2_f2
        %165 =       OpFOrdEqual %v2bool %48 %164
        %166 =       OpAll %bool %165
                     OpBranch %163

        %163 = OpLabel
        %167 =   OpPhi %bool %false %124 %166 %162
                 OpSelectionMerge %169 None
                 OpBranchConditional %167 %168 %169

        %168 =     OpLabel
        %170 =       OpFunctionCall %v3float %returns_float3_f3
        %171 =       OpFOrdEqual %v3bool %53 %170
        %172 =       OpAll %bool %171
                     OpBranch %169

        %169 = OpLabel
        %173 =   OpPhi %bool %false %163 %172 %168
                 OpSelectionMerge %175 None
                 OpBranchConditional %173 %174 %175

        %174 =     OpLabel
        %176 =       OpFunctionCall %v4float %returns_float4_f4
        %177 =       OpFOrdEqual %v4bool %57 %176
        %178 =       OpAll %bool %177
                     OpBranch %175

        %175 = OpLabel
        %179 =   OpPhi %bool %false %169 %178 %174
                 OpSelectionMerge %181 None
                 OpBranchConditional %179 %180 %181

        %180 =     OpLabel
        %182 =       OpFunctionCall %mat2v2float %returns_float2x2_f22
        %183 =       OpCompositeExtract %v2float %182 0
        %184 =       OpFOrdEqual %v2bool %61 %183
        %185 =       OpAll %bool %184
        %186 =       OpCompositeExtract %v2float %182 1
        %187 =       OpFOrdEqual %v2bool %62 %186
        %188 =       OpAll %bool %187
        %189 =       OpLogicalAnd %bool %185 %188
                     OpBranch %181

        %181 = OpLabel
        %190 =   OpPhi %bool %false %175 %189 %180
                 OpSelectionMerge %192 None
                 OpBranchConditional %190 %191 %192

        %191 =     OpLabel
        %193 =       OpFunctionCall %mat3v3float %returns_float3x3_f33
        %194 =       OpCompositeExtract %v3float %193 0
        %195 =       OpFOrdEqual %v3bool %67 %194
        %196 =       OpAll %bool %195
        %197 =       OpCompositeExtract %v3float %193 1
        %198 =       OpFOrdEqual %v3bool %68 %197
        %199 =       OpAll %bool %198
        %200 =       OpLogicalAnd %bool %196 %199
        %201 =       OpCompositeExtract %v3float %193 2
        %202 =       OpFOrdEqual %v3bool %69 %201
        %203 =       OpAll %bool %202
        %204 =       OpLogicalAnd %bool %200 %203
                     OpBranch %192

        %192 = OpLabel
        %205 =   OpPhi %bool %false %181 %204 %191
                 OpSelectionMerge %207 None
                 OpBranchConditional %205 %206 %207

        %206 =     OpLabel
        %208 =       OpFunctionCall %mat4v4float %returns_float4x4_f44
        %209 =       OpCompositeExtract %v4float %208 0
        %210 =       OpFOrdEqual %v4bool %74 %209
        %211 =       OpAll %bool %210
        %212 =       OpCompositeExtract %v4float %208 1
        %213 =       OpFOrdEqual %v4bool %75 %212
        %214 =       OpAll %bool %213
        %215 =       OpLogicalAnd %bool %211 %214
        %216 =       OpCompositeExtract %v4float %208 2
        %217 =       OpFOrdEqual %v4bool %76 %216
        %218 =       OpAll %bool %217
        %219 =       OpLogicalAnd %bool %215 %218
        %220 =       OpCompositeExtract %v4float %208 3
        %221 =       OpFOrdEqual %v4bool %77 %220
        %222 =       OpAll %bool %221
        %223 =       OpLogicalAnd %bool %219 %222
                     OpBranch %207

        %207 = OpLabel
        %224 =   OpPhi %bool %false %192 %223 %206
                 OpSelectionMerge %226 None
                 OpBranchConditional %224 %225 %226

        %225 =     OpLabel
        %227 =       OpFunctionCall %float %returns_half_h
        %228 =       OpFOrdEqual %bool %float_1 %227
                     OpBranch %226

        %226 = OpLabel
        %229 =   OpPhi %bool %false %207 %228 %225
                 OpSelectionMerge %231 None
                 OpBranchConditional %229 %230 %231

        %230 =     OpLabel
        %232 =       OpFunctionCall %v2float %returns_half2_h2
        %233 =       OpFOrdEqual %v2bool %48 %232
        %234 =       OpAll %bool %233
                     OpBranch %231

        %231 = OpLabel
        %235 =   OpPhi %bool %false %226 %234 %230
                 OpSelectionMerge %237 None
                 OpBranchConditional %235 %236 %237

        %236 =     OpLabel
        %238 =       OpFunctionCall %v3float %returns_half3_h3
        %239 =       OpFOrdEqual %v3bool %53 %238
        %240 =       OpAll %bool %239
                     OpBranch %237

        %237 = OpLabel
        %241 =   OpPhi %bool %false %231 %240 %236
                 OpSelectionMerge %243 None
                 OpBranchConditional %241 %242 %243

        %242 =     OpLabel
        %244 =       OpFunctionCall %v4float %returns_half4_h4
        %245 =       OpFOrdEqual %v4bool %57 %244
        %246 =       OpAll %bool %245
                     OpBranch %243

        %243 = OpLabel
        %247 =   OpPhi %bool %false %237 %246 %242
                 OpSelectionMerge %249 None
                 OpBranchConditional %247 %248 %249

        %248 =     OpLabel
        %250 =       OpFunctionCall %mat2v2float %returns_half2x2_h22
        %251 =       OpCompositeExtract %v2float %250 0     ; RelaxedPrecision
        %252 =       OpFOrdEqual %v2bool %61 %251           ; RelaxedPrecision
        %253 =       OpAll %bool %252
        %254 =       OpCompositeExtract %v2float %250 1     ; RelaxedPrecision
        %255 =       OpFOrdEqual %v2bool %62 %254           ; RelaxedPrecision
        %256 =       OpAll %bool %255
        %257 =       OpLogicalAnd %bool %253 %256
                     OpBranch %249

        %249 = OpLabel
        %258 =   OpPhi %bool %false %243 %257 %248
                 OpSelectionMerge %260 None
                 OpBranchConditional %258 %259 %260

        %259 =     OpLabel
        %261 =       OpFunctionCall %mat3v3float %returns_half3x3_h33
        %262 =       OpCompositeExtract %v3float %261 0     ; RelaxedPrecision
        %263 =       OpFOrdEqual %v3bool %67 %262           ; RelaxedPrecision
        %264 =       OpAll %bool %263
        %265 =       OpCompositeExtract %v3float %261 1     ; RelaxedPrecision
        %266 =       OpFOrdEqual %v3bool %68 %265           ; RelaxedPrecision
        %267 =       OpAll %bool %266
        %268 =       OpLogicalAnd %bool %264 %267
        %269 =       OpCompositeExtract %v3float %261 2     ; RelaxedPrecision
        %270 =       OpFOrdEqual %v3bool %69 %269           ; RelaxedPrecision
        %271 =       OpAll %bool %270
        %272 =       OpLogicalAnd %bool %268 %271
                     OpBranch %260

        %260 = OpLabel
        %273 =   OpPhi %bool %false %249 %272 %259
                 OpSelectionMerge %275 None
                 OpBranchConditional %273 %274 %275

        %274 =     OpLabel
        %276 =       OpFunctionCall %mat4v4float %returns_half4x4_h44
        %277 =       OpCompositeExtract %v4float %276 0     ; RelaxedPrecision
        %278 =       OpFOrdEqual %v4bool %74 %277           ; RelaxedPrecision
        %279 =       OpAll %bool %278
        %280 =       OpCompositeExtract %v4float %276 1     ; RelaxedPrecision
        %281 =       OpFOrdEqual %v4bool %75 %280           ; RelaxedPrecision
        %282 =       OpAll %bool %281
        %283 =       OpLogicalAnd %bool %279 %282
        %284 =       OpCompositeExtract %v4float %276 2     ; RelaxedPrecision
        %285 =       OpFOrdEqual %v4bool %76 %284           ; RelaxedPrecision
        %286 =       OpAll %bool %285
        %287 =       OpLogicalAnd %bool %283 %286
        %288 =       OpCompositeExtract %v4float %276 3     ; RelaxedPrecision
        %289 =       OpFOrdEqual %v4bool %77 %288           ; RelaxedPrecision
        %290 =       OpAll %bool %289
        %291 =       OpLogicalAnd %bool %287 %290
                     OpBranch %275

        %275 = OpLabel
        %292 =   OpPhi %bool %false %260 %291 %274
                 OpSelectionMerge %294 None
                 OpBranchConditional %292 %293 %294

        %293 =     OpLabel
        %295 =       OpFunctionCall %bool %returns_bool_b
        %296 =       OpLogicalEqual %bool %true %295
                     OpBranch %294

        %294 = OpLabel
        %297 =   OpPhi %bool %false %275 %296 %293
                 OpSelectionMerge %299 None
                 OpBranchConditional %297 %298 %299

        %298 =     OpLabel
        %300 =       OpFunctionCall %v2bool %returns_bool2_b2
        %301 =       OpLogicalEqual %v2bool %95 %300
        %302 =       OpAll %bool %301
                     OpBranch %299

        %299 = OpLabel
        %303 =   OpPhi %bool %false %294 %302 %298
                 OpSelectionMerge %305 None
                 OpBranchConditional %303 %304 %305

        %304 =     OpLabel
        %306 =       OpFunctionCall %v3bool %returns_bool3_b3
        %307 =       OpLogicalEqual %v3bool %99 %306
        %308 =       OpAll %bool %307
                     OpBranch %305

        %305 = OpLabel
        %309 =   OpPhi %bool %false %299 %308 %304
                 OpSelectionMerge %311 None
                 OpBranchConditional %309 %310 %311

        %310 =     OpLabel
        %312 =       OpFunctionCall %v4bool %returns_bool4_b4
        %313 =       OpLogicalEqual %v4bool %103 %312
        %314 =       OpAll %bool %313
                     OpBranch %311

        %311 = OpLabel
        %315 =   OpPhi %bool %false %305 %314 %310
                 OpSelectionMerge %317 None
                 OpBranchConditional %315 %316 %317

        %316 =     OpLabel
        %318 =       OpFunctionCall %int %returns_int_i
        %319 =       OpIEqual %bool %int_1 %318
                     OpBranch %317

        %317 = OpLabel
        %320 =   OpPhi %bool %false %311 %319 %316
                 OpSelectionMerge %322 None
                 OpBranchConditional %320 %321 %322

        %321 =     OpLabel
        %323 =       OpFunctionCall %v2int %returns_int2_i2
        %324 =       OpIEqual %v2bool %111 %323
        %325 =       OpAll %bool %324
                     OpBranch %322

        %322 = OpLabel
        %326 =   OpPhi %bool %false %317 %325 %321
                 OpSelectionMerge %328 None
                 OpBranchConditional %326 %327 %328

        %327 =     OpLabel
        %329 =       OpFunctionCall %v3int %returns_int3_i3
        %330 =       OpIEqual %v3bool %116 %329
        %331 =       OpAll %bool %330
                     OpBranch %328

        %328 = OpLabel
        %332 =   OpPhi %bool %false %322 %331 %327
                 OpSelectionMerge %334 None
                 OpBranchConditional %332 %333 %334

        %333 =     OpLabel
        %335 =       OpFunctionCall %v4int %returns_int4_i4
        %336 =       OpIEqual %v4bool %121 %335
        %337 =       OpAll %bool %336
                     OpBranch %334

        %334 = OpLabel
        %338 =   OpPhi %bool %false %328 %337 %333
                 OpSelectionMerge %342 None
                 OpBranchConditional %338 %340 %341

        %340 =     OpLabel
        %343 =       OpAccessChain %_ptr_Uniform_v4float %32 %int_0
        %346 =       OpLoad %v4float %343           ; RelaxedPrecision
                     OpStore %339 %346
                     OpBranch %342

        %341 =     OpLabel
        %347 =       OpAccessChain %_ptr_Uniform_v4float %32 %int_1
        %348 =       OpLoad %v4float %347           ; RelaxedPrecision
                     OpStore %339 %348
                     OpBranch %342

        %342 = OpLabel
        %349 =   OpLoad %v4float %339               ; RelaxedPrecision
                 OpReturnValue %349
               OpFunctionEnd
