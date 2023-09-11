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
               OpName %splat_2 "splat_2"
               OpName %m "m"
               OpName %m_0 "m"
               OpName %m_1 "m"
               OpName %m_2 "m"
               OpName %m_3 "m"
               OpName %m_4 "m"
               OpName %main "main"
               OpName %_0_ok "_0_ok"
               OpName %_1_splat_4 "_1_splat_4"
               OpName %_2_splat_2 "_2_splat_2"
               OpName %_3_m "_3_m"
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
               OpDecorate %splat_2 RelaxedPrecision
               OpDecorate %m RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %m_0 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %m_1 RelaxedPrecision
               OpDecorate %216 RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %228 RelaxedPrecision
               OpDecorate %230 RelaxedPrecision
               OpDecorate %m_2 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %241 RelaxedPrecision
               OpDecorate %242 RelaxedPrecision
               OpDecorate %247 RelaxedPrecision
               OpDecorate %249 RelaxedPrecision
               OpDecorate %m_3 RelaxedPrecision
               OpDecorate %258 RelaxedPrecision
               OpDecorate %266 RelaxedPrecision
               OpDecorate %267 RelaxedPrecision
               OpDecorate %269 RelaxedPrecision
               OpDecorate %270 RelaxedPrecision
               OpDecorate %m_4 RelaxedPrecision
               OpDecorate %283 RelaxedPrecision
               OpDecorate %299 RelaxedPrecision
               OpDecorate %300 RelaxedPrecision
               OpDecorate %302 RelaxedPrecision
               OpDecorate %303 RelaxedPrecision
               OpDecorate %306 RelaxedPrecision
               OpDecorate %307 RelaxedPrecision
               OpDecorate %493 RelaxedPrecision
               OpDecorate %496 RelaxedPrecision
               OpDecorate %497 RelaxedPrecision
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
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
    %float_4 = OpConstant %float 4
         %32 = OpConstantComposite %v3float %float_4 %float_4 %float_4
         %33 = OpConstantComposite %mat3v3float %32 %32 %32
    %float_2 = OpConstant %float 2
         %36 = OpConstantComposite %v3float %float_2 %float_2 %float_2
         %37 = OpConstantComposite %mat3v3float %36 %36 %36
         %39 = OpConstantComposite %v3float %float_2 %float_0 %float_0
         %40 = OpConstantComposite %v3float %float_0 %float_2 %float_0
         %41 = OpConstantComposite %v3float %float_0 %float_0 %float_2
         %42 = OpConstantComposite %mat3v3float %39 %40 %41
      %false = OpConstantFalse %bool
    %float_6 = OpConstant %float 6
         %51 = OpConstantComposite %v3float %float_6 %float_4 %float_4
         %52 = OpConstantComposite %v3float %float_4 %float_6 %float_4
         %53 = OpConstantComposite %v3float %float_4 %float_4 %float_6
         %54 = OpConstantComposite %mat3v3float %51 %52 %53
     %v3bool = OpTypeVector %bool 3
   %float_n2 = OpConstant %float -2
   %float_n4 = OpConstant %float -4
         %73 = OpConstantComposite %v3float %float_n2 %float_n4 %float_n4
         %74 = OpConstantComposite %v3float %float_n4 %float_n2 %float_n4
         %75 = OpConstantComposite %v3float %float_n4 %float_n4 %float_n2
         %76 = OpConstantComposite %mat3v3float %73 %74 %75
  %float_0_5 = OpConstant %float 0.5
         %93 = OpConstantComposite %v3float %float_0_5 %float_0 %float_0
         %94 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
         %95 = OpConstantComposite %v3float %float_0 %float_0 %float_0_5
         %96 = OpConstantComposite %mat3v3float %93 %94 %95
        %127 = OpConstantComposite %v3float %float_2 %float_4 %float_4
        %128 = OpConstantComposite %v3float %float_4 %float_2 %float_4
        %129 = OpConstantComposite %v3float %float_4 %float_4 %float_2
        %130 = OpConstantComposite %mat3v3float %127 %128 %129
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
    %float_1 = OpConstant %float 1
    %float_3 = OpConstant %float 3
    %float_5 = OpConstant %float 5
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
    %float_9 = OpConstant %float 9
   %float_10 = OpConstant %float 10
   %float_11 = OpConstant %float 11
   %float_12 = OpConstant %float 12
   %float_13 = OpConstant %float 13
   %float_14 = OpConstant %float 14
   %float_15 = OpConstant %float 15
   %float_16 = OpConstant %float 16
        %171 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
        %172 = OpConstantComposite %v4float %float_5 %float_6 %float_7 %float_8
        %173 = OpConstantComposite %v4float %float_9 %float_10 %float_11 %float_12
        %174 = OpConstantComposite %v4float %float_13 %float_14 %float_15 %float_16
        %175 = OpConstantComposite %mat4v4float %171 %172 %173 %174
        %176 = OpConstantComposite %v4float %float_16 %float_15 %float_14 %float_13
        %177 = OpConstantComposite %v4float %float_12 %float_11 %float_10 %float_9
        %178 = OpConstantComposite %v4float %float_8 %float_7 %float_6 %float_5
        %179 = OpConstantComposite %v4float %float_4 %float_3 %float_2 %float_1
        %180 = OpConstantComposite %mat4v4float %176 %177 %178 %179
   %float_17 = OpConstant %float 17
        %189 = OpConstantComposite %v4float %float_17 %float_17 %float_17 %float_17
        %190 = OpConstantComposite %mat4v4float %189 %189 %189 %189
     %v4bool = OpTypeVector %bool 4
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
   %float_20 = OpConstant %float 20
   %float_30 = OpConstant %float 30
   %float_40 = OpConstant %float 40
        %210 = OpConstantComposite %v2float %float_10 %float_20
        %211 = OpConstantComposite %v2float %float_30 %float_40
        %212 = OpConstantComposite %mat2v2float %210 %211
        %213 = OpConstantComposite %v2float %float_1 %float_2
        %214 = OpConstantComposite %v2float %float_3 %float_4
        %215 = OpConstantComposite %mat2v2float %213 %214
   %float_18 = OpConstant %float 18
   %float_27 = OpConstant %float 27
   %float_36 = OpConstant %float 36
        %224 = OpConstantComposite %v2float %float_9 %float_18
        %225 = OpConstantComposite %v2float %float_27 %float_36
        %226 = OpConstantComposite %mat2v2float %224 %225
     %v2bool = OpTypeVector %bool 2
        %235 = OpConstantComposite %v2float %float_2 %float_4
        %236 = OpConstantComposite %v2float %float_6 %float_8
        %237 = OpConstantComposite %mat2v2float %235 %236
        %238 = OpConstantComposite %v2float %float_2 %float_2
        %239 = OpConstantComposite %mat2v2float %238 %235
        %245 = OpConstantComposite %v2float %float_3 %float_2
        %246 = OpConstantComposite %mat2v2float %213 %245
        %254 = OpConstantComposite %v2float %float_7 %float_4
        %255 = OpConstantComposite %mat2v2float %213 %254
        %256 = OpConstantComposite %v2float %float_3 %float_5
        %257 = OpConstantComposite %mat2v2float %256 %245
   %float_38 = OpConstant %float 38
   %float_26 = OpConstant %float 26
        %263 = OpConstantComposite %v2float %float_38 %float_26
        %264 = OpConstantComposite %v2float %float_17 %float_14
        %265 = OpConstantComposite %mat2v2float %263 %264
        %275 = OpConstantComposite %v3float %float_10 %float_4 %float_2
        %276 = OpConstantComposite %v3float %float_20 %float_5 %float_3
        %277 = OpConstantComposite %v3float %float_10 %float_6 %float_5
        %278 = OpConstantComposite %mat3v3float %275 %276 %277
        %279 = OpConstantComposite %v3float %float_3 %float_3 %float_4
        %280 = OpConstantComposite %v3float %float_2 %float_3 %float_4
        %281 = OpConstantComposite %v3float %float_4 %float_9 %float_2
        %282 = OpConstantComposite %mat3v3float %279 %280 %281
  %float_130 = OpConstant %float 130
   %float_51 = OpConstant %float 51
   %float_35 = OpConstant %float 35
  %float_120 = OpConstant %float 120
   %float_47 = OpConstant %float 47
   %float_33 = OpConstant %float 33
  %float_240 = OpConstant %float 240
   %float_73 = OpConstant %float 73
   %float_45 = OpConstant %float 45
        %295 = OpConstantComposite %v3float %float_130 %float_51 %float_35
        %296 = OpConstantComposite %v3float %float_120 %float_47 %float_33
        %297 = OpConstantComposite %v3float %float_240 %float_73 %float_45
        %298 = OpConstantComposite %mat3v3float %295 %296 %297
        %311 = OpTypeFunction %v4float %_ptr_Function_v2float
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
    %splat_4 = OpVariable %_ptr_Function_mat3v3float Function
    %splat_2 = OpVariable %_ptr_Function_mat3v3float Function
          %m = OpVariable %_ptr_Function_mat3v3float Function
        %m_0 = OpVariable %_ptr_Function_mat4v4float Function
        %m_1 = OpVariable %_ptr_Function_mat2v2float Function
        %m_2 = OpVariable %_ptr_Function_mat2v2float Function
        %m_3 = OpVariable %_ptr_Function_mat2v2float Function
        %m_4 = OpVariable %_ptr_Function_mat3v3float Function
               OpStore %ok %true
               OpStore %splat_4 %33
               OpStore %splat_2 %37
               OpStore %m %42
         %43 = OpFAdd %v3float %39 %32
         %44 = OpFAdd %v3float %40 %32
         %45 = OpFAdd %v3float %41 %32
         %46 = OpCompositeConstruct %mat3v3float %43 %44 %45
               OpStore %m %46
               OpSelectionMerge %49 None
               OpBranchConditional %true %48 %49
         %48 = OpLabel
         %56 = OpFOrdEqual %v3bool %43 %51
         %57 = OpAll %bool %56
         %58 = OpFOrdEqual %v3bool %44 %52
         %59 = OpAll %bool %58
         %60 = OpLogicalAnd %bool %57 %59
         %61 = OpFOrdEqual %v3bool %45 %53
         %62 = OpAll %bool %61
         %63 = OpLogicalAnd %bool %60 %62
               OpBranch %49
         %49 = OpLabel
         %64 = OpPhi %bool %false %23 %63 %48
               OpStore %ok %64
               OpStore %m %42
         %65 = OpFSub %v3float %39 %32
         %66 = OpFSub %v3float %40 %32
         %67 = OpFSub %v3float %41 %32
         %68 = OpCompositeConstruct %mat3v3float %65 %66 %67
               OpStore %m %68
               OpSelectionMerge %70 None
               OpBranchConditional %64 %69 %70
         %69 = OpLabel
         %77 = OpFOrdEqual %v3bool %65 %73
         %78 = OpAll %bool %77
         %79 = OpFOrdEqual %v3bool %66 %74
         %80 = OpAll %bool %79
         %81 = OpLogicalAnd %bool %78 %80
         %82 = OpFOrdEqual %v3bool %67 %75
         %83 = OpAll %bool %82
         %84 = OpLogicalAnd %bool %81 %83
               OpBranch %70
         %70 = OpLabel
         %85 = OpPhi %bool %false %49 %84 %69
               OpStore %ok %85
               OpStore %m %42
         %86 = OpFDiv %v3float %39 %32
         %87 = OpFDiv %v3float %40 %32
         %88 = OpFDiv %v3float %41 %32
         %89 = OpCompositeConstruct %mat3v3float %86 %87 %88
               OpStore %m %89
               OpSelectionMerge %91 None
               OpBranchConditional %85 %90 %91
         %90 = OpLabel
         %97 = OpFOrdEqual %v3bool %86 %93
         %98 = OpAll %bool %97
         %99 = OpFOrdEqual %v3bool %87 %94
        %100 = OpAll %bool %99
        %101 = OpLogicalAnd %bool %98 %100
        %102 = OpFOrdEqual %v3bool %88 %95
        %103 = OpAll %bool %102
        %104 = OpLogicalAnd %bool %101 %103
               OpBranch %91
         %91 = OpLabel
        %105 = OpPhi %bool %false %70 %104 %90
               OpStore %ok %105
               OpStore %m %33
        %106 = OpFAdd %v3float %32 %39
        %107 = OpFAdd %v3float %32 %40
        %108 = OpFAdd %v3float %32 %41
        %109 = OpCompositeConstruct %mat3v3float %106 %107 %108
               OpStore %m %109
               OpSelectionMerge %111 None
               OpBranchConditional %105 %110 %111
        %110 = OpLabel
        %112 = OpFOrdEqual %v3bool %106 %51
        %113 = OpAll %bool %112
        %114 = OpFOrdEqual %v3bool %107 %52
        %115 = OpAll %bool %114
        %116 = OpLogicalAnd %bool %113 %115
        %117 = OpFOrdEqual %v3bool %108 %53
        %118 = OpAll %bool %117
        %119 = OpLogicalAnd %bool %116 %118
               OpBranch %111
        %111 = OpLabel
        %120 = OpPhi %bool %false %91 %119 %110
               OpStore %ok %120
               OpStore %m %33
        %121 = OpFSub %v3float %32 %39
        %122 = OpFSub %v3float %32 %40
        %123 = OpFSub %v3float %32 %41
        %124 = OpCompositeConstruct %mat3v3float %121 %122 %123
               OpStore %m %124
               OpSelectionMerge %126 None
               OpBranchConditional %120 %125 %126
        %125 = OpLabel
        %131 = OpFOrdEqual %v3bool %121 %127
        %132 = OpAll %bool %131
        %133 = OpFOrdEqual %v3bool %122 %128
        %134 = OpAll %bool %133
        %135 = OpLogicalAnd %bool %132 %134
        %136 = OpFOrdEqual %v3bool %123 %129
        %137 = OpAll %bool %136
        %138 = OpLogicalAnd %bool %135 %137
               OpBranch %126
        %126 = OpLabel
        %139 = OpPhi %bool %false %111 %138 %125
               OpStore %ok %139
               OpStore %m %33
        %140 = OpFDiv %v3float %32 %36
        %141 = OpFDiv %v3float %32 %36
        %142 = OpFDiv %v3float %32 %36
        %143 = OpCompositeConstruct %mat3v3float %140 %141 %142
               OpStore %m %143
               OpSelectionMerge %145 None
               OpBranchConditional %139 %144 %145
        %144 = OpLabel
        %146 = OpFOrdEqual %v3bool %140 %36
        %147 = OpAll %bool %146
        %148 = OpFOrdEqual %v3bool %141 %36
        %149 = OpAll %bool %148
        %150 = OpLogicalAnd %bool %147 %149
        %151 = OpFOrdEqual %v3bool %142 %36
        %152 = OpAll %bool %151
        %153 = OpLogicalAnd %bool %150 %152
               OpBranch %145
        %145 = OpLabel
        %154 = OpPhi %bool %false %126 %153 %144
               OpStore %ok %154
               OpStore %m_0 %175
        %181 = OpFAdd %v4float %171 %176
        %182 = OpFAdd %v4float %172 %177
        %183 = OpFAdd %v4float %173 %178
        %184 = OpFAdd %v4float %174 %179
        %185 = OpCompositeConstruct %mat4v4float %181 %182 %183 %184
               OpStore %m_0 %185
               OpSelectionMerge %187 None
               OpBranchConditional %154 %186 %187
        %186 = OpLabel
        %192 = OpFOrdEqual %v4bool %181 %189
        %193 = OpAll %bool %192
        %194 = OpFOrdEqual %v4bool %182 %189
        %195 = OpAll %bool %194
        %196 = OpLogicalAnd %bool %193 %195
        %197 = OpFOrdEqual %v4bool %183 %189
        %198 = OpAll %bool %197
        %199 = OpLogicalAnd %bool %196 %198
        %200 = OpFOrdEqual %v4bool %184 %189
        %201 = OpAll %bool %200
        %202 = OpLogicalAnd %bool %199 %201
               OpBranch %187
        %187 = OpLabel
        %203 = OpPhi %bool %false %145 %202 %186
               OpStore %ok %203
               OpStore %m_1 %212
        %216 = OpFSub %v2float %210 %213
        %217 = OpFSub %v2float %211 %214
        %218 = OpCompositeConstruct %mat2v2float %216 %217
               OpStore %m_1 %218
               OpSelectionMerge %220 None
               OpBranchConditional %203 %219 %220
        %219 = OpLabel
        %228 = OpFOrdEqual %v2bool %216 %224
        %229 = OpAll %bool %228
        %230 = OpFOrdEqual %v2bool %217 %225
        %231 = OpAll %bool %230
        %232 = OpLogicalAnd %bool %229 %231
               OpBranch %220
        %220 = OpLabel
        %233 = OpPhi %bool %false %187 %232 %219
               OpStore %ok %233
               OpStore %m_2 %237
        %240 = OpFDiv %v2float %235 %238
        %241 = OpFDiv %v2float %236 %235
        %242 = OpCompositeConstruct %mat2v2float %240 %241
               OpStore %m_2 %242
               OpSelectionMerge %244 None
               OpBranchConditional %233 %243 %244
        %243 = OpLabel
        %247 = OpFOrdEqual %v2bool %240 %213
        %248 = OpAll %bool %247
        %249 = OpFOrdEqual %v2bool %241 %245
        %250 = OpAll %bool %249
        %251 = OpLogicalAnd %bool %248 %250
               OpBranch %244
        %244 = OpLabel
        %252 = OpPhi %bool %false %220 %251 %243
               OpStore %ok %252
               OpStore %m_3 %255
        %258 = OpMatrixTimesMatrix %mat2v2float %255 %257
               OpStore %m_3 %258
               OpSelectionMerge %260 None
               OpBranchConditional %252 %259 %260
        %259 = OpLabel
        %266 = OpCompositeExtract %v2float %258 0
        %267 = OpFOrdEqual %v2bool %266 %263
        %268 = OpAll %bool %267
        %269 = OpCompositeExtract %v2float %258 1
        %270 = OpFOrdEqual %v2bool %269 %264
        %271 = OpAll %bool %270
        %272 = OpLogicalAnd %bool %268 %271
               OpBranch %260
        %260 = OpLabel
        %273 = OpPhi %bool %false %244 %272 %259
               OpStore %ok %273
               OpStore %m_4 %278
        %283 = OpMatrixTimesMatrix %mat3v3float %278 %282
               OpStore %m_4 %283
               OpSelectionMerge %285 None
               OpBranchConditional %273 %284 %285
        %284 = OpLabel
        %299 = OpCompositeExtract %v3float %283 0
        %300 = OpFOrdEqual %v3bool %299 %295
        %301 = OpAll %bool %300
        %302 = OpCompositeExtract %v3float %283 1
        %303 = OpFOrdEqual %v3bool %302 %296
        %304 = OpAll %bool %303
        %305 = OpLogicalAnd %bool %301 %304
        %306 = OpCompositeExtract %v3float %283 2
        %307 = OpFOrdEqual %v3bool %306 %297
        %308 = OpAll %bool %307
        %309 = OpLogicalAnd %bool %305 %308
               OpBranch %285
        %285 = OpLabel
        %310 = OpPhi %bool %false %260 %309 %284
               OpStore %ok %310
               OpReturnValue %310
               OpFunctionEnd
       %main = OpFunction %v4float None %311
        %312 = OpFunctionParameter %_ptr_Function_v2float
        %313 = OpLabel
      %_0_ok = OpVariable %_ptr_Function_bool Function
 %_1_splat_4 = OpVariable %_ptr_Function_mat3v3float Function
 %_2_splat_2 = OpVariable %_ptr_Function_mat3v3float Function
       %_3_m = OpVariable %_ptr_Function_mat3v3float Function
       %_4_m = OpVariable %_ptr_Function_mat4v4float Function
       %_5_m = OpVariable %_ptr_Function_mat2v2float Function
       %_6_m = OpVariable %_ptr_Function_mat2v2float Function
       %_7_m = OpVariable %_ptr_Function_mat2v2float Function
       %_8_m = OpVariable %_ptr_Function_mat3v3float Function
        %484 = OpVariable %_ptr_Function_v4float Function
               OpStore %_0_ok %true
               OpStore %_1_splat_4 %33
               OpStore %_2_splat_2 %37
               OpStore %_3_m %42
        %318 = OpFAdd %v3float %39 %32
        %319 = OpFAdd %v3float %40 %32
        %320 = OpFAdd %v3float %41 %32
        %321 = OpCompositeConstruct %mat3v3float %318 %319 %320
               OpStore %_3_m %321
               OpSelectionMerge %323 None
               OpBranchConditional %true %322 %323
        %322 = OpLabel
        %324 = OpFOrdEqual %v3bool %318 %51
        %325 = OpAll %bool %324
        %326 = OpFOrdEqual %v3bool %319 %52
        %327 = OpAll %bool %326
        %328 = OpLogicalAnd %bool %325 %327
        %329 = OpFOrdEqual %v3bool %320 %53
        %330 = OpAll %bool %329
        %331 = OpLogicalAnd %bool %328 %330
               OpBranch %323
        %323 = OpLabel
        %332 = OpPhi %bool %false %313 %331 %322
               OpStore %_0_ok %332
               OpStore %_3_m %42
        %333 = OpFSub %v3float %39 %32
        %334 = OpFSub %v3float %40 %32
        %335 = OpFSub %v3float %41 %32
        %336 = OpCompositeConstruct %mat3v3float %333 %334 %335
               OpStore %_3_m %336
               OpSelectionMerge %338 None
               OpBranchConditional %332 %337 %338
        %337 = OpLabel
        %339 = OpFOrdEqual %v3bool %333 %73
        %340 = OpAll %bool %339
        %341 = OpFOrdEqual %v3bool %334 %74
        %342 = OpAll %bool %341
        %343 = OpLogicalAnd %bool %340 %342
        %344 = OpFOrdEqual %v3bool %335 %75
        %345 = OpAll %bool %344
        %346 = OpLogicalAnd %bool %343 %345
               OpBranch %338
        %338 = OpLabel
        %347 = OpPhi %bool %false %323 %346 %337
               OpStore %_0_ok %347
               OpStore %_3_m %42
        %348 = OpFDiv %v3float %39 %32
        %349 = OpFDiv %v3float %40 %32
        %350 = OpFDiv %v3float %41 %32
        %351 = OpCompositeConstruct %mat3v3float %348 %349 %350
               OpStore %_3_m %351
               OpSelectionMerge %353 None
               OpBranchConditional %347 %352 %353
        %352 = OpLabel
        %354 = OpFOrdEqual %v3bool %348 %93
        %355 = OpAll %bool %354
        %356 = OpFOrdEqual %v3bool %349 %94
        %357 = OpAll %bool %356
        %358 = OpLogicalAnd %bool %355 %357
        %359 = OpFOrdEqual %v3bool %350 %95
        %360 = OpAll %bool %359
        %361 = OpLogicalAnd %bool %358 %360
               OpBranch %353
        %353 = OpLabel
        %362 = OpPhi %bool %false %338 %361 %352
               OpStore %_0_ok %362
               OpStore %_3_m %33
        %363 = OpFAdd %v3float %32 %39
        %364 = OpFAdd %v3float %32 %40
        %365 = OpFAdd %v3float %32 %41
        %366 = OpCompositeConstruct %mat3v3float %363 %364 %365
               OpStore %_3_m %366
               OpSelectionMerge %368 None
               OpBranchConditional %362 %367 %368
        %367 = OpLabel
        %369 = OpFOrdEqual %v3bool %363 %51
        %370 = OpAll %bool %369
        %371 = OpFOrdEqual %v3bool %364 %52
        %372 = OpAll %bool %371
        %373 = OpLogicalAnd %bool %370 %372
        %374 = OpFOrdEqual %v3bool %365 %53
        %375 = OpAll %bool %374
        %376 = OpLogicalAnd %bool %373 %375
               OpBranch %368
        %368 = OpLabel
        %377 = OpPhi %bool %false %353 %376 %367
               OpStore %_0_ok %377
               OpStore %_3_m %33
        %378 = OpFSub %v3float %32 %39
        %379 = OpFSub %v3float %32 %40
        %380 = OpFSub %v3float %32 %41
        %381 = OpCompositeConstruct %mat3v3float %378 %379 %380
               OpStore %_3_m %381
               OpSelectionMerge %383 None
               OpBranchConditional %377 %382 %383
        %382 = OpLabel
        %384 = OpFOrdEqual %v3bool %378 %127
        %385 = OpAll %bool %384
        %386 = OpFOrdEqual %v3bool %379 %128
        %387 = OpAll %bool %386
        %388 = OpLogicalAnd %bool %385 %387
        %389 = OpFOrdEqual %v3bool %380 %129
        %390 = OpAll %bool %389
        %391 = OpLogicalAnd %bool %388 %390
               OpBranch %383
        %383 = OpLabel
        %392 = OpPhi %bool %false %368 %391 %382
               OpStore %_0_ok %392
               OpStore %_3_m %33
        %393 = OpFDiv %v3float %32 %36
        %394 = OpFDiv %v3float %32 %36
        %395 = OpFDiv %v3float %32 %36
        %396 = OpCompositeConstruct %mat3v3float %393 %394 %395
               OpStore %_3_m %396
               OpSelectionMerge %398 None
               OpBranchConditional %392 %397 %398
        %397 = OpLabel
        %399 = OpFOrdEqual %v3bool %393 %36
        %400 = OpAll %bool %399
        %401 = OpFOrdEqual %v3bool %394 %36
        %402 = OpAll %bool %401
        %403 = OpLogicalAnd %bool %400 %402
        %404 = OpFOrdEqual %v3bool %395 %36
        %405 = OpAll %bool %404
        %406 = OpLogicalAnd %bool %403 %405
               OpBranch %398
        %398 = OpLabel
        %407 = OpPhi %bool %false %383 %406 %397
               OpStore %_0_ok %407
               OpStore %_4_m %175
        %409 = OpFAdd %v4float %171 %176
        %410 = OpFAdd %v4float %172 %177
        %411 = OpFAdd %v4float %173 %178
        %412 = OpFAdd %v4float %174 %179
        %413 = OpCompositeConstruct %mat4v4float %409 %410 %411 %412
               OpStore %_4_m %413
               OpSelectionMerge %415 None
               OpBranchConditional %407 %414 %415
        %414 = OpLabel
        %416 = OpFOrdEqual %v4bool %409 %189
        %417 = OpAll %bool %416
        %418 = OpFOrdEqual %v4bool %410 %189
        %419 = OpAll %bool %418
        %420 = OpLogicalAnd %bool %417 %419
        %421 = OpFOrdEqual %v4bool %411 %189
        %422 = OpAll %bool %421
        %423 = OpLogicalAnd %bool %420 %422
        %424 = OpFOrdEqual %v4bool %412 %189
        %425 = OpAll %bool %424
        %426 = OpLogicalAnd %bool %423 %425
               OpBranch %415
        %415 = OpLabel
        %427 = OpPhi %bool %false %398 %426 %414
               OpStore %_0_ok %427
               OpStore %_5_m %212
        %429 = OpFSub %v2float %210 %213
        %430 = OpFSub %v2float %211 %214
        %431 = OpCompositeConstruct %mat2v2float %429 %430
               OpStore %_5_m %431
               OpSelectionMerge %433 None
               OpBranchConditional %427 %432 %433
        %432 = OpLabel
        %434 = OpFOrdEqual %v2bool %429 %224
        %435 = OpAll %bool %434
        %436 = OpFOrdEqual %v2bool %430 %225
        %437 = OpAll %bool %436
        %438 = OpLogicalAnd %bool %435 %437
               OpBranch %433
        %433 = OpLabel
        %439 = OpPhi %bool %false %415 %438 %432
               OpStore %_0_ok %439
               OpStore %_6_m %237
        %441 = OpFDiv %v2float %235 %238
        %442 = OpFDiv %v2float %236 %235
        %443 = OpCompositeConstruct %mat2v2float %441 %442
               OpStore %_6_m %443
               OpSelectionMerge %445 None
               OpBranchConditional %439 %444 %445
        %444 = OpLabel
        %446 = OpFOrdEqual %v2bool %441 %213
        %447 = OpAll %bool %446
        %448 = OpFOrdEqual %v2bool %442 %245
        %449 = OpAll %bool %448
        %450 = OpLogicalAnd %bool %447 %449
               OpBranch %445
        %445 = OpLabel
        %451 = OpPhi %bool %false %433 %450 %444
               OpStore %_0_ok %451
               OpStore %_7_m %255
        %453 = OpMatrixTimesMatrix %mat2v2float %255 %257
               OpStore %_7_m %453
               OpSelectionMerge %455 None
               OpBranchConditional %451 %454 %455
        %454 = OpLabel
        %456 = OpCompositeExtract %v2float %453 0
        %457 = OpFOrdEqual %v2bool %456 %263
        %458 = OpAll %bool %457
        %459 = OpCompositeExtract %v2float %453 1
        %460 = OpFOrdEqual %v2bool %459 %264
        %461 = OpAll %bool %460
        %462 = OpLogicalAnd %bool %458 %461
               OpBranch %455
        %455 = OpLabel
        %463 = OpPhi %bool %false %445 %462 %454
               OpStore %_0_ok %463
               OpStore %_8_m %278
        %465 = OpMatrixTimesMatrix %mat3v3float %278 %282
               OpStore %_8_m %465
               OpSelectionMerge %467 None
               OpBranchConditional %463 %466 %467
        %466 = OpLabel
        %468 = OpCompositeExtract %v3float %465 0
        %469 = OpFOrdEqual %v3bool %468 %295
        %470 = OpAll %bool %469
        %471 = OpCompositeExtract %v3float %465 1
        %472 = OpFOrdEqual %v3bool %471 %296
        %473 = OpAll %bool %472
        %474 = OpLogicalAnd %bool %470 %473
        %475 = OpCompositeExtract %v3float %465 2
        %476 = OpFOrdEqual %v3bool %475 %297
        %477 = OpAll %bool %476
        %478 = OpLogicalAnd %bool %474 %477
               OpBranch %467
        %467 = OpLabel
        %479 = OpPhi %bool %false %455 %478 %466
               OpStore %_0_ok %479
               OpSelectionMerge %481 None
               OpBranchConditional %479 %480 %481
        %480 = OpLabel
        %482 = OpFunctionCall %bool %test_matrix_op_matrix_half_b
               OpBranch %481
        %481 = OpLabel
        %483 = OpPhi %bool %false %467 %482 %480
               OpSelectionMerge %488 None
               OpBranchConditional %483 %486 %487
        %486 = OpLabel
        %489 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %493 = OpLoad %v4float %489
               OpStore %484 %493
               OpBranch %488
        %487 = OpLabel
        %494 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %496 = OpLoad %v4float %494
               OpStore %484 %496
               OpBranch %488
        %488 = OpLabel
        %497 = OpLoad %v4float %484
               OpReturnValue %497
               OpFunctionEnd
