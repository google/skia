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
               OpName %main "main"
               OpName %ok "ok"
               OpName %i "i"
               OpName %f "f"
               OpName %f2 "f2"
               OpName %i4 "i4"
               OpName %m3x3 "m3x3"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %103 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %347 RelaxedPrecision
               OpDecorate %349 RelaxedPrecision
               OpDecorate %350 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
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
         %98 = OpConstantComposite %v2float %float_0_5 %float_0_5
      %int_0 = OpConstant %int 0
        %141 = OpConstantComposite %v2float %float_1 %float_1
        %146 = OpConstantComposite %v2float %float_1_5 %float_1_5
     %v2bool = OpTypeVector %bool 2
        %154 = OpConstantComposite %v2float %float_2_5 %float_2_5
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
      %int_8 = OpConstant %int 8
      %int_9 = OpConstant %int 9
     %int_10 = OpConstant %int 10
        %184 = OpConstantComposite %v4int %int_7 %int_8 %int_9 %int_10
        %185 = OpConstantComposite %v4int %int_1 %int_1 %int_1 %int_1
     %int_11 = OpConstant %int 11
        %191 = OpConstantComposite %v4int %int_8 %int_9 %int_10 %int_11
     %v4bool = OpTypeVector %bool 4
     %int_12 = OpConstant %int 12
        %200 = OpConstantComposite %v4int %int_9 %int_10 %int_11 %int_12
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
        %236 = OpConstantComposite %v3float %float_1 %float_2 %float_3
        %237 = OpConstantComposite %v3float %float_4 %float_5 %float_6
        %238 = OpConstantComposite %v3float %float_7 %float_8 %float_9
        %239 = OpConstantComposite %mat3v3float %236 %237 %238
        %240 = OpConstantComposite %v3float %float_1 %float_1 %float_1
        %241 = OpConstantComposite %mat3v3float %240 %240 %240
   %float_10 = OpConstant %float 10
        %253 = OpConstantComposite %v3float %float_2 %float_3 %float_4
        %254 = OpConstantComposite %v3float %float_5 %float_6 %float_7
        %255 = OpConstantComposite %v3float %float_8 %float_9 %float_10
        %256 = OpConstantComposite %mat3v3float %253 %254 %255
     %v3bool = OpTypeVector %bool 3
   %float_11 = OpConstant %float 11
        %271 = OpConstantComposite %v3float %float_3 %float_4 %float_5
        %272 = OpConstantComposite %v3float %float_6 %float_7 %float_8
        %273 = OpConstantComposite %v3float %float_9 %float_10 %float_11
        %274 = OpConstantComposite %mat3v3float %271 %272 %273
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
         %ok = OpVariable %_ptr_Function_bool Function
          %i = OpVariable %_ptr_Function_int Function
          %f = OpVariable %_ptr_Function_float Function
         %f2 = OpVariable %_ptr_Function_v2float Function
         %i4 = OpVariable %_ptr_Function_v4int Function
       %m3x3 = OpVariable %_ptr_Function_mat3v3float Function
        %340 = OpVariable %_ptr_Function_v4float Function
               OpStore %ok %true
               OpStore %i %int_5
         %32 = OpIAdd %int %int_5 %int_1
               OpStore %i %32
               OpSelectionMerge %35 None
               OpBranchConditional %true %34 %35
         %34 = OpLabel
         %36 = OpIAdd %int %32 %int_1
               OpStore %i %36
         %38 = OpIEqual %bool %32 %int_6
               OpBranch %35
         %35 = OpLabel
         %39 = OpPhi %bool %false %22 %38 %34
               OpStore %ok %39
               OpSelectionMerge %41 None
               OpBranchConditional %39 %40 %41
         %40 = OpLabel
         %42 = OpLoad %int %i
         %44 = OpIEqual %bool %42 %int_7
               OpBranch %41
         %41 = OpLabel
         %45 = OpPhi %bool %false %35 %44 %40
               OpStore %ok %45
               OpSelectionMerge %47 None
               OpBranchConditional %45 %46 %47
         %46 = OpLabel
         %48 = OpLoad %int %i
         %49 = OpISub %int %48 %int_1
               OpStore %i %49
         %50 = OpIEqual %bool %48 %int_7
               OpBranch %47
         %47 = OpLabel
         %51 = OpPhi %bool %false %41 %50 %46
               OpStore %ok %51
               OpSelectionMerge %53 None
               OpBranchConditional %51 %52 %53
         %52 = OpLabel
         %54 = OpLoad %int %i
         %55 = OpIEqual %bool %54 %int_6
               OpBranch %53
         %53 = OpLabel
         %56 = OpPhi %bool %false %47 %55 %52
               OpStore %ok %56
         %57 = OpLoad %int %i
         %58 = OpISub %int %57 %int_1
               OpStore %i %58
               OpSelectionMerge %60 None
               OpBranchConditional %56 %59 %60
         %59 = OpLabel
         %61 = OpIEqual %bool %58 %int_5
               OpBranch %60
         %60 = OpLabel
         %62 = OpPhi %bool %false %53 %61 %59
               OpStore %ok %62
               OpStore %f %float_0_5
         %67 = OpFAdd %float %float_0_5 %float_1
               OpStore %f %67
               OpSelectionMerge %69 None
               OpBranchConditional %62 %68 %69
         %68 = OpLabel
         %70 = OpFAdd %float %67 %float_1
               OpStore %f %70
         %72 = OpFOrdEqual %bool %67 %float_1_5
               OpBranch %69
         %69 = OpLabel
         %73 = OpPhi %bool %false %60 %72 %68
               OpStore %ok %73
               OpSelectionMerge %75 None
               OpBranchConditional %73 %74 %75
         %74 = OpLabel
         %76 = OpLoad %float %f
         %78 = OpFOrdEqual %bool %76 %float_2_5
               OpBranch %75
         %75 = OpLabel
         %79 = OpPhi %bool %false %69 %78 %74
               OpStore %ok %79
               OpSelectionMerge %81 None
               OpBranchConditional %79 %80 %81
         %80 = OpLabel
         %82 = OpLoad %float %f
         %83 = OpFSub %float %82 %float_1
               OpStore %f %83
         %84 = OpFOrdEqual %bool %82 %float_2_5
               OpBranch %81
         %81 = OpLabel
         %85 = OpPhi %bool %false %75 %84 %80
               OpStore %ok %85
               OpSelectionMerge %87 None
               OpBranchConditional %85 %86 %87
         %86 = OpLabel
         %88 = OpLoad %float %f
         %89 = OpFOrdEqual %bool %88 %float_1_5
               OpBranch %87
         %87 = OpLabel
         %90 = OpPhi %bool %false %81 %89 %86
               OpStore %ok %90
         %91 = OpLoad %float %f
         %92 = OpFSub %float %91 %float_1
               OpStore %f %92
               OpSelectionMerge %94 None
               OpBranchConditional %90 %93 %94
         %93 = OpLabel
         %95 = OpFOrdEqual %bool %92 %float_0_5
               OpBranch %94
         %94 = OpLabel
         %96 = OpPhi %bool %false %87 %95 %93
               OpStore %ok %96
               OpStore %f2 %98
         %99 = OpAccessChain %_ptr_Function_float %f2 %int_0
        %101 = OpLoad %float %99
        %102 = OpFAdd %float %101 %float_1
               OpStore %99 %102
        %103 = OpLoad %bool %ok
               OpSelectionMerge %105 None
               OpBranchConditional %103 %104 %105
        %104 = OpLabel
        %106 = OpAccessChain %_ptr_Function_float %f2 %int_0
        %107 = OpLoad %float %106
        %108 = OpFAdd %float %107 %float_1
               OpStore %106 %108
        %109 = OpFOrdEqual %bool %107 %float_1_5
               OpBranch %105
        %105 = OpLabel
        %110 = OpPhi %bool %false %94 %109 %104
               OpStore %ok %110
               OpSelectionMerge %112 None
               OpBranchConditional %110 %111 %112
        %111 = OpLabel
        %113 = OpLoad %v2float %f2
        %114 = OpCompositeExtract %float %113 0
        %115 = OpFOrdEqual %bool %114 %float_2_5
               OpBranch %112
        %112 = OpLabel
        %116 = OpPhi %bool %false %105 %115 %111
               OpStore %ok %116
               OpSelectionMerge %118 None
               OpBranchConditional %116 %117 %118
        %117 = OpLabel
        %119 = OpAccessChain %_ptr_Function_float %f2 %int_0
        %120 = OpLoad %float %119
        %121 = OpFSub %float %120 %float_1
               OpStore %119 %121
        %122 = OpFOrdEqual %bool %120 %float_2_5
               OpBranch %118
        %118 = OpLabel
        %123 = OpPhi %bool %false %112 %122 %117
               OpStore %ok %123
               OpSelectionMerge %125 None
               OpBranchConditional %123 %124 %125
        %124 = OpLabel
        %126 = OpLoad %v2float %f2
        %127 = OpCompositeExtract %float %126 0
        %128 = OpFOrdEqual %bool %127 %float_1_5
               OpBranch %125
        %125 = OpLabel
        %129 = OpPhi %bool %false %118 %128 %124
               OpStore %ok %129
        %130 = OpAccessChain %_ptr_Function_float %f2 %int_0
        %131 = OpLoad %float %130
        %132 = OpFSub %float %131 %float_1
               OpStore %130 %132
        %133 = OpLoad %bool %ok
               OpSelectionMerge %135 None
               OpBranchConditional %133 %134 %135
        %134 = OpLabel
        %136 = OpLoad %v2float %f2
        %137 = OpCompositeExtract %float %136 0
        %138 = OpFOrdEqual %bool %137 %float_0_5
               OpBranch %135
        %135 = OpLabel
        %139 = OpPhi %bool %false %125 %138 %134
               OpStore %ok %139
        %140 = OpLoad %v2float %f2
        %142 = OpFAdd %v2float %140 %141
               OpStore %f2 %142
               OpSelectionMerge %144 None
               OpBranchConditional %139 %143 %144
        %143 = OpLabel
        %145 = OpFAdd %v2float %142 %141
               OpStore %f2 %145
        %147 = OpFOrdEqual %v2bool %142 %146
        %149 = OpAll %bool %147
               OpBranch %144
        %144 = OpLabel
        %150 = OpPhi %bool %false %135 %149 %143
               OpStore %ok %150
               OpSelectionMerge %152 None
               OpBranchConditional %150 %151 %152
        %151 = OpLabel
        %153 = OpLoad %v2float %f2
        %155 = OpFOrdEqual %v2bool %153 %154
        %156 = OpAll %bool %155
               OpBranch %152
        %152 = OpLabel
        %157 = OpPhi %bool %false %144 %156 %151
               OpStore %ok %157
               OpSelectionMerge %159 None
               OpBranchConditional %157 %158 %159
        %158 = OpLabel
        %160 = OpLoad %v2float %f2
        %161 = OpFSub %v2float %160 %141
               OpStore %f2 %161
        %162 = OpFOrdEqual %v2bool %160 %154
        %163 = OpAll %bool %162
               OpBranch %159
        %159 = OpLabel
        %164 = OpPhi %bool %false %152 %163 %158
               OpStore %ok %164
               OpSelectionMerge %166 None
               OpBranchConditional %164 %165 %166
        %165 = OpLabel
        %167 = OpLoad %v2float %f2
        %168 = OpFOrdEqual %v2bool %167 %146
        %169 = OpAll %bool %168
               OpBranch %166
        %166 = OpLabel
        %170 = OpPhi %bool %false %159 %169 %165
               OpStore %ok %170
        %171 = OpLoad %v2float %f2
        %172 = OpFSub %v2float %171 %141
               OpStore %f2 %172
               OpSelectionMerge %174 None
               OpBranchConditional %170 %173 %174
        %173 = OpLabel
        %175 = OpFOrdEqual %v2bool %172 %98
        %176 = OpAll %bool %175
               OpBranch %174
        %174 = OpLabel
        %177 = OpPhi %bool %false %166 %176 %173
               OpStore %ok %177
               OpStore %i4 %184
        %186 = OpIAdd %v4int %184 %185
               OpStore %i4 %186
               OpSelectionMerge %188 None
               OpBranchConditional %177 %187 %188
        %187 = OpLabel
        %189 = OpIAdd %v4int %186 %185
               OpStore %i4 %189
        %192 = OpIEqual %v4bool %186 %191
        %194 = OpAll %bool %192
               OpBranch %188
        %188 = OpLabel
        %195 = OpPhi %bool %false %174 %194 %187
               OpStore %ok %195
               OpSelectionMerge %197 None
               OpBranchConditional %195 %196 %197
        %196 = OpLabel
        %198 = OpLoad %v4int %i4
        %201 = OpIEqual %v4bool %198 %200
        %202 = OpAll %bool %201
               OpBranch %197
        %197 = OpLabel
        %203 = OpPhi %bool %false %188 %202 %196
               OpStore %ok %203
               OpSelectionMerge %205 None
               OpBranchConditional %203 %204 %205
        %204 = OpLabel
        %206 = OpLoad %v4int %i4
        %207 = OpISub %v4int %206 %185
               OpStore %i4 %207
        %208 = OpIEqual %v4bool %206 %200
        %209 = OpAll %bool %208
               OpBranch %205
        %205 = OpLabel
        %210 = OpPhi %bool %false %197 %209 %204
               OpStore %ok %210
               OpSelectionMerge %212 None
               OpBranchConditional %210 %211 %212
        %211 = OpLabel
        %213 = OpLoad %v4int %i4
        %214 = OpIEqual %v4bool %213 %191
        %215 = OpAll %bool %214
               OpBranch %212
        %212 = OpLabel
        %216 = OpPhi %bool %false %205 %215 %211
               OpStore %ok %216
        %217 = OpLoad %v4int %i4
        %218 = OpISub %v4int %217 %185
               OpStore %i4 %218
               OpSelectionMerge %220 None
               OpBranchConditional %216 %219 %220
        %219 = OpLabel
        %221 = OpIEqual %v4bool %218 %184
        %222 = OpAll %bool %221
               OpBranch %220
        %220 = OpLabel
        %223 = OpPhi %bool %false %212 %222 %219
               OpStore %ok %223
               OpStore %m3x3 %239
        %242 = OpFAdd %v3float %236 %240
        %243 = OpFAdd %v3float %237 %240
        %244 = OpFAdd %v3float %238 %240
        %245 = OpCompositeConstruct %mat3v3float %242 %243 %244
               OpStore %m3x3 %245
               OpSelectionMerge %247 None
               OpBranchConditional %223 %246 %247
        %246 = OpLabel
        %248 = OpFAdd %v3float %242 %240
        %249 = OpFAdd %v3float %243 %240
        %250 = OpFAdd %v3float %244 %240
        %251 = OpCompositeConstruct %mat3v3float %248 %249 %250
               OpStore %m3x3 %251
        %258 = OpFOrdEqual %v3bool %242 %253
        %259 = OpAll %bool %258
        %260 = OpFOrdEqual %v3bool %243 %254
        %261 = OpAll %bool %260
        %262 = OpLogicalAnd %bool %259 %261
        %263 = OpFOrdEqual %v3bool %244 %255
        %264 = OpAll %bool %263
        %265 = OpLogicalAnd %bool %262 %264
               OpBranch %247
        %247 = OpLabel
        %266 = OpPhi %bool %false %220 %265 %246
               OpStore %ok %266
               OpSelectionMerge %268 None
               OpBranchConditional %266 %267 %268
        %267 = OpLabel
        %269 = OpLoad %mat3v3float %m3x3
        %275 = OpCompositeExtract %v3float %269 0
        %276 = OpFOrdEqual %v3bool %275 %271
        %277 = OpAll %bool %276
        %278 = OpCompositeExtract %v3float %269 1
        %279 = OpFOrdEqual %v3bool %278 %272
        %280 = OpAll %bool %279
        %281 = OpLogicalAnd %bool %277 %280
        %282 = OpCompositeExtract %v3float %269 2
        %283 = OpFOrdEqual %v3bool %282 %273
        %284 = OpAll %bool %283
        %285 = OpLogicalAnd %bool %281 %284
               OpBranch %268
        %268 = OpLabel
        %286 = OpPhi %bool %false %247 %285 %267
               OpStore %ok %286
               OpSelectionMerge %288 None
               OpBranchConditional %286 %287 %288
        %287 = OpLabel
        %289 = OpLoad %mat3v3float %m3x3
        %290 = OpCompositeExtract %v3float %289 0
        %291 = OpFSub %v3float %290 %240
        %292 = OpCompositeExtract %v3float %289 1
        %293 = OpFSub %v3float %292 %240
        %294 = OpCompositeExtract %v3float %289 2
        %295 = OpFSub %v3float %294 %240
        %296 = OpCompositeConstruct %mat3v3float %291 %293 %295
               OpStore %m3x3 %296
        %297 = OpFOrdEqual %v3bool %290 %271
        %298 = OpAll %bool %297
        %299 = OpFOrdEqual %v3bool %292 %272
        %300 = OpAll %bool %299
        %301 = OpLogicalAnd %bool %298 %300
        %302 = OpFOrdEqual %v3bool %294 %273
        %303 = OpAll %bool %302
        %304 = OpLogicalAnd %bool %301 %303
               OpBranch %288
        %288 = OpLabel
        %305 = OpPhi %bool %false %268 %304 %287
               OpStore %ok %305
               OpSelectionMerge %307 None
               OpBranchConditional %305 %306 %307
        %306 = OpLabel
        %308 = OpLoad %mat3v3float %m3x3
        %309 = OpCompositeExtract %v3float %308 0
        %310 = OpFOrdEqual %v3bool %309 %253
        %311 = OpAll %bool %310
        %312 = OpCompositeExtract %v3float %308 1
        %313 = OpFOrdEqual %v3bool %312 %254
        %314 = OpAll %bool %313
        %315 = OpLogicalAnd %bool %311 %314
        %316 = OpCompositeExtract %v3float %308 2
        %317 = OpFOrdEqual %v3bool %316 %255
        %318 = OpAll %bool %317
        %319 = OpLogicalAnd %bool %315 %318
               OpBranch %307
        %307 = OpLabel
        %320 = OpPhi %bool %false %288 %319 %306
               OpStore %ok %320
        %321 = OpLoad %mat3v3float %m3x3
        %322 = OpCompositeExtract %v3float %321 0
        %323 = OpFSub %v3float %322 %240
        %324 = OpCompositeExtract %v3float %321 1
        %325 = OpFSub %v3float %324 %240
        %326 = OpCompositeExtract %v3float %321 2
        %327 = OpFSub %v3float %326 %240
        %328 = OpCompositeConstruct %mat3v3float %323 %325 %327
               OpStore %m3x3 %328
               OpSelectionMerge %330 None
               OpBranchConditional %320 %329 %330
        %329 = OpLabel
        %331 = OpFOrdEqual %v3bool %323 %236
        %332 = OpAll %bool %331
        %333 = OpFOrdEqual %v3bool %325 %237
        %334 = OpAll %bool %333
        %335 = OpLogicalAnd %bool %332 %334
        %336 = OpFOrdEqual %v3bool %327 %238
        %337 = OpAll %bool %336
        %338 = OpLogicalAnd %bool %335 %337
               OpBranch %330
        %330 = OpLabel
        %339 = OpPhi %bool %false %307 %338 %329
               OpStore %ok %339
               OpSelectionMerge %344 None
               OpBranchConditional %339 %342 %343
        %342 = OpLabel
        %345 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %347 = OpLoad %v4float %345
               OpStore %340 %347
               OpBranch %344
        %343 = OpLabel
        %348 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %349 = OpLoad %v4float %348
               OpStore %340 %349
               OpBranch %344
        %344 = OpLabel
        %350 = OpLoad %v4float %340
               OpReturnValue %350
               OpFunctionEnd
