               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorWhite"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpMemberName %_UniformBuffer 3 "testMatrix2x2"
               OpMemberName %_UniformBuffer 4 "testMatrix3x3"
               OpMemberName %_UniformBuffer 5 "testMatrix4x4"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %test_iscalar_b "test_iscalar_b"
               OpName %x "x"
               OpName %test_fvec_b "test_fvec_b"
               OpName %x_0 "x"
               OpName %test_ivec_b "test_ivec_b"
               OpName %x_1 "x"
               OpName %test_mat2_b "test_mat2_b"
               OpName %negated "negated"
               OpName %x_2 "x"
               OpName %test_mat3_b "test_mat3_b"
               OpName %negated_0 "negated"
               OpName %x_3 "x"
               OpName %test_mat4_b "test_mat4_b"
               OpName %negated_1 "negated"
               OpName %x_4 "x"
               OpName %test_hmat2_b "test_hmat2_b"
               OpName %negated_2 "negated"
               OpName %x_5 "x"
               OpName %test_hmat3_b "test_hmat3_b"
               OpName %negated_3 "negated"
               OpName %x_6 "x"
               OpName %test_hmat4_b "test_hmat4_b"
               OpName %negated_4 "negated"
               OpName %x_7 "x"
               OpName %main "main"
               OpName %_0_x "_0_x"
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
               OpMemberDecorate %_UniformBuffer 3 ColMajor
               OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 4 Offset 80
               OpMemberDecorate %_UniformBuffer 4 ColMajor
               OpMemberDecorate %_UniformBuffer 4 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 5 Offset 128
               OpMemberDecorate %_UniformBuffer 5 ColMajor
               OpMemberDecorate %_UniformBuffer 5 MatrixStride 16
               OpDecorate %_UniformBuffer Block
               OpDecorate %16 Binding 0
               OpDecorate %16 DescriptorSet 0
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %x_0 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %negated_2 RelaxedPrecision
               OpDecorate %x_5 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %negated_3 RelaxedPrecision
               OpDecorate %x_6 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %negated_4 RelaxedPrecision
               OpDecorate %x_7 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
               OpDecorate %212 RelaxedPrecision
               OpDecorate %213 RelaxedPrecision
               OpDecorate %214 RelaxedPrecision
               OpDecorate %215 RelaxedPrecision
               OpDecorate %216 RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %219 RelaxedPrecision
               OpDecorate %221 RelaxedPrecision
               OpDecorate %224 RelaxedPrecision
               OpDecorate %227 RelaxedPrecision
               OpDecorate %236 RelaxedPrecision
               OpDecorate %237 RelaxedPrecision
               OpDecorate %284 RelaxedPrecision
               OpDecorate %287 RelaxedPrecision
               OpDecorate %288 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %mat2v2float %mat3v3float %mat4v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %16 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %26 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %29 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %34 = OpTypeFunction %bool
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
     %int_n1 = OpConstant %int -1
   %float_n1 = OpConstant %float -1
         %55 = OpConstantComposite %v2float %float_n1 %float_n1
     %v2bool = OpTypeVector %bool 2
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
         %69 = OpConstantComposite %v2int %int_n1 %int_n1
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
   %float_n2 = OpConstant %float -2
   %float_n3 = OpConstant %float -3
   %float_n4 = OpConstant %float -4
         %78 = OpConstantComposite %v2float %float_n1 %float_n2
         %79 = OpConstantComposite %v2float %float_n3 %float_n4
         %80 = OpConstantComposite %mat2v2float %78 %79
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
      %int_3 = OpConstant %int 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
   %float_n5 = OpConstant %float -5
   %float_n6 = OpConstant %float -6
   %float_n7 = OpConstant %float -7
   %float_n8 = OpConstant %float -8
   %float_n9 = OpConstant %float -9
        %104 = OpConstantComposite %v3float %float_n1 %float_n2 %float_n3
        %105 = OpConstantComposite %v3float %float_n4 %float_n5 %float_n6
        %106 = OpConstantComposite %v3float %float_n7 %float_n8 %float_n9
        %107 = OpConstantComposite %mat3v3float %104 %105 %106
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_4 = OpConstant %int 4
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
  %float_n10 = OpConstant %float -10
  %float_n11 = OpConstant %float -11
  %float_n12 = OpConstant %float -12
  %float_n13 = OpConstant %float -13
  %float_n14 = OpConstant %float -14
  %float_n15 = OpConstant %float -15
  %float_n16 = OpConstant %float -16
        %139 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n3 %float_n4
        %140 = OpConstantComposite %v4float %float_n5 %float_n6 %float_n7 %float_n8
        %141 = OpConstantComposite %v4float %float_n9 %float_n10 %float_n11 %float_n12
        %142 = OpConstantComposite %v4float %float_n13 %float_n14 %float_n15 %float_n16
        %143 = OpConstantComposite %mat4v4float %139 %140 %141 %142
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
      %int_5 = OpConstant %int 5
     %v4bool = OpTypeVector %bool 4
        %230 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
      %false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %26
         %27 = OpLabel
         %30 = OpVariable %_ptr_Function_v2float Function
               OpStore %30 %29
         %32 = OpFunctionCall %v4float %main %30
               OpStore %sk_FragColor %32
               OpReturn
               OpFunctionEnd
%test_iscalar_b = OpFunction %bool None %34
         %35 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
         %39 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
         %42 = OpLoad %v4float %39
         %43 = OpCompositeExtract %float %42 0
         %44 = OpConvertFToS %int %43
               OpStore %x %44
         %45 = OpSNegate %int %44
               OpStore %x %45
         %47 = OpIEqual %bool %45 %int_n1
               OpReturnValue %47
               OpFunctionEnd
%test_fvec_b = OpFunction %bool None %34
         %48 = OpLabel
        %x_0 = OpVariable %_ptr_Function_v2float Function
         %50 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
         %51 = OpLoad %v4float %50
         %52 = OpVectorShuffle %v2float %51 %51 0 1
               OpStore %x_0 %52
         %53 = OpFNegate %v2float %52
               OpStore %x_0 %53
         %56 = OpFOrdEqual %v2bool %53 %55
         %58 = OpAll %bool %56
               OpReturnValue %58
               OpFunctionEnd
%test_ivec_b = OpFunction %bool None %34
         %59 = OpLabel
        %x_1 = OpVariable %_ptr_Function_v2int Function
         %63 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
         %64 = OpLoad %v4float %63
         %65 = OpCompositeExtract %float %64 0
         %66 = OpConvertFToS %int %65
         %67 = OpCompositeConstruct %v2int %66 %66
               OpStore %x_1 %67
         %68 = OpSNegate %v2int %67
               OpStore %x_1 %68
         %70 = OpIEqual %v2bool %68 %69
         %71 = OpAll %bool %70
               OpReturnValue %71
               OpFunctionEnd
%test_mat2_b = OpFunction %bool None %34
         %72 = OpLabel
    %negated = OpVariable %_ptr_Function_mat2v2float Function
        %x_2 = OpVariable %_ptr_Function_mat2v2float Function
               OpStore %negated %80
         %82 = OpAccessChain %_ptr_Uniform_mat2v2float %16 %int_3
         %85 = OpLoad %mat2v2float %82
               OpStore %x_2 %85
         %86 = OpCompositeExtract %v2float %85 0
         %87 = OpFNegate %v2float %86
         %88 = OpCompositeExtract %v2float %85 1
         %89 = OpFNegate %v2float %88
         %90 = OpCompositeConstruct %mat2v2float %87 %89
               OpStore %x_2 %90
         %91 = OpFOrdEqual %v2bool %87 %78
         %92 = OpAll %bool %91
         %93 = OpFOrdEqual %v2bool %89 %79
         %94 = OpAll %bool %93
         %95 = OpLogicalAnd %bool %92 %94
               OpReturnValue %95
               OpFunctionEnd
%test_mat3_b = OpFunction %bool None %34
         %96 = OpLabel
  %negated_0 = OpVariable %_ptr_Function_mat3v3float Function
        %x_3 = OpVariable %_ptr_Function_mat3v3float Function
               OpStore %negated_0 %107
        %109 = OpAccessChain %_ptr_Uniform_mat3v3float %16 %int_4
        %112 = OpLoad %mat3v3float %109
               OpStore %x_3 %112
        %113 = OpCompositeExtract %v3float %112 0
        %114 = OpFNegate %v3float %113
        %115 = OpCompositeExtract %v3float %112 1
        %116 = OpFNegate %v3float %115
        %117 = OpCompositeExtract %v3float %112 2
        %118 = OpFNegate %v3float %117
        %119 = OpCompositeConstruct %mat3v3float %114 %116 %118
               OpStore %x_3 %119
        %121 = OpFOrdEqual %v3bool %114 %104
        %122 = OpAll %bool %121
        %123 = OpFOrdEqual %v3bool %116 %105
        %124 = OpAll %bool %123
        %125 = OpLogicalAnd %bool %122 %124
        %126 = OpFOrdEqual %v3bool %118 %106
        %127 = OpAll %bool %126
        %128 = OpLogicalAnd %bool %125 %127
               OpReturnValue %128
               OpFunctionEnd
%test_mat4_b = OpFunction %bool None %34
        %129 = OpLabel
  %negated_1 = OpVariable %_ptr_Function_mat4v4float Function
        %x_4 = OpVariable %_ptr_Function_mat4v4float Function
               OpStore %negated_1 %143
        %145 = OpAccessChain %_ptr_Uniform_mat4v4float %16 %int_5
        %148 = OpLoad %mat4v4float %145
               OpStore %x_4 %148
        %149 = OpCompositeExtract %v4float %148 0
        %150 = OpFNegate %v4float %149
        %151 = OpCompositeExtract %v4float %148 1
        %152 = OpFNegate %v4float %151
        %153 = OpCompositeExtract %v4float %148 2
        %154 = OpFNegate %v4float %153
        %155 = OpCompositeExtract %v4float %148 3
        %156 = OpFNegate %v4float %155
        %157 = OpCompositeConstruct %mat4v4float %150 %152 %154 %156
               OpStore %x_4 %157
        %159 = OpFOrdEqual %v4bool %150 %139
        %160 = OpAll %bool %159
        %161 = OpFOrdEqual %v4bool %152 %140
        %162 = OpAll %bool %161
        %163 = OpLogicalAnd %bool %160 %162
        %164 = OpFOrdEqual %v4bool %154 %141
        %165 = OpAll %bool %164
        %166 = OpLogicalAnd %bool %163 %165
        %167 = OpFOrdEqual %v4bool %156 %142
        %168 = OpAll %bool %167
        %169 = OpLogicalAnd %bool %166 %168
               OpReturnValue %169
               OpFunctionEnd
%test_hmat2_b = OpFunction %bool None %34
        %170 = OpLabel
  %negated_2 = OpVariable %_ptr_Function_mat2v2float Function
        %x_5 = OpVariable %_ptr_Function_mat2v2float Function
               OpStore %negated_2 %80
        %173 = OpAccessChain %_ptr_Uniform_mat2v2float %16 %int_3
        %174 = OpLoad %mat2v2float %173
               OpStore %x_5 %174
        %175 = OpCompositeExtract %v2float %174 0
        %176 = OpFNegate %v2float %175
        %177 = OpCompositeExtract %v2float %174 1
        %178 = OpFNegate %v2float %177
        %179 = OpCompositeConstruct %mat2v2float %176 %178
               OpStore %x_5 %179
        %180 = OpFOrdEqual %v2bool %176 %78
        %181 = OpAll %bool %180
        %182 = OpFOrdEqual %v2bool %178 %79
        %183 = OpAll %bool %182
        %184 = OpLogicalAnd %bool %181 %183
               OpReturnValue %184
               OpFunctionEnd
%test_hmat3_b = OpFunction %bool None %34
        %185 = OpLabel
  %negated_3 = OpVariable %_ptr_Function_mat3v3float Function
        %x_6 = OpVariable %_ptr_Function_mat3v3float Function
               OpStore %negated_3 %107
        %188 = OpAccessChain %_ptr_Uniform_mat3v3float %16 %int_4
        %189 = OpLoad %mat3v3float %188
               OpStore %x_6 %189
        %190 = OpCompositeExtract %v3float %189 0
        %191 = OpFNegate %v3float %190
        %192 = OpCompositeExtract %v3float %189 1
        %193 = OpFNegate %v3float %192
        %194 = OpCompositeExtract %v3float %189 2
        %195 = OpFNegate %v3float %194
        %196 = OpCompositeConstruct %mat3v3float %191 %193 %195
               OpStore %x_6 %196
        %197 = OpFOrdEqual %v3bool %191 %104
        %198 = OpAll %bool %197
        %199 = OpFOrdEqual %v3bool %193 %105
        %200 = OpAll %bool %199
        %201 = OpLogicalAnd %bool %198 %200
        %202 = OpFOrdEqual %v3bool %195 %106
        %203 = OpAll %bool %202
        %204 = OpLogicalAnd %bool %201 %203
               OpReturnValue %204
               OpFunctionEnd
%test_hmat4_b = OpFunction %bool None %34
        %205 = OpLabel
  %negated_4 = OpVariable %_ptr_Function_mat4v4float Function
        %x_7 = OpVariable %_ptr_Function_mat4v4float Function
               OpStore %negated_4 %143
        %208 = OpAccessChain %_ptr_Uniform_mat4v4float %16 %int_5
        %209 = OpLoad %mat4v4float %208
               OpStore %x_7 %209
        %210 = OpCompositeExtract %v4float %209 0
        %211 = OpFNegate %v4float %210
        %212 = OpCompositeExtract %v4float %209 1
        %213 = OpFNegate %v4float %212
        %214 = OpCompositeExtract %v4float %209 2
        %215 = OpFNegate %v4float %214
        %216 = OpCompositeExtract %v4float %209 3
        %217 = OpFNegate %v4float %216
        %218 = OpCompositeConstruct %mat4v4float %211 %213 %215 %217
               OpStore %x_7 %218
        %219 = OpFOrdEqual %v4bool %211 %139
        %220 = OpAll %bool %219
        %221 = OpFOrdEqual %v4bool %213 %140
        %222 = OpAll %bool %221
        %223 = OpLogicalAnd %bool %220 %222
        %224 = OpFOrdEqual %v4bool %215 %141
        %225 = OpAll %bool %224
        %226 = OpLogicalAnd %bool %223 %225
        %227 = OpFOrdEqual %v4bool %217 %142
        %228 = OpAll %bool %227
        %229 = OpLogicalAnd %bool %226 %228
               OpReturnValue %229
               OpFunctionEnd
       %main = OpFunction %v4float None %230
        %231 = OpFunctionParameter %_ptr_Function_v2float
        %232 = OpLabel
       %_0_x = OpVariable %_ptr_Function_float Function
        %277 = OpVariable %_ptr_Function_v4float Function
        %235 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
        %236 = OpLoad %v4float %235
        %237 = OpCompositeExtract %float %236 0
               OpStore %_0_x %237
        %238 = OpFNegate %float %237
               OpStore %_0_x %238
        %240 = OpFOrdEqual %bool %238 %float_n1
               OpSelectionMerge %242 None
               OpBranchConditional %240 %241 %242
        %241 = OpLabel
        %243 = OpFunctionCall %bool %test_iscalar_b
               OpBranch %242
        %242 = OpLabel
        %244 = OpPhi %bool %false %232 %243 %241
               OpSelectionMerge %246 None
               OpBranchConditional %244 %245 %246
        %245 = OpLabel
        %247 = OpFunctionCall %bool %test_fvec_b
               OpBranch %246
        %246 = OpLabel
        %248 = OpPhi %bool %false %242 %247 %245
               OpSelectionMerge %250 None
               OpBranchConditional %248 %249 %250
        %249 = OpLabel
        %251 = OpFunctionCall %bool %test_ivec_b
               OpBranch %250
        %250 = OpLabel
        %252 = OpPhi %bool %false %246 %251 %249
               OpSelectionMerge %254 None
               OpBranchConditional %252 %253 %254
        %253 = OpLabel
        %255 = OpFunctionCall %bool %test_mat2_b
               OpBranch %254
        %254 = OpLabel
        %256 = OpPhi %bool %false %250 %255 %253
               OpSelectionMerge %258 None
               OpBranchConditional %256 %257 %258
        %257 = OpLabel
        %259 = OpFunctionCall %bool %test_mat3_b
               OpBranch %258
        %258 = OpLabel
        %260 = OpPhi %bool %false %254 %259 %257
               OpSelectionMerge %262 None
               OpBranchConditional %260 %261 %262
        %261 = OpLabel
        %263 = OpFunctionCall %bool %test_mat4_b
               OpBranch %262
        %262 = OpLabel
        %264 = OpPhi %bool %false %258 %263 %261
               OpSelectionMerge %266 None
               OpBranchConditional %264 %265 %266
        %265 = OpLabel
        %267 = OpFunctionCall %bool %test_hmat2_b
               OpBranch %266
        %266 = OpLabel
        %268 = OpPhi %bool %false %262 %267 %265
               OpSelectionMerge %270 None
               OpBranchConditional %268 %269 %270
        %269 = OpLabel
        %271 = OpFunctionCall %bool %test_hmat3_b
               OpBranch %270
        %270 = OpLabel
        %272 = OpPhi %bool %false %266 %271 %269
               OpSelectionMerge %274 None
               OpBranchConditional %272 %273 %274
        %273 = OpLabel
        %275 = OpFunctionCall %bool %test_hmat4_b
               OpBranch %274
        %274 = OpLabel
        %276 = OpPhi %bool %false %270 %275 %273
               OpSelectionMerge %281 None
               OpBranchConditional %276 %279 %280
        %279 = OpLabel
        %282 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
        %284 = OpLoad %v4float %282
               OpStore %277 %284
               OpBranch %281
        %280 = OpLabel
        %285 = OpAccessChain %_ptr_Uniform_v4float %16 %int_2
        %287 = OpLoad %v4float %285
               OpStore %277 %287
               OpBranch %281
        %281 = OpLabel
        %288 = OpLoad %v4float %277
               OpReturnValue %288
               OpFunctionEnd
