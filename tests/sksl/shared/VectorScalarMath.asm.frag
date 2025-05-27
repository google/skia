               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
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
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %_1_inputRed RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
               OpDecorate %_2_inputGreen RelaxedPrecision
               OpDecorate %214 RelaxedPrecision
               OpDecorate %_3_x RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %226 RelaxedPrecision
               OpDecorate %227 RelaxedPrecision
               OpDecorate %236 RelaxedPrecision
               OpDecorate %237 RelaxedPrecision
               OpDecorate %238 RelaxedPrecision
               OpDecorate %246 RelaxedPrecision
               OpDecorate %249 RelaxedPrecision
               OpDecorate %250 RelaxedPrecision
               OpDecorate %251 RelaxedPrecision
               OpDecorate %258 RelaxedPrecision
               OpDecorate %259 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
               OpDecorate %261 RelaxedPrecision
               OpDecorate %271 RelaxedPrecision
               OpDecorate %272 RelaxedPrecision
               OpDecorate %279 RelaxedPrecision
               OpDecorate %286 RelaxedPrecision
               OpDecorate %288 RelaxedPrecision
               OpDecorate %295 RelaxedPrecision
               OpDecorate %296 RelaxedPrecision
               OpDecorate %297 RelaxedPrecision
               OpDecorate %305 RelaxedPrecision
               OpDecorate %306 RelaxedPrecision
               OpDecorate %307 RelaxedPrecision
               OpDecorate %308 RelaxedPrecision
               OpDecorate %316 RelaxedPrecision
               OpDecorate %318 RelaxedPrecision
               OpDecorate %319 RelaxedPrecision
               OpDecorate %320 RelaxedPrecision
               OpDecorate %329 RelaxedPrecision
               OpDecorate %330 RelaxedPrecision
               OpDecorate %337 RelaxedPrecision
               OpDecorate %338 RelaxedPrecision
               OpDecorate %340 RelaxedPrecision
               OpDecorate %342 RelaxedPrecision
               OpDecorate %348 RelaxedPrecision
               OpDecorate %349 RelaxedPrecision
               OpDecorate %350 RelaxedPrecision
               OpDecorate %351 RelaxedPrecision
               OpDecorate %366 RelaxedPrecision
               OpDecorate %368 RelaxedPrecision
               OpDecorate %369 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %22 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
        %int = OpTypeInt 32 1
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
         %59 = OpConstantComposite %v4int %int_2 %int_2 %int_2 %int_2
      %false = OpConstantFalse %bool
      %int_3 = OpConstant %int 3
         %65 = OpConstantComposite %v4int %int_3 %int_2 %int_2 %int_3
     %v4bool = OpTypeVector %bool 4
     %int_n1 = OpConstant %int -1
     %int_n2 = OpConstant %int -2
         %76 = OpConstantComposite %v4int %int_n1 %int_n1 %int_n2 %int_n2
         %85 = OpConstantComposite %v4int %int_2 %int_1 %int_1 %int_2
      %v3int = OpTypeVector %int 3
      %int_9 = OpConstant %int 9
         %92 = OpConstantComposite %v3int %int_9 %int_9 %int_9
         %98 = OpConstantComposite %v4int %int_9 %int_9 %int_9 %int_2
      %v2int = OpTypeVector %int 2
      %int_4 = OpConstant %int 4
        %105 = OpConstantComposite %v2int %int_4 %int_4
        %111 = OpConstantComposite %v4int %int_2 %int_0 %int_9 %int_2
      %int_5 = OpConstant %int 5
        %116 = OpConstantComposite %v4int %int_5 %int_5 %int_5 %int_5
        %121 = OpConstantComposite %v4int %int_0 %int_5 %int_5 %int_0
     %int_10 = OpConstant %int 10
        %133 = OpConstantComposite %v4int %int_10 %int_10 %int_10 %int_10
        %137 = OpConstantComposite %v4int %int_9 %int_9 %int_10 %int_10
        %146 = OpConstantComposite %v4int %int_1 %int_2 %int_1 %int_2
      %int_8 = OpConstant %int 8
        %152 = OpConstantComposite %v3int %int_8 %int_8 %int_8
        %158 = OpConstantComposite %v4int %int_8 %int_8 %int_8 %int_2
     %int_36 = OpConstant %int 36
        %164 = OpConstantComposite %v2int %int_36 %int_36
     %int_18 = OpConstant %int 18
        %171 = OpConstantComposite %v4int %int_4 %int_18 %int_8 %int_2
     %int_37 = OpConstant %int 37
        %176 = OpConstantComposite %v4int %int_37 %int_37 %int_37 %int_37
        %181 = OpConstantComposite %v4int %int_2 %int_9 %int_18 %int_4
        %187 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
        %204 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_2 = OpConstant %float 2
        %217 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
    %float_3 = OpConstant %float 3
        %222 = OpConstantComposite %v4float %float_3 %float_2 %float_2 %float_3
   %float_n1 = OpConstant %float -1
   %float_n2 = OpConstant %float -2
        %232 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n2 %float_n2
    %float_1 = OpConstant %float 1
        %242 = OpConstantComposite %v4float %float_2 %float_1 %float_1 %float_2
    %v3float = OpTypeVector %float 3
    %float_9 = OpConstant %float 9
        %254 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_2
   %float_18 = OpConstant %float 18
    %float_4 = OpConstant %float 4
        %266 = OpConstantComposite %v4float %float_18 %float_4 %float_9 %float_2
    %float_5 = OpConstant %float 5
        %275 = OpConstantComposite %v4float %float_0 %float_5 %float_5 %float_0
   %float_10 = OpConstant %float 10
        %287 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
        %291 = OpConstantComposite %v4float %float_9 %float_9 %float_10 %float_10
        %300 = OpConstantComposite %v4float %float_1 %float_2 %float_1 %float_2
    %float_8 = OpConstant %float 8
        %311 = OpConstantComposite %v4float %float_8 %float_8 %float_8 %float_2
   %float_32 = OpConstant %float 32
        %317 = OpConstantComposite %v2float %float_32 %float_32
   %float_16 = OpConstant %float 16
        %324 = OpConstantComposite %v4float %float_4 %float_16 %float_8 %float_2
        %328 = OpConstantComposite %v4float %float_32 %float_32 %float_32 %float_32
        %333 = OpConstantComposite %v4float %float_2 %float_8 %float_16 %float_4
        %339 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
  %float_0_5 = OpConstant %float 0.5
%_entrypoint_v = OpFunction %void None %13
         %14 = OpLabel
         %18 = OpVariable %_ptr_Function_v2float Function
               OpStore %18 %17
         %20 = OpFunctionCall %v4float %main %18
               OpStore %sk_FragColor %20
               OpReturn
               OpFunctionEnd
 %test_int_b = OpFunction %bool None %22
         %23 = OpLabel
         %ok = OpVariable %_ptr_Function_bool Function
   %inputRed = OpVariable %_ptr_Function_v4int Function
 %inputGreen = OpVariable %_ptr_Function_v4int Function
          %x = OpVariable %_ptr_Function_v4int Function
               OpStore %ok %true
         %31 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %34 = OpLoad %v4float %31
         %35 = OpCompositeExtract %float %34 0
         %36 = OpConvertFToS %int %35
         %37 = OpCompositeExtract %float %34 1
         %38 = OpConvertFToS %int %37
         %39 = OpCompositeExtract %float %34 2
         %40 = OpConvertFToS %int %39
         %41 = OpCompositeExtract %float %34 3
         %42 = OpConvertFToS %int %41
         %43 = OpCompositeConstruct %v4int %36 %38 %40 %42
               OpStore %inputRed %43
         %45 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
         %47 = OpLoad %v4float %45
         %48 = OpCompositeExtract %float %47 0
         %49 = OpConvertFToS %int %48
         %50 = OpCompositeExtract %float %47 1
         %51 = OpConvertFToS %int %50
         %52 = OpCompositeExtract %float %47 2
         %53 = OpConvertFToS %int %52
         %54 = OpCompositeExtract %float %47 3
         %55 = OpConvertFToS %int %54
         %56 = OpCompositeConstruct %v4int %49 %51 %53 %55
               OpStore %inputGreen %56
         %60 = OpIAdd %v4int %43 %59
               OpStore %x %60
               OpSelectionMerge %63 None
               OpBranchConditional %true %62 %63
         %62 = OpLabel
         %66 = OpIEqual %v4bool %60 %65
         %68 = OpAll %bool %66
               OpBranch %63
         %63 = OpLabel
         %69 = OpPhi %bool %false %23 %68 %62
               OpStore %ok %69
         %70 = OpVectorShuffle %v4int %56 %56 1 3 0 2
         %71 = OpISub %v4int %70 %59
               OpStore %x %71
               OpSelectionMerge %73 None
               OpBranchConditional %69 %72 %73
         %72 = OpLabel
         %77 = OpIEqual %v4bool %71 %76
         %78 = OpAll %bool %77
               OpBranch %73
         %73 = OpLabel
         %79 = OpPhi %bool %false %63 %78 %72
               OpStore %ok %79
         %80 = OpCompositeExtract %int %56 1
         %81 = OpCompositeConstruct %v4int %80 %80 %80 %80
         %82 = OpIAdd %v4int %43 %81
               OpStore %x %82
               OpSelectionMerge %84 None
               OpBranchConditional %79 %83 %84
         %83 = OpLabel
         %86 = OpIEqual %v4bool %82 %85
         %87 = OpAll %bool %86
               OpBranch %84
         %84 = OpLabel
         %88 = OpPhi %bool %false %73 %87 %83
               OpStore %ok %88
         %89 = OpVectorShuffle %v3int %56 %56 3 1 3
         %93 = OpIMul %v3int %89 %92
         %94 = OpLoad %v4int %x
         %95 = OpVectorShuffle %v4int %94 %93 4 5 6 3
               OpStore %x %95
               OpSelectionMerge %97 None
               OpBranchConditional %88 %96 %97
         %96 = OpLabel
         %99 = OpIEqual %v4bool %95 %98
        %100 = OpAll %bool %99
               OpBranch %97
         %97 = OpLabel
        %101 = OpPhi %bool %false %84 %100 %96
               OpStore %ok %101
        %102 = OpVectorShuffle %v2int %95 %95 2 3
        %106 = OpSDiv %v2int %102 %105
        %107 = OpLoad %v4int %x
        %108 = OpVectorShuffle %v4int %107 %106 4 5 2 3
               OpStore %x %108
               OpSelectionMerge %110 None
               OpBranchConditional %101 %109 %110
        %109 = OpLabel
        %112 = OpIEqual %v4bool %108 %111
        %113 = OpAll %bool %112
               OpBranch %110
        %110 = OpLabel
        %114 = OpPhi %bool %false %97 %113 %109
               OpStore %ok %114
        %117 = OpIMul %v4int %43 %116
        %118 = OpVectorShuffle %v4int %117 %117 1 0 3 2
               OpStore %x %118
               OpSelectionMerge %120 None
               OpBranchConditional %114 %119 %120
        %119 = OpLabel
        %122 = OpIEqual %v4bool %118 %121
        %123 = OpAll %bool %122
               OpBranch %120
        %120 = OpLabel
        %124 = OpPhi %bool %false %110 %123 %119
               OpStore %ok %124
        %125 = OpIAdd %v4int %59 %43
               OpStore %x %125
               OpSelectionMerge %127 None
               OpBranchConditional %124 %126 %127
        %126 = OpLabel
        %128 = OpIEqual %v4bool %125 %65
        %129 = OpAll %bool %128
               OpBranch %127
        %127 = OpLabel
        %130 = OpPhi %bool %false %120 %129 %126
               OpStore %ok %130
        %132 = OpVectorShuffle %v4int %56 %56 1 3 0 2
        %134 = OpISub %v4int %133 %132
               OpStore %x %134
               OpSelectionMerge %136 None
               OpBranchConditional %130 %135 %136
        %135 = OpLabel
        %138 = OpIEqual %v4bool %134 %137
        %139 = OpAll %bool %138
               OpBranch %136
        %136 = OpLabel
        %140 = OpPhi %bool %false %127 %139 %135
               OpStore %ok %140
        %141 = OpCompositeExtract %int %43 0
        %142 = OpCompositeConstruct %v4int %141 %141 %141 %141
        %143 = OpIAdd %v4int %142 %56
               OpStore %x %143
               OpSelectionMerge %145 None
               OpBranchConditional %140 %144 %145
        %144 = OpLabel
        %147 = OpIEqual %v4bool %143 %146
        %148 = OpAll %bool %147
               OpBranch %145
        %145 = OpLabel
        %149 = OpPhi %bool %false %136 %148 %144
               OpStore %ok %149
        %151 = OpVectorShuffle %v3int %56 %56 3 1 3
        %153 = OpIMul %v3int %152 %151
        %154 = OpLoad %v4int %x
        %155 = OpVectorShuffle %v4int %154 %153 4 5 6 3
               OpStore %x %155
               OpSelectionMerge %157 None
               OpBranchConditional %149 %156 %157
        %156 = OpLabel
        %159 = OpIEqual %v4bool %155 %158
        %160 = OpAll %bool %159
               OpBranch %157
        %157 = OpLabel
        %161 = OpPhi %bool %false %145 %160 %156
               OpStore %ok %161
        %163 = OpVectorShuffle %v2int %155 %155 2 3
        %165 = OpSDiv %v2int %164 %163
        %166 = OpLoad %v4int %x
        %167 = OpVectorShuffle %v4int %166 %165 4 5 2 3
               OpStore %x %167
               OpSelectionMerge %169 None
               OpBranchConditional %161 %168 %169
        %168 = OpLabel
        %172 = OpIEqual %v4bool %167 %171
        %173 = OpAll %bool %172
               OpBranch %169
        %169 = OpLabel
        %174 = OpPhi %bool %false %157 %173 %168
               OpStore %ok %174
        %177 = OpSDiv %v4int %176 %167
        %178 = OpVectorShuffle %v4int %177 %177 1 0 3 2
               OpStore %x %178
               OpSelectionMerge %180 None
               OpBranchConditional %174 %179 %180
        %179 = OpLabel
        %182 = OpIEqual %v4bool %178 %181
        %183 = OpAll %bool %182
               OpBranch %180
        %180 = OpLabel
        %184 = OpPhi %bool %false %169 %183 %179
               OpStore %ok %184
        %185 = OpIAdd %v4int %178 %59
               OpStore %x %185
        %186 = OpIMul %v4int %185 %59
               OpStore %x %186
        %188 = OpISub %v4int %186 %187
               OpStore %x %188
        %189 = OpSDiv %v4int %188 %59
               OpStore %x %189
               OpSelectionMerge %191 None
               OpBranchConditional %184 %190 %191
        %190 = OpLabel
        %192 = OpIEqual %v4bool %189 %181
        %193 = OpAll %bool %192
               OpBranch %191
        %191 = OpLabel
        %194 = OpPhi %bool %false %180 %193 %190
               OpStore %ok %194
        %195 = OpIAdd %v4int %189 %59
               OpStore %x %195
        %196 = OpIMul %v4int %195 %59
               OpStore %x %196
        %197 = OpISub %v4int %196 %187
               OpStore %x %197
        %198 = OpSDiv %v4int %197 %59
               OpStore %x %198
               OpSelectionMerge %200 None
               OpBranchConditional %194 %199 %200
        %199 = OpLabel
        %201 = OpIEqual %v4bool %198 %181
        %202 = OpAll %bool %201
               OpBranch %200
        %200 = OpLabel
        %203 = OpPhi %bool %false %191 %202 %199
               OpStore %ok %203
               OpReturnValue %203
               OpFunctionEnd
       %main = OpFunction %v4float None %204
        %205 = OpFunctionParameter %_ptr_Function_v2float
        %206 = OpLabel
      %_0_ok = OpVariable %_ptr_Function_bool Function
%_1_inputRed = OpVariable %_ptr_Function_v4float Function
%_2_inputGreen = OpVariable %_ptr_Function_v4float Function
       %_3_x = OpVariable %_ptr_Function_v4float Function
        %361 = OpVariable %_ptr_Function_v4float Function
               OpStore %_0_ok %true
        %210 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %211 = OpLoad %v4float %210
               OpStore %_1_inputRed %211
        %213 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %214 = OpLoad %v4float %213
               OpStore %_2_inputGreen %214
        %218 = OpFAdd %v4float %211 %217
               OpStore %_3_x %218
               OpSelectionMerge %220 None
               OpBranchConditional %true %219 %220
        %219 = OpLabel
        %223 = OpFOrdEqual %v4bool %218 %222
        %224 = OpAll %bool %223
               OpBranch %220
        %220 = OpLabel
        %225 = OpPhi %bool %false %206 %224 %219
               OpStore %_0_ok %225
        %226 = OpVectorShuffle %v4float %214 %214 1 3 0 2
        %227 = OpFSub %v4float %226 %217
               OpStore %_3_x %227
               OpSelectionMerge %229 None
               OpBranchConditional %225 %228 %229
        %228 = OpLabel
        %233 = OpFOrdEqual %v4bool %227 %232
        %234 = OpAll %bool %233
               OpBranch %229
        %229 = OpLabel
        %235 = OpPhi %bool %false %220 %234 %228
               OpStore %_0_ok %235
        %236 = OpCompositeExtract %float %214 1
        %237 = OpCompositeConstruct %v4float %236 %236 %236 %236
        %238 = OpFAdd %v4float %211 %237
               OpStore %_3_x %238
               OpSelectionMerge %240 None
               OpBranchConditional %235 %239 %240
        %239 = OpLabel
        %243 = OpFOrdEqual %v4bool %238 %242
        %244 = OpAll %bool %243
               OpBranch %240
        %240 = OpLabel
        %245 = OpPhi %bool %false %229 %244 %239
               OpStore %_0_ok %245
        %246 = OpVectorShuffle %v3float %214 %214 3 1 3
        %249 = OpVectorTimesScalar %v3float %246 %float_9
        %250 = OpLoad %v4float %_3_x
        %251 = OpVectorShuffle %v4float %250 %249 4 5 6 3
               OpStore %_3_x %251
               OpSelectionMerge %253 None
               OpBranchConditional %245 %252 %253
        %252 = OpLabel
        %255 = OpFOrdEqual %v4bool %251 %254
        %256 = OpAll %bool %255
               OpBranch %253
        %253 = OpLabel
        %257 = OpPhi %bool %false %240 %256 %252
               OpStore %_0_ok %257
        %258 = OpVectorShuffle %v2float %251 %251 2 3
        %259 = OpVectorTimesScalar %v2float %258 %float_2
        %260 = OpLoad %v4float %_3_x
        %261 = OpVectorShuffle %v4float %260 %259 4 5 2 3
               OpStore %_3_x %261
               OpSelectionMerge %263 None
               OpBranchConditional %257 %262 %263
        %262 = OpLabel
        %267 = OpFOrdEqual %v4bool %261 %266
        %268 = OpAll %bool %267
               OpBranch %263
        %263 = OpLabel
        %269 = OpPhi %bool %false %253 %268 %262
               OpStore %_0_ok %269
        %271 = OpVectorTimesScalar %v4float %211 %float_5
        %272 = OpVectorShuffle %v4float %271 %271 1 0 3 2
               OpStore %_3_x %272
               OpSelectionMerge %274 None
               OpBranchConditional %269 %273 %274
        %273 = OpLabel
        %276 = OpFOrdEqual %v4bool %272 %275
        %277 = OpAll %bool %276
               OpBranch %274
        %274 = OpLabel
        %278 = OpPhi %bool %false %263 %277 %273
               OpStore %_0_ok %278
        %279 = OpFAdd %v4float %217 %211
               OpStore %_3_x %279
               OpSelectionMerge %281 None
               OpBranchConditional %278 %280 %281
        %280 = OpLabel
        %282 = OpFOrdEqual %v4bool %279 %222
        %283 = OpAll %bool %282
               OpBranch %281
        %281 = OpLabel
        %284 = OpPhi %bool %false %274 %283 %280
               OpStore %_0_ok %284
        %286 = OpVectorShuffle %v4float %214 %214 1 3 0 2
        %288 = OpFSub %v4float %287 %286
               OpStore %_3_x %288
               OpSelectionMerge %290 None
               OpBranchConditional %284 %289 %290
        %289 = OpLabel
        %292 = OpFOrdEqual %v4bool %288 %291
        %293 = OpAll %bool %292
               OpBranch %290
        %290 = OpLabel
        %294 = OpPhi %bool %false %281 %293 %289
               OpStore %_0_ok %294
        %295 = OpCompositeExtract %float %211 0
        %296 = OpCompositeConstruct %v4float %295 %295 %295 %295
        %297 = OpFAdd %v4float %296 %214
               OpStore %_3_x %297
               OpSelectionMerge %299 None
               OpBranchConditional %294 %298 %299
        %298 = OpLabel
        %301 = OpFOrdEqual %v4bool %297 %300
        %302 = OpAll %bool %301
               OpBranch %299
        %299 = OpLabel
        %303 = OpPhi %bool %false %290 %302 %298
               OpStore %_0_ok %303
        %305 = OpVectorShuffle %v3float %214 %214 3 1 3
        %306 = OpVectorTimesScalar %v3float %305 %float_8
        %307 = OpLoad %v4float %_3_x
        %308 = OpVectorShuffle %v4float %307 %306 4 5 6 3
               OpStore %_3_x %308
               OpSelectionMerge %310 None
               OpBranchConditional %303 %309 %310
        %309 = OpLabel
        %312 = OpFOrdEqual %v4bool %308 %311
        %313 = OpAll %bool %312
               OpBranch %310
        %310 = OpLabel
        %314 = OpPhi %bool %false %299 %313 %309
               OpStore %_0_ok %314
        %316 = OpVectorShuffle %v2float %308 %308 2 3
        %318 = OpFDiv %v2float %317 %316
        %319 = OpLoad %v4float %_3_x
        %320 = OpVectorShuffle %v4float %319 %318 4 5 2 3
               OpStore %_3_x %320
               OpSelectionMerge %322 None
               OpBranchConditional %314 %321 %322
        %321 = OpLabel
        %325 = OpFOrdEqual %v4bool %320 %324
        %326 = OpAll %bool %325
               OpBranch %322
        %322 = OpLabel
        %327 = OpPhi %bool %false %310 %326 %321
               OpStore %_0_ok %327
        %329 = OpFDiv %v4float %328 %320
        %330 = OpVectorShuffle %v4float %329 %329 1 0 3 2
               OpStore %_3_x %330
               OpSelectionMerge %332 None
               OpBranchConditional %327 %331 %332
        %331 = OpLabel
        %334 = OpFOrdEqual %v4bool %330 %333
        %335 = OpAll %bool %334
               OpBranch %332
        %332 = OpLabel
        %336 = OpPhi %bool %false %322 %335 %331
               OpStore %_0_ok %336
        %337 = OpFAdd %v4float %330 %217
               OpStore %_3_x %337
        %338 = OpVectorTimesScalar %v4float %337 %float_2
               OpStore %_3_x %338
        %340 = OpFSub %v4float %338 %339
               OpStore %_3_x %340
        %342 = OpVectorTimesScalar %v4float %340 %float_0_5
               OpStore %_3_x %342
               OpSelectionMerge %344 None
               OpBranchConditional %336 %343 %344
        %343 = OpLabel
        %345 = OpFOrdEqual %v4bool %342 %333
        %346 = OpAll %bool %345
               OpBranch %344
        %344 = OpLabel
        %347 = OpPhi %bool %false %332 %346 %343
               OpStore %_0_ok %347
        %348 = OpFAdd %v4float %342 %217
               OpStore %_3_x %348
        %349 = OpVectorTimesScalar %v4float %348 %float_2
               OpStore %_3_x %349
        %350 = OpFSub %v4float %349 %339
               OpStore %_3_x %350
        %351 = OpVectorTimesScalar %v4float %350 %float_0_5
               OpStore %_3_x %351
               OpSelectionMerge %353 None
               OpBranchConditional %347 %352 %353
        %352 = OpLabel
        %354 = OpFOrdEqual %v4bool %351 %333
        %355 = OpAll %bool %354
               OpBranch %353
        %353 = OpLabel
        %356 = OpPhi %bool %false %344 %355 %352
               OpStore %_0_ok %356
               OpSelectionMerge %358 None
               OpBranchConditional %356 %357 %358
        %357 = OpLabel
        %359 = OpFunctionCall %bool %test_int_b
               OpBranch %358
        %358 = OpLabel
        %360 = OpPhi %bool %false %353 %359 %357
               OpSelectionMerge %364 None
               OpBranchConditional %360 %362 %363
        %362 = OpLabel
        %365 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %366 = OpLoad %v4float %365
               OpStore %361 %366
               OpBranch %364
        %363 = OpLabel
        %367 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %368 = OpLoad %v4float %367
               OpStore %361 %368
               OpBranch %364
        %364 = OpLabel
        %369 = OpLoad %v4float %361
               OpReturnValue %369
               OpFunctionEnd
