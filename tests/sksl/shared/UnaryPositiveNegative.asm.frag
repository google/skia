               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %16
               OpName %_UniformBuffer "_UniformBuffer"  ; id %26
               OpMemberName %_UniformBuffer 0 "colorWhite"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpMemberName %_UniformBuffer 3 "testMatrix2x2"
               OpMemberName %_UniformBuffer 4 "testMatrix3x3"
               OpMemberName %_UniformBuffer 5 "testMatrix4x4"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %28
               OpName %test_iscalar_b "test_iscalar_b"  ; id %6
               OpName %x "x"                            ; id %40
               OpName %test_fvec_b "test_fvec_b"        ; id %7
               OpName %x_0 "x"                          ; id %52
               OpName %test_ivec_b "test_ivec_b"        ; id %8
               OpName %x_1 "x"                          ; id %63
               OpName %test_mat2_b "test_mat2_b"        ; id %9
               OpName %negated "negated"                ; id %76
               OpName %x_2 "x"                          ; id %84
               OpName %test_mat3_b "test_mat3_b"        ; id %10
               OpName %negated_0 "negated"              ; id %100
               OpName %x_3 "x"                          ; id %111
               OpName %test_mat4_b "test_mat4_b"        ; id %11
               OpName %negated_1 "negated"              ; id %133
               OpName %x_4 "x"                          ; id %147
               OpName %test_hmat2_b "test_hmat2_b"      ; id %12
               OpName %negated_2 "negated"              ; id %174
               OpName %x_5 "x"                          ; id %175
               OpName %test_hmat3_b "test_hmat3_b"      ; id %13
               OpName %negated_3 "negated"              ; id %189
               OpName %x_6 "x"                          ; id %190
               OpName %test_hmat4_b "test_hmat4_b"      ; id %14
               OpName %negated_4 "negated"              ; id %209
               OpName %x_7 "x"                          ; id %210
               OpName %main "main"                      ; id %15
               OpName %_0_x "_0_x"                      ; id %236

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
               OpDecorate %20 Binding 0
               OpDecorate %20 DescriptorSet 0
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %x_0 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %negated_2 RelaxedPrecision
               OpDecorate %x_5 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %negated_3 RelaxedPrecision
               OpDecorate %x_6 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %negated_4 RelaxedPrecision
               OpDecorate %x_7 RelaxedPrecision
               OpDecorate %213 RelaxedPrecision
               OpDecorate %214 RelaxedPrecision
               OpDecorate %215 RelaxedPrecision
               OpDecorate %216 RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %219 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %221 RelaxedPrecision
               OpDecorate %222 RelaxedPrecision
               OpDecorate %224 RelaxedPrecision
               OpDecorate %227 RelaxedPrecision
               OpDecorate %230 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %287 RelaxedPrecision
               OpDecorate %290 RelaxedPrecision
               OpDecorate %291 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %mat2v2float %mat3v3float %mat4v4float    ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %20 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %30 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %33 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %38 = OpTypeFunction %bool
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
     %int_n1 = OpConstant %int -1
   %float_n1 = OpConstant %float -1
         %58 = OpConstantComposite %v2float %float_n1 %float_n1
     %v2bool = OpTypeVector %bool 2
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
         %72 = OpConstantComposite %v2int %int_n1 %int_n1
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
   %float_n2 = OpConstant %float -2
   %float_n3 = OpConstant %float -3
   %float_n4 = OpConstant %float -4
         %81 = OpConstantComposite %v2float %float_n1 %float_n2
         %82 = OpConstantComposite %v2float %float_n3 %float_n4
         %83 = OpConstantComposite %mat2v2float %81 %82
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
      %int_3 = OpConstant %int 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
   %float_n5 = OpConstant %float -5
   %float_n6 = OpConstant %float -6
   %float_n7 = OpConstant %float -7
   %float_n8 = OpConstant %float -8
   %float_n9 = OpConstant %float -9
        %107 = OpConstantComposite %v3float %float_n1 %float_n2 %float_n3
        %108 = OpConstantComposite %v3float %float_n4 %float_n5 %float_n6
        %109 = OpConstantComposite %v3float %float_n7 %float_n8 %float_n9
        %110 = OpConstantComposite %mat3v3float %107 %108 %109
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
        %142 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n3 %float_n4
        %143 = OpConstantComposite %v4float %float_n5 %float_n6 %float_n7 %float_n8
        %144 = OpConstantComposite %v4float %float_n9 %float_n10 %float_n11 %float_n12
        %145 = OpConstantComposite %v4float %float_n13 %float_n14 %float_n15 %float_n16
        %146 = OpConstantComposite %mat4v4float %142 %143 %144 %145
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
      %int_5 = OpConstant %int 5
     %v4bool = OpTypeVector %bool 4
        %233 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
      %false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %30

         %31 = OpLabel
         %34 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %34 %33
         %36 =   OpFunctionCall %v4float %main %34
                 OpStore %sk_FragColor %36
                 OpReturn
               OpFunctionEnd


               ; Function test_iscalar_b
%test_iscalar_b = OpFunction %bool None %38

         %39 = OpLabel
          %x =   OpVariable %_ptr_Function_int Function
         %42 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_0
         %45 =   OpLoad %v4float %42                ; RelaxedPrecision
         %46 =   OpCompositeExtract %float %45 0    ; RelaxedPrecision
         %47 =   OpConvertFToS %int %46
                 OpStore %x %47
         %48 =   OpSNegate %int %47
                 OpStore %x %48
         %50 =   OpIEqual %bool %48 %int_n1
                 OpReturnValue %50
               OpFunctionEnd


               ; Function test_fvec_b
%test_fvec_b = OpFunction %bool None %38

         %51 = OpLabel
        %x_0 =   OpVariable %_ptr_Function_v2float Function     ; RelaxedPrecision
         %53 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_0
         %54 =   OpLoad %v4float %53                ; RelaxedPrecision
         %55 =   OpVectorShuffle %v2float %54 %54 0 1   ; RelaxedPrecision
                 OpStore %x_0 %55
         %56 =   OpFNegate %v2float %55             ; RelaxedPrecision
                 OpStore %x_0 %56
         %59 =   OpFOrdEqual %v2bool %56 %58
         %61 =   OpAll %bool %59
                 OpReturnValue %61
               OpFunctionEnd


               ; Function test_ivec_b
%test_ivec_b = OpFunction %bool None %38

         %62 = OpLabel
        %x_1 =   OpVariable %_ptr_Function_v2int Function
         %66 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_0
         %67 =   OpLoad %v4float %66                ; RelaxedPrecision
         %68 =   OpCompositeExtract %float %67 0    ; RelaxedPrecision
         %69 =   OpConvertFToS %int %68
         %70 =   OpCompositeConstruct %v2int %69 %69
                 OpStore %x_1 %70
         %71 =   OpSNegate %v2int %70
                 OpStore %x_1 %71
         %73 =   OpIEqual %v2bool %71 %72
         %74 =   OpAll %bool %73
                 OpReturnValue %74
               OpFunctionEnd


               ; Function test_mat2_b
%test_mat2_b = OpFunction %bool None %38

         %75 = OpLabel
    %negated =   OpVariable %_ptr_Function_mat2v2float Function
        %x_2 =   OpVariable %_ptr_Function_mat2v2float Function
                 OpStore %negated %83
         %85 =   OpAccessChain %_ptr_Uniform_mat2v2float %20 %int_3
         %88 =   OpLoad %mat2v2float %85
                 OpStore %x_2 %88
         %89 =   OpCompositeExtract %v2float %88 0
         %90 =   OpFNegate %v2float %89
         %91 =   OpCompositeExtract %v2float %88 1
         %92 =   OpFNegate %v2float %91
         %93 =   OpCompositeConstruct %mat2v2float %90 %92
                 OpStore %x_2 %93
         %94 =   OpFOrdEqual %v2bool %90 %81
         %95 =   OpAll %bool %94
         %96 =   OpFOrdEqual %v2bool %92 %82
         %97 =   OpAll %bool %96
         %98 =   OpLogicalAnd %bool %95 %97
                 OpReturnValue %98
               OpFunctionEnd


               ; Function test_mat3_b
%test_mat3_b = OpFunction %bool None %38

         %99 = OpLabel
  %negated_0 =   OpVariable %_ptr_Function_mat3v3float Function
        %x_3 =   OpVariable %_ptr_Function_mat3v3float Function
                 OpStore %negated_0 %110
        %112 =   OpAccessChain %_ptr_Uniform_mat3v3float %20 %int_4
        %115 =   OpLoad %mat3v3float %112
                 OpStore %x_3 %115
        %116 =   OpCompositeExtract %v3float %115 0
        %117 =   OpFNegate %v3float %116
        %118 =   OpCompositeExtract %v3float %115 1
        %119 =   OpFNegate %v3float %118
        %120 =   OpCompositeExtract %v3float %115 2
        %121 =   OpFNegate %v3float %120
        %122 =   OpCompositeConstruct %mat3v3float %117 %119 %121
                 OpStore %x_3 %122
        %124 =   OpFOrdEqual %v3bool %117 %107
        %125 =   OpAll %bool %124
        %126 =   OpFOrdEqual %v3bool %119 %108
        %127 =   OpAll %bool %126
        %128 =   OpLogicalAnd %bool %125 %127
        %129 =   OpFOrdEqual %v3bool %121 %109
        %130 =   OpAll %bool %129
        %131 =   OpLogicalAnd %bool %128 %130
                 OpReturnValue %131
               OpFunctionEnd


               ; Function test_mat4_b
%test_mat4_b = OpFunction %bool None %38

        %132 = OpLabel
  %negated_1 =   OpVariable %_ptr_Function_mat4v4float Function
        %x_4 =   OpVariable %_ptr_Function_mat4v4float Function
                 OpStore %negated_1 %146
        %148 =   OpAccessChain %_ptr_Uniform_mat4v4float %20 %int_5
        %151 =   OpLoad %mat4v4float %148
                 OpStore %x_4 %151
        %152 =   OpCompositeExtract %v4float %151 0
        %153 =   OpFNegate %v4float %152
        %154 =   OpCompositeExtract %v4float %151 1
        %155 =   OpFNegate %v4float %154
        %156 =   OpCompositeExtract %v4float %151 2
        %157 =   OpFNegate %v4float %156
        %158 =   OpCompositeExtract %v4float %151 3
        %159 =   OpFNegate %v4float %158
        %160 =   OpCompositeConstruct %mat4v4float %153 %155 %157 %159
                 OpStore %x_4 %160
        %162 =   OpFOrdEqual %v4bool %153 %142
        %163 =   OpAll %bool %162
        %164 =   OpFOrdEqual %v4bool %155 %143
        %165 =   OpAll %bool %164
        %166 =   OpLogicalAnd %bool %163 %165
        %167 =   OpFOrdEqual %v4bool %157 %144
        %168 =   OpAll %bool %167
        %169 =   OpLogicalAnd %bool %166 %168
        %170 =   OpFOrdEqual %v4bool %159 %145
        %171 =   OpAll %bool %170
        %172 =   OpLogicalAnd %bool %169 %171
                 OpReturnValue %172
               OpFunctionEnd


               ; Function test_hmat2_b
%test_hmat2_b = OpFunction %bool None %38

        %173 = OpLabel
  %negated_2 =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
        %x_5 =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
                 OpStore %negated_2 %83
        %176 =   OpAccessChain %_ptr_Uniform_mat2v2float %20 %int_3
        %177 =   OpLoad %mat2v2float %176
                 OpStore %x_5 %177
        %178 =   OpCompositeExtract %v2float %177 0     ; RelaxedPrecision
        %179 =   OpFNegate %v2float %178                ; RelaxedPrecision
        %180 =   OpCompositeExtract %v2float %177 1     ; RelaxedPrecision
        %181 =   OpFNegate %v2float %180                ; RelaxedPrecision
        %182 =   OpCompositeConstruct %mat2v2float %179 %181    ; RelaxedPrecision
                 OpStore %x_5 %182
        %183 =   OpFOrdEqual %v2bool %179 %81       ; RelaxedPrecision
        %184 =   OpAll %bool %183
        %185 =   OpFOrdEqual %v2bool %181 %82       ; RelaxedPrecision
        %186 =   OpAll %bool %185
        %187 =   OpLogicalAnd %bool %184 %186
                 OpReturnValue %187
               OpFunctionEnd


               ; Function test_hmat3_b
%test_hmat3_b = OpFunction %bool None %38

        %188 = OpLabel
  %negated_3 =   OpVariable %_ptr_Function_mat3v3float Function     ; RelaxedPrecision
        %x_6 =   OpVariable %_ptr_Function_mat3v3float Function     ; RelaxedPrecision
                 OpStore %negated_3 %110
        %191 =   OpAccessChain %_ptr_Uniform_mat3v3float %20 %int_4
        %192 =   OpLoad %mat3v3float %191
                 OpStore %x_6 %192
        %193 =   OpCompositeExtract %v3float %192 0     ; RelaxedPrecision
        %194 =   OpFNegate %v3float %193                ; RelaxedPrecision
        %195 =   OpCompositeExtract %v3float %192 1     ; RelaxedPrecision
        %196 =   OpFNegate %v3float %195                ; RelaxedPrecision
        %197 =   OpCompositeExtract %v3float %192 2     ; RelaxedPrecision
        %198 =   OpFNegate %v3float %197                ; RelaxedPrecision
        %199 =   OpCompositeConstruct %mat3v3float %194 %196 %198   ; RelaxedPrecision
                 OpStore %x_6 %199
        %200 =   OpFOrdEqual %v3bool %194 %107      ; RelaxedPrecision
        %201 =   OpAll %bool %200
        %202 =   OpFOrdEqual %v3bool %196 %108      ; RelaxedPrecision
        %203 =   OpAll %bool %202
        %204 =   OpLogicalAnd %bool %201 %203
        %205 =   OpFOrdEqual %v3bool %198 %109      ; RelaxedPrecision
        %206 =   OpAll %bool %205
        %207 =   OpLogicalAnd %bool %204 %206
                 OpReturnValue %207
               OpFunctionEnd


               ; Function test_hmat4_b
%test_hmat4_b = OpFunction %bool None %38

        %208 = OpLabel
  %negated_4 =   OpVariable %_ptr_Function_mat4v4float Function     ; RelaxedPrecision
        %x_7 =   OpVariable %_ptr_Function_mat4v4float Function     ; RelaxedPrecision
                 OpStore %negated_4 %146
        %211 =   OpAccessChain %_ptr_Uniform_mat4v4float %20 %int_5
        %212 =   OpLoad %mat4v4float %211
                 OpStore %x_7 %212
        %213 =   OpCompositeExtract %v4float %212 0     ; RelaxedPrecision
        %214 =   OpFNegate %v4float %213                ; RelaxedPrecision
        %215 =   OpCompositeExtract %v4float %212 1     ; RelaxedPrecision
        %216 =   OpFNegate %v4float %215                ; RelaxedPrecision
        %217 =   OpCompositeExtract %v4float %212 2     ; RelaxedPrecision
        %218 =   OpFNegate %v4float %217                ; RelaxedPrecision
        %219 =   OpCompositeExtract %v4float %212 3     ; RelaxedPrecision
        %220 =   OpFNegate %v4float %219                ; RelaxedPrecision
        %221 =   OpCompositeConstruct %mat4v4float %214 %216 %218 %220  ; RelaxedPrecision
                 OpStore %x_7 %221
        %222 =   OpFOrdEqual %v4bool %214 %142      ; RelaxedPrecision
        %223 =   OpAll %bool %222
        %224 =   OpFOrdEqual %v4bool %216 %143      ; RelaxedPrecision
        %225 =   OpAll %bool %224
        %226 =   OpLogicalAnd %bool %223 %225
        %227 =   OpFOrdEqual %v4bool %218 %144      ; RelaxedPrecision
        %228 =   OpAll %bool %227
        %229 =   OpLogicalAnd %bool %226 %228
        %230 =   OpFOrdEqual %v4bool %220 %145      ; RelaxedPrecision
        %231 =   OpAll %bool %230
        %232 =   OpLogicalAnd %bool %229 %231
                 OpReturnValue %232
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %233        ; RelaxedPrecision
        %234 = OpFunctionParameter %_ptr_Function_v2float

        %235 = OpLabel
       %_0_x =   OpVariable %_ptr_Function_float Function
        %280 =   OpVariable %_ptr_Function_v4float Function
        %238 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_0
        %239 =   OpLoad %v4float %238               ; RelaxedPrecision
        %240 =   OpCompositeExtract %float %239 0   ; RelaxedPrecision
                 OpStore %_0_x %240
        %241 =   OpFNegate %float %240
                 OpStore %_0_x %241
        %243 =   OpFOrdEqual %bool %241 %float_n1
                 OpSelectionMerge %245 None
                 OpBranchConditional %243 %244 %245

        %244 =     OpLabel
        %246 =       OpFunctionCall %bool %test_iscalar_b
                     OpBranch %245

        %245 = OpLabel
        %247 =   OpPhi %bool %false %235 %246 %244
                 OpSelectionMerge %249 None
                 OpBranchConditional %247 %248 %249

        %248 =     OpLabel
        %250 =       OpFunctionCall %bool %test_fvec_b
                     OpBranch %249

        %249 = OpLabel
        %251 =   OpPhi %bool %false %245 %250 %248
                 OpSelectionMerge %253 None
                 OpBranchConditional %251 %252 %253

        %252 =     OpLabel
        %254 =       OpFunctionCall %bool %test_ivec_b
                     OpBranch %253

        %253 = OpLabel
        %255 =   OpPhi %bool %false %249 %254 %252
                 OpSelectionMerge %257 None
                 OpBranchConditional %255 %256 %257

        %256 =     OpLabel
        %258 =       OpFunctionCall %bool %test_mat2_b
                     OpBranch %257

        %257 = OpLabel
        %259 =   OpPhi %bool %false %253 %258 %256
                 OpSelectionMerge %261 None
                 OpBranchConditional %259 %260 %261

        %260 =     OpLabel
        %262 =       OpFunctionCall %bool %test_mat3_b
                     OpBranch %261

        %261 = OpLabel
        %263 =   OpPhi %bool %false %257 %262 %260
                 OpSelectionMerge %265 None
                 OpBranchConditional %263 %264 %265

        %264 =     OpLabel
        %266 =       OpFunctionCall %bool %test_mat4_b
                     OpBranch %265

        %265 = OpLabel
        %267 =   OpPhi %bool %false %261 %266 %264
                 OpSelectionMerge %269 None
                 OpBranchConditional %267 %268 %269

        %268 =     OpLabel
        %270 =       OpFunctionCall %bool %test_hmat2_b
                     OpBranch %269

        %269 = OpLabel
        %271 =   OpPhi %bool %false %265 %270 %268
                 OpSelectionMerge %273 None
                 OpBranchConditional %271 %272 %273

        %272 =     OpLabel
        %274 =       OpFunctionCall %bool %test_hmat3_b
                     OpBranch %273

        %273 = OpLabel
        %275 =   OpPhi %bool %false %269 %274 %272
                 OpSelectionMerge %277 None
                 OpBranchConditional %275 %276 %277

        %276 =     OpLabel
        %278 =       OpFunctionCall %bool %test_hmat4_b
                     OpBranch %277

        %277 = OpLabel
        %279 =   OpPhi %bool %false %273 %278 %276
                 OpSelectionMerge %284 None
                 OpBranchConditional %279 %282 %283

        %282 =     OpLabel
        %285 =       OpAccessChain %_ptr_Uniform_v4float %20 %int_1
        %287 =       OpLoad %v4float %285           ; RelaxedPrecision
                     OpStore %280 %287
                     OpBranch %284

        %283 =     OpLabel
        %288 =       OpAccessChain %_ptr_Uniform_v4float %20 %int_2
        %290 =       OpLoad %v4float %288           ; RelaxedPrecision
                     OpStore %280 %290
                     OpBranch %284

        %284 = OpLabel
        %291 =   OpLoad %v4float %280               ; RelaxedPrecision
                 OpReturnValue %291
               OpFunctionEnd
