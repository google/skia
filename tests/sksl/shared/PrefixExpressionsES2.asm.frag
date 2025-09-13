               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testMatrix2x2"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %main "main"                      ; id %6
               OpName %ok "ok"                          ; id %28
               OpName %i "i"                            ; id %32
               OpName %f "f"                            ; id %61
               OpName %f2 "f2"                          ; id %89
               OpName %i4 "i4"                          ; id %157
               OpName %m3x3 "m3x3"                      ; id %196
               OpName %iv "iv"                          ; id %342

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 ColMajor
               OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %95 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %297 RelaxedPrecision
               OpDecorate %298 RelaxedPrecision
               OpDecorate %305 RelaxedPrecision
               OpDecorate %306 RelaxedPrecision
               OpDecorate %307 RelaxedPrecision
               OpDecorate %314 RelaxedPrecision
               OpDecorate %315 RelaxedPrecision
               OpDecorate %369 RelaxedPrecision
               OpDecorate %371 RelaxedPrecision
               OpDecorate %372 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %25 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
%_ptr_Function_int = OpTypePointer Function %int
      %int_5 = OpConstant %int 5
      %int_1 = OpConstant %int 1
      %false = OpConstantFalse %bool
      %int_6 = OpConstant %int 6
      %int_7 = OpConstant %int 7
%_ptr_Function_float = OpTypePointer Function %float
  %float_0_5 = OpConstant %float 0.5
    %float_1 = OpConstant %float 1
  %float_1_5 = OpConstant %float 1.5
  %float_2_5 = OpConstant %float 2.5
         %90 = OpConstantComposite %v2float %float_0_5 %float_0_5
      %int_0 = OpConstant %int 0
        %126 = OpConstantComposite %v2float %float_1 %float_1
        %131 = OpConstantComposite %v2float %float_1_5 %float_1_5
     %v2bool = OpTypeVector %bool 2
        %139 = OpConstantComposite %v2float %float_2_5 %float_2_5
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
      %int_8 = OpConstant %int 8
      %int_9 = OpConstant %int 9
     %int_10 = OpConstant %int 10
        %163 = OpConstantComposite %v4int %int_7 %int_8 %int_9 %int_10
        %164 = OpConstantComposite %v4int %int_1 %int_1 %int_1 %int_1
     %int_11 = OpConstant %int 11
        %169 = OpConstantComposite %v4int %int_8 %int_9 %int_10 %int_11
     %v4bool = OpTypeVector %bool 4
     %int_12 = OpConstant %int 12
        %178 = OpConstantComposite %v4int %int_9 %int_10 %int_11 %int_12
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
    %float_5 = OpConstant %float 5
    %float_6 = OpConstant %float 6
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
    %float_9 = OpConstant %float 9
        %208 = OpConstantComposite %v3float %float_1 %float_2 %float_3
        %209 = OpConstantComposite %v3float %float_4 %float_5 %float_6
        %210 = OpConstantComposite %v3float %float_7 %float_8 %float_9
        %211 = OpConstantComposite %mat3v3float %208 %209 %210
        %212 = OpConstantComposite %v3float %float_1 %float_1 %float_1
        %213 = OpConstantComposite %mat3v3float %212 %212 %212
   %float_10 = OpConstant %float 10
        %221 = OpConstantComposite %v3float %float_2 %float_3 %float_4
        %222 = OpConstantComposite %v3float %float_5 %float_6 %float_7
        %223 = OpConstantComposite %v3float %float_8 %float_9 %float_10
        %224 = OpConstantComposite %mat3v3float %221 %222 %223
     %v3bool = OpTypeVector %bool 3
   %float_11 = OpConstant %float 11
        %242 = OpConstantComposite %v3float %float_3 %float_4 %float_5
        %243 = OpConstantComposite %v3float %float_6 %float_7 %float_8
        %244 = OpConstantComposite %v3float %float_9 %float_10 %float_11
        %245 = OpConstantComposite %mat3v3float %242 %243 %244
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
   %float_n1 = OpConstant %float -1
        %312 = OpConstantComposite %v4float %float_0 %float_n1 %float_0 %float_n1
   %float_n2 = OpConstant %float -2
   %float_n3 = OpConstant %float -3
   %float_n4 = OpConstant %float -4
        %324 = OpConstantComposite %v2float %float_n1 %float_n2
        %325 = OpConstantComposite %v2float %float_n3 %float_n4
        %326 = OpConstantComposite %mat2v2float %324 %325
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
      %int_2 = OpConstant %int 2
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
     %int_n5 = OpConstant %int -5
        %359 = OpConstantComposite %v2int %int_n5 %int_5
%_ptr_Function_v4float = OpTypePointer Function %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %18

         %19 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %25         ; RelaxedPrecision
         %26 = OpFunctionParameter %_ptr_Function_v2float

         %27 = OpLabel
         %ok =   OpVariable %_ptr_Function_bool Function
          %i =   OpVariable %_ptr_Function_int Function
          %f =   OpVariable %_ptr_Function_float Function
         %f2 =   OpVariable %_ptr_Function_v2float Function
         %i4 =   OpVariable %_ptr_Function_v4int Function
       %m3x3 =   OpVariable %_ptr_Function_mat3v3float Function
         %iv =   OpVariable %_ptr_Function_v2int Function
        %363 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %ok %true
                 OpStore %i %int_5
         %36 =   OpIAdd %int %int_5 %int_1
                 OpStore %i %36
                 OpSelectionMerge %39 None
                 OpBranchConditional %true %38 %39

         %38 =     OpLabel
         %41 =       OpIEqual %bool %36 %int_6
                     OpBranch %39

         %39 = OpLabel
         %42 =   OpPhi %bool %false %27 %41 %38
                 OpStore %ok %42
                 OpSelectionMerge %44 None
                 OpBranchConditional %42 %43 %44

         %43 =     OpLabel
         %45 =       OpIAdd %int %36 %int_1
                     OpStore %i %45
         %47 =       OpIEqual %bool %45 %int_7
                     OpBranch %44

         %44 = OpLabel
         %48 =   OpPhi %bool %false %39 %47 %43
                 OpStore %ok %48
                 OpSelectionMerge %50 None
                 OpBranchConditional %48 %49 %50

         %49 =     OpLabel
         %51 =       OpLoad %int %i
         %52 =       OpISub %int %51 %int_1
                     OpStore %i %52
         %53 =       OpIEqual %bool %52 %int_6
                     OpBranch %50

         %50 = OpLabel
         %54 =   OpPhi %bool %false %44 %53 %49
                 OpStore %ok %54
         %55 =   OpLoad %int %i
         %56 =   OpISub %int %55 %int_1
                 OpStore %i %56
                 OpSelectionMerge %58 None
                 OpBranchConditional %54 %57 %58

         %57 =     OpLabel
         %59 =       OpIEqual %bool %56 %int_5
                     OpBranch %58

         %58 = OpLabel
         %60 =   OpPhi %bool %false %50 %59 %57
                 OpStore %ok %60
                 OpStore %f %float_0_5
         %65 =   OpFAdd %float %float_0_5 %float_1
                 OpStore %f %65
                 OpSelectionMerge %67 None
                 OpBranchConditional %60 %66 %67

         %66 =     OpLabel
         %69 =       OpFOrdEqual %bool %65 %float_1_5
                     OpBranch %67

         %67 = OpLabel
         %70 =   OpPhi %bool %false %58 %69 %66
                 OpStore %ok %70
                 OpSelectionMerge %72 None
                 OpBranchConditional %70 %71 %72

         %71 =     OpLabel
         %73 =       OpFAdd %float %65 %float_1
                     OpStore %f %73
         %75 =       OpFOrdEqual %bool %73 %float_2_5
                     OpBranch %72

         %72 = OpLabel
         %76 =   OpPhi %bool %false %67 %75 %71
                 OpStore %ok %76
                 OpSelectionMerge %78 None
                 OpBranchConditional %76 %77 %78

         %77 =     OpLabel
         %79 =       OpLoad %float %f
         %80 =       OpFSub %float %79 %float_1
                     OpStore %f %80
         %81 =       OpFOrdEqual %bool %80 %float_1_5
                     OpBranch %78

         %78 = OpLabel
         %82 =   OpPhi %bool %false %72 %81 %77
                 OpStore %ok %82
         %83 =   OpLoad %float %f
         %84 =   OpFSub %float %83 %float_1
                 OpStore %f %84
                 OpSelectionMerge %86 None
                 OpBranchConditional %82 %85 %86

         %85 =     OpLabel
         %87 =       OpFOrdEqual %bool %84 %float_0_5
                     OpBranch %86

         %86 = OpLabel
         %88 =   OpPhi %bool %false %78 %87 %85
                 OpStore %ok %88
                 OpStore %f2 %90
         %91 =   OpAccessChain %_ptr_Function_float %f2 %int_0
         %93 =   OpLoad %float %91
         %94 =   OpFAdd %float %93 %float_1
                 OpStore %91 %94
         %95 =   OpLoad %bool %ok                   ; RelaxedPrecision
                 OpSelectionMerge %97 None
                 OpBranchConditional %95 %96 %97

         %96 =     OpLabel
         %98 =       OpLoad %v2float %f2
         %99 =       OpCompositeExtract %float %98 0
        %100 =       OpFOrdEqual %bool %99 %float_1_5
                     OpBranch %97

         %97 = OpLabel
        %101 =   OpPhi %bool %false %86 %100 %96
                 OpStore %ok %101
                 OpSelectionMerge %103 None
                 OpBranchConditional %101 %102 %103

        %102 =     OpLabel
        %104 =       OpAccessChain %_ptr_Function_float %f2 %int_0
        %105 =       OpLoad %float %104
        %106 =       OpFAdd %float %105 %float_1
                     OpStore %104 %106
        %107 =       OpFOrdEqual %bool %106 %float_2_5
                     OpBranch %103

        %103 = OpLabel
        %108 =   OpPhi %bool %false %97 %107 %102
                 OpStore %ok %108
                 OpSelectionMerge %110 None
                 OpBranchConditional %108 %109 %110

        %109 =     OpLabel
        %111 =       OpAccessChain %_ptr_Function_float %f2 %int_0
        %112 =       OpLoad %float %111
        %113 =       OpFSub %float %112 %float_1
                     OpStore %111 %113
        %114 =       OpFOrdEqual %bool %113 %float_1_5
                     OpBranch %110

        %110 = OpLabel
        %115 =   OpPhi %bool %false %103 %114 %109
                 OpStore %ok %115
        %116 =   OpAccessChain %_ptr_Function_float %f2 %int_0
        %117 =   OpLoad %float %116
        %118 =   OpFSub %float %117 %float_1
                 OpStore %116 %118
        %119 =   OpLoad %bool %ok                   ; RelaxedPrecision
                 OpSelectionMerge %121 None
                 OpBranchConditional %119 %120 %121

        %120 =     OpLabel
        %122 =       OpLoad %v2float %f2
        %123 =       OpCompositeExtract %float %122 0
        %124 =       OpFOrdEqual %bool %123 %float_0_5
                     OpBranch %121

        %121 = OpLabel
        %125 =   OpPhi %bool %false %110 %124 %120
                 OpStore %ok %125
        %127 =   OpLoad %v2float %f2
        %128 =   OpFAdd %v2float %127 %126
                 OpStore %f2 %128
                 OpSelectionMerge %130 None
                 OpBranchConditional %125 %129 %130

        %129 =     OpLabel
        %132 =       OpFOrdEqual %v2bool %128 %131
        %134 =       OpAll %bool %132
                     OpBranch %130

        %130 = OpLabel
        %135 =   OpPhi %bool %false %121 %134 %129
                 OpStore %ok %135
                 OpSelectionMerge %137 None
                 OpBranchConditional %135 %136 %137

        %136 =     OpLabel
        %138 =       OpFAdd %v2float %128 %126
                     OpStore %f2 %138
        %140 =       OpFOrdEqual %v2bool %138 %139
        %141 =       OpAll %bool %140
                     OpBranch %137

        %137 = OpLabel
        %142 =   OpPhi %bool %false %130 %141 %136
                 OpStore %ok %142
                 OpSelectionMerge %144 None
                 OpBranchConditional %142 %143 %144

        %143 =     OpLabel
        %145 =       OpLoad %v2float %f2
        %146 =       OpFSub %v2float %145 %126
                     OpStore %f2 %146
        %147 =       OpFOrdEqual %v2bool %146 %131
        %148 =       OpAll %bool %147
                     OpBranch %144

        %144 = OpLabel
        %149 =   OpPhi %bool %false %137 %148 %143
                 OpStore %ok %149
        %150 =   OpLoad %v2float %f2
        %151 =   OpFSub %v2float %150 %126
                 OpStore %f2 %151
                 OpSelectionMerge %153 None
                 OpBranchConditional %149 %152 %153

        %152 =     OpLabel
        %154 =       OpFOrdEqual %v2bool %151 %90
        %155 =       OpAll %bool %154
                     OpBranch %153

        %153 = OpLabel
        %156 =   OpPhi %bool %false %144 %155 %152
                 OpStore %ok %156
                 OpStore %i4 %163
        %165 =   OpIAdd %v4int %163 %164
                 OpStore %i4 %165
                 OpSelectionMerge %167 None
                 OpBranchConditional %156 %166 %167

        %166 =     OpLabel
        %170 =       OpIEqual %v4bool %165 %169
        %172 =       OpAll %bool %170
                     OpBranch %167

        %167 = OpLabel
        %173 =   OpPhi %bool %false %153 %172 %166
                 OpStore %ok %173
                 OpSelectionMerge %175 None
                 OpBranchConditional %173 %174 %175

        %174 =     OpLabel
        %176 =       OpIAdd %v4int %165 %164
                     OpStore %i4 %176
        %179 =       OpIEqual %v4bool %176 %178
        %180 =       OpAll %bool %179
                     OpBranch %175

        %175 = OpLabel
        %181 =   OpPhi %bool %false %167 %180 %174
                 OpStore %ok %181
                 OpSelectionMerge %183 None
                 OpBranchConditional %181 %182 %183

        %182 =     OpLabel
        %184 =       OpLoad %v4int %i4
        %185 =       OpISub %v4int %184 %164
                     OpStore %i4 %185
        %186 =       OpIEqual %v4bool %185 %169
        %187 =       OpAll %bool %186
                     OpBranch %183

        %183 = OpLabel
        %188 =   OpPhi %bool %false %175 %187 %182
                 OpStore %ok %188
        %189 =   OpLoad %v4int %i4
        %190 =   OpISub %v4int %189 %164
                 OpStore %i4 %190
                 OpSelectionMerge %192 None
                 OpBranchConditional %188 %191 %192

        %191 =     OpLabel
        %193 =       OpIEqual %v4bool %190 %163
        %194 =       OpAll %bool %193
                     OpBranch %192

        %192 = OpLabel
        %195 =   OpPhi %bool %false %183 %194 %191
                 OpStore %ok %195
                 OpStore %m3x3 %211
        %214 =   OpFAdd %v3float %208 %212
        %215 =   OpFAdd %v3float %209 %212
        %216 =   OpFAdd %v3float %210 %212
        %217 =   OpCompositeConstruct %mat3v3float %214 %215 %216
                 OpStore %m3x3 %217
                 OpSelectionMerge %219 None
                 OpBranchConditional %195 %218 %219

        %218 =     OpLabel
        %226 =       OpFOrdEqual %v3bool %214 %221
        %227 =       OpAll %bool %226
        %228 =       OpFOrdEqual %v3bool %215 %222
        %229 =       OpAll %bool %228
        %230 =       OpLogicalAnd %bool %227 %229
        %231 =       OpFOrdEqual %v3bool %216 %223
        %232 =       OpAll %bool %231
        %233 =       OpLogicalAnd %bool %230 %232
                     OpBranch %219

        %219 = OpLabel
        %234 =   OpPhi %bool %false %192 %233 %218
                 OpStore %ok %234
                 OpSelectionMerge %236 None
                 OpBranchConditional %234 %235 %236

        %235 =     OpLabel
        %237 =       OpFAdd %v3float %214 %212
        %238 =       OpFAdd %v3float %215 %212
        %239 =       OpFAdd %v3float %216 %212
        %240 =       OpCompositeConstruct %mat3v3float %237 %238 %239
                     OpStore %m3x3 %240
        %246 =       OpFOrdEqual %v3bool %237 %242
        %247 =       OpAll %bool %246
        %248 =       OpFOrdEqual %v3bool %238 %243
        %249 =       OpAll %bool %248
        %250 =       OpLogicalAnd %bool %247 %249
        %251 =       OpFOrdEqual %v3bool %239 %244
        %252 =       OpAll %bool %251
        %253 =       OpLogicalAnd %bool %250 %252
                     OpBranch %236

        %236 = OpLabel
        %254 =   OpPhi %bool %false %219 %253 %235
                 OpStore %ok %254
                 OpSelectionMerge %256 None
                 OpBranchConditional %254 %255 %256

        %255 =     OpLabel
        %257 =       OpLoad %mat3v3float %m3x3
        %258 =       OpCompositeExtract %v3float %257 0
        %259 =       OpFSub %v3float %258 %212
        %260 =       OpCompositeExtract %v3float %257 1
        %261 =       OpFSub %v3float %260 %212
        %262 =       OpCompositeExtract %v3float %257 2
        %263 =       OpFSub %v3float %262 %212
        %264 =       OpCompositeConstruct %mat3v3float %259 %261 %263
                     OpStore %m3x3 %264
        %265 =       OpFOrdEqual %v3bool %259 %221
        %266 =       OpAll %bool %265
        %267 =       OpFOrdEqual %v3bool %261 %222
        %268 =       OpAll %bool %267
        %269 =       OpLogicalAnd %bool %266 %268
        %270 =       OpFOrdEqual %v3bool %263 %223
        %271 =       OpAll %bool %270
        %272 =       OpLogicalAnd %bool %269 %271
                     OpBranch %256

        %256 = OpLabel
        %273 =   OpPhi %bool %false %236 %272 %255
                 OpStore %ok %273
        %274 =   OpLoad %mat3v3float %m3x3
        %275 =   OpCompositeExtract %v3float %274 0
        %276 =   OpFSub %v3float %275 %212
        %277 =   OpCompositeExtract %v3float %274 1
        %278 =   OpFSub %v3float %277 %212
        %279 =   OpCompositeExtract %v3float %274 2
        %280 =   OpFSub %v3float %279 %212
        %281 =   OpCompositeConstruct %mat3v3float %276 %278 %280
                 OpStore %m3x3 %281
                 OpSelectionMerge %283 None
                 OpBranchConditional %273 %282 %283

        %282 =     OpLabel
        %284 =       OpFOrdEqual %v3bool %276 %208
        %285 =       OpAll %bool %284
        %286 =       OpFOrdEqual %v3bool %278 %209
        %287 =       OpAll %bool %286
        %288 =       OpLogicalAnd %bool %285 %287
        %289 =       OpFOrdEqual %v3bool %280 %210
        %290 =       OpAll %bool %289
        %291 =       OpLogicalAnd %bool %288 %290
                     OpBranch %283

        %283 = OpLabel
        %292 =   OpPhi %bool %false %256 %291 %282
                 OpStore %ok %292
                 OpSelectionMerge %294 None
                 OpBranchConditional %292 %293 %294

        %293 =     OpLabel
        %295 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %297 =       OpLoad %v4float %295           ; RelaxedPrecision
        %298 =       OpCompositeExtract %float %297 0   ; RelaxedPrecision
        %299 =       OpFUnordNotEqual %bool %298 %float_1
                     OpBranch %294

        %294 = OpLabel
        %300 =   OpPhi %bool %false %283 %299 %293
                 OpStore %ok %300
                 OpSelectionMerge %302 None
                 OpBranchConditional %300 %301 %302

        %301 =     OpLabel
        %304 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %305 =       OpLoad %v4float %304           ; RelaxedPrecision
        %306 =       OpCompositeExtract %float %305 1   ; RelaxedPrecision
        %307 =       OpFNegate %float %306              ; RelaxedPrecision
        %308 =       OpFOrdEqual %bool %float_n1 %307
                     OpBranch %302

        %302 = OpLabel
        %309 =   OpPhi %bool %false %294 %308 %301
                 OpStore %ok %309
                 OpSelectionMerge %311 None
                 OpBranchConditional %309 %310 %311

        %310 =     OpLabel
        %313 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %314 =       OpLoad %v4float %313           ; RelaxedPrecision
        %315 =       OpFNegate %v4float %314        ; RelaxedPrecision
        %316 =       OpFOrdEqual %v4bool %312 %315
        %317 =       OpAll %bool %316
                     OpBranch %311

        %311 = OpLabel
        %318 =   OpPhi %bool %false %302 %317 %310
                 OpStore %ok %318
                 OpSelectionMerge %320 None
                 OpBranchConditional %318 %319 %320

        %319 =     OpLabel
        %327 =       OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
        %330 =       OpLoad %mat2v2float %327
        %331 =       OpCompositeExtract %v2float %330 0
        %332 =       OpFNegate %v2float %331
        %333 =       OpCompositeExtract %v2float %330 1
        %334 =       OpFNegate %v2float %333
        %335 =       OpCompositeConstruct %mat2v2float %332 %334
        %336 =       OpFOrdEqual %v2bool %324 %332
        %337 =       OpAll %bool %336
        %338 =       OpFOrdEqual %v2bool %325 %334
        %339 =       OpAll %bool %338
        %340 =       OpLogicalAnd %bool %337 %339
                     OpBranch %320

        %320 = OpLabel
        %341 =   OpPhi %bool %false %311 %340 %319
                 OpStore %ok %341
        %345 =   OpLoad %int %i
        %346 =   OpLoad %int %i
        %347 =   OpSNegate %int %346
        %348 =   OpCompositeConstruct %v2int %345 %347
                 OpStore %iv %348
                 OpSelectionMerge %350 None
                 OpBranchConditional %341 %349 %350

        %349 =     OpLabel
        %351 =       OpLoad %int %i
        %352 =       OpSNegate %int %351
        %354 =       OpIEqual %bool %352 %int_n5
                     OpBranch %350

        %350 = OpLabel
        %355 =   OpPhi %bool %false %320 %354 %349
                 OpStore %ok %355
                 OpSelectionMerge %357 None
                 OpBranchConditional %355 %356 %357

        %356 =     OpLabel
        %358 =       OpSNegate %v2int %348
        %360 =       OpIEqual %v2bool %358 %359
        %361 =       OpAll %bool %360
                     OpBranch %357

        %357 = OpLabel
        %362 =   OpPhi %bool %false %350 %361 %356
                 OpStore %ok %362
                 OpSelectionMerge %367 None
                 OpBranchConditional %362 %365 %366

        %365 =     OpLabel
        %368 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %369 =       OpLoad %v4float %368           ; RelaxedPrecision
                     OpStore %363 %369
                     OpBranch %367

        %366 =     OpLabel
        %370 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %371 =       OpLoad %v4float %370           ; RelaxedPrecision
                     OpStore %363 %371
                     OpBranch %367

        %367 = OpLabel
        %372 =   OpLoad %v4float %363               ; RelaxedPrecision
                 OpReturnValue %372
               OpFunctionEnd
