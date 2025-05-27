               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorRed"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %test_matrix_op_matrix_half_b "test_matrix_op_matrix_half_b"
               OpName %ok "ok"
               OpName %splat_4 "splat_4"
               OpName %m "m"
               OpName %splat_4_0 "splat_4"
               OpName %m_0 "m"
               OpName %m_1 "m"
               OpName %m_2 "m"
               OpName %m_3 "m"
               OpName %m_4 "m"
               OpName %main "main"
               OpName %_0_ok "_0_ok"
               OpName %_1_splat_4 "_1_splat_4"
               OpName %_2_m "_2_m"
               OpName %_3_splat_4 "_3_splat_4"
               OpName %_4_m "_4_m"
               OpName %_5_m "_5_m"
               OpName %_6_m "_6_m"
               OpName %_7_m "_7_m"
               OpName %_8_m "_8_m"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %splat_4 RelaxedPrecision
               OpDecorate %m RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %splat_4_0 RelaxedPrecision
               OpDecorate %m_0 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %m_1 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %m_2 RelaxedPrecision
               OpDecorate %219 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %221 RelaxedPrecision
               OpDecorate %222 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %238 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %243 RelaxedPrecision
               OpDecorate %246 RelaxedPrecision
               OpDecorate %m_3 RelaxedPrecision
               OpDecorate %258 RelaxedPrecision
               OpDecorate %259 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
               OpDecorate %267 RelaxedPrecision
               OpDecorate %269 RelaxedPrecision
               OpDecorate %m_4 RelaxedPrecision
               OpDecorate %281 RelaxedPrecision
               OpDecorate %292 RelaxedPrecision
               OpDecorate %293 RelaxedPrecision
               OpDecorate %295 RelaxedPrecision
               OpDecorate %296 RelaxedPrecision
               OpDecorate %463 RelaxedPrecision
               OpDecorate %466 RelaxedPrecision
               OpDecorate %467 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
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
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
    %float_4 = OpConstant %float 4
         %31 = OpConstantComposite %v2float %float_4 %float_4
         %32 = OpConstantComposite %mat3v2float %31 %31 %31
    %float_2 = OpConstant %float 2
         %35 = OpConstantComposite %v2float %float_2 %float_0
         %36 = OpConstantComposite %v2float %float_0 %float_2
         %37 = OpConstantComposite %mat3v2float %35 %36 %17
      %false = OpConstantFalse %bool
    %float_6 = OpConstant %float 6
         %46 = OpConstantComposite %v2float %float_6 %float_4
         %47 = OpConstantComposite %v2float %float_4 %float_6
         %48 = OpConstantComposite %mat3v2float %46 %47 %31
     %v2bool = OpTypeVector %bool 2
   %float_n2 = OpConstant %float -2
   %float_n4 = OpConstant %float -4
         %67 = OpConstantComposite %v2float %float_n2 %float_n4
         %68 = OpConstantComposite %v2float %float_n4 %float_n2
         %69 = OpConstantComposite %v2float %float_n4 %float_n4
         %70 = OpConstantComposite %mat3v2float %67 %68 %69
  %float_0_5 = OpConstant %float 0.5
         %87 = OpConstantComposite %v2float %float_0_5 %float_0
         %88 = OpConstantComposite %v2float %float_0 %float_0_5
         %89 = OpConstantComposite %mat3v2float %87 %88 %17
    %v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
        %103 = OpConstantComposite %v3float %float_4 %float_4 %float_4
        %104 = OpConstantComposite %mat2v3float %103 %103
        %106 = OpConstantComposite %v3float %float_2 %float_0 %float_0
        %107 = OpConstantComposite %v3float %float_0 %float_2 %float_0
        %108 = OpConstantComposite %mat2v3float %106 %107
        %114 = OpConstantComposite %v3float %float_6 %float_4 %float_4
        %115 = OpConstantComposite %v3float %float_4 %float_6 %float_4
        %116 = OpConstantComposite %mat2v3float %114 %115
     %v3bool = OpTypeVector %bool 3
        %129 = OpConstantComposite %v3float %float_2 %float_4 %float_4
        %130 = OpConstantComposite %v3float %float_4 %float_2 %float_4
        %131 = OpConstantComposite %mat2v3float %129 %130
        %138 = OpConstantComposite %v3float %float_2 %float_2 %float_2
        %139 = OpConstantComposite %mat2v3float %138 %138
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
    %float_1 = OpConstant %float 1
    %float_3 = OpConstant %float 3
    %float_5 = OpConstant %float 5
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
    %float_9 = OpConstant %float 9
   %float_10 = OpConstant %float 10
   %float_11 = OpConstant %float 11
   %float_12 = OpConstant %float 12
        %163 = OpConstantComposite %v3float %float_1 %float_2 %float_3
        %164 = OpConstantComposite %v3float %float_4 %float_5 %float_6
        %165 = OpConstantComposite %v3float %float_7 %float_8 %float_9
        %166 = OpConstantComposite %v3float %float_10 %float_11 %float_12
        %167 = OpConstantComposite %mat4v3float %163 %164 %165 %166
   %float_16 = OpConstant %float 16
   %float_15 = OpConstant %float 15
   %float_14 = OpConstant %float 14
   %float_13 = OpConstant %float 13
        %172 = OpConstantComposite %v3float %float_16 %float_15 %float_14
        %173 = OpConstantComposite %v3float %float_13 %float_12 %float_11
        %174 = OpConstantComposite %v3float %float_10 %float_9 %float_8
        %175 = OpConstantComposite %v3float %float_7 %float_6 %float_5
        %176 = OpConstantComposite %mat4v3float %172 %173 %174 %175
   %float_17 = OpConstant %float 17
        %185 = OpConstantComposite %v3float %float_17 %float_17 %float_17
        %186 = OpConstantComposite %mat4v3float %185 %185 %185 %185
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
   %float_20 = OpConstant %float 20
   %float_30 = OpConstant %float 30
   %float_40 = OpConstant %float 40
   %float_50 = OpConstant %float 50
   %float_60 = OpConstant %float 60
   %float_70 = OpConstant %float 70
   %float_80 = OpConstant %float 80
        %209 = OpConstantComposite %v2float %float_10 %float_20
        %210 = OpConstantComposite %v2float %float_30 %float_40
        %211 = OpConstantComposite %v2float %float_50 %float_60
        %212 = OpConstantComposite %v2float %float_70 %float_80
        %213 = OpConstantComposite %mat4v2float %209 %210 %211 %212
        %214 = OpConstantComposite %v2float %float_1 %float_2
        %215 = OpConstantComposite %v2float %float_3 %float_4
        %216 = OpConstantComposite %v2float %float_5 %float_6
        %217 = OpConstantComposite %v2float %float_7 %float_8
        %218 = OpConstantComposite %mat4v2float %214 %215 %216 %217
   %float_18 = OpConstant %float 18
   %float_27 = OpConstant %float 27
   %float_36 = OpConstant %float 36
   %float_45 = OpConstant %float 45
   %float_54 = OpConstant %float 54
   %float_63 = OpConstant %float 63
   %float_72 = OpConstant %float 72
        %233 = OpConstantComposite %v2float %float_9 %float_18
        %234 = OpConstantComposite %v2float %float_27 %float_36
        %235 = OpConstantComposite %v2float %float_45 %float_54
        %236 = OpConstantComposite %v2float %float_63 %float_72
        %237 = OpConstantComposite %mat4v2float %233 %234 %235 %236
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
        %253 = OpConstantComposite %v4float %float_10 %float_20 %float_30 %float_40
        %254 = OpConstantComposite %mat2v4float %253 %253
        %255 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
        %256 = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
        %257 = OpConstantComposite %mat2v4float %255 %256
        %263 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
        %264 = OpConstantComposite %v4float %float_2 %float_4 %float_6 %float_8
        %265 = OpConstantComposite %mat2v4float %263 %264
     %v4bool = OpTypeVector %bool 4
        %274 = OpConstantComposite %v3float %float_7 %float_9 %float_11
        %275 = OpConstantComposite %v3float %float_8 %float_10 %float_12
        %276 = OpConstantComposite %mat2v3float %274 %275
        %277 = OpConstantComposite %v2float %float_1 %float_4
        %278 = OpConstantComposite %v2float %float_2 %float_5
%mat2v2float = OpTypeMatrix %v2float 2
        %280 = OpConstantComposite %mat2v2float %277 %278
   %float_39 = OpConstant %float 39
   %float_49 = OpConstant %float 49
   %float_59 = OpConstant %float 59
   %float_68 = OpConstant %float 68
   %float_82 = OpConstant %float 82
        %289 = OpConstantComposite %v3float %float_39 %float_49 %float_59
        %290 = OpConstantComposite %v3float %float_54 %float_68 %float_82
        %291 = OpConstantComposite %mat2v3float %289 %290
        %300 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %13
         %14 = OpLabel
         %18 = OpVariable %_ptr_Function_v2float Function
               OpStore %18 %17
         %20 = OpFunctionCall %v4float %main %18
               OpStore %sk_FragColor %20
               OpReturn
               OpFunctionEnd
%test_matrix_op_matrix_half_b = OpFunction %bool None %22
         %23 = OpLabel
         %ok = OpVariable %_ptr_Function_bool Function
    %splat_4 = OpVariable %_ptr_Function_mat3v2float Function
          %m = OpVariable %_ptr_Function_mat3v2float Function
  %splat_4_0 = OpVariable %_ptr_Function_mat2v3float Function
        %m_0 = OpVariable %_ptr_Function_mat2v3float Function
        %m_1 = OpVariable %_ptr_Function_mat4v3float Function
        %m_2 = OpVariable %_ptr_Function_mat4v2float Function
        %m_3 = OpVariable %_ptr_Function_mat2v4float Function
        %m_4 = OpVariable %_ptr_Function_mat2v3float Function
               OpStore %ok %true
               OpStore %splat_4 %32
               OpStore %m %37
         %38 = OpFAdd %v2float %35 %31
         %39 = OpFAdd %v2float %36 %31
         %40 = OpFAdd %v2float %17 %31
         %41 = OpCompositeConstruct %mat3v2float %38 %39 %40
               OpStore %m %41
               OpSelectionMerge %44 None
               OpBranchConditional %true %43 %44
         %43 = OpLabel
         %50 = OpFOrdEqual %v2bool %38 %46
         %51 = OpAll %bool %50
         %52 = OpFOrdEqual %v2bool %39 %47
         %53 = OpAll %bool %52
         %54 = OpLogicalAnd %bool %51 %53
         %55 = OpFOrdEqual %v2bool %40 %31
         %56 = OpAll %bool %55
         %57 = OpLogicalAnd %bool %54 %56
               OpBranch %44
         %44 = OpLabel
         %58 = OpPhi %bool %false %23 %57 %43
               OpStore %ok %58
               OpStore %m %37
         %59 = OpFSub %v2float %35 %31
         %60 = OpFSub %v2float %36 %31
         %61 = OpFSub %v2float %17 %31
         %62 = OpCompositeConstruct %mat3v2float %59 %60 %61
               OpStore %m %62
               OpSelectionMerge %64 None
               OpBranchConditional %58 %63 %64
         %63 = OpLabel
         %71 = OpFOrdEqual %v2bool %59 %67
         %72 = OpAll %bool %71
         %73 = OpFOrdEqual %v2bool %60 %68
         %74 = OpAll %bool %73
         %75 = OpLogicalAnd %bool %72 %74
         %76 = OpFOrdEqual %v2bool %61 %69
         %77 = OpAll %bool %76
         %78 = OpLogicalAnd %bool %75 %77
               OpBranch %64
         %64 = OpLabel
         %79 = OpPhi %bool %false %44 %78 %63
               OpStore %ok %79
               OpStore %m %37
         %80 = OpFDiv %v2float %35 %31
         %81 = OpFDiv %v2float %36 %31
         %82 = OpFDiv %v2float %17 %31
         %83 = OpCompositeConstruct %mat3v2float %80 %81 %82
               OpStore %m %83
               OpSelectionMerge %85 None
               OpBranchConditional %79 %84 %85
         %84 = OpLabel
         %90 = OpFOrdEqual %v2bool %80 %87
         %91 = OpAll %bool %90
         %92 = OpFOrdEqual %v2bool %81 %88
         %93 = OpAll %bool %92
         %94 = OpLogicalAnd %bool %91 %93
         %95 = OpFOrdEqual %v2bool %82 %17
         %96 = OpAll %bool %95
         %97 = OpLogicalAnd %bool %94 %96
               OpBranch %85
         %85 = OpLabel
         %98 = OpPhi %bool %false %64 %97 %84
               OpStore %ok %98
               OpStore %splat_4_0 %104
               OpStore %m_0 %104
        %109 = OpFAdd %v3float %103 %106
        %110 = OpFAdd %v3float %103 %107
        %111 = OpCompositeConstruct %mat2v3float %109 %110
               OpStore %m_0 %111
               OpSelectionMerge %113 None
               OpBranchConditional %98 %112 %113
        %112 = OpLabel
        %118 = OpFOrdEqual %v3bool %109 %114
        %119 = OpAll %bool %118
        %120 = OpFOrdEqual %v3bool %110 %115
        %121 = OpAll %bool %120
        %122 = OpLogicalAnd %bool %119 %121
               OpBranch %113
        %113 = OpLabel
        %123 = OpPhi %bool %false %85 %122 %112
               OpStore %ok %123
               OpStore %m_0 %104
        %124 = OpFSub %v3float %103 %106
        %125 = OpFSub %v3float %103 %107
        %126 = OpCompositeConstruct %mat2v3float %124 %125
               OpStore %m_0 %126
               OpSelectionMerge %128 None
               OpBranchConditional %123 %127 %128
        %127 = OpLabel
        %132 = OpFOrdEqual %v3bool %124 %129
        %133 = OpAll %bool %132
        %134 = OpFOrdEqual %v3bool %125 %130
        %135 = OpAll %bool %134
        %136 = OpLogicalAnd %bool %133 %135
               OpBranch %128
        %128 = OpLabel
        %137 = OpPhi %bool %false %113 %136 %127
               OpStore %ok %137
               OpStore %m_0 %104
        %140 = OpFDiv %v3float %103 %138
        %141 = OpFDiv %v3float %103 %138
        %142 = OpCompositeConstruct %mat2v3float %140 %141
               OpStore %m_0 %142
               OpSelectionMerge %144 None
               OpBranchConditional %137 %143 %144
        %143 = OpLabel
        %145 = OpFOrdEqual %v3bool %140 %138
        %146 = OpAll %bool %145
        %147 = OpFOrdEqual %v3bool %141 %138
        %148 = OpAll %bool %147
        %149 = OpLogicalAnd %bool %146 %148
               OpBranch %144
        %144 = OpLabel
        %150 = OpPhi %bool %false %128 %149 %143
               OpStore %ok %150
               OpStore %m_1 %167
        %177 = OpFAdd %v3float %163 %172
        %178 = OpFAdd %v3float %164 %173
        %179 = OpFAdd %v3float %165 %174
        %180 = OpFAdd %v3float %166 %175
        %181 = OpCompositeConstruct %mat4v3float %177 %178 %179 %180
               OpStore %m_1 %181
               OpSelectionMerge %183 None
               OpBranchConditional %150 %182 %183
        %182 = OpLabel
        %187 = OpFOrdEqual %v3bool %177 %185
        %188 = OpAll %bool %187
        %189 = OpFOrdEqual %v3bool %178 %185
        %190 = OpAll %bool %189
        %191 = OpLogicalAnd %bool %188 %190
        %192 = OpFOrdEqual %v3bool %179 %185
        %193 = OpAll %bool %192
        %194 = OpLogicalAnd %bool %191 %193
        %195 = OpFOrdEqual %v3bool %180 %185
        %196 = OpAll %bool %195
        %197 = OpLogicalAnd %bool %194 %196
               OpBranch %183
        %183 = OpLabel
        %198 = OpPhi %bool %false %144 %197 %182
               OpStore %ok %198
               OpStore %m_2 %213
        %219 = OpFSub %v2float %209 %214
        %220 = OpFSub %v2float %210 %215
        %221 = OpFSub %v2float %211 %216
        %222 = OpFSub %v2float %212 %217
        %223 = OpCompositeConstruct %mat4v2float %219 %220 %221 %222
               OpStore %m_2 %223
               OpSelectionMerge %225 None
               OpBranchConditional %198 %224 %225
        %224 = OpLabel
        %238 = OpFOrdEqual %v2bool %219 %233
        %239 = OpAll %bool %238
        %240 = OpFOrdEqual %v2bool %220 %234
        %241 = OpAll %bool %240
        %242 = OpLogicalAnd %bool %239 %241
        %243 = OpFOrdEqual %v2bool %221 %235
        %244 = OpAll %bool %243
        %245 = OpLogicalAnd %bool %242 %244
        %246 = OpFOrdEqual %v2bool %222 %236
        %247 = OpAll %bool %246
        %248 = OpLogicalAnd %bool %245 %247
               OpBranch %225
        %225 = OpLabel
        %249 = OpPhi %bool %false %183 %248 %224
               OpStore %ok %249
               OpStore %m_3 %254
        %258 = OpFDiv %v4float %253 %255
        %259 = OpFDiv %v4float %253 %256
        %260 = OpCompositeConstruct %mat2v4float %258 %259
               OpStore %m_3 %260
               OpSelectionMerge %262 None
               OpBranchConditional %249 %261 %262
        %261 = OpLabel
        %267 = OpFOrdEqual %v4bool %258 %263
        %268 = OpAll %bool %267
        %269 = OpFOrdEqual %v4bool %259 %264
        %270 = OpAll %bool %269
        %271 = OpLogicalAnd %bool %268 %270
               OpBranch %262
        %262 = OpLabel
        %272 = OpPhi %bool %false %225 %271 %261
               OpStore %ok %272
               OpStore %m_4 %276
        %281 = OpMatrixTimesMatrix %mat2v3float %276 %280
               OpStore %m_4 %281
               OpSelectionMerge %283 None
               OpBranchConditional %272 %282 %283
        %282 = OpLabel
        %292 = OpCompositeExtract %v3float %281 0
        %293 = OpFOrdEqual %v3bool %292 %289
        %294 = OpAll %bool %293
        %295 = OpCompositeExtract %v3float %281 1
        %296 = OpFOrdEqual %v3bool %295 %290
        %297 = OpAll %bool %296
        %298 = OpLogicalAnd %bool %294 %297
               OpBranch %283
        %283 = OpLabel
        %299 = OpPhi %bool %false %262 %298 %282
               OpStore %ok %299
               OpReturnValue %299
               OpFunctionEnd
       %main = OpFunction %v4float None %300
        %301 = OpFunctionParameter %_ptr_Function_v2float
        %302 = OpLabel
      %_0_ok = OpVariable %_ptr_Function_bool Function
 %_1_splat_4 = OpVariable %_ptr_Function_mat3v2float Function
       %_2_m = OpVariable %_ptr_Function_mat3v2float Function
 %_3_splat_4 = OpVariable %_ptr_Function_mat2v3float Function
       %_4_m = OpVariable %_ptr_Function_mat2v3float Function
       %_5_m = OpVariable %_ptr_Function_mat4v3float Function
       %_6_m = OpVariable %_ptr_Function_mat4v2float Function
       %_7_m = OpVariable %_ptr_Function_mat2v4float Function
       %_8_m = OpVariable %_ptr_Function_mat2v3float Function
        %454 = OpVariable %_ptr_Function_v4float Function
               OpStore %_0_ok %true
               OpStore %_1_splat_4 %32
               OpStore %_2_m %37
        %306 = OpFAdd %v2float %35 %31
        %307 = OpFAdd %v2float %36 %31
        %308 = OpFAdd %v2float %17 %31
        %309 = OpCompositeConstruct %mat3v2float %306 %307 %308
               OpStore %_2_m %309
               OpSelectionMerge %311 None
               OpBranchConditional %true %310 %311
        %310 = OpLabel
        %312 = OpFOrdEqual %v2bool %306 %46
        %313 = OpAll %bool %312
        %314 = OpFOrdEqual %v2bool %307 %47
        %315 = OpAll %bool %314
        %316 = OpLogicalAnd %bool %313 %315
        %317 = OpFOrdEqual %v2bool %308 %31
        %318 = OpAll %bool %317
        %319 = OpLogicalAnd %bool %316 %318
               OpBranch %311
        %311 = OpLabel
        %320 = OpPhi %bool %false %302 %319 %310
               OpStore %_0_ok %320
               OpStore %_2_m %37
        %321 = OpFSub %v2float %35 %31
        %322 = OpFSub %v2float %36 %31
        %323 = OpFSub %v2float %17 %31
        %324 = OpCompositeConstruct %mat3v2float %321 %322 %323
               OpStore %_2_m %324
               OpSelectionMerge %326 None
               OpBranchConditional %320 %325 %326
        %325 = OpLabel
        %327 = OpFOrdEqual %v2bool %321 %67
        %328 = OpAll %bool %327
        %329 = OpFOrdEqual %v2bool %322 %68
        %330 = OpAll %bool %329
        %331 = OpLogicalAnd %bool %328 %330
        %332 = OpFOrdEqual %v2bool %323 %69
        %333 = OpAll %bool %332
        %334 = OpLogicalAnd %bool %331 %333
               OpBranch %326
        %326 = OpLabel
        %335 = OpPhi %bool %false %311 %334 %325
               OpStore %_0_ok %335
               OpStore %_2_m %37
        %336 = OpFDiv %v2float %35 %31
        %337 = OpFDiv %v2float %36 %31
        %338 = OpFDiv %v2float %17 %31
        %339 = OpCompositeConstruct %mat3v2float %336 %337 %338
               OpStore %_2_m %339
               OpSelectionMerge %341 None
               OpBranchConditional %335 %340 %341
        %340 = OpLabel
        %342 = OpFOrdEqual %v2bool %336 %87
        %343 = OpAll %bool %342
        %344 = OpFOrdEqual %v2bool %337 %88
        %345 = OpAll %bool %344
        %346 = OpLogicalAnd %bool %343 %345
        %347 = OpFOrdEqual %v2bool %338 %17
        %348 = OpAll %bool %347
        %349 = OpLogicalAnd %bool %346 %348
               OpBranch %341
        %341 = OpLabel
        %350 = OpPhi %bool %false %326 %349 %340
               OpStore %_0_ok %350
               OpStore %_3_splat_4 %104
               OpStore %_4_m %104
        %353 = OpFAdd %v3float %103 %106
        %354 = OpFAdd %v3float %103 %107
        %355 = OpCompositeConstruct %mat2v3float %353 %354
               OpStore %_4_m %355
               OpSelectionMerge %357 None
               OpBranchConditional %350 %356 %357
        %356 = OpLabel
        %358 = OpFOrdEqual %v3bool %353 %114
        %359 = OpAll %bool %358
        %360 = OpFOrdEqual %v3bool %354 %115
        %361 = OpAll %bool %360
        %362 = OpLogicalAnd %bool %359 %361
               OpBranch %357
        %357 = OpLabel
        %363 = OpPhi %bool %false %341 %362 %356
               OpStore %_0_ok %363
               OpStore %_4_m %104
        %364 = OpFSub %v3float %103 %106
        %365 = OpFSub %v3float %103 %107
        %366 = OpCompositeConstruct %mat2v3float %364 %365
               OpStore %_4_m %366
               OpSelectionMerge %368 None
               OpBranchConditional %363 %367 %368
        %367 = OpLabel
        %369 = OpFOrdEqual %v3bool %364 %129
        %370 = OpAll %bool %369
        %371 = OpFOrdEqual %v3bool %365 %130
        %372 = OpAll %bool %371
        %373 = OpLogicalAnd %bool %370 %372
               OpBranch %368
        %368 = OpLabel
        %374 = OpPhi %bool %false %357 %373 %367
               OpStore %_0_ok %374
               OpStore %_4_m %104
        %375 = OpFDiv %v3float %103 %138
        %376 = OpFDiv %v3float %103 %138
        %377 = OpCompositeConstruct %mat2v3float %375 %376
               OpStore %_4_m %377
               OpSelectionMerge %379 None
               OpBranchConditional %374 %378 %379
        %378 = OpLabel
        %380 = OpFOrdEqual %v3bool %375 %138
        %381 = OpAll %bool %380
        %382 = OpFOrdEqual %v3bool %376 %138
        %383 = OpAll %bool %382
        %384 = OpLogicalAnd %bool %381 %383
               OpBranch %379
        %379 = OpLabel
        %385 = OpPhi %bool %false %368 %384 %378
               OpStore %_0_ok %385
               OpStore %_5_m %167
        %387 = OpFAdd %v3float %163 %172
        %388 = OpFAdd %v3float %164 %173
        %389 = OpFAdd %v3float %165 %174
        %390 = OpFAdd %v3float %166 %175
        %391 = OpCompositeConstruct %mat4v3float %387 %388 %389 %390
               OpStore %_5_m %391
               OpSelectionMerge %393 None
               OpBranchConditional %385 %392 %393
        %392 = OpLabel
        %394 = OpFOrdEqual %v3bool %387 %185
        %395 = OpAll %bool %394
        %396 = OpFOrdEqual %v3bool %388 %185
        %397 = OpAll %bool %396
        %398 = OpLogicalAnd %bool %395 %397
        %399 = OpFOrdEqual %v3bool %389 %185
        %400 = OpAll %bool %399
        %401 = OpLogicalAnd %bool %398 %400
        %402 = OpFOrdEqual %v3bool %390 %185
        %403 = OpAll %bool %402
        %404 = OpLogicalAnd %bool %401 %403
               OpBranch %393
        %393 = OpLabel
        %405 = OpPhi %bool %false %379 %404 %392
               OpStore %_0_ok %405
               OpStore %_6_m %213
        %407 = OpFSub %v2float %209 %214
        %408 = OpFSub %v2float %210 %215
        %409 = OpFSub %v2float %211 %216
        %410 = OpFSub %v2float %212 %217
        %411 = OpCompositeConstruct %mat4v2float %407 %408 %409 %410
               OpStore %_6_m %411
               OpSelectionMerge %413 None
               OpBranchConditional %405 %412 %413
        %412 = OpLabel
        %414 = OpFOrdEqual %v2bool %407 %233
        %415 = OpAll %bool %414
        %416 = OpFOrdEqual %v2bool %408 %234
        %417 = OpAll %bool %416
        %418 = OpLogicalAnd %bool %415 %417
        %419 = OpFOrdEqual %v2bool %409 %235
        %420 = OpAll %bool %419
        %421 = OpLogicalAnd %bool %418 %420
        %422 = OpFOrdEqual %v2bool %410 %236
        %423 = OpAll %bool %422
        %424 = OpLogicalAnd %bool %421 %423
               OpBranch %413
        %413 = OpLabel
        %425 = OpPhi %bool %false %393 %424 %412
               OpStore %_0_ok %425
               OpStore %_7_m %254
        %427 = OpFDiv %v4float %253 %255
        %428 = OpFDiv %v4float %253 %256
        %429 = OpCompositeConstruct %mat2v4float %427 %428
               OpStore %_7_m %429
               OpSelectionMerge %431 None
               OpBranchConditional %425 %430 %431
        %430 = OpLabel
        %432 = OpFOrdEqual %v4bool %427 %263
        %433 = OpAll %bool %432
        %434 = OpFOrdEqual %v4bool %428 %264
        %435 = OpAll %bool %434
        %436 = OpLogicalAnd %bool %433 %435
               OpBranch %431
        %431 = OpLabel
        %437 = OpPhi %bool %false %413 %436 %430
               OpStore %_0_ok %437
               OpStore %_8_m %276
        %439 = OpMatrixTimesMatrix %mat2v3float %276 %280
               OpStore %_8_m %439
               OpSelectionMerge %441 None
               OpBranchConditional %437 %440 %441
        %440 = OpLabel
        %442 = OpCompositeExtract %v3float %439 0
        %443 = OpFOrdEqual %v3bool %442 %289
        %444 = OpAll %bool %443
        %445 = OpCompositeExtract %v3float %439 1
        %446 = OpFOrdEqual %v3bool %445 %290
        %447 = OpAll %bool %446
        %448 = OpLogicalAnd %bool %444 %447
               OpBranch %441
        %441 = OpLabel
        %449 = OpPhi %bool %false %431 %448 %440
               OpStore %_0_ok %449
               OpSelectionMerge %451 None
               OpBranchConditional %449 %450 %451
        %450 = OpLabel
        %452 = OpFunctionCall %bool %test_matrix_op_matrix_half_b
               OpBranch %451
        %451 = OpLabel
        %453 = OpPhi %bool %false %441 %452 %450
               OpSelectionMerge %458 None
               OpBranchConditional %453 %456 %457
        %456 = OpLabel
        %459 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %463 = OpLoad %v4float %459
               OpStore %454 %463
               OpBranch %458
        %457 = OpLabel
        %464 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %466 = OpLoad %v4float %464
               OpStore %454 %466
               OpBranch %458
        %458 = OpLabel
        %467 = OpLoad %v4float %454
               OpReturnValue %467
               OpFunctionEnd
