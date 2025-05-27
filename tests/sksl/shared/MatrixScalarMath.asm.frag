               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testInputs"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %test_bifffff22 "test_bifffff22"
               OpName %one "one"
               OpName %m2 "m2"
               OpName %divisionTest_b "divisionTest_b"
               OpName %ten "ten"
               OpName %mat "mat"
               OpName %div "div"
               OpName %main "main"
               OpName %f1 "f1"
               OpName %f2 "f2"
               OpName %f3 "f3"
               OpName %f4 "f4"
               OpName %_0_expected "_0_expected"
               OpName %_1_one "_1_one"
               OpName %_2_m2 "_2_m2"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpDecorate %_UniformBuffer Block
               OpDecorate %9 Binding 0
               OpDecorate %9 DescriptorSet 0
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %313 RelaxedPrecision
               OpDecorate %315 RelaxedPrecision
               OpDecorate %316 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %9 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
         %28 = OpTypeFunction %bool %_ptr_Function_int %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_mat2v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
         %61 = OpConstantComposite %v2float %float_1 %float_1
         %62 = OpConstantComposite %mat2v2float %61 %61
    %float_2 = OpConstant %float 2
  %float_0_5 = OpConstant %float 0.5
      %false = OpConstantFalse %bool
      %int_0 = OpConstant %int 0
        %117 = OpTypeFunction %bool
   %float_10 = OpConstant %float 10
      %int_2 = OpConstant %int 2
    %float_8 = OpConstant %float 8
        %149 = OpConstantComposite %v4float %float_8 %float_8 %float_8 %float_8
%float_0_00999999978 = OpConstant %float 0.00999999978
        %152 = OpConstantComposite %v4float %float_0_00999999978 %float_0_00999999978 %float_0_00999999978 %float_0_00999999978
     %v4bool = OpTypeVector %bool 4
        %166 = OpTypeFunction %v4float %_ptr_Function_v2float
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %14
         %15 = OpLabel
         %19 = OpVariable %_ptr_Function_v2float Function
               OpStore %19 %18
         %21 = OpFunctionCall %v4float %main %19
               OpStore %sk_FragColor %21
               OpReturn
               OpFunctionEnd
%test_bifffff22 = OpFunction %bool None %28
         %29 = OpFunctionParameter %_ptr_Function_int
         %30 = OpFunctionParameter %_ptr_Function_float
         %31 = OpFunctionParameter %_ptr_Function_float
         %32 = OpFunctionParameter %_ptr_Function_float
         %33 = OpFunctionParameter %_ptr_Function_float
         %34 = OpFunctionParameter %_ptr_Function_mat2v2float
         %35 = OpLabel
        %one = OpVariable %_ptr_Function_float Function
         %m2 = OpVariable %_ptr_Function_mat2v2float Function
         %37 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
         %40 = OpLoad %v4float %37
         %41 = OpCompositeExtract %float %40 0
               OpStore %one %41
         %43 = OpLoad %float %30
         %44 = OpFMul %float %43 %41
         %45 = OpLoad %float %31
         %46 = OpFMul %float %45 %41
         %47 = OpLoad %float %32
         %48 = OpFMul %float %47 %41
         %49 = OpLoad %float %33
         %50 = OpFMul %float %49 %41
         %51 = OpCompositeConstruct %v2float %44 %46
         %52 = OpCompositeConstruct %v2float %48 %50
         %53 = OpCompositeConstruct %mat2v2float %51 %52
               OpStore %m2 %53
         %54 = OpLoad %int %29
               OpSelectionMerge %55 None
               OpSwitch %54 %55 1 %56 2 %57 3 %58 4 %59
         %56 = OpLabel
         %63 = OpFAdd %v2float %61 %51
         %64 = OpFAdd %v2float %61 %52
         %65 = OpCompositeConstruct %mat2v2float %63 %64
               OpStore %m2 %65
               OpBranch %55
         %57 = OpLabel
         %66 = OpLoad %mat2v2float %m2
         %67 = OpCompositeExtract %v2float %66 0
         %68 = OpFSub %v2float %67 %61
         %69 = OpCompositeExtract %v2float %66 1
         %70 = OpFSub %v2float %69 %61
         %71 = OpCompositeConstruct %mat2v2float %68 %70
               OpStore %m2 %71
               OpBranch %55
         %58 = OpLabel
         %72 = OpLoad %mat2v2float %m2
         %74 = OpMatrixTimesScalar %mat2v2float %72 %float_2
               OpStore %m2 %74
               OpBranch %55
         %59 = OpLabel
         %75 = OpLoad %mat2v2float %m2
         %77 = OpMatrixTimesScalar %mat2v2float %75 %float_0_5
               OpStore %m2 %77
               OpBranch %55
         %55 = OpLabel
         %80 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
         %81 = OpLoad %v2float %80
         %82 = OpCompositeExtract %float %81 0
         %83 = OpAccessChain %_ptr_Function_v2float %34 %int_0
         %84 = OpLoad %v2float %83
         %85 = OpCompositeExtract %float %84 0
         %86 = OpFOrdEqual %bool %82 %85
               OpSelectionMerge %88 None
               OpBranchConditional %86 %87 %88
         %87 = OpLabel
         %89 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
         %90 = OpLoad %v2float %89
         %91 = OpCompositeExtract %float %90 1
         %92 = OpAccessChain %_ptr_Function_v2float %34 %int_0
         %93 = OpLoad %v2float %92
         %94 = OpCompositeExtract %float %93 1
         %95 = OpFOrdEqual %bool %91 %94
               OpBranch %88
         %88 = OpLabel
         %96 = OpPhi %bool %false %55 %95 %87
               OpSelectionMerge %98 None
               OpBranchConditional %96 %97 %98
         %97 = OpLabel
         %99 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
        %100 = OpLoad %v2float %99
        %101 = OpCompositeExtract %float %100 0
        %102 = OpAccessChain %_ptr_Function_v2float %34 %int_1
        %103 = OpLoad %v2float %102
        %104 = OpCompositeExtract %float %103 0
        %105 = OpFOrdEqual %bool %101 %104
               OpBranch %98
         %98 = OpLabel
        %106 = OpPhi %bool %false %88 %105 %97
               OpSelectionMerge %108 None
               OpBranchConditional %106 %107 %108
        %107 = OpLabel
        %109 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
        %110 = OpLoad %v2float %109
        %111 = OpCompositeExtract %float %110 1
        %112 = OpAccessChain %_ptr_Function_v2float %34 %int_1
        %113 = OpLoad %v2float %112
        %114 = OpCompositeExtract %float %113 1
        %115 = OpFOrdEqual %bool %111 %114
               OpBranch %108
        %108 = OpLabel
        %116 = OpPhi %bool %false %98 %115 %107
               OpReturnValue %116
               OpFunctionEnd
%divisionTest_b = OpFunction %bool None %117
        %118 = OpLabel
        %ten = OpVariable %_ptr_Function_float Function
        %mat = OpVariable %_ptr_Function_mat2v2float Function
        %div = OpVariable %_ptr_Function_mat2v2float Function
        %120 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
        %121 = OpLoad %v4float %120
        %122 = OpCompositeExtract %float %121 0
        %124 = OpFMul %float %122 %float_10
               OpStore %ten %124
        %126 = OpCompositeConstruct %v2float %124 %124
        %127 = OpCompositeConstruct %mat2v2float %126 %126
               OpStore %mat %127
        %129 = OpAccessChain %_ptr_Uniform_v4float %9 %int_2
        %131 = OpLoad %v4float %129
        %132 = OpCompositeExtract %float %131 0
        %133 = OpFDiv %float %float_1 %132
        %134 = OpMatrixTimesScalar %mat2v2float %127 %133
               OpStore %div %134
        %135 = OpAccessChain %_ptr_Uniform_v4float %9 %int_2
        %136 = OpLoad %v4float %135
        %137 = OpCompositeExtract %float %136 0
        %138 = OpFDiv %float %float_1 %137
        %139 = OpMatrixTimesScalar %mat2v2float %127 %138
               OpStore %mat %139
        %143 = OpCompositeExtract %float %134 0 0
        %144 = OpCompositeExtract %float %134 0 1
        %145 = OpCompositeExtract %float %134 1 0
        %146 = OpCompositeExtract %float %134 1 1
        %147 = OpCompositeConstruct %v4float %143 %144 %145 %146
        %150 = OpFAdd %v4float %147 %149
        %142 = OpExtInst %v4float %1 FAbs %150
        %141 = OpFOrdLessThan %v4bool %142 %152
        %140 = OpAll %bool %141
               OpSelectionMerge %155 None
               OpBranchConditional %140 %154 %155
        %154 = OpLabel
        %159 = OpCompositeExtract %float %139 0 0
        %160 = OpCompositeExtract %float %139 0 1
        %161 = OpCompositeExtract %float %139 1 0
        %162 = OpCompositeExtract %float %139 1 1
        %163 = OpCompositeConstruct %v4float %159 %160 %161 %162
        %164 = OpFAdd %v4float %163 %149
        %158 = OpExtInst %v4float %1 FAbs %164
        %157 = OpFOrdLessThan %v4bool %158 %152
        %156 = OpAll %bool %157
               OpBranch %155
        %155 = OpLabel
        %165 = OpPhi %bool %false %118 %156 %154
               OpReturnValue %165
               OpFunctionEnd
       %main = OpFunction %v4float None %166
        %167 = OpFunctionParameter %_ptr_Function_v2float
        %168 = OpLabel
         %f1 = OpVariable %_ptr_Function_float Function
         %f2 = OpVariable %_ptr_Function_float Function
         %f3 = OpVariable %_ptr_Function_float Function
         %f4 = OpVariable %_ptr_Function_float Function
%_0_expected = OpVariable %_ptr_Function_mat2v2float Function
     %_1_one = OpVariable %_ptr_Function_float Function
      %_2_m2 = OpVariable %_ptr_Function_mat2v2float Function
        %252 = OpVariable %_ptr_Function_int Function
        %253 = OpVariable %_ptr_Function_float Function
        %254 = OpVariable %_ptr_Function_float Function
        %255 = OpVariable %_ptr_Function_float Function
        %256 = OpVariable %_ptr_Function_float Function
        %264 = OpVariable %_ptr_Function_mat2v2float Function
        %270 = OpVariable %_ptr_Function_int Function
        %271 = OpVariable %_ptr_Function_float Function
        %272 = OpVariable %_ptr_Function_float Function
        %273 = OpVariable %_ptr_Function_float Function
        %274 = OpVariable %_ptr_Function_float Function
        %282 = OpVariable %_ptr_Function_mat2v2float Function
        %288 = OpVariable %_ptr_Function_int Function
        %289 = OpVariable %_ptr_Function_float Function
        %290 = OpVariable %_ptr_Function_float Function
        %291 = OpVariable %_ptr_Function_float Function
        %292 = OpVariable %_ptr_Function_float Function
        %300 = OpVariable %_ptr_Function_mat2v2float Function
        %307 = OpVariable %_ptr_Function_v4float Function
        %170 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %171 = OpLoad %v4float %170
        %172 = OpCompositeExtract %float %171 1
               OpStore %f1 %172
        %174 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %175 = OpLoad %v4float %174
        %176 = OpCompositeExtract %float %175 1
        %177 = OpFMul %float %float_2 %176
               OpStore %f2 %177
        %180 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %181 = OpLoad %v4float %180
        %182 = OpCompositeExtract %float %181 1
        %183 = OpFMul %float %float_3 %182
               OpStore %f3 %183
        %186 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %187 = OpLoad %v4float %186
        %188 = OpCompositeExtract %float %187 1
        %189 = OpFMul %float %float_4 %188
               OpStore %f4 %189
        %191 = OpFAdd %float %172 %float_1
        %192 = OpFAdd %float %177 %float_1
        %193 = OpFAdd %float %183 %float_1
        %194 = OpFAdd %float %189 %float_1
        %195 = OpCompositeConstruct %v2float %191 %192
        %196 = OpCompositeConstruct %v2float %193 %194
        %197 = OpCompositeConstruct %mat2v2float %195 %196
               OpStore %_0_expected %197
        %199 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
        %200 = OpLoad %v4float %199
        %201 = OpCompositeExtract %float %200 0
               OpStore %_1_one %201
        %203 = OpFMul %float %172 %201
        %204 = OpFMul %float %177 %201
        %205 = OpFMul %float %183 %201
        %206 = OpFMul %float %189 %201
        %207 = OpCompositeConstruct %v2float %203 %204
        %208 = OpCompositeConstruct %v2float %205 %206
        %209 = OpCompositeConstruct %mat2v2float %207 %208
               OpStore %_2_m2 %209
        %210 = OpFAdd %v2float %61 %207
        %211 = OpFAdd %v2float %61 %208
        %212 = OpCompositeConstruct %mat2v2float %210 %211
               OpStore %_2_m2 %212
        %213 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
        %214 = OpLoad %v2float %213
        %215 = OpCompositeExtract %float %214 0
        %216 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
        %217 = OpLoad %v2float %216
        %218 = OpCompositeExtract %float %217 0
        %219 = OpFOrdEqual %bool %215 %218
               OpSelectionMerge %221 None
               OpBranchConditional %219 %220 %221
        %220 = OpLabel
        %222 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
        %223 = OpLoad %v2float %222
        %224 = OpCompositeExtract %float %223 1
        %225 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
        %226 = OpLoad %v2float %225
        %227 = OpCompositeExtract %float %226 1
        %228 = OpFOrdEqual %bool %224 %227
               OpBranch %221
        %221 = OpLabel
        %229 = OpPhi %bool %false %168 %228 %220
               OpSelectionMerge %231 None
               OpBranchConditional %229 %230 %231
        %230 = OpLabel
        %232 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
        %233 = OpLoad %v2float %232
        %234 = OpCompositeExtract %float %233 0
        %235 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
        %236 = OpLoad %v2float %235
        %237 = OpCompositeExtract %float %236 0
        %238 = OpFOrdEqual %bool %234 %237
               OpBranch %231
        %231 = OpLabel
        %239 = OpPhi %bool %false %221 %238 %230
               OpSelectionMerge %241 None
               OpBranchConditional %239 %240 %241
        %240 = OpLabel
        %242 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
        %243 = OpLoad %v2float %242
        %244 = OpCompositeExtract %float %243 1
        %245 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
        %246 = OpLoad %v2float %245
        %247 = OpCompositeExtract %float %246 1
        %248 = OpFOrdEqual %bool %244 %247
               OpBranch %241
        %241 = OpLabel
        %249 = OpPhi %bool %false %231 %248 %240
               OpSelectionMerge %251 None
               OpBranchConditional %249 %250 %251
        %250 = OpLabel
               OpStore %252 %int_2
               OpStore %253 %172
               OpStore %254 %177
               OpStore %255 %183
               OpStore %256 %189
        %257 = OpFSub %float %172 %float_1
        %258 = OpFSub %float %177 %float_1
        %259 = OpFSub %float %183 %float_1
        %260 = OpFSub %float %189 %float_1
        %261 = OpCompositeConstruct %v2float %257 %258
        %262 = OpCompositeConstruct %v2float %259 %260
        %263 = OpCompositeConstruct %mat2v2float %261 %262
               OpStore %264 %263
        %265 = OpFunctionCall %bool %test_bifffff22 %252 %253 %254 %255 %256 %264
               OpBranch %251
        %251 = OpLabel
        %266 = OpPhi %bool %false %241 %265 %250
               OpSelectionMerge %268 None
               OpBranchConditional %266 %267 %268
        %267 = OpLabel
               OpStore %270 %int_3
               OpStore %271 %172
               OpStore %272 %177
               OpStore %273 %183
               OpStore %274 %189
        %275 = OpFMul %float %172 %float_2
        %276 = OpFMul %float %177 %float_2
        %277 = OpFMul %float %183 %float_2
        %278 = OpFMul %float %189 %float_2
        %279 = OpCompositeConstruct %v2float %275 %276
        %280 = OpCompositeConstruct %v2float %277 %278
        %281 = OpCompositeConstruct %mat2v2float %279 %280
               OpStore %282 %281
        %283 = OpFunctionCall %bool %test_bifffff22 %270 %271 %272 %273 %274 %282
               OpBranch %268
        %268 = OpLabel
        %284 = OpPhi %bool %false %251 %283 %267
               OpSelectionMerge %286 None
               OpBranchConditional %284 %285 %286
        %285 = OpLabel
               OpStore %288 %int_4
               OpStore %289 %172
               OpStore %290 %177
               OpStore %291 %183
               OpStore %292 %189
        %293 = OpFMul %float %172 %float_0_5
        %294 = OpFMul %float %177 %float_0_5
        %295 = OpFMul %float %183 %float_0_5
        %296 = OpFMul %float %189 %float_0_5
        %297 = OpCompositeConstruct %v2float %293 %294
        %298 = OpCompositeConstruct %v2float %295 %296
        %299 = OpCompositeConstruct %mat2v2float %297 %298
               OpStore %300 %299
        %301 = OpFunctionCall %bool %test_bifffff22 %288 %289 %290 %291 %292 %300
               OpBranch %286
        %286 = OpLabel
        %302 = OpPhi %bool %false %268 %301 %285
               OpSelectionMerge %304 None
               OpBranchConditional %302 %303 %304
        %303 = OpLabel
        %305 = OpFunctionCall %bool %divisionTest_b
               OpBranch %304
        %304 = OpLabel
        %306 = OpPhi %bool %false %286 %305 %303
               OpSelectionMerge %311 None
               OpBranchConditional %306 %309 %310
        %309 = OpLabel
        %312 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %313 = OpLoad %v4float %312
               OpStore %307 %313
               OpBranch %311
        %310 = OpLabel
        %314 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
        %315 = OpLoad %v4float %314
               OpStore %307 %315
               OpBranch %311
        %311 = OpLabel
        %316 = OpLoad %v4float %307
               OpReturnValue %316
               OpFunctionEnd
