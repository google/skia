               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "colorBlack"
               OpMemberName %_UniformBuffer 3 "colorWhite"
               OpMemberName %_UniformBuffer 4 "testInputs"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %intGreen "intGreen"
               OpName %intRed "intRed"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 3 Offset 48
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 4 Offset 64
               OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %30 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %209 RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
               OpDecorate %212 RelaxedPrecision
               OpDecorate %214 RelaxedPrecision
               OpDecorate %215 RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %221 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %230 RelaxedPrecision
               OpDecorate %233 RelaxedPrecision
               OpDecorate %234 RelaxedPrecision
               OpDecorate %236 RelaxedPrecision
               OpDecorate %237 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %242 RelaxedPrecision
               OpDecorate %243 RelaxedPrecision
               OpDecorate %251 RelaxedPrecision
               OpDecorate %253 RelaxedPrecision
               OpDecorate %255 RelaxedPrecision
               OpDecorate %257 RelaxedPrecision
               OpDecorate %259 RelaxedPrecision
               OpDecorate %267 RelaxedPrecision
               OpDecorate %268 RelaxedPrecision
               OpDecorate %270 RelaxedPrecision
               OpDecorate %271 RelaxedPrecision
               OpDecorate %273 RelaxedPrecision
               OpDecorate %274 RelaxedPrecision
               OpDecorate %276 RelaxedPrecision
               OpDecorate %277 RelaxedPrecision
               OpDecorate %279 RelaxedPrecision
               OpDecorate %280 RelaxedPrecision
               OpDecorate %287 RelaxedPrecision
               OpDecorate %288 RelaxedPrecision
               OpDecorate %290 RelaxedPrecision
               OpDecorate %291 RelaxedPrecision
               OpDecorate %293 RelaxedPrecision
               OpDecorate %294 RelaxedPrecision
               OpDecorate %296 RelaxedPrecision
               OpDecorate %297 RelaxedPrecision
               OpDecorate %299 RelaxedPrecision
               OpDecorate %300 RelaxedPrecision
               OpDecorate %308 RelaxedPrecision
               OpDecorate %309 RelaxedPrecision
               OpDecorate %311 RelaxedPrecision
               OpDecorate %312 RelaxedPrecision
               OpDecorate %314 RelaxedPrecision
               OpDecorate %315 RelaxedPrecision
               OpDecorate %317 RelaxedPrecision
               OpDecorate %318 RelaxedPrecision
               OpDecorate %320 RelaxedPrecision
               OpDecorate %321 RelaxedPrecision
               OpDecorate %329 RelaxedPrecision
               OpDecorate %331 RelaxedPrecision
               OpDecorate %333 RelaxedPrecision
               OpDecorate %335 RelaxedPrecision
               OpDecorate %337 RelaxedPrecision
               OpDecorate %344 RelaxedPrecision
               OpDecorate %345 RelaxedPrecision
               OpDecorate %353 RelaxedPrecision
               OpDecorate %354 RelaxedPrecision
               OpDecorate %362 RelaxedPrecision
               OpDecorate %363 RelaxedPrecision
               OpDecorate %371 RelaxedPrecision
               OpDecorate %378 RelaxedPrecision
               OpDecorate %379 RelaxedPrecision
               OpDecorate %386 RelaxedPrecision
               OpDecorate %387 RelaxedPrecision
               OpDecorate %395 RelaxedPrecision
               OpDecorate %396 RelaxedPrecision
               OpDecorate %404 RelaxedPrecision
               OpDecorate %414 RelaxedPrecision
               OpDecorate %416 RelaxedPrecision
               OpDecorate %417 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
        %int = OpTypeInt 32 1
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
  %float_100 = OpConstant %float 100
      %int_1 = OpConstant %int 1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
      %v2int = OpTypeVector %int 2
     %v2bool = OpTypeVector %bool 2
         %69 = OpConstantComposite %v2bool %false %false
      %v3int = OpTypeVector %int 3
     %v3bool = OpTypeVector %bool 3
         %83 = OpConstantComposite %v3bool %false %false %false
     %v4bool = OpTypeVector %bool 4
         %94 = OpConstantComposite %v4bool %false %false %false %false
       %true = OpConstantTrue %bool
        %109 = OpConstantComposite %v2bool %true %true
        %121 = OpConstantComposite %v3bool %true %true %true
        %131 = OpConstantComposite %v4bool %true %true %true %true
    %int_100 = OpConstant %int 100
        %142 = OpConstantComposite %v2int %int_0 %int_100
        %149 = OpConstantComposite %v3int %int_0 %int_100 %int_0
        %156 = OpConstantComposite %v4int %int_0 %int_100 %int_0 %int_100
        %166 = OpConstantComposite %v2int %int_100 %int_0
        %173 = OpConstantComposite %v3int %int_100 %int_0 %int_0
        %180 = OpConstantComposite %v4int %int_100 %int_0 %int_0 %int_100
    %v3float = OpTypeVector %float 3
    %float_1 = OpConstant %float 1
        %351 = OpConstantComposite %v2float %float_0 %float_1
        %360 = OpConstantComposite %v3float %float_0 %float_1 %float_0
        %369 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
        %384 = OpConstantComposite %v2float %float_1 %float_0
        %393 = OpConstantComposite %v3float %float_1 %float_0 %float_0
        %402 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
   %intGreen = OpVariable %_ptr_Function_v4int Function
     %intRed = OpVariable %_ptr_Function_v4int Function
        %408 = OpVariable %_ptr_Function_v4float Function
         %27 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %30 = OpLoad %v4float %27
         %32 = OpVectorTimesScalar %v4float %30 %float_100
         %33 = OpCompositeExtract %float %32 0
         %34 = OpConvertFToS %int %33
         %35 = OpCompositeExtract %float %32 1
         %36 = OpConvertFToS %int %35
         %37 = OpCompositeExtract %float %32 2
         %38 = OpConvertFToS %int %37
         %39 = OpCompositeExtract %float %32 3
         %40 = OpConvertFToS %int %39
         %41 = OpCompositeConstruct %v4int %34 %36 %38 %40
               OpStore %intGreen %41
         %43 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %45 = OpLoad %v4float %43
         %46 = OpVectorTimesScalar %v4float %45 %float_100
         %47 = OpCompositeExtract %float %46 0
         %48 = OpConvertFToS %int %47
         %49 = OpCompositeExtract %float %46 1
         %50 = OpConvertFToS %int %49
         %51 = OpCompositeExtract %float %46 2
         %52 = OpConvertFToS %int %51
         %53 = OpCompositeExtract %float %46 3
         %54 = OpConvertFToS %int %53
         %55 = OpCompositeConstruct %v4int %48 %50 %52 %54
               OpStore %intRed %55
         %59 = OpCompositeExtract %int %41 0
         %60 = OpCompositeExtract %int %55 0
         %58 = OpSelect %int %false %60 %59
         %61 = OpIEqual %bool %58 %59
               OpSelectionMerge %63 None
               OpBranchConditional %61 %62 %63
         %62 = OpLabel
         %65 = OpVectorShuffle %v2int %41 %41 0 1
         %67 = OpVectorShuffle %v2int %55 %55 0 1
         %70 = OpVectorShuffle %v2int %41 %41 0 1
         %71 = OpVectorShuffle %v2int %55 %55 0 1
         %64 = OpSelect %v2int %69 %71 %70
         %72 = OpVectorShuffle %v2int %41 %41 0 1
         %73 = OpIEqual %v2bool %64 %72
         %74 = OpAll %bool %73
               OpBranch %63
         %63 = OpLabel
         %75 = OpPhi %bool %false %22 %74 %62
               OpSelectionMerge %77 None
               OpBranchConditional %75 %76 %77
         %76 = OpLabel
         %79 = OpVectorShuffle %v3int %41 %41 0 1 2
         %81 = OpVectorShuffle %v3int %55 %55 0 1 2
         %84 = OpVectorShuffle %v3int %41 %41 0 1 2
         %85 = OpVectorShuffle %v3int %55 %55 0 1 2
         %78 = OpSelect %v3int %83 %85 %84
         %86 = OpVectorShuffle %v3int %41 %41 0 1 2
         %87 = OpIEqual %v3bool %78 %86
         %88 = OpAll %bool %87
               OpBranch %77
         %77 = OpLabel
         %89 = OpPhi %bool %false %63 %88 %76
               OpSelectionMerge %91 None
               OpBranchConditional %89 %90 %91
         %90 = OpLabel
         %92 = OpSelect %v4int %94 %55 %41
         %95 = OpIEqual %v4bool %92 %41
         %96 = OpAll %bool %95
               OpBranch %91
         %91 = OpLabel
         %97 = OpPhi %bool %false %77 %96 %90
               OpSelectionMerge %99 None
               OpBranchConditional %97 %98 %99
         %98 = OpLabel
        %100 = OpSelect %int %true %60 %59
        %102 = OpIEqual %bool %100 %60
               OpBranch %99
         %99 = OpLabel
        %103 = OpPhi %bool %false %91 %102 %98
               OpSelectionMerge %105 None
               OpBranchConditional %103 %104 %105
        %104 = OpLabel
        %107 = OpVectorShuffle %v2int %41 %41 0 1
        %108 = OpVectorShuffle %v2int %55 %55 0 1
        %110 = OpVectorShuffle %v2int %41 %41 0 1
        %111 = OpVectorShuffle %v2int %55 %55 0 1
        %106 = OpSelect %v2int %109 %111 %110
        %112 = OpVectorShuffle %v2int %55 %55 0 1
        %113 = OpIEqual %v2bool %106 %112
        %114 = OpAll %bool %113
               OpBranch %105
        %105 = OpLabel
        %115 = OpPhi %bool %false %99 %114 %104
               OpSelectionMerge %117 None
               OpBranchConditional %115 %116 %117
        %116 = OpLabel
        %119 = OpVectorShuffle %v3int %41 %41 0 1 2
        %120 = OpVectorShuffle %v3int %55 %55 0 1 2
        %122 = OpVectorShuffle %v3int %41 %41 0 1 2
        %123 = OpVectorShuffle %v3int %55 %55 0 1 2
        %118 = OpSelect %v3int %121 %123 %122
        %124 = OpVectorShuffle %v3int %55 %55 0 1 2
        %125 = OpIEqual %v3bool %118 %124
        %126 = OpAll %bool %125
               OpBranch %117
        %117 = OpLabel
        %127 = OpPhi %bool %false %105 %126 %116
               OpSelectionMerge %129 None
               OpBranchConditional %127 %128 %129
        %128 = OpLabel
        %130 = OpSelect %v4int %131 %55 %41
        %132 = OpIEqual %v4bool %130 %55
        %133 = OpAll %bool %132
               OpBranch %129
        %129 = OpLabel
        %134 = OpPhi %bool %false %117 %133 %128
               OpSelectionMerge %136 None
               OpBranchConditional %134 %135 %136
        %135 = OpLabel
        %137 = OpIEqual %bool %int_0 %59
               OpBranch %136
        %136 = OpLabel
        %138 = OpPhi %bool %false %129 %137 %135
               OpSelectionMerge %140 None
               OpBranchConditional %138 %139 %140
        %139 = OpLabel
        %143 = OpVectorShuffle %v2int %41 %41 0 1
        %144 = OpIEqual %v2bool %142 %143
        %145 = OpAll %bool %144
               OpBranch %140
        %140 = OpLabel
        %146 = OpPhi %bool %false %136 %145 %139
               OpSelectionMerge %148 None
               OpBranchConditional %146 %147 %148
        %147 = OpLabel
        %150 = OpVectorShuffle %v3int %41 %41 0 1 2
        %151 = OpIEqual %v3bool %149 %150
        %152 = OpAll %bool %151
               OpBranch %148
        %148 = OpLabel
        %153 = OpPhi %bool %false %140 %152 %147
               OpSelectionMerge %155 None
               OpBranchConditional %153 %154 %155
        %154 = OpLabel
        %157 = OpIEqual %v4bool %156 %41
        %158 = OpAll %bool %157
               OpBranch %155
        %155 = OpLabel
        %159 = OpPhi %bool %false %148 %158 %154
               OpSelectionMerge %161 None
               OpBranchConditional %159 %160 %161
        %160 = OpLabel
        %162 = OpIEqual %bool %int_100 %60
               OpBranch %161
        %161 = OpLabel
        %163 = OpPhi %bool %false %155 %162 %160
               OpSelectionMerge %165 None
               OpBranchConditional %163 %164 %165
        %164 = OpLabel
        %167 = OpVectorShuffle %v2int %55 %55 0 1
        %168 = OpIEqual %v2bool %166 %167
        %169 = OpAll %bool %168
               OpBranch %165
        %165 = OpLabel
        %170 = OpPhi %bool %false %161 %169 %164
               OpSelectionMerge %172 None
               OpBranchConditional %170 %171 %172
        %171 = OpLabel
        %174 = OpVectorShuffle %v3int %55 %55 0 1 2
        %175 = OpIEqual %v3bool %173 %174
        %176 = OpAll %bool %175
               OpBranch %172
        %172 = OpLabel
        %177 = OpPhi %bool %false %165 %176 %171
               OpSelectionMerge %179 None
               OpBranchConditional %177 %178 %179
        %178 = OpLabel
        %181 = OpIEqual %v4bool %180 %55
        %182 = OpAll %bool %181
               OpBranch %179
        %179 = OpLabel
        %183 = OpPhi %bool %false %172 %182 %178
               OpSelectionMerge %185 None
               OpBranchConditional %183 %184 %185
        %184 = OpLabel
        %187 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %188 = OpLoad %v4float %187
        %189 = OpCompositeExtract %float %188 0
        %190 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %191 = OpLoad %v4float %190
        %192 = OpCompositeExtract %float %191 0
        %193 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %194 = OpLoad %v4float %193
        %195 = OpCompositeExtract %float %194 0
        %196 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %197 = OpLoad %v4float %196
        %198 = OpCompositeExtract %float %197 0
        %186 = OpSelect %float %false %198 %195
        %199 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %200 = OpLoad %v4float %199
        %201 = OpCompositeExtract %float %200 0
        %202 = OpFOrdEqual %bool %186 %201
               OpBranch %185
        %185 = OpLabel
        %203 = OpPhi %bool %false %179 %202 %184
               OpSelectionMerge %205 None
               OpBranchConditional %203 %204 %205
        %204 = OpLabel
        %207 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %208 = OpLoad %v4float %207
        %209 = OpVectorShuffle %v2float %208 %208 0 1
        %210 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %211 = OpLoad %v4float %210
        %212 = OpVectorShuffle %v2float %211 %211 0 1
        %213 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %214 = OpLoad %v4float %213
        %215 = OpVectorShuffle %v2float %214 %214 0 1
        %216 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %217 = OpLoad %v4float %216
        %218 = OpVectorShuffle %v2float %217 %217 0 1
        %206 = OpSelect %v2float %69 %218 %215
        %219 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %220 = OpLoad %v4float %219
        %221 = OpVectorShuffle %v2float %220 %220 0 1
        %222 = OpFOrdEqual %v2bool %206 %221
        %223 = OpAll %bool %222
               OpBranch %205
        %205 = OpLabel
        %224 = OpPhi %bool %false %185 %223 %204
               OpSelectionMerge %226 None
               OpBranchConditional %224 %225 %226
        %225 = OpLabel
        %228 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %229 = OpLoad %v4float %228
        %230 = OpVectorShuffle %v3float %229 %229 0 1 2
        %232 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %233 = OpLoad %v4float %232
        %234 = OpVectorShuffle %v3float %233 %233 0 1 2
        %235 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %236 = OpLoad %v4float %235
        %237 = OpVectorShuffle %v3float %236 %236 0 1 2
        %238 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %239 = OpLoad %v4float %238
        %240 = OpVectorShuffle %v3float %239 %239 0 1 2
        %227 = OpSelect %v3float %83 %240 %237
        %241 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %242 = OpLoad %v4float %241
        %243 = OpVectorShuffle %v3float %242 %242 0 1 2
        %244 = OpFOrdEqual %v3bool %227 %243
        %245 = OpAll %bool %244
               OpBranch %226
        %226 = OpLabel
        %246 = OpPhi %bool %false %205 %245 %225
               OpSelectionMerge %248 None
               OpBranchConditional %246 %247 %248
        %247 = OpLabel
        %250 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %251 = OpLoad %v4float %250
        %252 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %253 = OpLoad %v4float %252
        %254 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %255 = OpLoad %v4float %254
        %256 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %257 = OpLoad %v4float %256
        %249 = OpSelect %v4float %94 %257 %255
        %258 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %259 = OpLoad %v4float %258
        %260 = OpFOrdEqual %v4bool %249 %259
        %261 = OpAll %bool %260
               OpBranch %248
        %248 = OpLabel
        %262 = OpPhi %bool %false %226 %261 %247
               OpSelectionMerge %264 None
               OpBranchConditional %262 %263 %264
        %263 = OpLabel
        %266 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %267 = OpLoad %v4float %266
        %268 = OpCompositeExtract %float %267 0
        %269 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %270 = OpLoad %v4float %269
        %271 = OpCompositeExtract %float %270 0
        %272 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %273 = OpLoad %v4float %272
        %274 = OpCompositeExtract %float %273 0
        %275 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %276 = OpLoad %v4float %275
        %277 = OpCompositeExtract %float %276 0
        %265 = OpSelect %float %true %277 %274
        %278 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %279 = OpLoad %v4float %278
        %280 = OpCompositeExtract %float %279 0
        %281 = OpFOrdEqual %bool %265 %280
               OpBranch %264
        %264 = OpLabel
        %282 = OpPhi %bool %false %248 %281 %263
               OpSelectionMerge %284 None
               OpBranchConditional %282 %283 %284
        %283 = OpLabel
        %286 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %287 = OpLoad %v4float %286
        %288 = OpVectorShuffle %v2float %287 %287 0 1
        %289 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %290 = OpLoad %v4float %289
        %291 = OpVectorShuffle %v2float %290 %290 0 1
        %292 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %293 = OpLoad %v4float %292
        %294 = OpVectorShuffle %v2float %293 %293 0 1
        %295 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %296 = OpLoad %v4float %295
        %297 = OpVectorShuffle %v2float %296 %296 0 1
        %285 = OpSelect %v2float %109 %297 %294
        %298 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %299 = OpLoad %v4float %298
        %300 = OpVectorShuffle %v2float %299 %299 0 1
        %301 = OpFOrdEqual %v2bool %285 %300
        %302 = OpAll %bool %301
               OpBranch %284
        %284 = OpLabel
        %303 = OpPhi %bool %false %264 %302 %283
               OpSelectionMerge %305 None
               OpBranchConditional %303 %304 %305
        %304 = OpLabel
        %307 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %308 = OpLoad %v4float %307
        %309 = OpVectorShuffle %v3float %308 %308 0 1 2
        %310 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %311 = OpLoad %v4float %310
        %312 = OpVectorShuffle %v3float %311 %311 0 1 2
        %313 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %314 = OpLoad %v4float %313
        %315 = OpVectorShuffle %v3float %314 %314 0 1 2
        %316 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %317 = OpLoad %v4float %316
        %318 = OpVectorShuffle %v3float %317 %317 0 1 2
        %306 = OpSelect %v3float %121 %318 %315
        %319 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %320 = OpLoad %v4float %319
        %321 = OpVectorShuffle %v3float %320 %320 0 1 2
        %322 = OpFOrdEqual %v3bool %306 %321
        %323 = OpAll %bool %322
               OpBranch %305
        %305 = OpLabel
        %324 = OpPhi %bool %false %284 %323 %304
               OpSelectionMerge %326 None
               OpBranchConditional %324 %325 %326
        %325 = OpLabel
        %328 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %329 = OpLoad %v4float %328
        %330 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %331 = OpLoad %v4float %330
        %332 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %333 = OpLoad %v4float %332
        %334 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %335 = OpLoad %v4float %334
        %327 = OpSelect %v4float %131 %335 %333
        %336 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %337 = OpLoad %v4float %336
        %338 = OpFOrdEqual %v4bool %327 %337
        %339 = OpAll %bool %338
               OpBranch %326
        %326 = OpLabel
        %340 = OpPhi %bool %false %305 %339 %325
               OpSelectionMerge %342 None
               OpBranchConditional %340 %341 %342
        %341 = OpLabel
        %343 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %344 = OpLoad %v4float %343
        %345 = OpCompositeExtract %float %344 0
        %346 = OpFOrdEqual %bool %float_0 %345
               OpBranch %342
        %342 = OpLabel
        %347 = OpPhi %bool %false %326 %346 %341
               OpSelectionMerge %349 None
               OpBranchConditional %347 %348 %349
        %348 = OpLabel
        %352 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %353 = OpLoad %v4float %352
        %354 = OpVectorShuffle %v2float %353 %353 0 1
        %355 = OpFOrdEqual %v2bool %351 %354
        %356 = OpAll %bool %355
               OpBranch %349
        %349 = OpLabel
        %357 = OpPhi %bool %false %342 %356 %348
               OpSelectionMerge %359 None
               OpBranchConditional %357 %358 %359
        %358 = OpLabel
        %361 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %362 = OpLoad %v4float %361
        %363 = OpVectorShuffle %v3float %362 %362 0 1 2
        %364 = OpFOrdEqual %v3bool %360 %363
        %365 = OpAll %bool %364
               OpBranch %359
        %359 = OpLabel
        %366 = OpPhi %bool %false %349 %365 %358
               OpSelectionMerge %368 None
               OpBranchConditional %366 %367 %368
        %367 = OpLabel
        %370 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %371 = OpLoad %v4float %370
        %372 = OpFOrdEqual %v4bool %369 %371
        %373 = OpAll %bool %372
               OpBranch %368
        %368 = OpLabel
        %374 = OpPhi %bool %false %359 %373 %367
               OpSelectionMerge %376 None
               OpBranchConditional %374 %375 %376
        %375 = OpLabel
        %377 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %378 = OpLoad %v4float %377
        %379 = OpCompositeExtract %float %378 0
        %380 = OpFOrdEqual %bool %float_1 %379
               OpBranch %376
        %376 = OpLabel
        %381 = OpPhi %bool %false %368 %380 %375
               OpSelectionMerge %383 None
               OpBranchConditional %381 %382 %383
        %382 = OpLabel
        %385 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %386 = OpLoad %v4float %385
        %387 = OpVectorShuffle %v2float %386 %386 0 1
        %388 = OpFOrdEqual %v2bool %384 %387
        %389 = OpAll %bool %388
               OpBranch %383
        %383 = OpLabel
        %390 = OpPhi %bool %false %376 %389 %382
               OpSelectionMerge %392 None
               OpBranchConditional %390 %391 %392
        %391 = OpLabel
        %394 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %395 = OpLoad %v4float %394
        %396 = OpVectorShuffle %v3float %395 %395 0 1 2
        %397 = OpFOrdEqual %v3bool %393 %396
        %398 = OpAll %bool %397
               OpBranch %392
        %392 = OpLabel
        %399 = OpPhi %bool %false %383 %398 %391
               OpSelectionMerge %401 None
               OpBranchConditional %399 %400 %401
        %400 = OpLabel
        %403 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %404 = OpLoad %v4float %403
        %405 = OpFOrdEqual %v4bool %402 %404
        %406 = OpAll %bool %405
               OpBranch %401
        %401 = OpLabel
        %407 = OpPhi %bool %false %392 %406 %400
               OpSelectionMerge %412 None
               OpBranchConditional %407 %410 %411
        %410 = OpLabel
        %413 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %414 = OpLoad %v4float %413
               OpStore %408 %414
               OpBranch %412
        %411 = OpLabel
        %415 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %416 = OpLoad %v4float %415
               OpStore %408 %416
               OpBranch %412
        %412 = OpLabel
        %417 = OpLoad %v4float %408
               OpReturnValue %417
               OpFunctionEnd
