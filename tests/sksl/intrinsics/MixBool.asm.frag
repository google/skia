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
               OpMemberName %_UniformBuffer 2 "colorBlack"
               OpMemberName %_UniformBuffer 3 "colorWhite"
               OpMemberName %_UniformBuffer 4 "testInputs"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %intGreen "intGreen"
               OpName %intRed "intRed"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
               OpDecorate %213 RelaxedPrecision
               OpDecorate %214 RelaxedPrecision
               OpDecorate %216 RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %219 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %222 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %231 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %235 RelaxedPrecision
               OpDecorate %236 RelaxedPrecision
               OpDecorate %238 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %241 RelaxedPrecision
               OpDecorate %242 RelaxedPrecision
               OpDecorate %244 RelaxedPrecision
               OpDecorate %245 RelaxedPrecision
               OpDecorate %253 RelaxedPrecision
               OpDecorate %255 RelaxedPrecision
               OpDecorate %257 RelaxedPrecision
               OpDecorate %259 RelaxedPrecision
               OpDecorate %261 RelaxedPrecision
               OpDecorate %269 RelaxedPrecision
               OpDecorate %270 RelaxedPrecision
               OpDecorate %272 RelaxedPrecision
               OpDecorate %273 RelaxedPrecision
               OpDecorate %275 RelaxedPrecision
               OpDecorate %276 RelaxedPrecision
               OpDecorate %278 RelaxedPrecision
               OpDecorate %279 RelaxedPrecision
               OpDecorate %281 RelaxedPrecision
               OpDecorate %282 RelaxedPrecision
               OpDecorate %289 RelaxedPrecision
               OpDecorate %290 RelaxedPrecision
               OpDecorate %292 RelaxedPrecision
               OpDecorate %293 RelaxedPrecision
               OpDecorate %295 RelaxedPrecision
               OpDecorate %296 RelaxedPrecision
               OpDecorate %298 RelaxedPrecision
               OpDecorate %299 RelaxedPrecision
               OpDecorate %301 RelaxedPrecision
               OpDecorate %302 RelaxedPrecision
               OpDecorate %310 RelaxedPrecision
               OpDecorate %311 RelaxedPrecision
               OpDecorate %313 RelaxedPrecision
               OpDecorate %314 RelaxedPrecision
               OpDecorate %316 RelaxedPrecision
               OpDecorate %317 RelaxedPrecision
               OpDecorate %319 RelaxedPrecision
               OpDecorate %320 RelaxedPrecision
               OpDecorate %322 RelaxedPrecision
               OpDecorate %323 RelaxedPrecision
               OpDecorate %331 RelaxedPrecision
               OpDecorate %333 RelaxedPrecision
               OpDecorate %335 RelaxedPrecision
               OpDecorate %337 RelaxedPrecision
               OpDecorate %339 RelaxedPrecision
               OpDecorate %346 RelaxedPrecision
               OpDecorate %347 RelaxedPrecision
               OpDecorate %355 RelaxedPrecision
               OpDecorate %356 RelaxedPrecision
               OpDecorate %364 RelaxedPrecision
               OpDecorate %365 RelaxedPrecision
               OpDecorate %373 RelaxedPrecision
               OpDecorate %380 RelaxedPrecision
               OpDecorate %381 RelaxedPrecision
               OpDecorate %388 RelaxedPrecision
               OpDecorate %389 RelaxedPrecision
               OpDecorate %397 RelaxedPrecision
               OpDecorate %398 RelaxedPrecision
               OpDecorate %406 RelaxedPrecision
               OpDecorate %416 RelaxedPrecision
               OpDecorate %418 RelaxedPrecision
               OpDecorate %419 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
        %int = OpTypeInt 32 1
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
  %float_100 = OpConstant %float 100
      %int_1 = OpConstant %int 1
      %false = OpConstantFalse %bool
      %v2int = OpTypeVector %int 2
     %v2bool = OpTypeVector %bool 2
         %71 = OpConstantComposite %v2bool %false %false
      %v3int = OpTypeVector %int 3
     %v3bool = OpTypeVector %bool 3
         %85 = OpConstantComposite %v3bool %false %false %false
     %v4bool = OpTypeVector %bool 4
         %96 = OpConstantComposite %v4bool %false %false %false %false
       %true = OpConstantTrue %bool
        %111 = OpConstantComposite %v2bool %true %true
        %123 = OpConstantComposite %v3bool %true %true %true
        %133 = OpConstantComposite %v4bool %true %true %true %true
    %int_100 = OpConstant %int 100
        %144 = OpConstantComposite %v2int %int_0 %int_100
        %151 = OpConstantComposite %v3int %int_0 %int_100 %int_0
        %158 = OpConstantComposite %v4int %int_0 %int_100 %int_0 %int_100
        %168 = OpConstantComposite %v2int %int_100 %int_0
        %175 = OpConstantComposite %v3int %int_100 %int_0 %int_0
        %182 = OpConstantComposite %v4int %int_100 %int_0 %int_0 %int_100
    %v3float = OpTypeVector %float 3
    %float_1 = OpConstant %float 1
        %353 = OpConstantComposite %v2float %float_0 %float_1
        %362 = OpConstantComposite %v3float %float_0 %float_1 %float_0
        %371 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
        %386 = OpConstantComposite %v2float %float_1 %float_0
        %395 = OpConstantComposite %v3float %float_1 %float_0 %float_0
        %404 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
   %intGreen = OpVariable %_ptr_Function_v4int Function
     %intRed = OpVariable %_ptr_Function_v4int Function
        %410 = OpVariable %_ptr_Function_v4float Function
         %30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %33 = OpLoad %v4float %30
         %35 = OpVectorTimesScalar %v4float %33 %float_100
         %36 = OpCompositeExtract %float %35 0
         %37 = OpConvertFToS %int %36
         %38 = OpCompositeExtract %float %35 1
         %39 = OpConvertFToS %int %38
         %40 = OpCompositeExtract %float %35 2
         %41 = OpConvertFToS %int %40
         %42 = OpCompositeExtract %float %35 3
         %43 = OpConvertFToS %int %42
         %44 = OpCompositeConstruct %v4int %37 %39 %41 %43
               OpStore %intGreen %44
         %46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %48 = OpLoad %v4float %46
         %49 = OpVectorTimesScalar %v4float %48 %float_100
         %50 = OpCompositeExtract %float %49 0
         %51 = OpConvertFToS %int %50
         %52 = OpCompositeExtract %float %49 1
         %53 = OpConvertFToS %int %52
         %54 = OpCompositeExtract %float %49 2
         %55 = OpConvertFToS %int %54
         %56 = OpCompositeExtract %float %49 3
         %57 = OpConvertFToS %int %56
         %58 = OpCompositeConstruct %v4int %51 %53 %55 %57
               OpStore %intRed %58
         %61 = OpCompositeExtract %int %44 0
         %62 = OpCompositeExtract %int %58 0
         %60 = OpSelect %int %false %62 %61
         %63 = OpIEqual %bool %60 %61
               OpSelectionMerge %65 None
               OpBranchConditional %63 %64 %65
         %64 = OpLabel
         %67 = OpVectorShuffle %v2int %44 %44 0 1
         %69 = OpVectorShuffle %v2int %58 %58 0 1
         %72 = OpVectorShuffle %v2int %44 %44 0 1
         %73 = OpVectorShuffle %v2int %58 %58 0 1
         %66 = OpSelect %v2int %71 %73 %72
         %74 = OpVectorShuffle %v2int %44 %44 0 1
         %75 = OpIEqual %v2bool %66 %74
         %76 = OpAll %bool %75
               OpBranch %65
         %65 = OpLabel
         %77 = OpPhi %bool %false %25 %76 %64
               OpSelectionMerge %79 None
               OpBranchConditional %77 %78 %79
         %78 = OpLabel
         %81 = OpVectorShuffle %v3int %44 %44 0 1 2
         %83 = OpVectorShuffle %v3int %58 %58 0 1 2
         %86 = OpVectorShuffle %v3int %44 %44 0 1 2
         %87 = OpVectorShuffle %v3int %58 %58 0 1 2
         %80 = OpSelect %v3int %85 %87 %86
         %88 = OpVectorShuffle %v3int %44 %44 0 1 2
         %89 = OpIEqual %v3bool %80 %88
         %90 = OpAll %bool %89
               OpBranch %79
         %79 = OpLabel
         %91 = OpPhi %bool %false %65 %90 %78
               OpSelectionMerge %93 None
               OpBranchConditional %91 %92 %93
         %92 = OpLabel
         %94 = OpSelect %v4int %96 %58 %44
         %97 = OpIEqual %v4bool %94 %44
         %98 = OpAll %bool %97
               OpBranch %93
         %93 = OpLabel
         %99 = OpPhi %bool %false %79 %98 %92
               OpSelectionMerge %101 None
               OpBranchConditional %99 %100 %101
        %100 = OpLabel
        %102 = OpSelect %int %true %62 %61
        %104 = OpIEqual %bool %102 %62
               OpBranch %101
        %101 = OpLabel
        %105 = OpPhi %bool %false %93 %104 %100
               OpSelectionMerge %107 None
               OpBranchConditional %105 %106 %107
        %106 = OpLabel
        %109 = OpVectorShuffle %v2int %44 %44 0 1
        %110 = OpVectorShuffle %v2int %58 %58 0 1
        %112 = OpVectorShuffle %v2int %44 %44 0 1
        %113 = OpVectorShuffle %v2int %58 %58 0 1
        %108 = OpSelect %v2int %111 %113 %112
        %114 = OpVectorShuffle %v2int %58 %58 0 1
        %115 = OpIEqual %v2bool %108 %114
        %116 = OpAll %bool %115
               OpBranch %107
        %107 = OpLabel
        %117 = OpPhi %bool %false %101 %116 %106
               OpSelectionMerge %119 None
               OpBranchConditional %117 %118 %119
        %118 = OpLabel
        %121 = OpVectorShuffle %v3int %44 %44 0 1 2
        %122 = OpVectorShuffle %v3int %58 %58 0 1 2
        %124 = OpVectorShuffle %v3int %44 %44 0 1 2
        %125 = OpVectorShuffle %v3int %58 %58 0 1 2
        %120 = OpSelect %v3int %123 %125 %124
        %126 = OpVectorShuffle %v3int %58 %58 0 1 2
        %127 = OpIEqual %v3bool %120 %126
        %128 = OpAll %bool %127
               OpBranch %119
        %119 = OpLabel
        %129 = OpPhi %bool %false %107 %128 %118
               OpSelectionMerge %131 None
               OpBranchConditional %129 %130 %131
        %130 = OpLabel
        %132 = OpSelect %v4int %133 %58 %44
        %134 = OpIEqual %v4bool %132 %58
        %135 = OpAll %bool %134
               OpBranch %131
        %131 = OpLabel
        %136 = OpPhi %bool %false %119 %135 %130
               OpSelectionMerge %138 None
               OpBranchConditional %136 %137 %138
        %137 = OpLabel
        %139 = OpIEqual %bool %int_0 %61
               OpBranch %138
        %138 = OpLabel
        %140 = OpPhi %bool %false %131 %139 %137
               OpSelectionMerge %142 None
               OpBranchConditional %140 %141 %142
        %141 = OpLabel
        %145 = OpVectorShuffle %v2int %44 %44 0 1
        %146 = OpIEqual %v2bool %144 %145
        %147 = OpAll %bool %146
               OpBranch %142
        %142 = OpLabel
        %148 = OpPhi %bool %false %138 %147 %141
               OpSelectionMerge %150 None
               OpBranchConditional %148 %149 %150
        %149 = OpLabel
        %152 = OpVectorShuffle %v3int %44 %44 0 1 2
        %153 = OpIEqual %v3bool %151 %152
        %154 = OpAll %bool %153
               OpBranch %150
        %150 = OpLabel
        %155 = OpPhi %bool %false %142 %154 %149
               OpSelectionMerge %157 None
               OpBranchConditional %155 %156 %157
        %156 = OpLabel
        %159 = OpIEqual %v4bool %158 %44
        %160 = OpAll %bool %159
               OpBranch %157
        %157 = OpLabel
        %161 = OpPhi %bool %false %150 %160 %156
               OpSelectionMerge %163 None
               OpBranchConditional %161 %162 %163
        %162 = OpLabel
        %164 = OpIEqual %bool %int_100 %62
               OpBranch %163
        %163 = OpLabel
        %165 = OpPhi %bool %false %157 %164 %162
               OpSelectionMerge %167 None
               OpBranchConditional %165 %166 %167
        %166 = OpLabel
        %169 = OpVectorShuffle %v2int %58 %58 0 1
        %170 = OpIEqual %v2bool %168 %169
        %171 = OpAll %bool %170
               OpBranch %167
        %167 = OpLabel
        %172 = OpPhi %bool %false %163 %171 %166
               OpSelectionMerge %174 None
               OpBranchConditional %172 %173 %174
        %173 = OpLabel
        %176 = OpVectorShuffle %v3int %58 %58 0 1 2
        %177 = OpIEqual %v3bool %175 %176
        %178 = OpAll %bool %177
               OpBranch %174
        %174 = OpLabel
        %179 = OpPhi %bool %false %167 %178 %173
               OpSelectionMerge %181 None
               OpBranchConditional %179 %180 %181
        %180 = OpLabel
        %183 = OpIEqual %v4bool %182 %58
        %184 = OpAll %bool %183
               OpBranch %181
        %181 = OpLabel
        %185 = OpPhi %bool %false %174 %184 %180
               OpSelectionMerge %187 None
               OpBranchConditional %185 %186 %187
        %186 = OpLabel
        %189 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %190 = OpLoad %v4float %189
        %191 = OpCompositeExtract %float %190 0
        %192 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %193 = OpLoad %v4float %192
        %194 = OpCompositeExtract %float %193 0
        %195 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %196 = OpLoad %v4float %195
        %197 = OpCompositeExtract %float %196 0
        %198 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %199 = OpLoad %v4float %198
        %200 = OpCompositeExtract %float %199 0
        %188 = OpSelect %float %false %200 %197
        %201 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %202 = OpLoad %v4float %201
        %203 = OpCompositeExtract %float %202 0
        %204 = OpFOrdEqual %bool %188 %203
               OpBranch %187
        %187 = OpLabel
        %205 = OpPhi %bool %false %181 %204 %186
               OpSelectionMerge %207 None
               OpBranchConditional %205 %206 %207
        %206 = OpLabel
        %209 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %210 = OpLoad %v4float %209
        %211 = OpVectorShuffle %v2float %210 %210 0 1
        %212 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %213 = OpLoad %v4float %212
        %214 = OpVectorShuffle %v2float %213 %213 0 1
        %215 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %216 = OpLoad %v4float %215
        %217 = OpVectorShuffle %v2float %216 %216 0 1
        %218 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %219 = OpLoad %v4float %218
        %220 = OpVectorShuffle %v2float %219 %219 0 1
        %208 = OpSelect %v2float %71 %220 %217
        %221 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %222 = OpLoad %v4float %221
        %223 = OpVectorShuffle %v2float %222 %222 0 1
        %224 = OpFOrdEqual %v2bool %208 %223
        %225 = OpAll %bool %224
               OpBranch %207
        %207 = OpLabel
        %226 = OpPhi %bool %false %187 %225 %206
               OpSelectionMerge %228 None
               OpBranchConditional %226 %227 %228
        %227 = OpLabel
        %230 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %231 = OpLoad %v4float %230
        %232 = OpVectorShuffle %v3float %231 %231 0 1 2
        %234 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %235 = OpLoad %v4float %234
        %236 = OpVectorShuffle %v3float %235 %235 0 1 2
        %237 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %238 = OpLoad %v4float %237
        %239 = OpVectorShuffle %v3float %238 %238 0 1 2
        %240 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %241 = OpLoad %v4float %240
        %242 = OpVectorShuffle %v3float %241 %241 0 1 2
        %229 = OpSelect %v3float %85 %242 %239
        %243 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %244 = OpLoad %v4float %243
        %245 = OpVectorShuffle %v3float %244 %244 0 1 2
        %246 = OpFOrdEqual %v3bool %229 %245
        %247 = OpAll %bool %246
               OpBranch %228
        %228 = OpLabel
        %248 = OpPhi %bool %false %207 %247 %227
               OpSelectionMerge %250 None
               OpBranchConditional %248 %249 %250
        %249 = OpLabel
        %252 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %253 = OpLoad %v4float %252
        %254 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %255 = OpLoad %v4float %254
        %256 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %257 = OpLoad %v4float %256
        %258 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %259 = OpLoad %v4float %258
        %251 = OpSelect %v4float %96 %259 %257
        %260 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %261 = OpLoad %v4float %260
        %262 = OpFOrdEqual %v4bool %251 %261
        %263 = OpAll %bool %262
               OpBranch %250
        %250 = OpLabel
        %264 = OpPhi %bool %false %228 %263 %249
               OpSelectionMerge %266 None
               OpBranchConditional %264 %265 %266
        %265 = OpLabel
        %268 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %269 = OpLoad %v4float %268
        %270 = OpCompositeExtract %float %269 0
        %271 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %272 = OpLoad %v4float %271
        %273 = OpCompositeExtract %float %272 0
        %274 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %275 = OpLoad %v4float %274
        %276 = OpCompositeExtract %float %275 0
        %277 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %278 = OpLoad %v4float %277
        %279 = OpCompositeExtract %float %278 0
        %267 = OpSelect %float %true %279 %276
        %280 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %281 = OpLoad %v4float %280
        %282 = OpCompositeExtract %float %281 0
        %283 = OpFOrdEqual %bool %267 %282
               OpBranch %266
        %266 = OpLabel
        %284 = OpPhi %bool %false %250 %283 %265
               OpSelectionMerge %286 None
               OpBranchConditional %284 %285 %286
        %285 = OpLabel
        %288 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %289 = OpLoad %v4float %288
        %290 = OpVectorShuffle %v2float %289 %289 0 1
        %291 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %292 = OpLoad %v4float %291
        %293 = OpVectorShuffle %v2float %292 %292 0 1
        %294 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %295 = OpLoad %v4float %294
        %296 = OpVectorShuffle %v2float %295 %295 0 1
        %297 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %298 = OpLoad %v4float %297
        %299 = OpVectorShuffle %v2float %298 %298 0 1
        %287 = OpSelect %v2float %111 %299 %296
        %300 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %301 = OpLoad %v4float %300
        %302 = OpVectorShuffle %v2float %301 %301 0 1
        %303 = OpFOrdEqual %v2bool %287 %302
        %304 = OpAll %bool %303
               OpBranch %286
        %286 = OpLabel
        %305 = OpPhi %bool %false %266 %304 %285
               OpSelectionMerge %307 None
               OpBranchConditional %305 %306 %307
        %306 = OpLabel
        %309 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %310 = OpLoad %v4float %309
        %311 = OpVectorShuffle %v3float %310 %310 0 1 2
        %312 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %313 = OpLoad %v4float %312
        %314 = OpVectorShuffle %v3float %313 %313 0 1 2
        %315 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %316 = OpLoad %v4float %315
        %317 = OpVectorShuffle %v3float %316 %316 0 1 2
        %318 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %319 = OpLoad %v4float %318
        %320 = OpVectorShuffle %v3float %319 %319 0 1 2
        %308 = OpSelect %v3float %123 %320 %317
        %321 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %322 = OpLoad %v4float %321
        %323 = OpVectorShuffle %v3float %322 %322 0 1 2
        %324 = OpFOrdEqual %v3bool %308 %323
        %325 = OpAll %bool %324
               OpBranch %307
        %307 = OpLabel
        %326 = OpPhi %bool %false %286 %325 %306
               OpSelectionMerge %328 None
               OpBranchConditional %326 %327 %328
        %327 = OpLabel
        %330 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %331 = OpLoad %v4float %330
        %332 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %333 = OpLoad %v4float %332
        %334 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %335 = OpLoad %v4float %334
        %336 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %337 = OpLoad %v4float %336
        %329 = OpSelect %v4float %133 %337 %335
        %338 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %339 = OpLoad %v4float %338
        %340 = OpFOrdEqual %v4bool %329 %339
        %341 = OpAll %bool %340
               OpBranch %328
        %328 = OpLabel
        %342 = OpPhi %bool %false %307 %341 %327
               OpSelectionMerge %344 None
               OpBranchConditional %342 %343 %344
        %343 = OpLabel
        %345 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %346 = OpLoad %v4float %345
        %347 = OpCompositeExtract %float %346 0
        %348 = OpFOrdEqual %bool %float_0 %347
               OpBranch %344
        %344 = OpLabel
        %349 = OpPhi %bool %false %328 %348 %343
               OpSelectionMerge %351 None
               OpBranchConditional %349 %350 %351
        %350 = OpLabel
        %354 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %355 = OpLoad %v4float %354
        %356 = OpVectorShuffle %v2float %355 %355 0 1
        %357 = OpFOrdEqual %v2bool %353 %356
        %358 = OpAll %bool %357
               OpBranch %351
        %351 = OpLabel
        %359 = OpPhi %bool %false %344 %358 %350
               OpSelectionMerge %361 None
               OpBranchConditional %359 %360 %361
        %360 = OpLabel
        %363 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %364 = OpLoad %v4float %363
        %365 = OpVectorShuffle %v3float %364 %364 0 1 2
        %366 = OpFOrdEqual %v3bool %362 %365
        %367 = OpAll %bool %366
               OpBranch %361
        %361 = OpLabel
        %368 = OpPhi %bool %false %351 %367 %360
               OpSelectionMerge %370 None
               OpBranchConditional %368 %369 %370
        %369 = OpLabel
        %372 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %373 = OpLoad %v4float %372
        %374 = OpFOrdEqual %v4bool %371 %373
        %375 = OpAll %bool %374
               OpBranch %370
        %370 = OpLabel
        %376 = OpPhi %bool %false %361 %375 %369
               OpSelectionMerge %378 None
               OpBranchConditional %376 %377 %378
        %377 = OpLabel
        %379 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %380 = OpLoad %v4float %379
        %381 = OpCompositeExtract %float %380 0
        %382 = OpFOrdEqual %bool %float_1 %381
               OpBranch %378
        %378 = OpLabel
        %383 = OpPhi %bool %false %370 %382 %377
               OpSelectionMerge %385 None
               OpBranchConditional %383 %384 %385
        %384 = OpLabel
        %387 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %388 = OpLoad %v4float %387
        %389 = OpVectorShuffle %v2float %388 %388 0 1
        %390 = OpFOrdEqual %v2bool %386 %389
        %391 = OpAll %bool %390
               OpBranch %385
        %385 = OpLabel
        %392 = OpPhi %bool %false %378 %391 %384
               OpSelectionMerge %394 None
               OpBranchConditional %392 %393 %394
        %393 = OpLabel
        %396 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %397 = OpLoad %v4float %396
        %398 = OpVectorShuffle %v3float %397 %397 0 1 2
        %399 = OpFOrdEqual %v3bool %395 %398
        %400 = OpAll %bool %399
               OpBranch %394
        %394 = OpLabel
        %401 = OpPhi %bool %false %385 %400 %393
               OpSelectionMerge %403 None
               OpBranchConditional %401 %402 %403
        %402 = OpLabel
        %405 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %406 = OpLoad %v4float %405
        %407 = OpFOrdEqual %v4bool %404 %406
        %408 = OpAll %bool %407
               OpBranch %403
        %403 = OpLabel
        %409 = OpPhi %bool %false %394 %408 %402
               OpSelectionMerge %414 None
               OpBranchConditional %409 %412 %413
        %412 = OpLabel
        %415 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %416 = OpLoad %v4float %415
               OpStore %410 %416
               OpBranch %414
        %413 = OpLabel
        %417 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %418 = OpLoad %v4float %417
               OpStore %410 %418
               OpBranch %414
        %414 = OpLabel
        %419 = OpLoad %v4float %410
               OpReturnValue %419
               OpFunctionEnd
