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
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %315 RelaxedPrecision
               OpDecorate %317 RelaxedPrecision
               OpDecorate %318 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
         %30 = OpTypeFunction %bool %_ptr_Function_int %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_mat2v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
         %63 = OpConstantComposite %v2float %float_1 %float_1
         %64 = OpConstantComposite %mat2v2float %63 %63
    %float_2 = OpConstant %float 2
  %float_0_5 = OpConstant %float 0.5
      %false = OpConstantFalse %bool
      %int_0 = OpConstant %int 0
        %119 = OpTypeFunction %bool
   %float_10 = OpConstant %float 10
      %int_2 = OpConstant %int 2
    %float_8 = OpConstant %float 8
        %151 = OpConstantComposite %v4float %float_8 %float_8 %float_8 %float_8
%float_0_00999999978 = OpConstant %float 0.00999999978
        %154 = OpConstantComposite %v4float %float_0_00999999978 %float_0_00999999978 %float_0_00999999978 %float_0_00999999978
     %v4bool = OpTypeVector %bool 4
        %168 = OpTypeFunction %v4float %_ptr_Function_v2float
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %17
         %18 = OpLabel
         %22 = OpVariable %_ptr_Function_v2float Function
               OpStore %22 %21
         %24 = OpFunctionCall %v4float %main %22
               OpStore %sk_FragColor %24
               OpReturn
               OpFunctionEnd
%test_bifffff22 = OpFunction %bool None %30
         %31 = OpFunctionParameter %_ptr_Function_int
         %32 = OpFunctionParameter %_ptr_Function_float
         %33 = OpFunctionParameter %_ptr_Function_float
         %34 = OpFunctionParameter %_ptr_Function_float
         %35 = OpFunctionParameter %_ptr_Function_float
         %36 = OpFunctionParameter %_ptr_Function_mat2v2float
         %37 = OpLabel
        %one = OpVariable %_ptr_Function_float Function
         %m2 = OpVariable %_ptr_Function_mat2v2float Function
         %39 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %42 = OpLoad %v4float %39
         %43 = OpCompositeExtract %float %42 0
               OpStore %one %43
         %45 = OpLoad %float %32
         %46 = OpFMul %float %45 %43
         %47 = OpLoad %float %33
         %48 = OpFMul %float %47 %43
         %49 = OpLoad %float %34
         %50 = OpFMul %float %49 %43
         %51 = OpLoad %float %35
         %52 = OpFMul %float %51 %43
         %53 = OpCompositeConstruct %v2float %46 %48
         %54 = OpCompositeConstruct %v2float %50 %52
         %55 = OpCompositeConstruct %mat2v2float %53 %54
               OpStore %m2 %55
         %56 = OpLoad %int %31
               OpSelectionMerge %57 None
               OpSwitch %56 %57 1 %58 2 %59 3 %60 4 %61
         %58 = OpLabel
         %65 = OpFAdd %v2float %63 %53
         %66 = OpFAdd %v2float %63 %54
         %67 = OpCompositeConstruct %mat2v2float %65 %66
               OpStore %m2 %67
               OpBranch %57
         %59 = OpLabel
         %68 = OpLoad %mat2v2float %m2
         %69 = OpCompositeExtract %v2float %68 0
         %70 = OpFSub %v2float %69 %63
         %71 = OpCompositeExtract %v2float %68 1
         %72 = OpFSub %v2float %71 %63
         %73 = OpCompositeConstruct %mat2v2float %70 %72
               OpStore %m2 %73
               OpBranch %57
         %60 = OpLabel
         %74 = OpLoad %mat2v2float %m2
         %76 = OpMatrixTimesScalar %mat2v2float %74 %float_2
               OpStore %m2 %76
               OpBranch %57
         %61 = OpLabel
         %77 = OpLoad %mat2v2float %m2
         %79 = OpMatrixTimesScalar %mat2v2float %77 %float_0_5
               OpStore %m2 %79
               OpBranch %57
         %57 = OpLabel
         %82 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
         %83 = OpLoad %v2float %82
         %84 = OpCompositeExtract %float %83 0
         %85 = OpAccessChain %_ptr_Function_v2float %36 %int_0
         %86 = OpLoad %v2float %85
         %87 = OpCompositeExtract %float %86 0
         %88 = OpFOrdEqual %bool %84 %87
               OpSelectionMerge %90 None
               OpBranchConditional %88 %89 %90
         %89 = OpLabel
         %91 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
         %92 = OpLoad %v2float %91
         %93 = OpCompositeExtract %float %92 1
         %94 = OpAccessChain %_ptr_Function_v2float %36 %int_0
         %95 = OpLoad %v2float %94
         %96 = OpCompositeExtract %float %95 1
         %97 = OpFOrdEqual %bool %93 %96
               OpBranch %90
         %90 = OpLabel
         %98 = OpPhi %bool %false %57 %97 %89
               OpSelectionMerge %100 None
               OpBranchConditional %98 %99 %100
         %99 = OpLabel
        %101 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
        %102 = OpLoad %v2float %101
        %103 = OpCompositeExtract %float %102 0
        %104 = OpAccessChain %_ptr_Function_v2float %36 %int_1
        %105 = OpLoad %v2float %104
        %106 = OpCompositeExtract %float %105 0
        %107 = OpFOrdEqual %bool %103 %106
               OpBranch %100
        %100 = OpLabel
        %108 = OpPhi %bool %false %90 %107 %99
               OpSelectionMerge %110 None
               OpBranchConditional %108 %109 %110
        %109 = OpLabel
        %111 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
        %112 = OpLoad %v2float %111
        %113 = OpCompositeExtract %float %112 1
        %114 = OpAccessChain %_ptr_Function_v2float %36 %int_1
        %115 = OpLoad %v2float %114
        %116 = OpCompositeExtract %float %115 1
        %117 = OpFOrdEqual %bool %113 %116
               OpBranch %110
        %110 = OpLabel
        %118 = OpPhi %bool %false %100 %117 %109
               OpReturnValue %118
               OpFunctionEnd
%divisionTest_b = OpFunction %bool None %119
        %120 = OpLabel
        %ten = OpVariable %_ptr_Function_float Function
        %mat = OpVariable %_ptr_Function_mat2v2float Function
        %div = OpVariable %_ptr_Function_mat2v2float Function
        %122 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %123 = OpLoad %v4float %122
        %124 = OpCompositeExtract %float %123 0
        %126 = OpFMul %float %124 %float_10
               OpStore %ten %126
        %128 = OpCompositeConstruct %v2float %126 %126
        %129 = OpCompositeConstruct %mat2v2float %128 %128
               OpStore %mat %129
        %131 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
        %133 = OpLoad %v4float %131
        %134 = OpCompositeExtract %float %133 0
        %135 = OpFDiv %float %float_1 %134
        %136 = OpMatrixTimesScalar %mat2v2float %129 %135
               OpStore %div %136
        %137 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
        %138 = OpLoad %v4float %137
        %139 = OpCompositeExtract %float %138 0
        %140 = OpFDiv %float %float_1 %139
        %141 = OpMatrixTimesScalar %mat2v2float %129 %140
               OpStore %mat %141
        %145 = OpCompositeExtract %float %136 0 0
        %146 = OpCompositeExtract %float %136 0 1
        %147 = OpCompositeExtract %float %136 1 0
        %148 = OpCompositeExtract %float %136 1 1
        %149 = OpCompositeConstruct %v4float %145 %146 %147 %148
        %152 = OpFAdd %v4float %149 %151
        %144 = OpExtInst %v4float %1 FAbs %152
        %143 = OpFOrdLessThan %v4bool %144 %154
        %142 = OpAll %bool %143
               OpSelectionMerge %157 None
               OpBranchConditional %142 %156 %157
        %156 = OpLabel
        %161 = OpCompositeExtract %float %141 0 0
        %162 = OpCompositeExtract %float %141 0 1
        %163 = OpCompositeExtract %float %141 1 0
        %164 = OpCompositeExtract %float %141 1 1
        %165 = OpCompositeConstruct %v4float %161 %162 %163 %164
        %166 = OpFAdd %v4float %165 %151
        %160 = OpExtInst %v4float %1 FAbs %166
        %159 = OpFOrdLessThan %v4bool %160 %154
        %158 = OpAll %bool %159
               OpBranch %157
        %157 = OpLabel
        %167 = OpPhi %bool %false %120 %158 %156
               OpReturnValue %167
               OpFunctionEnd
       %main = OpFunction %v4float None %168
        %169 = OpFunctionParameter %_ptr_Function_v2float
        %170 = OpLabel
         %f1 = OpVariable %_ptr_Function_float Function
         %f2 = OpVariable %_ptr_Function_float Function
         %f3 = OpVariable %_ptr_Function_float Function
         %f4 = OpVariable %_ptr_Function_float Function
%_0_expected = OpVariable %_ptr_Function_mat2v2float Function
     %_1_one = OpVariable %_ptr_Function_float Function
      %_2_m2 = OpVariable %_ptr_Function_mat2v2float Function
        %254 = OpVariable %_ptr_Function_int Function
        %255 = OpVariable %_ptr_Function_float Function
        %256 = OpVariable %_ptr_Function_float Function
        %257 = OpVariable %_ptr_Function_float Function
        %258 = OpVariable %_ptr_Function_float Function
        %266 = OpVariable %_ptr_Function_mat2v2float Function
        %272 = OpVariable %_ptr_Function_int Function
        %273 = OpVariable %_ptr_Function_float Function
        %274 = OpVariable %_ptr_Function_float Function
        %275 = OpVariable %_ptr_Function_float Function
        %276 = OpVariable %_ptr_Function_float Function
        %284 = OpVariable %_ptr_Function_mat2v2float Function
        %290 = OpVariable %_ptr_Function_int Function
        %291 = OpVariable %_ptr_Function_float Function
        %292 = OpVariable %_ptr_Function_float Function
        %293 = OpVariable %_ptr_Function_float Function
        %294 = OpVariable %_ptr_Function_float Function
        %302 = OpVariable %_ptr_Function_mat2v2float Function
        %309 = OpVariable %_ptr_Function_v4float Function
        %172 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %173 = OpLoad %v4float %172
        %174 = OpCompositeExtract %float %173 1
               OpStore %f1 %174
        %176 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %177 = OpLoad %v4float %176
        %178 = OpCompositeExtract %float %177 1
        %179 = OpFMul %float %float_2 %178
               OpStore %f2 %179
        %182 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %183 = OpLoad %v4float %182
        %184 = OpCompositeExtract %float %183 1
        %185 = OpFMul %float %float_3 %184
               OpStore %f3 %185
        %188 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %189 = OpLoad %v4float %188
        %190 = OpCompositeExtract %float %189 1
        %191 = OpFMul %float %float_4 %190
               OpStore %f4 %191
        %193 = OpFAdd %float %174 %float_1
        %194 = OpFAdd %float %179 %float_1
        %195 = OpFAdd %float %185 %float_1
        %196 = OpFAdd %float %191 %float_1
        %197 = OpCompositeConstruct %v2float %193 %194
        %198 = OpCompositeConstruct %v2float %195 %196
        %199 = OpCompositeConstruct %mat2v2float %197 %198
               OpStore %_0_expected %199
        %201 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %202 = OpLoad %v4float %201
        %203 = OpCompositeExtract %float %202 0
               OpStore %_1_one %203
        %205 = OpFMul %float %174 %203
        %206 = OpFMul %float %179 %203
        %207 = OpFMul %float %185 %203
        %208 = OpFMul %float %191 %203
        %209 = OpCompositeConstruct %v2float %205 %206
        %210 = OpCompositeConstruct %v2float %207 %208
        %211 = OpCompositeConstruct %mat2v2float %209 %210
               OpStore %_2_m2 %211
        %212 = OpFAdd %v2float %63 %209
        %213 = OpFAdd %v2float %63 %210
        %214 = OpCompositeConstruct %mat2v2float %212 %213
               OpStore %_2_m2 %214
        %215 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
        %216 = OpLoad %v2float %215
        %217 = OpCompositeExtract %float %216 0
        %218 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
        %219 = OpLoad %v2float %218
        %220 = OpCompositeExtract %float %219 0
        %221 = OpFOrdEqual %bool %217 %220
               OpSelectionMerge %223 None
               OpBranchConditional %221 %222 %223
        %222 = OpLabel
        %224 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
        %225 = OpLoad %v2float %224
        %226 = OpCompositeExtract %float %225 1
        %227 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
        %228 = OpLoad %v2float %227
        %229 = OpCompositeExtract %float %228 1
        %230 = OpFOrdEqual %bool %226 %229
               OpBranch %223
        %223 = OpLabel
        %231 = OpPhi %bool %false %170 %230 %222
               OpSelectionMerge %233 None
               OpBranchConditional %231 %232 %233
        %232 = OpLabel
        %234 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
        %235 = OpLoad %v2float %234
        %236 = OpCompositeExtract %float %235 0
        %237 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
        %238 = OpLoad %v2float %237
        %239 = OpCompositeExtract %float %238 0
        %240 = OpFOrdEqual %bool %236 %239
               OpBranch %233
        %233 = OpLabel
        %241 = OpPhi %bool %false %223 %240 %232
               OpSelectionMerge %243 None
               OpBranchConditional %241 %242 %243
        %242 = OpLabel
        %244 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
        %245 = OpLoad %v2float %244
        %246 = OpCompositeExtract %float %245 1
        %247 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
        %248 = OpLoad %v2float %247
        %249 = OpCompositeExtract %float %248 1
        %250 = OpFOrdEqual %bool %246 %249
               OpBranch %243
        %243 = OpLabel
        %251 = OpPhi %bool %false %233 %250 %242
               OpSelectionMerge %253 None
               OpBranchConditional %251 %252 %253
        %252 = OpLabel
               OpStore %254 %int_2
               OpStore %255 %174
               OpStore %256 %179
               OpStore %257 %185
               OpStore %258 %191
        %259 = OpFSub %float %174 %float_1
        %260 = OpFSub %float %179 %float_1
        %261 = OpFSub %float %185 %float_1
        %262 = OpFSub %float %191 %float_1
        %263 = OpCompositeConstruct %v2float %259 %260
        %264 = OpCompositeConstruct %v2float %261 %262
        %265 = OpCompositeConstruct %mat2v2float %263 %264
               OpStore %266 %265
        %267 = OpFunctionCall %bool %test_bifffff22 %254 %255 %256 %257 %258 %266
               OpBranch %253
        %253 = OpLabel
        %268 = OpPhi %bool %false %243 %267 %252
               OpSelectionMerge %270 None
               OpBranchConditional %268 %269 %270
        %269 = OpLabel
               OpStore %272 %int_3
               OpStore %273 %174
               OpStore %274 %179
               OpStore %275 %185
               OpStore %276 %191
        %277 = OpFMul %float %174 %float_2
        %278 = OpFMul %float %179 %float_2
        %279 = OpFMul %float %185 %float_2
        %280 = OpFMul %float %191 %float_2
        %281 = OpCompositeConstruct %v2float %277 %278
        %282 = OpCompositeConstruct %v2float %279 %280
        %283 = OpCompositeConstruct %mat2v2float %281 %282
               OpStore %284 %283
        %285 = OpFunctionCall %bool %test_bifffff22 %272 %273 %274 %275 %276 %284
               OpBranch %270
        %270 = OpLabel
        %286 = OpPhi %bool %false %253 %285 %269
               OpSelectionMerge %288 None
               OpBranchConditional %286 %287 %288
        %287 = OpLabel
               OpStore %290 %int_4
               OpStore %291 %174
               OpStore %292 %179
               OpStore %293 %185
               OpStore %294 %191
        %295 = OpFMul %float %174 %float_0_5
        %296 = OpFMul %float %179 %float_0_5
        %297 = OpFMul %float %185 %float_0_5
        %298 = OpFMul %float %191 %float_0_5
        %299 = OpCompositeConstruct %v2float %295 %296
        %300 = OpCompositeConstruct %v2float %297 %298
        %301 = OpCompositeConstruct %mat2v2float %299 %300
               OpStore %302 %301
        %303 = OpFunctionCall %bool %test_bifffff22 %290 %291 %292 %293 %294 %302
               OpBranch %288
        %288 = OpLabel
        %304 = OpPhi %bool %false %270 %303 %287
               OpSelectionMerge %306 None
               OpBranchConditional %304 %305 %306
        %305 = OpLabel
        %307 = OpFunctionCall %bool %divisionTest_b
               OpBranch %306
        %306 = OpLabel
        %308 = OpPhi %bool %false %288 %307 %305
               OpSelectionMerge %313 None
               OpBranchConditional %308 %311 %312
        %311 = OpLabel
        %314 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %315 = OpLoad %v4float %314
               OpStore %309 %315
               OpBranch %313
        %312 = OpLabel
        %316 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %317 = OpLoad %v4float %316
               OpStore %309 %317
               OpBranch %313
        %313 = OpLabel
        %318 = OpLoad %v4float %309
               OpReturnValue %318
               OpFunctionEnd
