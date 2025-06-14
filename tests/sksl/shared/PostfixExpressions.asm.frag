               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %ok "ok"                          ; id %27
               OpName %i "i"                            ; id %31
               OpName %f "f"                            ; id %66
               OpName %f2 "f2"                          ; id %100
               OpName %i4 "i4"                          ; id %181
               OpName %m3x3 "m3x3"                      ; id %227

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
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %106 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %350 RelaxedPrecision
               OpDecorate %352 RelaxedPrecision
               OpDecorate %353 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
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
        %101 = OpConstantComposite %v2float %float_0_5 %float_0_5
      %int_0 = OpConstant %int 0
        %144 = OpConstantComposite %v2float %float_1 %float_1
        %149 = OpConstantComposite %v2float %float_1_5 %float_1_5
     %v2bool = OpTypeVector %bool 2
        %157 = OpConstantComposite %v2float %float_2_5 %float_2_5
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
      %int_8 = OpConstant %int 8
      %int_9 = OpConstant %int 9
     %int_10 = OpConstant %int 10
        %187 = OpConstantComposite %v4int %int_7 %int_8 %int_9 %int_10
        %188 = OpConstantComposite %v4int %int_1 %int_1 %int_1 %int_1
     %int_11 = OpConstant %int 11
        %194 = OpConstantComposite %v4int %int_8 %int_9 %int_10 %int_11
     %v4bool = OpTypeVector %bool 4
     %int_12 = OpConstant %int 12
        %203 = OpConstantComposite %v4int %int_9 %int_10 %int_11 %int_12
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
        %239 = OpConstantComposite %v3float %float_1 %float_2 %float_3
        %240 = OpConstantComposite %v3float %float_4 %float_5 %float_6
        %241 = OpConstantComposite %v3float %float_7 %float_8 %float_9
        %242 = OpConstantComposite %mat3v3float %239 %240 %241
        %243 = OpConstantComposite %v3float %float_1 %float_1 %float_1
        %244 = OpConstantComposite %mat3v3float %243 %243 %243
   %float_10 = OpConstant %float 10
        %256 = OpConstantComposite %v3float %float_2 %float_3 %float_4
        %257 = OpConstantComposite %v3float %float_5 %float_6 %float_7
        %258 = OpConstantComposite %v3float %float_8 %float_9 %float_10
        %259 = OpConstantComposite %mat3v3float %256 %257 %258
     %v3bool = OpTypeVector %bool 3
   %float_11 = OpConstant %float 11
        %274 = OpConstantComposite %v3float %float_3 %float_4 %float_5
        %275 = OpConstantComposite %v3float %float_6 %float_7 %float_8
        %276 = OpConstantComposite %v3float %float_9 %float_10 %float_11
        %277 = OpConstantComposite %mat3v3float %274 %275 %276
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %16

         %17 = OpLabel
         %21 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %21 %20
         %23 =   OpFunctionCall %v4float %main %21
                 OpStore %sk_FragColor %23
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %24         ; RelaxedPrecision
         %25 = OpFunctionParameter %_ptr_Function_v2float

         %26 = OpLabel
         %ok =   OpVariable %_ptr_Function_bool Function
          %i =   OpVariable %_ptr_Function_int Function
          %f =   OpVariable %_ptr_Function_float Function
         %f2 =   OpVariable %_ptr_Function_v2float Function
         %i4 =   OpVariable %_ptr_Function_v4int Function
       %m3x3 =   OpVariable %_ptr_Function_mat3v3float Function
        %343 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %ok %true
                 OpStore %i %int_5
         %35 =   OpIAdd %int %int_5 %int_1
                 OpStore %i %35
                 OpSelectionMerge %38 None
                 OpBranchConditional %true %37 %38

         %37 =     OpLabel
         %39 =       OpIAdd %int %35 %int_1
                     OpStore %i %39
         %41 =       OpIEqual %bool %35 %int_6
                     OpBranch %38

         %38 = OpLabel
         %42 =   OpPhi %bool %false %26 %41 %37
                 OpStore %ok %42
                 OpSelectionMerge %44 None
                 OpBranchConditional %42 %43 %44

         %43 =     OpLabel
         %45 =       OpLoad %int %i
         %47 =       OpIEqual %bool %45 %int_7
                     OpBranch %44

         %44 = OpLabel
         %48 =   OpPhi %bool %false %38 %47 %43
                 OpStore %ok %48
                 OpSelectionMerge %50 None
                 OpBranchConditional %48 %49 %50

         %49 =     OpLabel
         %51 =       OpLoad %int %i
         %52 =       OpISub %int %51 %int_1
                     OpStore %i %52
         %53 =       OpIEqual %bool %51 %int_7
                     OpBranch %50

         %50 = OpLabel
         %54 =   OpPhi %bool %false %44 %53 %49
                 OpStore %ok %54
                 OpSelectionMerge %56 None
                 OpBranchConditional %54 %55 %56

         %55 =     OpLabel
         %57 =       OpLoad %int %i
         %58 =       OpIEqual %bool %57 %int_6
                     OpBranch %56

         %56 = OpLabel
         %59 =   OpPhi %bool %false %50 %58 %55
                 OpStore %ok %59
         %60 =   OpLoad %int %i
         %61 =   OpISub %int %60 %int_1
                 OpStore %i %61
                 OpSelectionMerge %63 None
                 OpBranchConditional %59 %62 %63

         %62 =     OpLabel
         %64 =       OpIEqual %bool %61 %int_5
                     OpBranch %63

         %63 = OpLabel
         %65 =   OpPhi %bool %false %56 %64 %62
                 OpStore %ok %65
                 OpStore %f %float_0_5
         %70 =   OpFAdd %float %float_0_5 %float_1
                 OpStore %f %70
                 OpSelectionMerge %72 None
                 OpBranchConditional %65 %71 %72

         %71 =     OpLabel
         %73 =       OpFAdd %float %70 %float_1
                     OpStore %f %73
         %75 =       OpFOrdEqual %bool %70 %float_1_5
                     OpBranch %72

         %72 = OpLabel
         %76 =   OpPhi %bool %false %63 %75 %71
                 OpStore %ok %76
                 OpSelectionMerge %78 None
                 OpBranchConditional %76 %77 %78

         %77 =     OpLabel
         %79 =       OpLoad %float %f
         %81 =       OpFOrdEqual %bool %79 %float_2_5
                     OpBranch %78

         %78 = OpLabel
         %82 =   OpPhi %bool %false %72 %81 %77
                 OpStore %ok %82
                 OpSelectionMerge %84 None
                 OpBranchConditional %82 %83 %84

         %83 =     OpLabel
         %85 =       OpLoad %float %f
         %86 =       OpFSub %float %85 %float_1
                     OpStore %f %86
         %87 =       OpFOrdEqual %bool %85 %float_2_5
                     OpBranch %84

         %84 = OpLabel
         %88 =   OpPhi %bool %false %78 %87 %83
                 OpStore %ok %88
                 OpSelectionMerge %90 None
                 OpBranchConditional %88 %89 %90

         %89 =     OpLabel
         %91 =       OpLoad %float %f
         %92 =       OpFOrdEqual %bool %91 %float_1_5
                     OpBranch %90

         %90 = OpLabel
         %93 =   OpPhi %bool %false %84 %92 %89
                 OpStore %ok %93
         %94 =   OpLoad %float %f
         %95 =   OpFSub %float %94 %float_1
                 OpStore %f %95
                 OpSelectionMerge %97 None
                 OpBranchConditional %93 %96 %97

         %96 =     OpLabel
         %98 =       OpFOrdEqual %bool %95 %float_0_5
                     OpBranch %97

         %97 = OpLabel
         %99 =   OpPhi %bool %false %90 %98 %96
                 OpStore %ok %99
                 OpStore %f2 %101
        %102 =   OpAccessChain %_ptr_Function_float %f2 %int_0
        %104 =   OpLoad %float %102
        %105 =   OpFAdd %float %104 %float_1
                 OpStore %102 %105
        %106 =   OpLoad %bool %ok                   ; RelaxedPrecision
                 OpSelectionMerge %108 None
                 OpBranchConditional %106 %107 %108

        %107 =     OpLabel
        %109 =       OpAccessChain %_ptr_Function_float %f2 %int_0
        %110 =       OpLoad %float %109
        %111 =       OpFAdd %float %110 %float_1
                     OpStore %109 %111
        %112 =       OpFOrdEqual %bool %110 %float_1_5
                     OpBranch %108

        %108 = OpLabel
        %113 =   OpPhi %bool %false %97 %112 %107
                 OpStore %ok %113
                 OpSelectionMerge %115 None
                 OpBranchConditional %113 %114 %115

        %114 =     OpLabel
        %116 =       OpLoad %v2float %f2
        %117 =       OpCompositeExtract %float %116 0
        %118 =       OpFOrdEqual %bool %117 %float_2_5
                     OpBranch %115

        %115 = OpLabel
        %119 =   OpPhi %bool %false %108 %118 %114
                 OpStore %ok %119
                 OpSelectionMerge %121 None
                 OpBranchConditional %119 %120 %121

        %120 =     OpLabel
        %122 =       OpAccessChain %_ptr_Function_float %f2 %int_0
        %123 =       OpLoad %float %122
        %124 =       OpFSub %float %123 %float_1
                     OpStore %122 %124
        %125 =       OpFOrdEqual %bool %123 %float_2_5
                     OpBranch %121

        %121 = OpLabel
        %126 =   OpPhi %bool %false %115 %125 %120
                 OpStore %ok %126
                 OpSelectionMerge %128 None
                 OpBranchConditional %126 %127 %128

        %127 =     OpLabel
        %129 =       OpLoad %v2float %f2
        %130 =       OpCompositeExtract %float %129 0
        %131 =       OpFOrdEqual %bool %130 %float_1_5
                     OpBranch %128

        %128 = OpLabel
        %132 =   OpPhi %bool %false %121 %131 %127
                 OpStore %ok %132
        %133 =   OpAccessChain %_ptr_Function_float %f2 %int_0
        %134 =   OpLoad %float %133
        %135 =   OpFSub %float %134 %float_1
                 OpStore %133 %135
        %136 =   OpLoad %bool %ok                   ; RelaxedPrecision
                 OpSelectionMerge %138 None
                 OpBranchConditional %136 %137 %138

        %137 =     OpLabel
        %139 =       OpLoad %v2float %f2
        %140 =       OpCompositeExtract %float %139 0
        %141 =       OpFOrdEqual %bool %140 %float_0_5
                     OpBranch %138

        %138 = OpLabel
        %142 =   OpPhi %bool %false %128 %141 %137
                 OpStore %ok %142
        %143 =   OpLoad %v2float %f2
        %145 =   OpFAdd %v2float %143 %144
                 OpStore %f2 %145
                 OpSelectionMerge %147 None
                 OpBranchConditional %142 %146 %147

        %146 =     OpLabel
        %148 =       OpFAdd %v2float %145 %144
                     OpStore %f2 %148
        %150 =       OpFOrdEqual %v2bool %145 %149
        %152 =       OpAll %bool %150
                     OpBranch %147

        %147 = OpLabel
        %153 =   OpPhi %bool %false %138 %152 %146
                 OpStore %ok %153
                 OpSelectionMerge %155 None
                 OpBranchConditional %153 %154 %155

        %154 =     OpLabel
        %156 =       OpLoad %v2float %f2
        %158 =       OpFOrdEqual %v2bool %156 %157
        %159 =       OpAll %bool %158
                     OpBranch %155

        %155 = OpLabel
        %160 =   OpPhi %bool %false %147 %159 %154
                 OpStore %ok %160
                 OpSelectionMerge %162 None
                 OpBranchConditional %160 %161 %162

        %161 =     OpLabel
        %163 =       OpLoad %v2float %f2
        %164 =       OpFSub %v2float %163 %144
                     OpStore %f2 %164
        %165 =       OpFOrdEqual %v2bool %163 %157
        %166 =       OpAll %bool %165
                     OpBranch %162

        %162 = OpLabel
        %167 =   OpPhi %bool %false %155 %166 %161
                 OpStore %ok %167
                 OpSelectionMerge %169 None
                 OpBranchConditional %167 %168 %169

        %168 =     OpLabel
        %170 =       OpLoad %v2float %f2
        %171 =       OpFOrdEqual %v2bool %170 %149
        %172 =       OpAll %bool %171
                     OpBranch %169

        %169 = OpLabel
        %173 =   OpPhi %bool %false %162 %172 %168
                 OpStore %ok %173
        %174 =   OpLoad %v2float %f2
        %175 =   OpFSub %v2float %174 %144
                 OpStore %f2 %175
                 OpSelectionMerge %177 None
                 OpBranchConditional %173 %176 %177

        %176 =     OpLabel
        %178 =       OpFOrdEqual %v2bool %175 %101
        %179 =       OpAll %bool %178
                     OpBranch %177

        %177 = OpLabel
        %180 =   OpPhi %bool %false %169 %179 %176
                 OpStore %ok %180
                 OpStore %i4 %187
        %189 =   OpIAdd %v4int %187 %188
                 OpStore %i4 %189
                 OpSelectionMerge %191 None
                 OpBranchConditional %180 %190 %191

        %190 =     OpLabel
        %192 =       OpIAdd %v4int %189 %188
                     OpStore %i4 %192
        %195 =       OpIEqual %v4bool %189 %194
        %197 =       OpAll %bool %195
                     OpBranch %191

        %191 = OpLabel
        %198 =   OpPhi %bool %false %177 %197 %190
                 OpStore %ok %198
                 OpSelectionMerge %200 None
                 OpBranchConditional %198 %199 %200

        %199 =     OpLabel
        %201 =       OpLoad %v4int %i4
        %204 =       OpIEqual %v4bool %201 %203
        %205 =       OpAll %bool %204
                     OpBranch %200

        %200 = OpLabel
        %206 =   OpPhi %bool %false %191 %205 %199
                 OpStore %ok %206
                 OpSelectionMerge %208 None
                 OpBranchConditional %206 %207 %208

        %207 =     OpLabel
        %209 =       OpLoad %v4int %i4
        %210 =       OpISub %v4int %209 %188
                     OpStore %i4 %210
        %211 =       OpIEqual %v4bool %209 %203
        %212 =       OpAll %bool %211
                     OpBranch %208

        %208 = OpLabel
        %213 =   OpPhi %bool %false %200 %212 %207
                 OpStore %ok %213
                 OpSelectionMerge %215 None
                 OpBranchConditional %213 %214 %215

        %214 =     OpLabel
        %216 =       OpLoad %v4int %i4
        %217 =       OpIEqual %v4bool %216 %194
        %218 =       OpAll %bool %217
                     OpBranch %215

        %215 = OpLabel
        %219 =   OpPhi %bool %false %208 %218 %214
                 OpStore %ok %219
        %220 =   OpLoad %v4int %i4
        %221 =   OpISub %v4int %220 %188
                 OpStore %i4 %221
                 OpSelectionMerge %223 None
                 OpBranchConditional %219 %222 %223

        %222 =     OpLabel
        %224 =       OpIEqual %v4bool %221 %187
        %225 =       OpAll %bool %224
                     OpBranch %223

        %223 = OpLabel
        %226 =   OpPhi %bool %false %215 %225 %222
                 OpStore %ok %226
                 OpStore %m3x3 %242
        %245 =   OpFAdd %v3float %239 %243
        %246 =   OpFAdd %v3float %240 %243
        %247 =   OpFAdd %v3float %241 %243
        %248 =   OpCompositeConstruct %mat3v3float %245 %246 %247
                 OpStore %m3x3 %248
                 OpSelectionMerge %250 None
                 OpBranchConditional %226 %249 %250

        %249 =     OpLabel
        %251 =       OpFAdd %v3float %245 %243
        %252 =       OpFAdd %v3float %246 %243
        %253 =       OpFAdd %v3float %247 %243
        %254 =       OpCompositeConstruct %mat3v3float %251 %252 %253
                     OpStore %m3x3 %254
        %261 =       OpFOrdEqual %v3bool %245 %256
        %262 =       OpAll %bool %261
        %263 =       OpFOrdEqual %v3bool %246 %257
        %264 =       OpAll %bool %263
        %265 =       OpLogicalAnd %bool %262 %264
        %266 =       OpFOrdEqual %v3bool %247 %258
        %267 =       OpAll %bool %266
        %268 =       OpLogicalAnd %bool %265 %267
                     OpBranch %250

        %250 = OpLabel
        %269 =   OpPhi %bool %false %223 %268 %249
                 OpStore %ok %269
                 OpSelectionMerge %271 None
                 OpBranchConditional %269 %270 %271

        %270 =     OpLabel
        %272 =       OpLoad %mat3v3float %m3x3
        %278 =       OpCompositeExtract %v3float %272 0
        %279 =       OpFOrdEqual %v3bool %278 %274
        %280 =       OpAll %bool %279
        %281 =       OpCompositeExtract %v3float %272 1
        %282 =       OpFOrdEqual %v3bool %281 %275
        %283 =       OpAll %bool %282
        %284 =       OpLogicalAnd %bool %280 %283
        %285 =       OpCompositeExtract %v3float %272 2
        %286 =       OpFOrdEqual %v3bool %285 %276
        %287 =       OpAll %bool %286
        %288 =       OpLogicalAnd %bool %284 %287
                     OpBranch %271

        %271 = OpLabel
        %289 =   OpPhi %bool %false %250 %288 %270
                 OpStore %ok %289
                 OpSelectionMerge %291 None
                 OpBranchConditional %289 %290 %291

        %290 =     OpLabel
        %292 =       OpLoad %mat3v3float %m3x3
        %293 =       OpCompositeExtract %v3float %292 0
        %294 =       OpFSub %v3float %293 %243
        %295 =       OpCompositeExtract %v3float %292 1
        %296 =       OpFSub %v3float %295 %243
        %297 =       OpCompositeExtract %v3float %292 2
        %298 =       OpFSub %v3float %297 %243
        %299 =       OpCompositeConstruct %mat3v3float %294 %296 %298
                     OpStore %m3x3 %299
        %300 =       OpFOrdEqual %v3bool %293 %274
        %301 =       OpAll %bool %300
        %302 =       OpFOrdEqual %v3bool %295 %275
        %303 =       OpAll %bool %302
        %304 =       OpLogicalAnd %bool %301 %303
        %305 =       OpFOrdEqual %v3bool %297 %276
        %306 =       OpAll %bool %305
        %307 =       OpLogicalAnd %bool %304 %306
                     OpBranch %291

        %291 = OpLabel
        %308 =   OpPhi %bool %false %271 %307 %290
                 OpStore %ok %308
                 OpSelectionMerge %310 None
                 OpBranchConditional %308 %309 %310

        %309 =     OpLabel
        %311 =       OpLoad %mat3v3float %m3x3
        %312 =       OpCompositeExtract %v3float %311 0
        %313 =       OpFOrdEqual %v3bool %312 %256
        %314 =       OpAll %bool %313
        %315 =       OpCompositeExtract %v3float %311 1
        %316 =       OpFOrdEqual %v3bool %315 %257
        %317 =       OpAll %bool %316
        %318 =       OpLogicalAnd %bool %314 %317
        %319 =       OpCompositeExtract %v3float %311 2
        %320 =       OpFOrdEqual %v3bool %319 %258
        %321 =       OpAll %bool %320
        %322 =       OpLogicalAnd %bool %318 %321
                     OpBranch %310

        %310 = OpLabel
        %323 =   OpPhi %bool %false %291 %322 %309
                 OpStore %ok %323
        %324 =   OpLoad %mat3v3float %m3x3
        %325 =   OpCompositeExtract %v3float %324 0
        %326 =   OpFSub %v3float %325 %243
        %327 =   OpCompositeExtract %v3float %324 1
        %328 =   OpFSub %v3float %327 %243
        %329 =   OpCompositeExtract %v3float %324 2
        %330 =   OpFSub %v3float %329 %243
        %331 =   OpCompositeConstruct %mat3v3float %326 %328 %330
                 OpStore %m3x3 %331
                 OpSelectionMerge %333 None
                 OpBranchConditional %323 %332 %333

        %332 =     OpLabel
        %334 =       OpFOrdEqual %v3bool %326 %239
        %335 =       OpAll %bool %334
        %336 =       OpFOrdEqual %v3bool %328 %240
        %337 =       OpAll %bool %336
        %338 =       OpLogicalAnd %bool %335 %337
        %339 =       OpFOrdEqual %v3bool %330 %241
        %340 =       OpAll %bool %339
        %341 =       OpLogicalAnd %bool %338 %340
                     OpBranch %333

        %333 = OpLabel
        %342 =   OpPhi %bool %false %310 %341 %332
                 OpStore %ok %342
                 OpSelectionMerge %347 None
                 OpBranchConditional %342 %345 %346

        %345 =     OpLabel
        %348 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %350 =       OpLoad %v4float %348           ; RelaxedPrecision
                     OpStore %343 %350
                     OpBranch %347

        %346 =     OpLabel
        %351 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %352 =       OpLoad %v4float %351           ; RelaxedPrecision
                     OpStore %343 %352
                     OpBranch %347

        %347 = OpLabel
        %353 =   OpLoad %v4float %343               ; RelaxedPrecision
                 OpReturnValue %353
               OpFunctionEnd
