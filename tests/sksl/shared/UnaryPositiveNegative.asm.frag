               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
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
               OpMemberDecorate %_UniformBuffer 3 ColMajor
               OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 4 Offset 80
               OpMemberDecorate %_UniformBuffer 4 ColMajor
               OpMemberDecorate %_UniformBuffer 4 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 5 Offset 128
               OpMemberDecorate %_UniformBuffer 5 ColMajor
               OpMemberDecorate %_UniformBuffer 5 MatrixStride 16
               OpDecorate %_UniformBuffer Block
               OpDecorate %19 Binding 0
               OpDecorate %19 DescriptorSet 0
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %x_0 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %negated_2 RelaxedPrecision
               OpDecorate %x_5 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %negated_3 RelaxedPrecision
               OpDecorate %x_6 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %negated_4 RelaxedPrecision
               OpDecorate %x_7 RelaxedPrecision
               OpDecorate %212 RelaxedPrecision
               OpDecorate %213 RelaxedPrecision
               OpDecorate %214 RelaxedPrecision
               OpDecorate %215 RelaxedPrecision
               OpDecorate %216 RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %219 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %221 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %226 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %238 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %286 RelaxedPrecision
               OpDecorate %289 RelaxedPrecision
               OpDecorate %290 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
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
         %19 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %29 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %32 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %36 = OpTypeFunction %bool
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
     %int_n1 = OpConstant %int -1
   %float_n1 = OpConstant %float -1
         %57 = OpConstantComposite %v2float %float_n1 %float_n1
     %v2bool = OpTypeVector %bool 2
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
         %71 = OpConstantComposite %v2int %int_n1 %int_n1
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
   %float_n2 = OpConstant %float -2
   %float_n3 = OpConstant %float -3
   %float_n4 = OpConstant %float -4
         %80 = OpConstantComposite %v2float %float_n1 %float_n2
         %81 = OpConstantComposite %v2float %float_n3 %float_n4
         %82 = OpConstantComposite %mat2v2float %80 %81
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
      %int_3 = OpConstant %int 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
   %float_n5 = OpConstant %float -5
   %float_n6 = OpConstant %float -6
   %float_n7 = OpConstant %float -7
   %float_n8 = OpConstant %float -8
   %float_n9 = OpConstant %float -9
        %106 = OpConstantComposite %v3float %float_n1 %float_n2 %float_n3
        %107 = OpConstantComposite %v3float %float_n4 %float_n5 %float_n6
        %108 = OpConstantComposite %v3float %float_n7 %float_n8 %float_n9
        %109 = OpConstantComposite %mat3v3float %106 %107 %108
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
        %141 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n3 %float_n4
        %142 = OpConstantComposite %v4float %float_n5 %float_n6 %float_n7 %float_n8
        %143 = OpConstantComposite %v4float %float_n9 %float_n10 %float_n11 %float_n12
        %144 = OpConstantComposite %v4float %float_n13 %float_n14 %float_n15 %float_n16
        %145 = OpConstantComposite %mat4v4float %141 %142 %143 %144
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
      %int_5 = OpConstant %int 5
     %v4bool = OpTypeVector %bool 4
        %232 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
      %false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %29
         %30 = OpLabel
         %33 = OpVariable %_ptr_Function_v2float Function
               OpStore %33 %32
         %35 = OpFunctionCall %v4float %main %33
               OpStore %sk_FragColor %35
               OpReturn
               OpFunctionEnd
%test_iscalar_b = OpFunction %bool None %36
         %37 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
         %41 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
         %44 = OpLoad %v4float %41
         %45 = OpCompositeExtract %float %44 0
         %46 = OpConvertFToS %int %45
               OpStore %x %46
         %47 = OpSNegate %int %46
               OpStore %x %47
         %49 = OpIEqual %bool %47 %int_n1
               OpReturnValue %49
               OpFunctionEnd
%test_fvec_b = OpFunction %bool None %36
         %50 = OpLabel
        %x_0 = OpVariable %_ptr_Function_v2float Function
         %52 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
         %53 = OpLoad %v4float %52
         %54 = OpVectorShuffle %v2float %53 %53 0 1
               OpStore %x_0 %54
         %55 = OpFNegate %v2float %54
               OpStore %x_0 %55
         %58 = OpFOrdEqual %v2bool %55 %57
         %60 = OpAll %bool %58
               OpReturnValue %60
               OpFunctionEnd
%test_ivec_b = OpFunction %bool None %36
         %61 = OpLabel
        %x_1 = OpVariable %_ptr_Function_v2int Function
         %65 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
         %66 = OpLoad %v4float %65
         %67 = OpCompositeExtract %float %66 0
         %68 = OpConvertFToS %int %67
         %69 = OpCompositeConstruct %v2int %68 %68
               OpStore %x_1 %69
         %70 = OpSNegate %v2int %69
               OpStore %x_1 %70
         %72 = OpIEqual %v2bool %70 %71
         %73 = OpAll %bool %72
               OpReturnValue %73
               OpFunctionEnd
%test_mat2_b = OpFunction %bool None %36
         %74 = OpLabel
    %negated = OpVariable %_ptr_Function_mat2v2float Function
        %x_2 = OpVariable %_ptr_Function_mat2v2float Function
               OpStore %negated %82
         %84 = OpAccessChain %_ptr_Uniform_mat2v2float %19 %int_3
         %87 = OpLoad %mat2v2float %84
               OpStore %x_2 %87
         %88 = OpCompositeExtract %v2float %87 0
         %89 = OpFNegate %v2float %88
         %90 = OpCompositeExtract %v2float %87 1
         %91 = OpFNegate %v2float %90
         %92 = OpCompositeConstruct %mat2v2float %89 %91
               OpStore %x_2 %92
         %93 = OpFOrdEqual %v2bool %89 %80
         %94 = OpAll %bool %93
         %95 = OpFOrdEqual %v2bool %91 %81
         %96 = OpAll %bool %95
         %97 = OpLogicalAnd %bool %94 %96
               OpReturnValue %97
               OpFunctionEnd
%test_mat3_b = OpFunction %bool None %36
         %98 = OpLabel
  %negated_0 = OpVariable %_ptr_Function_mat3v3float Function
        %x_3 = OpVariable %_ptr_Function_mat3v3float Function
               OpStore %negated_0 %109
        %111 = OpAccessChain %_ptr_Uniform_mat3v3float %19 %int_4
        %114 = OpLoad %mat3v3float %111
               OpStore %x_3 %114
        %115 = OpCompositeExtract %v3float %114 0
        %116 = OpFNegate %v3float %115
        %117 = OpCompositeExtract %v3float %114 1
        %118 = OpFNegate %v3float %117
        %119 = OpCompositeExtract %v3float %114 2
        %120 = OpFNegate %v3float %119
        %121 = OpCompositeConstruct %mat3v3float %116 %118 %120
               OpStore %x_3 %121
        %123 = OpFOrdEqual %v3bool %116 %106
        %124 = OpAll %bool %123
        %125 = OpFOrdEqual %v3bool %118 %107
        %126 = OpAll %bool %125
        %127 = OpLogicalAnd %bool %124 %126
        %128 = OpFOrdEqual %v3bool %120 %108
        %129 = OpAll %bool %128
        %130 = OpLogicalAnd %bool %127 %129
               OpReturnValue %130
               OpFunctionEnd
%test_mat4_b = OpFunction %bool None %36
        %131 = OpLabel
  %negated_1 = OpVariable %_ptr_Function_mat4v4float Function
        %x_4 = OpVariable %_ptr_Function_mat4v4float Function
               OpStore %negated_1 %145
        %147 = OpAccessChain %_ptr_Uniform_mat4v4float %19 %int_5
        %150 = OpLoad %mat4v4float %147
               OpStore %x_4 %150
        %151 = OpCompositeExtract %v4float %150 0
        %152 = OpFNegate %v4float %151
        %153 = OpCompositeExtract %v4float %150 1
        %154 = OpFNegate %v4float %153
        %155 = OpCompositeExtract %v4float %150 2
        %156 = OpFNegate %v4float %155
        %157 = OpCompositeExtract %v4float %150 3
        %158 = OpFNegate %v4float %157
        %159 = OpCompositeConstruct %mat4v4float %152 %154 %156 %158
               OpStore %x_4 %159
        %161 = OpFOrdEqual %v4bool %152 %141
        %162 = OpAll %bool %161
        %163 = OpFOrdEqual %v4bool %154 %142
        %164 = OpAll %bool %163
        %165 = OpLogicalAnd %bool %162 %164
        %166 = OpFOrdEqual %v4bool %156 %143
        %167 = OpAll %bool %166
        %168 = OpLogicalAnd %bool %165 %167
        %169 = OpFOrdEqual %v4bool %158 %144
        %170 = OpAll %bool %169
        %171 = OpLogicalAnd %bool %168 %170
               OpReturnValue %171
               OpFunctionEnd
%test_hmat2_b = OpFunction %bool None %36
        %172 = OpLabel
  %negated_2 = OpVariable %_ptr_Function_mat2v2float Function
        %x_5 = OpVariable %_ptr_Function_mat2v2float Function
               OpStore %negated_2 %82
        %175 = OpAccessChain %_ptr_Uniform_mat2v2float %19 %int_3
        %176 = OpLoad %mat2v2float %175
               OpStore %x_5 %176
        %177 = OpCompositeExtract %v2float %176 0
        %178 = OpFNegate %v2float %177
        %179 = OpCompositeExtract %v2float %176 1
        %180 = OpFNegate %v2float %179
        %181 = OpCompositeConstruct %mat2v2float %178 %180
               OpStore %x_5 %181
        %182 = OpFOrdEqual %v2bool %178 %80
        %183 = OpAll %bool %182
        %184 = OpFOrdEqual %v2bool %180 %81
        %185 = OpAll %bool %184
        %186 = OpLogicalAnd %bool %183 %185
               OpReturnValue %186
               OpFunctionEnd
%test_hmat3_b = OpFunction %bool None %36
        %187 = OpLabel
  %negated_3 = OpVariable %_ptr_Function_mat3v3float Function
        %x_6 = OpVariable %_ptr_Function_mat3v3float Function
               OpStore %negated_3 %109
        %190 = OpAccessChain %_ptr_Uniform_mat3v3float %19 %int_4
        %191 = OpLoad %mat3v3float %190
               OpStore %x_6 %191
        %192 = OpCompositeExtract %v3float %191 0
        %193 = OpFNegate %v3float %192
        %194 = OpCompositeExtract %v3float %191 1
        %195 = OpFNegate %v3float %194
        %196 = OpCompositeExtract %v3float %191 2
        %197 = OpFNegate %v3float %196
        %198 = OpCompositeConstruct %mat3v3float %193 %195 %197
               OpStore %x_6 %198
        %199 = OpFOrdEqual %v3bool %193 %106
        %200 = OpAll %bool %199
        %201 = OpFOrdEqual %v3bool %195 %107
        %202 = OpAll %bool %201
        %203 = OpLogicalAnd %bool %200 %202
        %204 = OpFOrdEqual %v3bool %197 %108
        %205 = OpAll %bool %204
        %206 = OpLogicalAnd %bool %203 %205
               OpReturnValue %206
               OpFunctionEnd
%test_hmat4_b = OpFunction %bool None %36
        %207 = OpLabel
  %negated_4 = OpVariable %_ptr_Function_mat4v4float Function
        %x_7 = OpVariable %_ptr_Function_mat4v4float Function
               OpStore %negated_4 %145
        %210 = OpAccessChain %_ptr_Uniform_mat4v4float %19 %int_5
        %211 = OpLoad %mat4v4float %210
               OpStore %x_7 %211
        %212 = OpCompositeExtract %v4float %211 0
        %213 = OpFNegate %v4float %212
        %214 = OpCompositeExtract %v4float %211 1
        %215 = OpFNegate %v4float %214
        %216 = OpCompositeExtract %v4float %211 2
        %217 = OpFNegate %v4float %216
        %218 = OpCompositeExtract %v4float %211 3
        %219 = OpFNegate %v4float %218
        %220 = OpCompositeConstruct %mat4v4float %213 %215 %217 %219
               OpStore %x_7 %220
        %221 = OpFOrdEqual %v4bool %213 %141
        %222 = OpAll %bool %221
        %223 = OpFOrdEqual %v4bool %215 %142
        %224 = OpAll %bool %223
        %225 = OpLogicalAnd %bool %222 %224
        %226 = OpFOrdEqual %v4bool %217 %143
        %227 = OpAll %bool %226
        %228 = OpLogicalAnd %bool %225 %227
        %229 = OpFOrdEqual %v4bool %219 %144
        %230 = OpAll %bool %229
        %231 = OpLogicalAnd %bool %228 %230
               OpReturnValue %231
               OpFunctionEnd
       %main = OpFunction %v4float None %232
        %233 = OpFunctionParameter %_ptr_Function_v2float
        %234 = OpLabel
       %_0_x = OpVariable %_ptr_Function_float Function
        %279 = OpVariable %_ptr_Function_v4float Function
        %237 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
        %238 = OpLoad %v4float %237
        %239 = OpCompositeExtract %float %238 0
               OpStore %_0_x %239
        %240 = OpFNegate %float %239
               OpStore %_0_x %240
        %242 = OpFOrdEqual %bool %240 %float_n1
               OpSelectionMerge %244 None
               OpBranchConditional %242 %243 %244
        %243 = OpLabel
        %245 = OpFunctionCall %bool %test_iscalar_b
               OpBranch %244
        %244 = OpLabel
        %246 = OpPhi %bool %false %234 %245 %243
               OpSelectionMerge %248 None
               OpBranchConditional %246 %247 %248
        %247 = OpLabel
        %249 = OpFunctionCall %bool %test_fvec_b
               OpBranch %248
        %248 = OpLabel
        %250 = OpPhi %bool %false %244 %249 %247
               OpSelectionMerge %252 None
               OpBranchConditional %250 %251 %252
        %251 = OpLabel
        %253 = OpFunctionCall %bool %test_ivec_b
               OpBranch %252
        %252 = OpLabel
        %254 = OpPhi %bool %false %248 %253 %251
               OpSelectionMerge %256 None
               OpBranchConditional %254 %255 %256
        %255 = OpLabel
        %257 = OpFunctionCall %bool %test_mat2_b
               OpBranch %256
        %256 = OpLabel
        %258 = OpPhi %bool %false %252 %257 %255
               OpSelectionMerge %260 None
               OpBranchConditional %258 %259 %260
        %259 = OpLabel
        %261 = OpFunctionCall %bool %test_mat3_b
               OpBranch %260
        %260 = OpLabel
        %262 = OpPhi %bool %false %256 %261 %259
               OpSelectionMerge %264 None
               OpBranchConditional %262 %263 %264
        %263 = OpLabel
        %265 = OpFunctionCall %bool %test_mat4_b
               OpBranch %264
        %264 = OpLabel
        %266 = OpPhi %bool %false %260 %265 %263
               OpSelectionMerge %268 None
               OpBranchConditional %266 %267 %268
        %267 = OpLabel
        %269 = OpFunctionCall %bool %test_hmat2_b
               OpBranch %268
        %268 = OpLabel
        %270 = OpPhi %bool %false %264 %269 %267
               OpSelectionMerge %272 None
               OpBranchConditional %270 %271 %272
        %271 = OpLabel
        %273 = OpFunctionCall %bool %test_hmat3_b
               OpBranch %272
        %272 = OpLabel
        %274 = OpPhi %bool %false %268 %273 %271
               OpSelectionMerge %276 None
               OpBranchConditional %274 %275 %276
        %275 = OpLabel
        %277 = OpFunctionCall %bool %test_hmat4_b
               OpBranch %276
        %276 = OpLabel
        %278 = OpPhi %bool %false %272 %277 %275
               OpSelectionMerge %283 None
               OpBranchConditional %278 %281 %282
        %281 = OpLabel
        %284 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
        %286 = OpLoad %v4float %284
               OpStore %279 %286
               OpBranch %283
        %282 = OpLabel
        %287 = OpAccessChain %_ptr_Uniform_v4float %19 %int_2
        %289 = OpLoad %v4float %287
               OpStore %279 %289
               OpBranch %283
        %283 = OpLabel
        %290 = OpLoad %v4float %279
               OpReturnValue %290
               OpFunctionEnd
