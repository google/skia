               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testMatrix2x2"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %ok "ok"
               OpName %i "i"
               OpName %f "f"
               OpName %f2 "f2"
               OpName %i4 "i4"
               OpName %m3x3 "m3x3"
               OpName %iv "iv"
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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %92 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %294 RelaxedPrecision
               OpDecorate %295 RelaxedPrecision
               OpDecorate %302 RelaxedPrecision
               OpDecorate %303 RelaxedPrecision
               OpDecorate %304 RelaxedPrecision
               OpDecorate %311 RelaxedPrecision
               OpDecorate %312 RelaxedPrecision
               OpDecorate %366 RelaxedPrecision
               OpDecorate %368 RelaxedPrecision
               OpDecorate %369 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %21 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
        %int = OpTypeInt 32 1
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
         %87 = OpConstantComposite %v2float %float_0_5 %float_0_5
      %int_0 = OpConstant %int 0
        %123 = OpConstantComposite %v2float %float_1 %float_1
        %128 = OpConstantComposite %v2float %float_1_5 %float_1_5
     %v2bool = OpTypeVector %bool 2
        %136 = OpConstantComposite %v2float %float_2_5 %float_2_5
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
      %int_8 = OpConstant %int 8
      %int_9 = OpConstant %int 9
     %int_10 = OpConstant %int 10
        %160 = OpConstantComposite %v4int %int_7 %int_8 %int_9 %int_10
        %161 = OpConstantComposite %v4int %int_1 %int_1 %int_1 %int_1
     %int_11 = OpConstant %int 11
        %166 = OpConstantComposite %v4int %int_8 %int_9 %int_10 %int_11
     %v4bool = OpTypeVector %bool 4
     %int_12 = OpConstant %int 12
        %175 = OpConstantComposite %v4int %int_9 %int_10 %int_11 %int_12
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
        %205 = OpConstantComposite %v3float %float_1 %float_2 %float_3
        %206 = OpConstantComposite %v3float %float_4 %float_5 %float_6
        %207 = OpConstantComposite %v3float %float_7 %float_8 %float_9
        %208 = OpConstantComposite %mat3v3float %205 %206 %207
        %209 = OpConstantComposite %v3float %float_1 %float_1 %float_1
        %210 = OpConstantComposite %mat3v3float %209 %209 %209
   %float_10 = OpConstant %float 10
        %218 = OpConstantComposite %v3float %float_2 %float_3 %float_4
        %219 = OpConstantComposite %v3float %float_5 %float_6 %float_7
        %220 = OpConstantComposite %v3float %float_8 %float_9 %float_10
        %221 = OpConstantComposite %mat3v3float %218 %219 %220
     %v3bool = OpTypeVector %bool 3
   %float_11 = OpConstant %float 11
        %239 = OpConstantComposite %v3float %float_3 %float_4 %float_5
        %240 = OpConstantComposite %v3float %float_6 %float_7 %float_8
        %241 = OpConstantComposite %v3float %float_9 %float_10 %float_11
        %242 = OpConstantComposite %mat3v3float %239 %240 %241
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
   %float_n1 = OpConstant %float -1
        %309 = OpConstantComposite %v4float %float_0 %float_n1 %float_0 %float_n1
   %float_n2 = OpConstant %float -2
   %float_n3 = OpConstant %float -3
   %float_n4 = OpConstant %float -4
        %321 = OpConstantComposite %v2float %float_n1 %float_n2
        %322 = OpConstantComposite %v2float %float_n3 %float_n4
        %323 = OpConstantComposite %mat2v2float %321 %322
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
      %int_2 = OpConstant %int 2
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
     %int_n5 = OpConstant %int -5
        %356 = OpConstantComposite %v2int %int_n5 %int_5
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %14
         %15 = OpLabel
         %18 = OpVariable %_ptr_Function_v2float Function
               OpStore %18 %17
         %20 = OpFunctionCall %v4float %main %18
               OpStore %sk_FragColor %20
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %21
         %22 = OpFunctionParameter %_ptr_Function_v2float
         %23 = OpLabel
         %ok = OpVariable %_ptr_Function_bool Function
          %i = OpVariable %_ptr_Function_int Function
          %f = OpVariable %_ptr_Function_float Function
         %f2 = OpVariable %_ptr_Function_v2float Function
         %i4 = OpVariable %_ptr_Function_v4int Function
       %m3x3 = OpVariable %_ptr_Function_mat3v3float Function
         %iv = OpVariable %_ptr_Function_v2int Function
        %360 = OpVariable %_ptr_Function_v4float Function
               OpStore %ok %true
               OpStore %i %int_5
         %33 = OpIAdd %int %int_5 %int_1
               OpStore %i %33
               OpSelectionMerge %36 None
               OpBranchConditional %true %35 %36
         %35 = OpLabel
         %38 = OpIEqual %bool %33 %int_6
               OpBranch %36
         %36 = OpLabel
         %39 = OpPhi %bool %false %23 %38 %35
               OpStore %ok %39
               OpSelectionMerge %41 None
               OpBranchConditional %39 %40 %41
         %40 = OpLabel
         %42 = OpIAdd %int %33 %int_1
               OpStore %i %42
         %44 = OpIEqual %bool %42 %int_7
               OpBranch %41
         %41 = OpLabel
         %45 = OpPhi %bool %false %36 %44 %40
               OpStore %ok %45
               OpSelectionMerge %47 None
               OpBranchConditional %45 %46 %47
         %46 = OpLabel
         %48 = OpLoad %int %i
         %49 = OpISub %int %48 %int_1
               OpStore %i %49
         %50 = OpIEqual %bool %49 %int_6
               OpBranch %47
         %47 = OpLabel
         %51 = OpPhi %bool %false %41 %50 %46
               OpStore %ok %51
         %52 = OpLoad %int %i
         %53 = OpISub %int %52 %int_1
               OpStore %i %53
               OpSelectionMerge %55 None
               OpBranchConditional %51 %54 %55
         %54 = OpLabel
         %56 = OpIEqual %bool %53 %int_5
               OpBranch %55
         %55 = OpLabel
         %57 = OpPhi %bool %false %47 %56 %54
               OpStore %ok %57
               OpStore %f %float_0_5
         %62 = OpFAdd %float %float_0_5 %float_1
               OpStore %f %62
               OpSelectionMerge %64 None
               OpBranchConditional %57 %63 %64
         %63 = OpLabel
         %66 = OpFOrdEqual %bool %62 %float_1_5
               OpBranch %64
         %64 = OpLabel
         %67 = OpPhi %bool %false %55 %66 %63
               OpStore %ok %67
               OpSelectionMerge %69 None
               OpBranchConditional %67 %68 %69
         %68 = OpLabel
         %70 = OpFAdd %float %62 %float_1
               OpStore %f %70
         %72 = OpFOrdEqual %bool %70 %float_2_5
               OpBranch %69
         %69 = OpLabel
         %73 = OpPhi %bool %false %64 %72 %68
               OpStore %ok %73
               OpSelectionMerge %75 None
               OpBranchConditional %73 %74 %75
         %74 = OpLabel
         %76 = OpLoad %float %f
         %77 = OpFSub %float %76 %float_1
               OpStore %f %77
         %78 = OpFOrdEqual %bool %77 %float_1_5
               OpBranch %75
         %75 = OpLabel
         %79 = OpPhi %bool %false %69 %78 %74
               OpStore %ok %79
         %80 = OpLoad %float %f
         %81 = OpFSub %float %80 %float_1
               OpStore %f %81
               OpSelectionMerge %83 None
               OpBranchConditional %79 %82 %83
         %82 = OpLabel
         %84 = OpFOrdEqual %bool %81 %float_0_5
               OpBranch %83
         %83 = OpLabel
         %85 = OpPhi %bool %false %75 %84 %82
               OpStore %ok %85
               OpStore %f2 %87
         %88 = OpAccessChain %_ptr_Function_float %f2 %int_0
         %90 = OpLoad %float %88
         %91 = OpFAdd %float %90 %float_1
               OpStore %88 %91
         %92 = OpLoad %bool %ok
               OpSelectionMerge %94 None
               OpBranchConditional %92 %93 %94
         %93 = OpLabel
         %95 = OpLoad %v2float %f2
         %96 = OpCompositeExtract %float %95 0
         %97 = OpFOrdEqual %bool %96 %float_1_5
               OpBranch %94
         %94 = OpLabel
         %98 = OpPhi %bool %false %83 %97 %93
               OpStore %ok %98
               OpSelectionMerge %100 None
               OpBranchConditional %98 %99 %100
         %99 = OpLabel
        %101 = OpAccessChain %_ptr_Function_float %f2 %int_0
        %102 = OpLoad %float %101
        %103 = OpFAdd %float %102 %float_1
               OpStore %101 %103
        %104 = OpFOrdEqual %bool %103 %float_2_5
               OpBranch %100
        %100 = OpLabel
        %105 = OpPhi %bool %false %94 %104 %99
               OpStore %ok %105
               OpSelectionMerge %107 None
               OpBranchConditional %105 %106 %107
        %106 = OpLabel
        %108 = OpAccessChain %_ptr_Function_float %f2 %int_0
        %109 = OpLoad %float %108
        %110 = OpFSub %float %109 %float_1
               OpStore %108 %110
        %111 = OpFOrdEqual %bool %110 %float_1_5
               OpBranch %107
        %107 = OpLabel
        %112 = OpPhi %bool %false %100 %111 %106
               OpStore %ok %112
        %113 = OpAccessChain %_ptr_Function_float %f2 %int_0
        %114 = OpLoad %float %113
        %115 = OpFSub %float %114 %float_1
               OpStore %113 %115
        %116 = OpLoad %bool %ok
               OpSelectionMerge %118 None
               OpBranchConditional %116 %117 %118
        %117 = OpLabel
        %119 = OpLoad %v2float %f2
        %120 = OpCompositeExtract %float %119 0
        %121 = OpFOrdEqual %bool %120 %float_0_5
               OpBranch %118
        %118 = OpLabel
        %122 = OpPhi %bool %false %107 %121 %117
               OpStore %ok %122
        %124 = OpLoad %v2float %f2
        %125 = OpFAdd %v2float %124 %123
               OpStore %f2 %125
               OpSelectionMerge %127 None
               OpBranchConditional %122 %126 %127
        %126 = OpLabel
        %129 = OpFOrdEqual %v2bool %125 %128
        %131 = OpAll %bool %129
               OpBranch %127
        %127 = OpLabel
        %132 = OpPhi %bool %false %118 %131 %126
               OpStore %ok %132
               OpSelectionMerge %134 None
               OpBranchConditional %132 %133 %134
        %133 = OpLabel
        %135 = OpFAdd %v2float %125 %123
               OpStore %f2 %135
        %137 = OpFOrdEqual %v2bool %135 %136
        %138 = OpAll %bool %137
               OpBranch %134
        %134 = OpLabel
        %139 = OpPhi %bool %false %127 %138 %133
               OpStore %ok %139
               OpSelectionMerge %141 None
               OpBranchConditional %139 %140 %141
        %140 = OpLabel
        %142 = OpLoad %v2float %f2
        %143 = OpFSub %v2float %142 %123
               OpStore %f2 %143
        %144 = OpFOrdEqual %v2bool %143 %128
        %145 = OpAll %bool %144
               OpBranch %141
        %141 = OpLabel
        %146 = OpPhi %bool %false %134 %145 %140
               OpStore %ok %146
        %147 = OpLoad %v2float %f2
        %148 = OpFSub %v2float %147 %123
               OpStore %f2 %148
               OpSelectionMerge %150 None
               OpBranchConditional %146 %149 %150
        %149 = OpLabel
        %151 = OpFOrdEqual %v2bool %148 %87
        %152 = OpAll %bool %151
               OpBranch %150
        %150 = OpLabel
        %153 = OpPhi %bool %false %141 %152 %149
               OpStore %ok %153
               OpStore %i4 %160
        %162 = OpIAdd %v4int %160 %161
               OpStore %i4 %162
               OpSelectionMerge %164 None
               OpBranchConditional %153 %163 %164
        %163 = OpLabel
        %167 = OpIEqual %v4bool %162 %166
        %169 = OpAll %bool %167
               OpBranch %164
        %164 = OpLabel
        %170 = OpPhi %bool %false %150 %169 %163
               OpStore %ok %170
               OpSelectionMerge %172 None
               OpBranchConditional %170 %171 %172
        %171 = OpLabel
        %173 = OpIAdd %v4int %162 %161
               OpStore %i4 %173
        %176 = OpIEqual %v4bool %173 %175
        %177 = OpAll %bool %176
               OpBranch %172
        %172 = OpLabel
        %178 = OpPhi %bool %false %164 %177 %171
               OpStore %ok %178
               OpSelectionMerge %180 None
               OpBranchConditional %178 %179 %180
        %179 = OpLabel
        %181 = OpLoad %v4int %i4
        %182 = OpISub %v4int %181 %161
               OpStore %i4 %182
        %183 = OpIEqual %v4bool %182 %166
        %184 = OpAll %bool %183
               OpBranch %180
        %180 = OpLabel
        %185 = OpPhi %bool %false %172 %184 %179
               OpStore %ok %185
        %186 = OpLoad %v4int %i4
        %187 = OpISub %v4int %186 %161
               OpStore %i4 %187
               OpSelectionMerge %189 None
               OpBranchConditional %185 %188 %189
        %188 = OpLabel
        %190 = OpIEqual %v4bool %187 %160
        %191 = OpAll %bool %190
               OpBranch %189
        %189 = OpLabel
        %192 = OpPhi %bool %false %180 %191 %188
               OpStore %ok %192
               OpStore %m3x3 %208
        %211 = OpFAdd %v3float %205 %209
        %212 = OpFAdd %v3float %206 %209
        %213 = OpFAdd %v3float %207 %209
        %214 = OpCompositeConstruct %mat3v3float %211 %212 %213
               OpStore %m3x3 %214
               OpSelectionMerge %216 None
               OpBranchConditional %192 %215 %216
        %215 = OpLabel
        %223 = OpFOrdEqual %v3bool %211 %218
        %224 = OpAll %bool %223
        %225 = OpFOrdEqual %v3bool %212 %219
        %226 = OpAll %bool %225
        %227 = OpLogicalAnd %bool %224 %226
        %228 = OpFOrdEqual %v3bool %213 %220
        %229 = OpAll %bool %228
        %230 = OpLogicalAnd %bool %227 %229
               OpBranch %216
        %216 = OpLabel
        %231 = OpPhi %bool %false %189 %230 %215
               OpStore %ok %231
               OpSelectionMerge %233 None
               OpBranchConditional %231 %232 %233
        %232 = OpLabel
        %234 = OpFAdd %v3float %211 %209
        %235 = OpFAdd %v3float %212 %209
        %236 = OpFAdd %v3float %213 %209
        %237 = OpCompositeConstruct %mat3v3float %234 %235 %236
               OpStore %m3x3 %237
        %243 = OpFOrdEqual %v3bool %234 %239
        %244 = OpAll %bool %243
        %245 = OpFOrdEqual %v3bool %235 %240
        %246 = OpAll %bool %245
        %247 = OpLogicalAnd %bool %244 %246
        %248 = OpFOrdEqual %v3bool %236 %241
        %249 = OpAll %bool %248
        %250 = OpLogicalAnd %bool %247 %249
               OpBranch %233
        %233 = OpLabel
        %251 = OpPhi %bool %false %216 %250 %232
               OpStore %ok %251
               OpSelectionMerge %253 None
               OpBranchConditional %251 %252 %253
        %252 = OpLabel
        %254 = OpLoad %mat3v3float %m3x3
        %255 = OpCompositeExtract %v3float %254 0
        %256 = OpFSub %v3float %255 %209
        %257 = OpCompositeExtract %v3float %254 1
        %258 = OpFSub %v3float %257 %209
        %259 = OpCompositeExtract %v3float %254 2
        %260 = OpFSub %v3float %259 %209
        %261 = OpCompositeConstruct %mat3v3float %256 %258 %260
               OpStore %m3x3 %261
        %262 = OpFOrdEqual %v3bool %256 %218
        %263 = OpAll %bool %262
        %264 = OpFOrdEqual %v3bool %258 %219
        %265 = OpAll %bool %264
        %266 = OpLogicalAnd %bool %263 %265
        %267 = OpFOrdEqual %v3bool %260 %220
        %268 = OpAll %bool %267
        %269 = OpLogicalAnd %bool %266 %268
               OpBranch %253
        %253 = OpLabel
        %270 = OpPhi %bool %false %233 %269 %252
               OpStore %ok %270
        %271 = OpLoad %mat3v3float %m3x3
        %272 = OpCompositeExtract %v3float %271 0
        %273 = OpFSub %v3float %272 %209
        %274 = OpCompositeExtract %v3float %271 1
        %275 = OpFSub %v3float %274 %209
        %276 = OpCompositeExtract %v3float %271 2
        %277 = OpFSub %v3float %276 %209
        %278 = OpCompositeConstruct %mat3v3float %273 %275 %277
               OpStore %m3x3 %278
               OpSelectionMerge %280 None
               OpBranchConditional %270 %279 %280
        %279 = OpLabel
        %281 = OpFOrdEqual %v3bool %273 %205
        %282 = OpAll %bool %281
        %283 = OpFOrdEqual %v3bool %275 %206
        %284 = OpAll %bool %283
        %285 = OpLogicalAnd %bool %282 %284
        %286 = OpFOrdEqual %v3bool %277 %207
        %287 = OpAll %bool %286
        %288 = OpLogicalAnd %bool %285 %287
               OpBranch %280
        %280 = OpLabel
        %289 = OpPhi %bool %false %253 %288 %279
               OpStore %ok %289
               OpSelectionMerge %291 None
               OpBranchConditional %289 %290 %291
        %290 = OpLabel
        %292 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %294 = OpLoad %v4float %292
        %295 = OpCompositeExtract %float %294 0
        %296 = OpFUnordNotEqual %bool %295 %float_1
               OpBranch %291
        %291 = OpLabel
        %297 = OpPhi %bool %false %280 %296 %290
               OpStore %ok %297
               OpSelectionMerge %299 None
               OpBranchConditional %297 %298 %299
        %298 = OpLabel
        %301 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %302 = OpLoad %v4float %301
        %303 = OpCompositeExtract %float %302 1
        %304 = OpFNegate %float %303
        %305 = OpFOrdEqual %bool %float_n1 %304
               OpBranch %299
        %299 = OpLabel
        %306 = OpPhi %bool %false %291 %305 %298
               OpStore %ok %306
               OpSelectionMerge %308 None
               OpBranchConditional %306 %307 %308
        %307 = OpLabel
        %310 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %311 = OpLoad %v4float %310
        %312 = OpFNegate %v4float %311
        %313 = OpFOrdEqual %v4bool %309 %312
        %314 = OpAll %bool %313
               OpBranch %308
        %308 = OpLabel
        %315 = OpPhi %bool %false %299 %314 %307
               OpStore %ok %315
               OpSelectionMerge %317 None
               OpBranchConditional %315 %316 %317
        %316 = OpLabel
        %324 = OpAccessChain %_ptr_Uniform_mat2v2float %7 %int_2
        %327 = OpLoad %mat2v2float %324
        %328 = OpCompositeExtract %v2float %327 0
        %329 = OpFNegate %v2float %328
        %330 = OpCompositeExtract %v2float %327 1
        %331 = OpFNegate %v2float %330
        %332 = OpCompositeConstruct %mat2v2float %329 %331
        %333 = OpFOrdEqual %v2bool %321 %329
        %334 = OpAll %bool %333
        %335 = OpFOrdEqual %v2bool %322 %331
        %336 = OpAll %bool %335
        %337 = OpLogicalAnd %bool %334 %336
               OpBranch %317
        %317 = OpLabel
        %338 = OpPhi %bool %false %308 %337 %316
               OpStore %ok %338
        %342 = OpLoad %int %i
        %343 = OpLoad %int %i
        %344 = OpSNegate %int %343
        %345 = OpCompositeConstruct %v2int %342 %344
               OpStore %iv %345
               OpSelectionMerge %347 None
               OpBranchConditional %338 %346 %347
        %346 = OpLabel
        %348 = OpLoad %int %i
        %349 = OpSNegate %int %348
        %351 = OpIEqual %bool %349 %int_n5
               OpBranch %347
        %347 = OpLabel
        %352 = OpPhi %bool %false %317 %351 %346
               OpStore %ok %352
               OpSelectionMerge %354 None
               OpBranchConditional %352 %353 %354
        %353 = OpLabel
        %355 = OpSNegate %v2int %345
        %357 = OpIEqual %v2bool %355 %356
        %358 = OpAll %bool %357
               OpBranch %354
        %354 = OpLabel
        %359 = OpPhi %bool %false %347 %358 %353
               OpStore %ok %359
               OpSelectionMerge %364 None
               OpBranchConditional %359 %362 %363
        %362 = OpLabel
        %365 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %366 = OpLoad %v4float %365
               OpStore %360 %366
               OpBranch %364
        %363 = OpLabel
        %367 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %368 = OpLoad %v4float %367
               OpStore %360 %368
               OpBranch %364
        %364 = OpLabel
        %369 = OpLoad %v4float %360
               OpReturnValue %369
               OpFunctionEnd
