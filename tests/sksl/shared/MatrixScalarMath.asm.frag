               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %9
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testInputs"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %test_bifffff22 "test_bifffff22"  ; id %6
               OpName %one "one"                        ; id %39
               OpName %m2 "m2"                          ; id %45
               OpName %divisionTest_b "divisionTest_b"  ; id %7
               OpName %ten "ten"                        ; id %122
               OpName %mat "mat"                        ; id %128
               OpName %div "div"                        ; id %131
               OpName %main "main"                      ; id %8
               OpName %f1 "f1"                          ; id %172
               OpName %f2 "f2"                          ; id %176
               OpName %f3 "f3"                          ; id %181
               OpName %f4 "f4"                          ; id %187
               OpName %_0_expected "_0_expected"        ; id %193
               OpName %_1_one "_1_one"                  ; id %201
               OpName %_2_m2 "_2_m2"                    ; id %205

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
               OpDecorate %_UniformBuffer Block
               OpDecorate %13 Binding 0
               OpDecorate %13 DescriptorSet 0
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %186 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %316 RelaxedPrecision
               OpDecorate %318 RelaxedPrecision
               OpDecorate %319 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
         %31 = OpTypeFunction %bool %_ptr_Function_int %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_mat2v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
         %64 = OpConstantComposite %v2float %float_1 %float_1
         %65 = OpConstantComposite %mat2v2float %64 %64
    %float_2 = OpConstant %float 2
  %float_0_5 = OpConstant %float 0.5
      %false = OpConstantFalse %bool
      %int_0 = OpConstant %int 0
        %120 = OpTypeFunction %bool
   %float_10 = OpConstant %float 10
      %int_2 = OpConstant %int 2
    %float_8 = OpConstant %float 8
        %152 = OpConstantComposite %v4float %float_8 %float_8 %float_8 %float_8
%float_0_00999999978 = OpConstant %float 0.00999999978
        %155 = OpConstantComposite %v4float %float_0_00999999978 %float_0_00999999978 %float_0_00999999978 %float_0_00999999978
     %v4bool = OpTypeVector %bool 4
        %169 = OpTypeFunction %v4float %_ptr_Function_v2float
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
%_ptr_Function_v4float = OpTypePointer Function %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %18

         %19 = OpLabel
         %23 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %23 %22
         %25 =   OpFunctionCall %v4float %main %23
                 OpStore %sk_FragColor %25
                 OpReturn
               OpFunctionEnd


               ; Function test_bifffff22
%test_bifffff22 = OpFunction %bool None %31
         %32 = OpFunctionParameter %_ptr_Function_int
         %33 = OpFunctionParameter %_ptr_Function_float
         %34 = OpFunctionParameter %_ptr_Function_float
         %35 = OpFunctionParameter %_ptr_Function_float
         %36 = OpFunctionParameter %_ptr_Function_float
         %37 = OpFunctionParameter %_ptr_Function_mat2v2float

         %38 = OpLabel
        %one =   OpVariable %_ptr_Function_float Function
         %m2 =   OpVariable %_ptr_Function_mat2v2float Function
         %40 =   OpAccessChain %_ptr_Uniform_v4float %13 %int_1
         %43 =   OpLoad %v4float %40                ; RelaxedPrecision
         %44 =   OpCompositeExtract %float %43 0    ; RelaxedPrecision
                 OpStore %one %44
         %46 =   OpLoad %float %33
         %47 =   OpFMul %float %46 %44
         %48 =   OpLoad %float %34
         %49 =   OpFMul %float %48 %44
         %50 =   OpLoad %float %35
         %51 =   OpFMul %float %50 %44
         %52 =   OpLoad %float %36
         %53 =   OpFMul %float %52 %44
         %54 =   OpCompositeConstruct %v2float %47 %49
         %55 =   OpCompositeConstruct %v2float %51 %53
         %56 =   OpCompositeConstruct %mat2v2float %54 %55
                 OpStore %m2 %56
         %57 =   OpLoad %int %32
                 OpSelectionMerge %58 None
                 OpSwitch %57 %58 1 %59 2 %60 3 %61 4 %62

         %59 =     OpLabel
         %66 =       OpFAdd %v2float %64 %54
         %67 =       OpFAdd %v2float %64 %55
         %68 =       OpCompositeConstruct %mat2v2float %66 %67
                     OpStore %m2 %68
                     OpBranch %58

         %60 =     OpLabel
         %69 =       OpLoad %mat2v2float %m2
         %70 =       OpCompositeExtract %v2float %69 0
         %71 =       OpFSub %v2float %70 %64
         %72 =       OpCompositeExtract %v2float %69 1
         %73 =       OpFSub %v2float %72 %64
         %74 =       OpCompositeConstruct %mat2v2float %71 %73
                     OpStore %m2 %74
                     OpBranch %58

         %61 =     OpLabel
         %75 =       OpLoad %mat2v2float %m2
         %77 =       OpMatrixTimesScalar %mat2v2float %75 %float_2
                     OpStore %m2 %77
                     OpBranch %58

         %62 =     OpLabel
         %78 =       OpLoad %mat2v2float %m2
         %80 =       OpMatrixTimesScalar %mat2v2float %78 %float_0_5
                     OpStore %m2 %80
                     OpBranch %58

         %58 = OpLabel
         %83 =   OpAccessChain %_ptr_Function_v2float %m2 %int_0
         %84 =   OpLoad %v2float %83
         %85 =   OpCompositeExtract %float %84 0
         %86 =   OpAccessChain %_ptr_Function_v2float %37 %int_0
         %87 =   OpLoad %v2float %86
         %88 =   OpCompositeExtract %float %87 0
         %89 =   OpFOrdEqual %bool %85 %88
                 OpSelectionMerge %91 None
                 OpBranchConditional %89 %90 %91

         %90 =     OpLabel
         %92 =       OpAccessChain %_ptr_Function_v2float %m2 %int_0
         %93 =       OpLoad %v2float %92
         %94 =       OpCompositeExtract %float %93 1
         %95 =       OpAccessChain %_ptr_Function_v2float %37 %int_0
         %96 =       OpLoad %v2float %95
         %97 =       OpCompositeExtract %float %96 1
         %98 =       OpFOrdEqual %bool %94 %97
                     OpBranch %91

         %91 = OpLabel
         %99 =   OpPhi %bool %false %58 %98 %90
                 OpSelectionMerge %101 None
                 OpBranchConditional %99 %100 %101

        %100 =     OpLabel
        %102 =       OpAccessChain %_ptr_Function_v2float %m2 %int_1
        %103 =       OpLoad %v2float %102
        %104 =       OpCompositeExtract %float %103 0
        %105 =       OpAccessChain %_ptr_Function_v2float %37 %int_1
        %106 =       OpLoad %v2float %105
        %107 =       OpCompositeExtract %float %106 0
        %108 =       OpFOrdEqual %bool %104 %107
                     OpBranch %101

        %101 = OpLabel
        %109 =   OpPhi %bool %false %91 %108 %100
                 OpSelectionMerge %111 None
                 OpBranchConditional %109 %110 %111

        %110 =     OpLabel
        %112 =       OpAccessChain %_ptr_Function_v2float %m2 %int_1
        %113 =       OpLoad %v2float %112
        %114 =       OpCompositeExtract %float %113 1
        %115 =       OpAccessChain %_ptr_Function_v2float %37 %int_1
        %116 =       OpLoad %v2float %115
        %117 =       OpCompositeExtract %float %116 1
        %118 =       OpFOrdEqual %bool %114 %117
                     OpBranch %111

        %111 = OpLabel
        %119 =   OpPhi %bool %false %101 %118 %110
                 OpReturnValue %119
               OpFunctionEnd


               ; Function divisionTest_b
%divisionTest_b = OpFunction %bool None %120

        %121 = OpLabel
        %ten =   OpVariable %_ptr_Function_float Function
        %mat =   OpVariable %_ptr_Function_mat2v2float Function
        %div =   OpVariable %_ptr_Function_mat2v2float Function
        %123 =   OpAccessChain %_ptr_Uniform_v4float %13 %int_1
        %124 =   OpLoad %v4float %123               ; RelaxedPrecision
        %125 =   OpCompositeExtract %float %124 0   ; RelaxedPrecision
        %127 =   OpFMul %float %125 %float_10       ; RelaxedPrecision
                 OpStore %ten %127
        %129 =   OpCompositeConstruct %v2float %127 %127
        %130 =   OpCompositeConstruct %mat2v2float %129 %129
                 OpStore %mat %130
        %132 =   OpAccessChain %_ptr_Uniform_v4float %13 %int_2
        %134 =   OpLoad %v4float %132
        %135 =   OpCompositeExtract %float %134 0
        %136 =   OpFDiv %float %float_1 %135
        %137 =   OpMatrixTimesScalar %mat2v2float %130 %136
                 OpStore %div %137
        %138 =   OpAccessChain %_ptr_Uniform_v4float %13 %int_2
        %139 =   OpLoad %v4float %138
        %140 =   OpCompositeExtract %float %139 0
        %141 =   OpFDiv %float %float_1 %140
        %142 =   OpMatrixTimesScalar %mat2v2float %130 %141
                 OpStore %mat %142
        %146 =   OpCompositeExtract %float %137 0 0
        %147 =   OpCompositeExtract %float %137 0 1
        %148 =   OpCompositeExtract %float %137 1 0
        %149 =   OpCompositeExtract %float %137 1 1
        %150 =   OpCompositeConstruct %v4float %146 %147 %148 %149
        %153 =   OpFAdd %v4float %150 %152
        %145 =   OpExtInst %v4float %5 FAbs %153
        %144 =   OpFOrdLessThan %v4bool %145 %155
        %143 =   OpAll %bool %144
                 OpSelectionMerge %158 None
                 OpBranchConditional %143 %157 %158

        %157 =     OpLabel
        %162 =       OpCompositeExtract %float %142 0 0
        %163 =       OpCompositeExtract %float %142 0 1
        %164 =       OpCompositeExtract %float %142 1 0
        %165 =       OpCompositeExtract %float %142 1 1
        %166 =       OpCompositeConstruct %v4float %162 %163 %164 %165
        %167 =       OpFAdd %v4float %166 %152
        %161 =       OpExtInst %v4float %5 FAbs %167
        %160 =       OpFOrdLessThan %v4bool %161 %155
        %159 =       OpAll %bool %160
                     OpBranch %158

        %158 = OpLabel
        %168 =   OpPhi %bool %false %121 %159 %157
                 OpReturnValue %168
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %169        ; RelaxedPrecision
        %170 = OpFunctionParameter %_ptr_Function_v2float

        %171 = OpLabel
         %f1 =   OpVariable %_ptr_Function_float Function
         %f2 =   OpVariable %_ptr_Function_float Function
         %f3 =   OpVariable %_ptr_Function_float Function
         %f4 =   OpVariable %_ptr_Function_float Function
%_0_expected =   OpVariable %_ptr_Function_mat2v2float Function
     %_1_one =   OpVariable %_ptr_Function_float Function
      %_2_m2 =   OpVariable %_ptr_Function_mat2v2float Function
        %255 =   OpVariable %_ptr_Function_int Function
        %256 =   OpVariable %_ptr_Function_float Function
        %257 =   OpVariable %_ptr_Function_float Function
        %258 =   OpVariable %_ptr_Function_float Function
        %259 =   OpVariable %_ptr_Function_float Function
        %267 =   OpVariable %_ptr_Function_mat2v2float Function
        %273 =   OpVariable %_ptr_Function_int Function
        %274 =   OpVariable %_ptr_Function_float Function
        %275 =   OpVariable %_ptr_Function_float Function
        %276 =   OpVariable %_ptr_Function_float Function
        %277 =   OpVariable %_ptr_Function_float Function
        %285 =   OpVariable %_ptr_Function_mat2v2float Function
        %291 =   OpVariable %_ptr_Function_int Function
        %292 =   OpVariable %_ptr_Function_float Function
        %293 =   OpVariable %_ptr_Function_float Function
        %294 =   OpVariable %_ptr_Function_float Function
        %295 =   OpVariable %_ptr_Function_float Function
        %303 =   OpVariable %_ptr_Function_mat2v2float Function
        %310 =   OpVariable %_ptr_Function_v4float Function
        %173 =   OpAccessChain %_ptr_Uniform_v4float %13 %int_0
        %174 =   OpLoad %v4float %173               ; RelaxedPrecision
        %175 =   OpCompositeExtract %float %174 1   ; RelaxedPrecision
                 OpStore %f1 %175
        %177 =   OpAccessChain %_ptr_Uniform_v4float %13 %int_0
        %178 =   OpLoad %v4float %177               ; RelaxedPrecision
        %179 =   OpCompositeExtract %float %178 1   ; RelaxedPrecision
        %180 =   OpFMul %float %float_2 %179        ; RelaxedPrecision
                 OpStore %f2 %180
        %183 =   OpAccessChain %_ptr_Uniform_v4float %13 %int_0
        %184 =   OpLoad %v4float %183               ; RelaxedPrecision
        %185 =   OpCompositeExtract %float %184 1   ; RelaxedPrecision
        %186 =   OpFMul %float %float_3 %185        ; RelaxedPrecision
                 OpStore %f3 %186
        %189 =   OpAccessChain %_ptr_Uniform_v4float %13 %int_0
        %190 =   OpLoad %v4float %189               ; RelaxedPrecision
        %191 =   OpCompositeExtract %float %190 1   ; RelaxedPrecision
        %192 =   OpFMul %float %float_4 %191        ; RelaxedPrecision
                 OpStore %f4 %192
        %194 =   OpFAdd %float %175 %float_1
        %195 =   OpFAdd %float %180 %float_1
        %196 =   OpFAdd %float %186 %float_1
        %197 =   OpFAdd %float %192 %float_1
        %198 =   OpCompositeConstruct %v2float %194 %195
        %199 =   OpCompositeConstruct %v2float %196 %197
        %200 =   OpCompositeConstruct %mat2v2float %198 %199
                 OpStore %_0_expected %200
        %202 =   OpAccessChain %_ptr_Uniform_v4float %13 %int_1
        %203 =   OpLoad %v4float %202               ; RelaxedPrecision
        %204 =   OpCompositeExtract %float %203 0   ; RelaxedPrecision
                 OpStore %_1_one %204
        %206 =   OpFMul %float %175 %204
        %207 =   OpFMul %float %180 %204
        %208 =   OpFMul %float %186 %204
        %209 =   OpFMul %float %192 %204
        %210 =   OpCompositeConstruct %v2float %206 %207
        %211 =   OpCompositeConstruct %v2float %208 %209
        %212 =   OpCompositeConstruct %mat2v2float %210 %211
                 OpStore %_2_m2 %212
        %213 =   OpFAdd %v2float %64 %210
        %214 =   OpFAdd %v2float %64 %211
        %215 =   OpCompositeConstruct %mat2v2float %213 %214
                 OpStore %_2_m2 %215
        %216 =   OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
        %217 =   OpLoad %v2float %216
        %218 =   OpCompositeExtract %float %217 0
        %219 =   OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
        %220 =   OpLoad %v2float %219
        %221 =   OpCompositeExtract %float %220 0
        %222 =   OpFOrdEqual %bool %218 %221
                 OpSelectionMerge %224 None
                 OpBranchConditional %222 %223 %224

        %223 =     OpLabel
        %225 =       OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
        %226 =       OpLoad %v2float %225
        %227 =       OpCompositeExtract %float %226 1
        %228 =       OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
        %229 =       OpLoad %v2float %228
        %230 =       OpCompositeExtract %float %229 1
        %231 =       OpFOrdEqual %bool %227 %230
                     OpBranch %224

        %224 = OpLabel
        %232 =   OpPhi %bool %false %171 %231 %223
                 OpSelectionMerge %234 None
                 OpBranchConditional %232 %233 %234

        %233 =     OpLabel
        %235 =       OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
        %236 =       OpLoad %v2float %235
        %237 =       OpCompositeExtract %float %236 0
        %238 =       OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
        %239 =       OpLoad %v2float %238
        %240 =       OpCompositeExtract %float %239 0
        %241 =       OpFOrdEqual %bool %237 %240
                     OpBranch %234

        %234 = OpLabel
        %242 =   OpPhi %bool %false %224 %241 %233
                 OpSelectionMerge %244 None
                 OpBranchConditional %242 %243 %244

        %243 =     OpLabel
        %245 =       OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
        %246 =       OpLoad %v2float %245
        %247 =       OpCompositeExtract %float %246 1
        %248 =       OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
        %249 =       OpLoad %v2float %248
        %250 =       OpCompositeExtract %float %249 1
        %251 =       OpFOrdEqual %bool %247 %250
                     OpBranch %244

        %244 = OpLabel
        %252 =   OpPhi %bool %false %234 %251 %243
                 OpSelectionMerge %254 None
                 OpBranchConditional %252 %253 %254

        %253 =     OpLabel
                     OpStore %255 %int_2
                     OpStore %256 %175
                     OpStore %257 %180
                     OpStore %258 %186
                     OpStore %259 %192
        %260 =       OpFSub %float %175 %float_1
        %261 =       OpFSub %float %180 %float_1
        %262 =       OpFSub %float %186 %float_1
        %263 =       OpFSub %float %192 %float_1
        %264 =       OpCompositeConstruct %v2float %260 %261
        %265 =       OpCompositeConstruct %v2float %262 %263
        %266 =       OpCompositeConstruct %mat2v2float %264 %265
                     OpStore %267 %266
        %268 =       OpFunctionCall %bool %test_bifffff22 %255 %256 %257 %258 %259 %267
                     OpBranch %254

        %254 = OpLabel
        %269 =   OpPhi %bool %false %244 %268 %253
                 OpSelectionMerge %271 None
                 OpBranchConditional %269 %270 %271

        %270 =     OpLabel
                     OpStore %273 %int_3
                     OpStore %274 %175
                     OpStore %275 %180
                     OpStore %276 %186
                     OpStore %277 %192
        %278 =       OpFMul %float %175 %float_2
        %279 =       OpFMul %float %180 %float_2
        %280 =       OpFMul %float %186 %float_2
        %281 =       OpFMul %float %192 %float_2
        %282 =       OpCompositeConstruct %v2float %278 %279
        %283 =       OpCompositeConstruct %v2float %280 %281
        %284 =       OpCompositeConstruct %mat2v2float %282 %283
                     OpStore %285 %284
        %286 =       OpFunctionCall %bool %test_bifffff22 %273 %274 %275 %276 %277 %285
                     OpBranch %271

        %271 = OpLabel
        %287 =   OpPhi %bool %false %254 %286 %270
                 OpSelectionMerge %289 None
                 OpBranchConditional %287 %288 %289

        %288 =     OpLabel
                     OpStore %291 %int_4
                     OpStore %292 %175
                     OpStore %293 %180
                     OpStore %294 %186
                     OpStore %295 %192
        %296 =       OpFMul %float %175 %float_0_5
        %297 =       OpFMul %float %180 %float_0_5
        %298 =       OpFMul %float %186 %float_0_5
        %299 =       OpFMul %float %192 %float_0_5
        %300 =       OpCompositeConstruct %v2float %296 %297
        %301 =       OpCompositeConstruct %v2float %298 %299
        %302 =       OpCompositeConstruct %mat2v2float %300 %301
                     OpStore %303 %302
        %304 =       OpFunctionCall %bool %test_bifffff22 %291 %292 %293 %294 %295 %303
                     OpBranch %289

        %289 = OpLabel
        %305 =   OpPhi %bool %false %271 %304 %288
                 OpSelectionMerge %307 None
                 OpBranchConditional %305 %306 %307

        %306 =     OpLabel
        %308 =       OpFunctionCall %bool %divisionTest_b
                     OpBranch %307

        %307 = OpLabel
        %309 =   OpPhi %bool %false %289 %308 %306
                 OpSelectionMerge %314 None
                 OpBranchConditional %309 %312 %313

        %312 =     OpLabel
        %315 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_0
        %316 =       OpLoad %v4float %315           ; RelaxedPrecision
                     OpStore %310 %316
                     OpBranch %314

        %313 =     OpLabel
        %317 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_1
        %318 =       OpLoad %v4float %317           ; RelaxedPrecision
                     OpStore %310 %318
                     OpBranch %314

        %314 = OpLabel
        %319 =   OpLoad %v4float %310               ; RelaxedPrecision
                 OpReturnValue %319
               OpFunctionEnd
