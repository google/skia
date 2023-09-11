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
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %splat_4 RelaxedPrecision
               OpDecorate %splat_2 RelaxedPrecision
               OpDecorate %m RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %m_0 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %186 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %m_1 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %219 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %230 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %m_2 RelaxedPrecision
               OpDecorate %242 RelaxedPrecision
               OpDecorate %243 RelaxedPrecision
               OpDecorate %244 RelaxedPrecision
               OpDecorate %249 RelaxedPrecision
               OpDecorate %251 RelaxedPrecision
               OpDecorate %m_3 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
               OpDecorate %268 RelaxedPrecision
               OpDecorate %269 RelaxedPrecision
               OpDecorate %271 RelaxedPrecision
               OpDecorate %272 RelaxedPrecision
               OpDecorate %m_4 RelaxedPrecision
               OpDecorate %285 RelaxedPrecision
               OpDecorate %301 RelaxedPrecision
               OpDecorate %302 RelaxedPrecision
               OpDecorate %304 RelaxedPrecision
               OpDecorate %305 RelaxedPrecision
               OpDecorate %308 RelaxedPrecision
               OpDecorate %309 RelaxedPrecision
               OpDecorate %495 RelaxedPrecision
               OpDecorate %498 RelaxedPrecision
               OpDecorate %499 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
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
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
    %float_4 = OpConstant %float 4
         %34 = OpConstantComposite %v3float %float_4 %float_4 %float_4
         %35 = OpConstantComposite %mat3v3float %34 %34 %34
    %float_2 = OpConstant %float 2
         %38 = OpConstantComposite %v3float %float_2 %float_2 %float_2
         %39 = OpConstantComposite %mat3v3float %38 %38 %38
         %41 = OpConstantComposite %v3float %float_2 %float_0 %float_0
         %42 = OpConstantComposite %v3float %float_0 %float_2 %float_0
         %43 = OpConstantComposite %v3float %float_0 %float_0 %float_2
         %44 = OpConstantComposite %mat3v3float %41 %42 %43
      %false = OpConstantFalse %bool
    %float_6 = OpConstant %float 6
         %53 = OpConstantComposite %v3float %float_6 %float_4 %float_4
         %54 = OpConstantComposite %v3float %float_4 %float_6 %float_4
         %55 = OpConstantComposite %v3float %float_4 %float_4 %float_6
         %56 = OpConstantComposite %mat3v3float %53 %54 %55
     %v3bool = OpTypeVector %bool 3
   %float_n2 = OpConstant %float -2
   %float_n4 = OpConstant %float -4
         %75 = OpConstantComposite %v3float %float_n2 %float_n4 %float_n4
         %76 = OpConstantComposite %v3float %float_n4 %float_n2 %float_n4
         %77 = OpConstantComposite %v3float %float_n4 %float_n4 %float_n2
         %78 = OpConstantComposite %mat3v3float %75 %76 %77
  %float_0_5 = OpConstant %float 0.5
         %95 = OpConstantComposite %v3float %float_0_5 %float_0 %float_0
         %96 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
         %97 = OpConstantComposite %v3float %float_0 %float_0 %float_0_5
         %98 = OpConstantComposite %mat3v3float %95 %96 %97
        %129 = OpConstantComposite %v3float %float_2 %float_4 %float_4
        %130 = OpConstantComposite %v3float %float_4 %float_2 %float_4
        %131 = OpConstantComposite %v3float %float_4 %float_4 %float_2
        %132 = OpConstantComposite %mat3v3float %129 %130 %131
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
        %173 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
        %174 = OpConstantComposite %v4float %float_5 %float_6 %float_7 %float_8
        %175 = OpConstantComposite %v4float %float_9 %float_10 %float_11 %float_12
        %176 = OpConstantComposite %v4float %float_13 %float_14 %float_15 %float_16
        %177 = OpConstantComposite %mat4v4float %173 %174 %175 %176
        %178 = OpConstantComposite %v4float %float_16 %float_15 %float_14 %float_13
        %179 = OpConstantComposite %v4float %float_12 %float_11 %float_10 %float_9
        %180 = OpConstantComposite %v4float %float_8 %float_7 %float_6 %float_5
        %181 = OpConstantComposite %v4float %float_4 %float_3 %float_2 %float_1
        %182 = OpConstantComposite %mat4v4float %178 %179 %180 %181
   %float_17 = OpConstant %float 17
        %191 = OpConstantComposite %v4float %float_17 %float_17 %float_17 %float_17
        %192 = OpConstantComposite %mat4v4float %191 %191 %191 %191
     %v4bool = OpTypeVector %bool 4
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
   %float_20 = OpConstant %float 20
   %float_30 = OpConstant %float 30
   %float_40 = OpConstant %float 40
        %212 = OpConstantComposite %v2float %float_10 %float_20
        %213 = OpConstantComposite %v2float %float_30 %float_40
        %214 = OpConstantComposite %mat2v2float %212 %213
        %215 = OpConstantComposite %v2float %float_1 %float_2
        %216 = OpConstantComposite %v2float %float_3 %float_4
        %217 = OpConstantComposite %mat2v2float %215 %216
   %float_18 = OpConstant %float 18
   %float_27 = OpConstant %float 27
   %float_36 = OpConstant %float 36
        %226 = OpConstantComposite %v2float %float_9 %float_18
        %227 = OpConstantComposite %v2float %float_27 %float_36
        %228 = OpConstantComposite %mat2v2float %226 %227
     %v2bool = OpTypeVector %bool 2
        %237 = OpConstantComposite %v2float %float_2 %float_4
        %238 = OpConstantComposite %v2float %float_6 %float_8
        %239 = OpConstantComposite %mat2v2float %237 %238
        %240 = OpConstantComposite %v2float %float_2 %float_2
        %241 = OpConstantComposite %mat2v2float %240 %237
        %247 = OpConstantComposite %v2float %float_3 %float_2
        %248 = OpConstantComposite %mat2v2float %215 %247
        %256 = OpConstantComposite %v2float %float_7 %float_4
        %257 = OpConstantComposite %mat2v2float %215 %256
        %258 = OpConstantComposite %v2float %float_3 %float_5
        %259 = OpConstantComposite %mat2v2float %258 %247
   %float_38 = OpConstant %float 38
   %float_26 = OpConstant %float 26
        %265 = OpConstantComposite %v2float %float_38 %float_26
        %266 = OpConstantComposite %v2float %float_17 %float_14
        %267 = OpConstantComposite %mat2v2float %265 %266
        %277 = OpConstantComposite %v3float %float_10 %float_4 %float_2
        %278 = OpConstantComposite %v3float %float_20 %float_5 %float_3
        %279 = OpConstantComposite %v3float %float_10 %float_6 %float_5
        %280 = OpConstantComposite %mat3v3float %277 %278 %279
        %281 = OpConstantComposite %v3float %float_3 %float_3 %float_4
        %282 = OpConstantComposite %v3float %float_2 %float_3 %float_4
        %283 = OpConstantComposite %v3float %float_4 %float_9 %float_2
        %284 = OpConstantComposite %mat3v3float %281 %282 %283
  %float_130 = OpConstant %float 130
   %float_51 = OpConstant %float 51
   %float_35 = OpConstant %float 35
  %float_120 = OpConstant %float 120
   %float_47 = OpConstant %float 47
   %float_33 = OpConstant %float 33
  %float_240 = OpConstant %float 240
   %float_73 = OpConstant %float 73
   %float_45 = OpConstant %float 45
        %297 = OpConstantComposite %v3float %float_130 %float_51 %float_35
        %298 = OpConstantComposite %v3float %float_120 %float_47 %float_33
        %299 = OpConstantComposite %v3float %float_240 %float_73 %float_45
        %300 = OpConstantComposite %mat3v3float %297 %298 %299
        %313 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %16
         %17 = OpLabel
         %21 = OpVariable %_ptr_Function_v2float Function
               OpStore %21 %20
         %23 = OpFunctionCall %v4float %main %21
               OpStore %sk_FragColor %23
               OpReturn
               OpFunctionEnd
%test_matrix_op_matrix_half_b = OpFunction %bool None %24
         %25 = OpLabel
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
               OpStore %splat_4 %35
               OpStore %splat_2 %39
               OpStore %m %44
         %45 = OpFAdd %v3float %41 %34
         %46 = OpFAdd %v3float %42 %34
         %47 = OpFAdd %v3float %43 %34
         %48 = OpCompositeConstruct %mat3v3float %45 %46 %47
               OpStore %m %48
               OpSelectionMerge %51 None
               OpBranchConditional %true %50 %51
         %50 = OpLabel
         %58 = OpFOrdEqual %v3bool %45 %53
         %59 = OpAll %bool %58
         %60 = OpFOrdEqual %v3bool %46 %54
         %61 = OpAll %bool %60
         %62 = OpLogicalAnd %bool %59 %61
         %63 = OpFOrdEqual %v3bool %47 %55
         %64 = OpAll %bool %63
         %65 = OpLogicalAnd %bool %62 %64
               OpBranch %51
         %51 = OpLabel
         %66 = OpPhi %bool %false %25 %65 %50
               OpStore %ok %66
               OpStore %m %44
         %67 = OpFSub %v3float %41 %34
         %68 = OpFSub %v3float %42 %34
         %69 = OpFSub %v3float %43 %34
         %70 = OpCompositeConstruct %mat3v3float %67 %68 %69
               OpStore %m %70
               OpSelectionMerge %72 None
               OpBranchConditional %66 %71 %72
         %71 = OpLabel
         %79 = OpFOrdEqual %v3bool %67 %75
         %80 = OpAll %bool %79
         %81 = OpFOrdEqual %v3bool %68 %76
         %82 = OpAll %bool %81
         %83 = OpLogicalAnd %bool %80 %82
         %84 = OpFOrdEqual %v3bool %69 %77
         %85 = OpAll %bool %84
         %86 = OpLogicalAnd %bool %83 %85
               OpBranch %72
         %72 = OpLabel
         %87 = OpPhi %bool %false %51 %86 %71
               OpStore %ok %87
               OpStore %m %44
         %88 = OpFDiv %v3float %41 %34
         %89 = OpFDiv %v3float %42 %34
         %90 = OpFDiv %v3float %43 %34
         %91 = OpCompositeConstruct %mat3v3float %88 %89 %90
               OpStore %m %91
               OpSelectionMerge %93 None
               OpBranchConditional %87 %92 %93
         %92 = OpLabel
         %99 = OpFOrdEqual %v3bool %88 %95
        %100 = OpAll %bool %99
        %101 = OpFOrdEqual %v3bool %89 %96
        %102 = OpAll %bool %101
        %103 = OpLogicalAnd %bool %100 %102
        %104 = OpFOrdEqual %v3bool %90 %97
        %105 = OpAll %bool %104
        %106 = OpLogicalAnd %bool %103 %105
               OpBranch %93
         %93 = OpLabel
        %107 = OpPhi %bool %false %72 %106 %92
               OpStore %ok %107
               OpStore %m %35
        %108 = OpFAdd %v3float %34 %41
        %109 = OpFAdd %v3float %34 %42
        %110 = OpFAdd %v3float %34 %43
        %111 = OpCompositeConstruct %mat3v3float %108 %109 %110
               OpStore %m %111
               OpSelectionMerge %113 None
               OpBranchConditional %107 %112 %113
        %112 = OpLabel
        %114 = OpFOrdEqual %v3bool %108 %53
        %115 = OpAll %bool %114
        %116 = OpFOrdEqual %v3bool %109 %54
        %117 = OpAll %bool %116
        %118 = OpLogicalAnd %bool %115 %117
        %119 = OpFOrdEqual %v3bool %110 %55
        %120 = OpAll %bool %119
        %121 = OpLogicalAnd %bool %118 %120
               OpBranch %113
        %113 = OpLabel
        %122 = OpPhi %bool %false %93 %121 %112
               OpStore %ok %122
               OpStore %m %35
        %123 = OpFSub %v3float %34 %41
        %124 = OpFSub %v3float %34 %42
        %125 = OpFSub %v3float %34 %43
        %126 = OpCompositeConstruct %mat3v3float %123 %124 %125
               OpStore %m %126
               OpSelectionMerge %128 None
               OpBranchConditional %122 %127 %128
        %127 = OpLabel
        %133 = OpFOrdEqual %v3bool %123 %129
        %134 = OpAll %bool %133
        %135 = OpFOrdEqual %v3bool %124 %130
        %136 = OpAll %bool %135
        %137 = OpLogicalAnd %bool %134 %136
        %138 = OpFOrdEqual %v3bool %125 %131
        %139 = OpAll %bool %138
        %140 = OpLogicalAnd %bool %137 %139
               OpBranch %128
        %128 = OpLabel
        %141 = OpPhi %bool %false %113 %140 %127
               OpStore %ok %141
               OpStore %m %35
        %142 = OpFDiv %v3float %34 %38
        %143 = OpFDiv %v3float %34 %38
        %144 = OpFDiv %v3float %34 %38
        %145 = OpCompositeConstruct %mat3v3float %142 %143 %144
               OpStore %m %145
               OpSelectionMerge %147 None
               OpBranchConditional %141 %146 %147
        %146 = OpLabel
        %148 = OpFOrdEqual %v3bool %142 %38
        %149 = OpAll %bool %148
        %150 = OpFOrdEqual %v3bool %143 %38
        %151 = OpAll %bool %150
        %152 = OpLogicalAnd %bool %149 %151
        %153 = OpFOrdEqual %v3bool %144 %38
        %154 = OpAll %bool %153
        %155 = OpLogicalAnd %bool %152 %154
               OpBranch %147
        %147 = OpLabel
        %156 = OpPhi %bool %false %128 %155 %146
               OpStore %ok %156
               OpStore %m_0 %177
        %183 = OpFAdd %v4float %173 %178
        %184 = OpFAdd %v4float %174 %179
        %185 = OpFAdd %v4float %175 %180
        %186 = OpFAdd %v4float %176 %181
        %187 = OpCompositeConstruct %mat4v4float %183 %184 %185 %186
               OpStore %m_0 %187
               OpSelectionMerge %189 None
               OpBranchConditional %156 %188 %189
        %188 = OpLabel
        %194 = OpFOrdEqual %v4bool %183 %191
        %195 = OpAll %bool %194
        %196 = OpFOrdEqual %v4bool %184 %191
        %197 = OpAll %bool %196
        %198 = OpLogicalAnd %bool %195 %197
        %199 = OpFOrdEqual %v4bool %185 %191
        %200 = OpAll %bool %199
        %201 = OpLogicalAnd %bool %198 %200
        %202 = OpFOrdEqual %v4bool %186 %191
        %203 = OpAll %bool %202
        %204 = OpLogicalAnd %bool %201 %203
               OpBranch %189
        %189 = OpLabel
        %205 = OpPhi %bool %false %147 %204 %188
               OpStore %ok %205
               OpStore %m_1 %214
        %218 = OpFSub %v2float %212 %215
        %219 = OpFSub %v2float %213 %216
        %220 = OpCompositeConstruct %mat2v2float %218 %219
               OpStore %m_1 %220
               OpSelectionMerge %222 None
               OpBranchConditional %205 %221 %222
        %221 = OpLabel
        %230 = OpFOrdEqual %v2bool %218 %226
        %231 = OpAll %bool %230
        %232 = OpFOrdEqual %v2bool %219 %227
        %233 = OpAll %bool %232
        %234 = OpLogicalAnd %bool %231 %233
               OpBranch %222
        %222 = OpLabel
        %235 = OpPhi %bool %false %189 %234 %221
               OpStore %ok %235
               OpStore %m_2 %239
        %242 = OpFDiv %v2float %237 %240
        %243 = OpFDiv %v2float %238 %237
        %244 = OpCompositeConstruct %mat2v2float %242 %243
               OpStore %m_2 %244
               OpSelectionMerge %246 None
               OpBranchConditional %235 %245 %246
        %245 = OpLabel
        %249 = OpFOrdEqual %v2bool %242 %215
        %250 = OpAll %bool %249
        %251 = OpFOrdEqual %v2bool %243 %247
        %252 = OpAll %bool %251
        %253 = OpLogicalAnd %bool %250 %252
               OpBranch %246
        %246 = OpLabel
        %254 = OpPhi %bool %false %222 %253 %245
               OpStore %ok %254
               OpStore %m_3 %257
        %260 = OpMatrixTimesMatrix %mat2v2float %257 %259
               OpStore %m_3 %260
               OpSelectionMerge %262 None
               OpBranchConditional %254 %261 %262
        %261 = OpLabel
        %268 = OpCompositeExtract %v2float %260 0
        %269 = OpFOrdEqual %v2bool %268 %265
        %270 = OpAll %bool %269
        %271 = OpCompositeExtract %v2float %260 1
        %272 = OpFOrdEqual %v2bool %271 %266
        %273 = OpAll %bool %272
        %274 = OpLogicalAnd %bool %270 %273
               OpBranch %262
        %262 = OpLabel
        %275 = OpPhi %bool %false %246 %274 %261
               OpStore %ok %275
               OpStore %m_4 %280
        %285 = OpMatrixTimesMatrix %mat3v3float %280 %284
               OpStore %m_4 %285
               OpSelectionMerge %287 None
               OpBranchConditional %275 %286 %287
        %286 = OpLabel
        %301 = OpCompositeExtract %v3float %285 0
        %302 = OpFOrdEqual %v3bool %301 %297
        %303 = OpAll %bool %302
        %304 = OpCompositeExtract %v3float %285 1
        %305 = OpFOrdEqual %v3bool %304 %298
        %306 = OpAll %bool %305
        %307 = OpLogicalAnd %bool %303 %306
        %308 = OpCompositeExtract %v3float %285 2
        %309 = OpFOrdEqual %v3bool %308 %299
        %310 = OpAll %bool %309
        %311 = OpLogicalAnd %bool %307 %310
               OpBranch %287
        %287 = OpLabel
        %312 = OpPhi %bool %false %262 %311 %286
               OpStore %ok %312
               OpReturnValue %312
               OpFunctionEnd
       %main = OpFunction %v4float None %313
        %314 = OpFunctionParameter %_ptr_Function_v2float
        %315 = OpLabel
      %_0_ok = OpVariable %_ptr_Function_bool Function
 %_1_splat_4 = OpVariable %_ptr_Function_mat3v3float Function
 %_2_splat_2 = OpVariable %_ptr_Function_mat3v3float Function
       %_3_m = OpVariable %_ptr_Function_mat3v3float Function
       %_4_m = OpVariable %_ptr_Function_mat4v4float Function
       %_5_m = OpVariable %_ptr_Function_mat2v2float Function
       %_6_m = OpVariable %_ptr_Function_mat2v2float Function
       %_7_m = OpVariable %_ptr_Function_mat2v2float Function
       %_8_m = OpVariable %_ptr_Function_mat3v3float Function
        %486 = OpVariable %_ptr_Function_v4float Function
               OpStore %_0_ok %true
               OpStore %_1_splat_4 %35
               OpStore %_2_splat_2 %39
               OpStore %_3_m %44
        %320 = OpFAdd %v3float %41 %34
        %321 = OpFAdd %v3float %42 %34
        %322 = OpFAdd %v3float %43 %34
        %323 = OpCompositeConstruct %mat3v3float %320 %321 %322
               OpStore %_3_m %323
               OpSelectionMerge %325 None
               OpBranchConditional %true %324 %325
        %324 = OpLabel
        %326 = OpFOrdEqual %v3bool %320 %53
        %327 = OpAll %bool %326
        %328 = OpFOrdEqual %v3bool %321 %54
        %329 = OpAll %bool %328
        %330 = OpLogicalAnd %bool %327 %329
        %331 = OpFOrdEqual %v3bool %322 %55
        %332 = OpAll %bool %331
        %333 = OpLogicalAnd %bool %330 %332
               OpBranch %325
        %325 = OpLabel
        %334 = OpPhi %bool %false %315 %333 %324
               OpStore %_0_ok %334
               OpStore %_3_m %44
        %335 = OpFSub %v3float %41 %34
        %336 = OpFSub %v3float %42 %34
        %337 = OpFSub %v3float %43 %34
        %338 = OpCompositeConstruct %mat3v3float %335 %336 %337
               OpStore %_3_m %338
               OpSelectionMerge %340 None
               OpBranchConditional %334 %339 %340
        %339 = OpLabel
        %341 = OpFOrdEqual %v3bool %335 %75
        %342 = OpAll %bool %341
        %343 = OpFOrdEqual %v3bool %336 %76
        %344 = OpAll %bool %343
        %345 = OpLogicalAnd %bool %342 %344
        %346 = OpFOrdEqual %v3bool %337 %77
        %347 = OpAll %bool %346
        %348 = OpLogicalAnd %bool %345 %347
               OpBranch %340
        %340 = OpLabel
        %349 = OpPhi %bool %false %325 %348 %339
               OpStore %_0_ok %349
               OpStore %_3_m %44
        %350 = OpFDiv %v3float %41 %34
        %351 = OpFDiv %v3float %42 %34
        %352 = OpFDiv %v3float %43 %34
        %353 = OpCompositeConstruct %mat3v3float %350 %351 %352
               OpStore %_3_m %353
               OpSelectionMerge %355 None
               OpBranchConditional %349 %354 %355
        %354 = OpLabel
        %356 = OpFOrdEqual %v3bool %350 %95
        %357 = OpAll %bool %356
        %358 = OpFOrdEqual %v3bool %351 %96
        %359 = OpAll %bool %358
        %360 = OpLogicalAnd %bool %357 %359
        %361 = OpFOrdEqual %v3bool %352 %97
        %362 = OpAll %bool %361
        %363 = OpLogicalAnd %bool %360 %362
               OpBranch %355
        %355 = OpLabel
        %364 = OpPhi %bool %false %340 %363 %354
               OpStore %_0_ok %364
               OpStore %_3_m %35
        %365 = OpFAdd %v3float %34 %41
        %366 = OpFAdd %v3float %34 %42
        %367 = OpFAdd %v3float %34 %43
        %368 = OpCompositeConstruct %mat3v3float %365 %366 %367
               OpStore %_3_m %368
               OpSelectionMerge %370 None
               OpBranchConditional %364 %369 %370
        %369 = OpLabel
        %371 = OpFOrdEqual %v3bool %365 %53
        %372 = OpAll %bool %371
        %373 = OpFOrdEqual %v3bool %366 %54
        %374 = OpAll %bool %373
        %375 = OpLogicalAnd %bool %372 %374
        %376 = OpFOrdEqual %v3bool %367 %55
        %377 = OpAll %bool %376
        %378 = OpLogicalAnd %bool %375 %377
               OpBranch %370
        %370 = OpLabel
        %379 = OpPhi %bool %false %355 %378 %369
               OpStore %_0_ok %379
               OpStore %_3_m %35
        %380 = OpFSub %v3float %34 %41
        %381 = OpFSub %v3float %34 %42
        %382 = OpFSub %v3float %34 %43
        %383 = OpCompositeConstruct %mat3v3float %380 %381 %382
               OpStore %_3_m %383
               OpSelectionMerge %385 None
               OpBranchConditional %379 %384 %385
        %384 = OpLabel
        %386 = OpFOrdEqual %v3bool %380 %129
        %387 = OpAll %bool %386
        %388 = OpFOrdEqual %v3bool %381 %130
        %389 = OpAll %bool %388
        %390 = OpLogicalAnd %bool %387 %389
        %391 = OpFOrdEqual %v3bool %382 %131
        %392 = OpAll %bool %391
        %393 = OpLogicalAnd %bool %390 %392
               OpBranch %385
        %385 = OpLabel
        %394 = OpPhi %bool %false %370 %393 %384
               OpStore %_0_ok %394
               OpStore %_3_m %35
        %395 = OpFDiv %v3float %34 %38
        %396 = OpFDiv %v3float %34 %38
        %397 = OpFDiv %v3float %34 %38
        %398 = OpCompositeConstruct %mat3v3float %395 %396 %397
               OpStore %_3_m %398
               OpSelectionMerge %400 None
               OpBranchConditional %394 %399 %400
        %399 = OpLabel
        %401 = OpFOrdEqual %v3bool %395 %38
        %402 = OpAll %bool %401
        %403 = OpFOrdEqual %v3bool %396 %38
        %404 = OpAll %bool %403
        %405 = OpLogicalAnd %bool %402 %404
        %406 = OpFOrdEqual %v3bool %397 %38
        %407 = OpAll %bool %406
        %408 = OpLogicalAnd %bool %405 %407
               OpBranch %400
        %400 = OpLabel
        %409 = OpPhi %bool %false %385 %408 %399
               OpStore %_0_ok %409
               OpStore %_4_m %177
        %411 = OpFAdd %v4float %173 %178
        %412 = OpFAdd %v4float %174 %179
        %413 = OpFAdd %v4float %175 %180
        %414 = OpFAdd %v4float %176 %181
        %415 = OpCompositeConstruct %mat4v4float %411 %412 %413 %414
               OpStore %_4_m %415
               OpSelectionMerge %417 None
               OpBranchConditional %409 %416 %417
        %416 = OpLabel
        %418 = OpFOrdEqual %v4bool %411 %191
        %419 = OpAll %bool %418
        %420 = OpFOrdEqual %v4bool %412 %191
        %421 = OpAll %bool %420
        %422 = OpLogicalAnd %bool %419 %421
        %423 = OpFOrdEqual %v4bool %413 %191
        %424 = OpAll %bool %423
        %425 = OpLogicalAnd %bool %422 %424
        %426 = OpFOrdEqual %v4bool %414 %191
        %427 = OpAll %bool %426
        %428 = OpLogicalAnd %bool %425 %427
               OpBranch %417
        %417 = OpLabel
        %429 = OpPhi %bool %false %400 %428 %416
               OpStore %_0_ok %429
               OpStore %_5_m %214
        %431 = OpFSub %v2float %212 %215
        %432 = OpFSub %v2float %213 %216
        %433 = OpCompositeConstruct %mat2v2float %431 %432
               OpStore %_5_m %433
               OpSelectionMerge %435 None
               OpBranchConditional %429 %434 %435
        %434 = OpLabel
        %436 = OpFOrdEqual %v2bool %431 %226
        %437 = OpAll %bool %436
        %438 = OpFOrdEqual %v2bool %432 %227
        %439 = OpAll %bool %438
        %440 = OpLogicalAnd %bool %437 %439
               OpBranch %435
        %435 = OpLabel
        %441 = OpPhi %bool %false %417 %440 %434
               OpStore %_0_ok %441
               OpStore %_6_m %239
        %443 = OpFDiv %v2float %237 %240
        %444 = OpFDiv %v2float %238 %237
        %445 = OpCompositeConstruct %mat2v2float %443 %444
               OpStore %_6_m %445
               OpSelectionMerge %447 None
               OpBranchConditional %441 %446 %447
        %446 = OpLabel
        %448 = OpFOrdEqual %v2bool %443 %215
        %449 = OpAll %bool %448
        %450 = OpFOrdEqual %v2bool %444 %247
        %451 = OpAll %bool %450
        %452 = OpLogicalAnd %bool %449 %451
               OpBranch %447
        %447 = OpLabel
        %453 = OpPhi %bool %false %435 %452 %446
               OpStore %_0_ok %453
               OpStore %_7_m %257
        %455 = OpMatrixTimesMatrix %mat2v2float %257 %259
               OpStore %_7_m %455
               OpSelectionMerge %457 None
               OpBranchConditional %453 %456 %457
        %456 = OpLabel
        %458 = OpCompositeExtract %v2float %455 0
        %459 = OpFOrdEqual %v2bool %458 %265
        %460 = OpAll %bool %459
        %461 = OpCompositeExtract %v2float %455 1
        %462 = OpFOrdEqual %v2bool %461 %266
        %463 = OpAll %bool %462
        %464 = OpLogicalAnd %bool %460 %463
               OpBranch %457
        %457 = OpLabel
        %465 = OpPhi %bool %false %447 %464 %456
               OpStore %_0_ok %465
               OpStore %_8_m %280
        %467 = OpMatrixTimesMatrix %mat3v3float %280 %284
               OpStore %_8_m %467
               OpSelectionMerge %469 None
               OpBranchConditional %465 %468 %469
        %468 = OpLabel
        %470 = OpCompositeExtract %v3float %467 0
        %471 = OpFOrdEqual %v3bool %470 %297
        %472 = OpAll %bool %471
        %473 = OpCompositeExtract %v3float %467 1
        %474 = OpFOrdEqual %v3bool %473 %298
        %475 = OpAll %bool %474
        %476 = OpLogicalAnd %bool %472 %475
        %477 = OpCompositeExtract %v3float %467 2
        %478 = OpFOrdEqual %v3bool %477 %299
        %479 = OpAll %bool %478
        %480 = OpLogicalAnd %bool %476 %479
               OpBranch %469
        %469 = OpLabel
        %481 = OpPhi %bool %false %457 %480 %468
               OpStore %_0_ok %481
               OpSelectionMerge %483 None
               OpBranchConditional %481 %482 %483
        %482 = OpLabel
        %484 = OpFunctionCall %bool %test_matrix_op_matrix_half_b
               OpBranch %483
        %483 = OpLabel
        %485 = OpPhi %bool %false %469 %484 %482
               OpSelectionMerge %490 None
               OpBranchConditional %485 %488 %489
        %488 = OpLabel
        %491 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %495 = OpLoad %v4float %491
               OpStore %486 %495
               OpBranch %490
        %489 = OpLabel
        %496 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %498 = OpLoad %v4float %496
               OpStore %486 %498
               OpBranch %490
        %490 = OpLabel
        %499 = OpLoad %v4float %486
               OpReturnValue %499
               OpFunctionEnd
