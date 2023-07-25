               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorRed"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %test_int_b "test_int_b"
               OpName %ok "ok"
               OpName %inputRed "inputRed"
               OpName %inputGreen "inputGreen"
               OpName %x "x"
               OpName %main "main"
               OpName %_0_ok "_0_ok"
               OpName %_1_inputRed "_1_inputRed"
               OpName %_2_inputGreen "_2_inputGreen"
               OpName %_3_x "_3_x"
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
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %_1_inputRed RelaxedPrecision
               OpDecorate %213 RelaxedPrecision
               OpDecorate %_2_inputGreen RelaxedPrecision
               OpDecorate %216 RelaxedPrecision
               OpDecorate %_3_x RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %228 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %238 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %248 RelaxedPrecision
               OpDecorate %251 RelaxedPrecision
               OpDecorate %252 RelaxedPrecision
               OpDecorate %253 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
               OpDecorate %261 RelaxedPrecision
               OpDecorate %262 RelaxedPrecision
               OpDecorate %263 RelaxedPrecision
               OpDecorate %273 RelaxedPrecision
               OpDecorate %274 RelaxedPrecision
               OpDecorate %281 RelaxedPrecision
               OpDecorate %288 RelaxedPrecision
               OpDecorate %290 RelaxedPrecision
               OpDecorate %297 RelaxedPrecision
               OpDecorate %298 RelaxedPrecision
               OpDecorate %299 RelaxedPrecision
               OpDecorate %307 RelaxedPrecision
               OpDecorate %308 RelaxedPrecision
               OpDecorate %309 RelaxedPrecision
               OpDecorate %310 RelaxedPrecision
               OpDecorate %318 RelaxedPrecision
               OpDecorate %320 RelaxedPrecision
               OpDecorate %321 RelaxedPrecision
               OpDecorate %322 RelaxedPrecision
               OpDecorate %331 RelaxedPrecision
               OpDecorate %332 RelaxedPrecision
               OpDecorate %339 RelaxedPrecision
               OpDecorate %340 RelaxedPrecision
               OpDecorate %342 RelaxedPrecision
               OpDecorate %344 RelaxedPrecision
               OpDecorate %350 RelaxedPrecision
               OpDecorate %351 RelaxedPrecision
               OpDecorate %352 RelaxedPrecision
               OpDecorate %353 RelaxedPrecision
               OpDecorate %368 RelaxedPrecision
               OpDecorate %370 RelaxedPrecision
               OpDecorate %371 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
        %int = OpTypeInt 32 1
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
         %61 = OpConstantComposite %v4int %int_2 %int_2 %int_2 %int_2
      %false = OpConstantFalse %bool
      %int_3 = OpConstant %int 3
         %67 = OpConstantComposite %v4int %int_3 %int_2 %int_2 %int_3
     %v4bool = OpTypeVector %bool 4
     %int_n1 = OpConstant %int -1
     %int_n2 = OpConstant %int -2
         %78 = OpConstantComposite %v4int %int_n1 %int_n1 %int_n2 %int_n2
         %87 = OpConstantComposite %v4int %int_2 %int_1 %int_1 %int_2
      %v3int = OpTypeVector %int 3
      %int_9 = OpConstant %int 9
         %94 = OpConstantComposite %v3int %int_9 %int_9 %int_9
        %100 = OpConstantComposite %v4int %int_9 %int_9 %int_9 %int_2
      %v2int = OpTypeVector %int 2
      %int_4 = OpConstant %int 4
        %107 = OpConstantComposite %v2int %int_4 %int_4
        %113 = OpConstantComposite %v4int %int_2 %int_0 %int_9 %int_2
      %int_5 = OpConstant %int 5
        %118 = OpConstantComposite %v4int %int_5 %int_5 %int_5 %int_5
        %123 = OpConstantComposite %v4int %int_0 %int_5 %int_5 %int_0
     %int_10 = OpConstant %int 10
        %135 = OpConstantComposite %v4int %int_10 %int_10 %int_10 %int_10
        %139 = OpConstantComposite %v4int %int_9 %int_9 %int_10 %int_10
        %148 = OpConstantComposite %v4int %int_1 %int_2 %int_1 %int_2
      %int_8 = OpConstant %int 8
        %154 = OpConstantComposite %v3int %int_8 %int_8 %int_8
        %160 = OpConstantComposite %v4int %int_8 %int_8 %int_8 %int_2
     %int_36 = OpConstant %int 36
        %166 = OpConstantComposite %v2int %int_36 %int_36
     %int_18 = OpConstant %int 18
        %173 = OpConstantComposite %v4int %int_4 %int_18 %int_8 %int_2
     %int_37 = OpConstant %int 37
        %178 = OpConstantComposite %v4int %int_37 %int_37 %int_37 %int_37
        %183 = OpConstantComposite %v4int %int_2 %int_9 %int_18 %int_4
        %189 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
        %206 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_2 = OpConstant %float 2
        %219 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
    %float_3 = OpConstant %float 3
        %224 = OpConstantComposite %v4float %float_3 %float_2 %float_2 %float_3
   %float_n1 = OpConstant %float -1
   %float_n2 = OpConstant %float -2
        %234 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n2 %float_n2
    %float_1 = OpConstant %float 1
        %244 = OpConstantComposite %v4float %float_2 %float_1 %float_1 %float_2
    %v3float = OpTypeVector %float 3
    %float_9 = OpConstant %float 9
        %256 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_2
   %float_18 = OpConstant %float 18
    %float_4 = OpConstant %float 4
        %268 = OpConstantComposite %v4float %float_18 %float_4 %float_9 %float_2
    %float_5 = OpConstant %float 5
        %277 = OpConstantComposite %v4float %float_0 %float_5 %float_5 %float_0
   %float_10 = OpConstant %float 10
        %289 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
        %293 = OpConstantComposite %v4float %float_9 %float_9 %float_10 %float_10
        %302 = OpConstantComposite %v4float %float_1 %float_2 %float_1 %float_2
    %float_8 = OpConstant %float 8
        %313 = OpConstantComposite %v4float %float_8 %float_8 %float_8 %float_2
   %float_32 = OpConstant %float 32
        %319 = OpConstantComposite %v2float %float_32 %float_32
   %float_16 = OpConstant %float 16
        %326 = OpConstantComposite %v4float %float_4 %float_16 %float_8 %float_2
        %330 = OpConstantComposite %v4float %float_32 %float_32 %float_32 %float_32
        %335 = OpConstantComposite %v4float %float_2 %float_8 %float_16 %float_4
        %341 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
  %float_0_5 = OpConstant %float 0.5
%_entrypoint_v = OpFunction %void None %16
         %17 = OpLabel
         %21 = OpVariable %_ptr_Function_v2float Function
               OpStore %21 %20
         %23 = OpFunctionCall %v4float %main %21
               OpStore %sk_FragColor %23
               OpReturn
               OpFunctionEnd
 %test_int_b = OpFunction %bool None %24
         %25 = OpLabel
         %ok = OpVariable %_ptr_Function_bool Function
   %inputRed = OpVariable %_ptr_Function_v4int Function
 %inputGreen = OpVariable %_ptr_Function_v4int Function
          %x = OpVariable %_ptr_Function_v4int Function
               OpStore %ok %true
         %33 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %36 = OpLoad %v4float %33
         %37 = OpCompositeExtract %float %36 0
         %38 = OpConvertFToS %int %37
         %39 = OpCompositeExtract %float %36 1
         %40 = OpConvertFToS %int %39
         %41 = OpCompositeExtract %float %36 2
         %42 = OpConvertFToS %int %41
         %43 = OpCompositeExtract %float %36 3
         %44 = OpConvertFToS %int %43
         %45 = OpCompositeConstruct %v4int %38 %40 %42 %44
               OpStore %inputRed %45
         %47 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %49 = OpLoad %v4float %47
         %50 = OpCompositeExtract %float %49 0
         %51 = OpConvertFToS %int %50
         %52 = OpCompositeExtract %float %49 1
         %53 = OpConvertFToS %int %52
         %54 = OpCompositeExtract %float %49 2
         %55 = OpConvertFToS %int %54
         %56 = OpCompositeExtract %float %49 3
         %57 = OpConvertFToS %int %56
         %58 = OpCompositeConstruct %v4int %51 %53 %55 %57
               OpStore %inputGreen %58
         %62 = OpIAdd %v4int %45 %61
               OpStore %x %62
               OpSelectionMerge %65 None
               OpBranchConditional %true %64 %65
         %64 = OpLabel
         %68 = OpIEqual %v4bool %62 %67
         %70 = OpAll %bool %68
               OpBranch %65
         %65 = OpLabel
         %71 = OpPhi %bool %false %25 %70 %64
               OpStore %ok %71
         %72 = OpVectorShuffle %v4int %58 %58 1 3 0 2
         %73 = OpISub %v4int %72 %61
               OpStore %x %73
               OpSelectionMerge %75 None
               OpBranchConditional %71 %74 %75
         %74 = OpLabel
         %79 = OpIEqual %v4bool %73 %78
         %80 = OpAll %bool %79
               OpBranch %75
         %75 = OpLabel
         %81 = OpPhi %bool %false %65 %80 %74
               OpStore %ok %81
         %82 = OpCompositeExtract %int %58 1
         %83 = OpCompositeConstruct %v4int %82 %82 %82 %82
         %84 = OpIAdd %v4int %45 %83
               OpStore %x %84
               OpSelectionMerge %86 None
               OpBranchConditional %81 %85 %86
         %85 = OpLabel
         %88 = OpIEqual %v4bool %84 %87
         %89 = OpAll %bool %88
               OpBranch %86
         %86 = OpLabel
         %90 = OpPhi %bool %false %75 %89 %85
               OpStore %ok %90
         %91 = OpVectorShuffle %v3int %58 %58 3 1 3
         %95 = OpIMul %v3int %91 %94
         %96 = OpLoad %v4int %x
         %97 = OpVectorShuffle %v4int %96 %95 4 5 6 3
               OpStore %x %97
               OpSelectionMerge %99 None
               OpBranchConditional %90 %98 %99
         %98 = OpLabel
        %101 = OpIEqual %v4bool %97 %100
        %102 = OpAll %bool %101
               OpBranch %99
         %99 = OpLabel
        %103 = OpPhi %bool %false %86 %102 %98
               OpStore %ok %103
        %104 = OpVectorShuffle %v2int %97 %97 2 3
        %108 = OpSDiv %v2int %104 %107
        %109 = OpLoad %v4int %x
        %110 = OpVectorShuffle %v4int %109 %108 4 5 2 3
               OpStore %x %110
               OpSelectionMerge %112 None
               OpBranchConditional %103 %111 %112
        %111 = OpLabel
        %114 = OpIEqual %v4bool %110 %113
        %115 = OpAll %bool %114
               OpBranch %112
        %112 = OpLabel
        %116 = OpPhi %bool %false %99 %115 %111
               OpStore %ok %116
        %119 = OpIMul %v4int %45 %118
        %120 = OpVectorShuffle %v4int %119 %119 1 0 3 2
               OpStore %x %120
               OpSelectionMerge %122 None
               OpBranchConditional %116 %121 %122
        %121 = OpLabel
        %124 = OpIEqual %v4bool %120 %123
        %125 = OpAll %bool %124
               OpBranch %122
        %122 = OpLabel
        %126 = OpPhi %bool %false %112 %125 %121
               OpStore %ok %126
        %127 = OpIAdd %v4int %61 %45
               OpStore %x %127
               OpSelectionMerge %129 None
               OpBranchConditional %126 %128 %129
        %128 = OpLabel
        %130 = OpIEqual %v4bool %127 %67
        %131 = OpAll %bool %130
               OpBranch %129
        %129 = OpLabel
        %132 = OpPhi %bool %false %122 %131 %128
               OpStore %ok %132
        %134 = OpVectorShuffle %v4int %58 %58 1 3 0 2
        %136 = OpISub %v4int %135 %134
               OpStore %x %136
               OpSelectionMerge %138 None
               OpBranchConditional %132 %137 %138
        %137 = OpLabel
        %140 = OpIEqual %v4bool %136 %139
        %141 = OpAll %bool %140
               OpBranch %138
        %138 = OpLabel
        %142 = OpPhi %bool %false %129 %141 %137
               OpStore %ok %142
        %143 = OpCompositeExtract %int %45 0
        %144 = OpCompositeConstruct %v4int %143 %143 %143 %143
        %145 = OpIAdd %v4int %144 %58
               OpStore %x %145
               OpSelectionMerge %147 None
               OpBranchConditional %142 %146 %147
        %146 = OpLabel
        %149 = OpIEqual %v4bool %145 %148
        %150 = OpAll %bool %149
               OpBranch %147
        %147 = OpLabel
        %151 = OpPhi %bool %false %138 %150 %146
               OpStore %ok %151
        %153 = OpVectorShuffle %v3int %58 %58 3 1 3
        %155 = OpIMul %v3int %154 %153
        %156 = OpLoad %v4int %x
        %157 = OpVectorShuffle %v4int %156 %155 4 5 6 3
               OpStore %x %157
               OpSelectionMerge %159 None
               OpBranchConditional %151 %158 %159
        %158 = OpLabel
        %161 = OpIEqual %v4bool %157 %160
        %162 = OpAll %bool %161
               OpBranch %159
        %159 = OpLabel
        %163 = OpPhi %bool %false %147 %162 %158
               OpStore %ok %163
        %165 = OpVectorShuffle %v2int %157 %157 2 3
        %167 = OpSDiv %v2int %166 %165
        %168 = OpLoad %v4int %x
        %169 = OpVectorShuffle %v4int %168 %167 4 5 2 3
               OpStore %x %169
               OpSelectionMerge %171 None
               OpBranchConditional %163 %170 %171
        %170 = OpLabel
        %174 = OpIEqual %v4bool %169 %173
        %175 = OpAll %bool %174
               OpBranch %171
        %171 = OpLabel
        %176 = OpPhi %bool %false %159 %175 %170
               OpStore %ok %176
        %179 = OpSDiv %v4int %178 %169
        %180 = OpVectorShuffle %v4int %179 %179 1 0 3 2
               OpStore %x %180
               OpSelectionMerge %182 None
               OpBranchConditional %176 %181 %182
        %181 = OpLabel
        %184 = OpIEqual %v4bool %180 %183
        %185 = OpAll %bool %184
               OpBranch %182
        %182 = OpLabel
        %186 = OpPhi %bool %false %171 %185 %181
               OpStore %ok %186
        %187 = OpIAdd %v4int %180 %61
               OpStore %x %187
        %188 = OpIMul %v4int %187 %61
               OpStore %x %188
        %190 = OpISub %v4int %188 %189
               OpStore %x %190
        %191 = OpSDiv %v4int %190 %61
               OpStore %x %191
               OpSelectionMerge %193 None
               OpBranchConditional %186 %192 %193
        %192 = OpLabel
        %194 = OpIEqual %v4bool %191 %183
        %195 = OpAll %bool %194
               OpBranch %193
        %193 = OpLabel
        %196 = OpPhi %bool %false %182 %195 %192
               OpStore %ok %196
        %197 = OpIAdd %v4int %191 %61
               OpStore %x %197
        %198 = OpIMul %v4int %197 %61
               OpStore %x %198
        %199 = OpISub %v4int %198 %189
               OpStore %x %199
        %200 = OpSDiv %v4int %199 %61
               OpStore %x %200
               OpSelectionMerge %202 None
               OpBranchConditional %196 %201 %202
        %201 = OpLabel
        %203 = OpIEqual %v4bool %200 %183
        %204 = OpAll %bool %203
               OpBranch %202
        %202 = OpLabel
        %205 = OpPhi %bool %false %193 %204 %201
               OpStore %ok %205
               OpReturnValue %205
               OpFunctionEnd
       %main = OpFunction %v4float None %206
        %207 = OpFunctionParameter %_ptr_Function_v2float
        %208 = OpLabel
      %_0_ok = OpVariable %_ptr_Function_bool Function
%_1_inputRed = OpVariable %_ptr_Function_v4float Function
%_2_inputGreen = OpVariable %_ptr_Function_v4float Function
       %_3_x = OpVariable %_ptr_Function_v4float Function
        %363 = OpVariable %_ptr_Function_v4float Function
               OpStore %_0_ok %true
        %212 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %213 = OpLoad %v4float %212
               OpStore %_1_inputRed %213
        %215 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %216 = OpLoad %v4float %215
               OpStore %_2_inputGreen %216
        %220 = OpFAdd %v4float %213 %219
               OpStore %_3_x %220
               OpSelectionMerge %222 None
               OpBranchConditional %true %221 %222
        %221 = OpLabel
        %225 = OpFOrdEqual %v4bool %220 %224
        %226 = OpAll %bool %225
               OpBranch %222
        %222 = OpLabel
        %227 = OpPhi %bool %false %208 %226 %221
               OpStore %_0_ok %227
        %228 = OpVectorShuffle %v4float %216 %216 1 3 0 2
        %229 = OpFSub %v4float %228 %219
               OpStore %_3_x %229
               OpSelectionMerge %231 None
               OpBranchConditional %227 %230 %231
        %230 = OpLabel
        %235 = OpFOrdEqual %v4bool %229 %234
        %236 = OpAll %bool %235
               OpBranch %231
        %231 = OpLabel
        %237 = OpPhi %bool %false %222 %236 %230
               OpStore %_0_ok %237
        %238 = OpCompositeExtract %float %216 1
        %239 = OpCompositeConstruct %v4float %238 %238 %238 %238
        %240 = OpFAdd %v4float %213 %239
               OpStore %_3_x %240
               OpSelectionMerge %242 None
               OpBranchConditional %237 %241 %242
        %241 = OpLabel
        %245 = OpFOrdEqual %v4bool %240 %244
        %246 = OpAll %bool %245
               OpBranch %242
        %242 = OpLabel
        %247 = OpPhi %bool %false %231 %246 %241
               OpStore %_0_ok %247
        %248 = OpVectorShuffle %v3float %216 %216 3 1 3
        %251 = OpVectorTimesScalar %v3float %248 %float_9
        %252 = OpLoad %v4float %_3_x
        %253 = OpVectorShuffle %v4float %252 %251 4 5 6 3
               OpStore %_3_x %253
               OpSelectionMerge %255 None
               OpBranchConditional %247 %254 %255
        %254 = OpLabel
        %257 = OpFOrdEqual %v4bool %253 %256
        %258 = OpAll %bool %257
               OpBranch %255
        %255 = OpLabel
        %259 = OpPhi %bool %false %242 %258 %254
               OpStore %_0_ok %259
        %260 = OpVectorShuffle %v2float %253 %253 2 3
        %261 = OpVectorTimesScalar %v2float %260 %float_2
        %262 = OpLoad %v4float %_3_x
        %263 = OpVectorShuffle %v4float %262 %261 4 5 2 3
               OpStore %_3_x %263
               OpSelectionMerge %265 None
               OpBranchConditional %259 %264 %265
        %264 = OpLabel
        %269 = OpFOrdEqual %v4bool %263 %268
        %270 = OpAll %bool %269
               OpBranch %265
        %265 = OpLabel
        %271 = OpPhi %bool %false %255 %270 %264
               OpStore %_0_ok %271
        %273 = OpVectorTimesScalar %v4float %213 %float_5
        %274 = OpVectorShuffle %v4float %273 %273 1 0 3 2
               OpStore %_3_x %274
               OpSelectionMerge %276 None
               OpBranchConditional %271 %275 %276
        %275 = OpLabel
        %278 = OpFOrdEqual %v4bool %274 %277
        %279 = OpAll %bool %278
               OpBranch %276
        %276 = OpLabel
        %280 = OpPhi %bool %false %265 %279 %275
               OpStore %_0_ok %280
        %281 = OpFAdd %v4float %219 %213
               OpStore %_3_x %281
               OpSelectionMerge %283 None
               OpBranchConditional %280 %282 %283
        %282 = OpLabel
        %284 = OpFOrdEqual %v4bool %281 %224
        %285 = OpAll %bool %284
               OpBranch %283
        %283 = OpLabel
        %286 = OpPhi %bool %false %276 %285 %282
               OpStore %_0_ok %286
        %288 = OpVectorShuffle %v4float %216 %216 1 3 0 2
        %290 = OpFSub %v4float %289 %288
               OpStore %_3_x %290
               OpSelectionMerge %292 None
               OpBranchConditional %286 %291 %292
        %291 = OpLabel
        %294 = OpFOrdEqual %v4bool %290 %293
        %295 = OpAll %bool %294
               OpBranch %292
        %292 = OpLabel
        %296 = OpPhi %bool %false %283 %295 %291
               OpStore %_0_ok %296
        %297 = OpCompositeExtract %float %213 0
        %298 = OpCompositeConstruct %v4float %297 %297 %297 %297
        %299 = OpFAdd %v4float %298 %216
               OpStore %_3_x %299
               OpSelectionMerge %301 None
               OpBranchConditional %296 %300 %301
        %300 = OpLabel
        %303 = OpFOrdEqual %v4bool %299 %302
        %304 = OpAll %bool %303
               OpBranch %301
        %301 = OpLabel
        %305 = OpPhi %bool %false %292 %304 %300
               OpStore %_0_ok %305
        %307 = OpVectorShuffle %v3float %216 %216 3 1 3
        %308 = OpVectorTimesScalar %v3float %307 %float_8
        %309 = OpLoad %v4float %_3_x
        %310 = OpVectorShuffle %v4float %309 %308 4 5 6 3
               OpStore %_3_x %310
               OpSelectionMerge %312 None
               OpBranchConditional %305 %311 %312
        %311 = OpLabel
        %314 = OpFOrdEqual %v4bool %310 %313
        %315 = OpAll %bool %314
               OpBranch %312
        %312 = OpLabel
        %316 = OpPhi %bool %false %301 %315 %311
               OpStore %_0_ok %316
        %318 = OpVectorShuffle %v2float %310 %310 2 3
        %320 = OpFDiv %v2float %319 %318
        %321 = OpLoad %v4float %_3_x
        %322 = OpVectorShuffle %v4float %321 %320 4 5 2 3
               OpStore %_3_x %322
               OpSelectionMerge %324 None
               OpBranchConditional %316 %323 %324
        %323 = OpLabel
        %327 = OpFOrdEqual %v4bool %322 %326
        %328 = OpAll %bool %327
               OpBranch %324
        %324 = OpLabel
        %329 = OpPhi %bool %false %312 %328 %323
               OpStore %_0_ok %329
        %331 = OpFDiv %v4float %330 %322
        %332 = OpVectorShuffle %v4float %331 %331 1 0 3 2
               OpStore %_3_x %332
               OpSelectionMerge %334 None
               OpBranchConditional %329 %333 %334
        %333 = OpLabel
        %336 = OpFOrdEqual %v4bool %332 %335
        %337 = OpAll %bool %336
               OpBranch %334
        %334 = OpLabel
        %338 = OpPhi %bool %false %324 %337 %333
               OpStore %_0_ok %338
        %339 = OpFAdd %v4float %332 %219
               OpStore %_3_x %339
        %340 = OpVectorTimesScalar %v4float %339 %float_2
               OpStore %_3_x %340
        %342 = OpFSub %v4float %340 %341
               OpStore %_3_x %342
        %344 = OpVectorTimesScalar %v4float %342 %float_0_5
               OpStore %_3_x %344
               OpSelectionMerge %346 None
               OpBranchConditional %338 %345 %346
        %345 = OpLabel
        %347 = OpFOrdEqual %v4bool %344 %335
        %348 = OpAll %bool %347
               OpBranch %346
        %346 = OpLabel
        %349 = OpPhi %bool %false %334 %348 %345
               OpStore %_0_ok %349
        %350 = OpFAdd %v4float %344 %219
               OpStore %_3_x %350
        %351 = OpVectorTimesScalar %v4float %350 %float_2
               OpStore %_3_x %351
        %352 = OpFSub %v4float %351 %341
               OpStore %_3_x %352
        %353 = OpVectorTimesScalar %v4float %352 %float_0_5
               OpStore %_3_x %353
               OpSelectionMerge %355 None
               OpBranchConditional %349 %354 %355
        %354 = OpLabel
        %356 = OpFOrdEqual %v4bool %353 %335
        %357 = OpAll %bool %356
               OpBranch %355
        %355 = OpLabel
        %358 = OpPhi %bool %false %346 %357 %354
               OpStore %_0_ok %358
               OpSelectionMerge %360 None
               OpBranchConditional %358 %359 %360
        %359 = OpLabel
        %361 = OpFunctionCall %bool %test_int_b
               OpBranch %360
        %360 = OpLabel
        %362 = OpPhi %bool %false %355 %361 %359
               OpSelectionMerge %366 None
               OpBranchConditional %362 %364 %365
        %364 = OpLabel
        %367 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %368 = OpLoad %v4float %367
               OpStore %363 %368
               OpBranch %366
        %365 = OpLabel
        %369 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %370 = OpLoad %v4float %369
               OpStore %363 %370
               OpBranch %366
        %366 = OpLabel
        %371 = OpLoad %v4float %363
               OpReturnValue %371
               OpFunctionEnd
