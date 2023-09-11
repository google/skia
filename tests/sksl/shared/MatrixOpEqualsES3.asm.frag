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
               OpDecorate %m RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %splat_4_0 RelaxedPrecision
               OpDecorate %m_0 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %m_1 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %m_2 RelaxedPrecision
               OpDecorate %221 RelaxedPrecision
               OpDecorate %222 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %224 RelaxedPrecision
               OpDecorate %225 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %242 RelaxedPrecision
               OpDecorate %245 RelaxedPrecision
               OpDecorate %248 RelaxedPrecision
               OpDecorate %m_3 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
               OpDecorate %261 RelaxedPrecision
               OpDecorate %262 RelaxedPrecision
               OpDecorate %269 RelaxedPrecision
               OpDecorate %271 RelaxedPrecision
               OpDecorate %m_4 RelaxedPrecision
               OpDecorate %283 RelaxedPrecision
               OpDecorate %294 RelaxedPrecision
               OpDecorate %295 RelaxedPrecision
               OpDecorate %297 RelaxedPrecision
               OpDecorate %298 RelaxedPrecision
               OpDecorate %465 RelaxedPrecision
               OpDecorate %468 RelaxedPrecision
               OpDecorate %469 RelaxedPrecision
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
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
    %float_4 = OpConstant %float 4
         %33 = OpConstantComposite %v2float %float_4 %float_4
         %34 = OpConstantComposite %mat3v2float %33 %33 %33
    %float_2 = OpConstant %float 2
         %37 = OpConstantComposite %v2float %float_2 %float_0
         %38 = OpConstantComposite %v2float %float_0 %float_2
         %39 = OpConstantComposite %mat3v2float %37 %38 %20
      %false = OpConstantFalse %bool
    %float_6 = OpConstant %float 6
         %48 = OpConstantComposite %v2float %float_6 %float_4
         %49 = OpConstantComposite %v2float %float_4 %float_6
         %50 = OpConstantComposite %mat3v2float %48 %49 %33
     %v2bool = OpTypeVector %bool 2
   %float_n2 = OpConstant %float -2
   %float_n4 = OpConstant %float -4
         %69 = OpConstantComposite %v2float %float_n2 %float_n4
         %70 = OpConstantComposite %v2float %float_n4 %float_n2
         %71 = OpConstantComposite %v2float %float_n4 %float_n4
         %72 = OpConstantComposite %mat3v2float %69 %70 %71
  %float_0_5 = OpConstant %float 0.5
         %89 = OpConstantComposite %v2float %float_0_5 %float_0
         %90 = OpConstantComposite %v2float %float_0 %float_0_5
         %91 = OpConstantComposite %mat3v2float %89 %90 %20
    %v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
        %105 = OpConstantComposite %v3float %float_4 %float_4 %float_4
        %106 = OpConstantComposite %mat2v3float %105 %105
        %108 = OpConstantComposite %v3float %float_2 %float_0 %float_0
        %109 = OpConstantComposite %v3float %float_0 %float_2 %float_0
        %110 = OpConstantComposite %mat2v3float %108 %109
        %116 = OpConstantComposite %v3float %float_6 %float_4 %float_4
        %117 = OpConstantComposite %v3float %float_4 %float_6 %float_4
        %118 = OpConstantComposite %mat2v3float %116 %117
     %v3bool = OpTypeVector %bool 3
        %131 = OpConstantComposite %v3float %float_2 %float_4 %float_4
        %132 = OpConstantComposite %v3float %float_4 %float_2 %float_4
        %133 = OpConstantComposite %mat2v3float %131 %132
        %140 = OpConstantComposite %v3float %float_2 %float_2 %float_2
        %141 = OpConstantComposite %mat2v3float %140 %140
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
        %165 = OpConstantComposite %v3float %float_1 %float_2 %float_3
        %166 = OpConstantComposite %v3float %float_4 %float_5 %float_6
        %167 = OpConstantComposite %v3float %float_7 %float_8 %float_9
        %168 = OpConstantComposite %v3float %float_10 %float_11 %float_12
        %169 = OpConstantComposite %mat4v3float %165 %166 %167 %168
   %float_16 = OpConstant %float 16
   %float_15 = OpConstant %float 15
   %float_14 = OpConstant %float 14
   %float_13 = OpConstant %float 13
        %174 = OpConstantComposite %v3float %float_16 %float_15 %float_14
        %175 = OpConstantComposite %v3float %float_13 %float_12 %float_11
        %176 = OpConstantComposite %v3float %float_10 %float_9 %float_8
        %177 = OpConstantComposite %v3float %float_7 %float_6 %float_5
        %178 = OpConstantComposite %mat4v3float %174 %175 %176 %177
   %float_17 = OpConstant %float 17
        %187 = OpConstantComposite %v3float %float_17 %float_17 %float_17
        %188 = OpConstantComposite %mat4v3float %187 %187 %187 %187
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
   %float_20 = OpConstant %float 20
   %float_30 = OpConstant %float 30
   %float_40 = OpConstant %float 40
   %float_50 = OpConstant %float 50
   %float_60 = OpConstant %float 60
   %float_70 = OpConstant %float 70
   %float_80 = OpConstant %float 80
        %211 = OpConstantComposite %v2float %float_10 %float_20
        %212 = OpConstantComposite %v2float %float_30 %float_40
        %213 = OpConstantComposite %v2float %float_50 %float_60
        %214 = OpConstantComposite %v2float %float_70 %float_80
        %215 = OpConstantComposite %mat4v2float %211 %212 %213 %214
        %216 = OpConstantComposite %v2float %float_1 %float_2
        %217 = OpConstantComposite %v2float %float_3 %float_4
        %218 = OpConstantComposite %v2float %float_5 %float_6
        %219 = OpConstantComposite %v2float %float_7 %float_8
        %220 = OpConstantComposite %mat4v2float %216 %217 %218 %219
   %float_18 = OpConstant %float 18
   %float_27 = OpConstant %float 27
   %float_36 = OpConstant %float 36
   %float_45 = OpConstant %float 45
   %float_54 = OpConstant %float 54
   %float_63 = OpConstant %float 63
   %float_72 = OpConstant %float 72
        %235 = OpConstantComposite %v2float %float_9 %float_18
        %236 = OpConstantComposite %v2float %float_27 %float_36
        %237 = OpConstantComposite %v2float %float_45 %float_54
        %238 = OpConstantComposite %v2float %float_63 %float_72
        %239 = OpConstantComposite %mat4v2float %235 %236 %237 %238
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
        %255 = OpConstantComposite %v4float %float_10 %float_20 %float_30 %float_40
        %256 = OpConstantComposite %mat2v4float %255 %255
        %257 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
        %258 = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
        %259 = OpConstantComposite %mat2v4float %257 %258
        %265 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
        %266 = OpConstantComposite %v4float %float_2 %float_4 %float_6 %float_8
        %267 = OpConstantComposite %mat2v4float %265 %266
     %v4bool = OpTypeVector %bool 4
        %276 = OpConstantComposite %v3float %float_7 %float_9 %float_11
        %277 = OpConstantComposite %v3float %float_8 %float_10 %float_12
        %278 = OpConstantComposite %mat2v3float %276 %277
        %279 = OpConstantComposite %v2float %float_1 %float_4
        %280 = OpConstantComposite %v2float %float_2 %float_5
%mat2v2float = OpTypeMatrix %v2float 2
        %282 = OpConstantComposite %mat2v2float %279 %280
   %float_39 = OpConstant %float 39
   %float_49 = OpConstant %float 49
   %float_59 = OpConstant %float 59
   %float_68 = OpConstant %float 68
   %float_82 = OpConstant %float 82
        %291 = OpConstantComposite %v3float %float_39 %float_49 %float_59
        %292 = OpConstantComposite %v3float %float_54 %float_68 %float_82
        %293 = OpConstantComposite %mat2v3float %291 %292
        %302 = OpTypeFunction %v4float %_ptr_Function_v2float
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
    %splat_4 = OpVariable %_ptr_Function_mat3v2float Function
          %m = OpVariable %_ptr_Function_mat3v2float Function
  %splat_4_0 = OpVariable %_ptr_Function_mat2v3float Function
        %m_0 = OpVariable %_ptr_Function_mat2v3float Function
        %m_1 = OpVariable %_ptr_Function_mat4v3float Function
        %m_2 = OpVariable %_ptr_Function_mat4v2float Function
        %m_3 = OpVariable %_ptr_Function_mat2v4float Function
        %m_4 = OpVariable %_ptr_Function_mat2v3float Function
               OpStore %ok %true
               OpStore %splat_4 %34
               OpStore %m %39
         %40 = OpFAdd %v2float %37 %33
         %41 = OpFAdd %v2float %38 %33
         %42 = OpFAdd %v2float %20 %33
         %43 = OpCompositeConstruct %mat3v2float %40 %41 %42
               OpStore %m %43
               OpSelectionMerge %46 None
               OpBranchConditional %true %45 %46
         %45 = OpLabel
         %52 = OpFOrdEqual %v2bool %40 %48
         %53 = OpAll %bool %52
         %54 = OpFOrdEqual %v2bool %41 %49
         %55 = OpAll %bool %54
         %56 = OpLogicalAnd %bool %53 %55
         %57 = OpFOrdEqual %v2bool %42 %33
         %58 = OpAll %bool %57
         %59 = OpLogicalAnd %bool %56 %58
               OpBranch %46
         %46 = OpLabel
         %60 = OpPhi %bool %false %25 %59 %45
               OpStore %ok %60
               OpStore %m %39
         %61 = OpFSub %v2float %37 %33
         %62 = OpFSub %v2float %38 %33
         %63 = OpFSub %v2float %20 %33
         %64 = OpCompositeConstruct %mat3v2float %61 %62 %63
               OpStore %m %64
               OpSelectionMerge %66 None
               OpBranchConditional %60 %65 %66
         %65 = OpLabel
         %73 = OpFOrdEqual %v2bool %61 %69
         %74 = OpAll %bool %73
         %75 = OpFOrdEqual %v2bool %62 %70
         %76 = OpAll %bool %75
         %77 = OpLogicalAnd %bool %74 %76
         %78 = OpFOrdEqual %v2bool %63 %71
         %79 = OpAll %bool %78
         %80 = OpLogicalAnd %bool %77 %79
               OpBranch %66
         %66 = OpLabel
         %81 = OpPhi %bool %false %46 %80 %65
               OpStore %ok %81
               OpStore %m %39
         %82 = OpFDiv %v2float %37 %33
         %83 = OpFDiv %v2float %38 %33
         %84 = OpFDiv %v2float %20 %33
         %85 = OpCompositeConstruct %mat3v2float %82 %83 %84
               OpStore %m %85
               OpSelectionMerge %87 None
               OpBranchConditional %81 %86 %87
         %86 = OpLabel
         %92 = OpFOrdEqual %v2bool %82 %89
         %93 = OpAll %bool %92
         %94 = OpFOrdEqual %v2bool %83 %90
         %95 = OpAll %bool %94
         %96 = OpLogicalAnd %bool %93 %95
         %97 = OpFOrdEqual %v2bool %84 %20
         %98 = OpAll %bool %97
         %99 = OpLogicalAnd %bool %96 %98
               OpBranch %87
         %87 = OpLabel
        %100 = OpPhi %bool %false %66 %99 %86
               OpStore %ok %100
               OpStore %splat_4_0 %106
               OpStore %m_0 %106
        %111 = OpFAdd %v3float %105 %108
        %112 = OpFAdd %v3float %105 %109
        %113 = OpCompositeConstruct %mat2v3float %111 %112
               OpStore %m_0 %113
               OpSelectionMerge %115 None
               OpBranchConditional %100 %114 %115
        %114 = OpLabel
        %120 = OpFOrdEqual %v3bool %111 %116
        %121 = OpAll %bool %120
        %122 = OpFOrdEqual %v3bool %112 %117
        %123 = OpAll %bool %122
        %124 = OpLogicalAnd %bool %121 %123
               OpBranch %115
        %115 = OpLabel
        %125 = OpPhi %bool %false %87 %124 %114
               OpStore %ok %125
               OpStore %m_0 %106
        %126 = OpFSub %v3float %105 %108
        %127 = OpFSub %v3float %105 %109
        %128 = OpCompositeConstruct %mat2v3float %126 %127
               OpStore %m_0 %128
               OpSelectionMerge %130 None
               OpBranchConditional %125 %129 %130
        %129 = OpLabel
        %134 = OpFOrdEqual %v3bool %126 %131
        %135 = OpAll %bool %134
        %136 = OpFOrdEqual %v3bool %127 %132
        %137 = OpAll %bool %136
        %138 = OpLogicalAnd %bool %135 %137
               OpBranch %130
        %130 = OpLabel
        %139 = OpPhi %bool %false %115 %138 %129
               OpStore %ok %139
               OpStore %m_0 %106
        %142 = OpFDiv %v3float %105 %140
        %143 = OpFDiv %v3float %105 %140
        %144 = OpCompositeConstruct %mat2v3float %142 %143
               OpStore %m_0 %144
               OpSelectionMerge %146 None
               OpBranchConditional %139 %145 %146
        %145 = OpLabel
        %147 = OpFOrdEqual %v3bool %142 %140
        %148 = OpAll %bool %147
        %149 = OpFOrdEqual %v3bool %143 %140
        %150 = OpAll %bool %149
        %151 = OpLogicalAnd %bool %148 %150
               OpBranch %146
        %146 = OpLabel
        %152 = OpPhi %bool %false %130 %151 %145
               OpStore %ok %152
               OpStore %m_1 %169
        %179 = OpFAdd %v3float %165 %174
        %180 = OpFAdd %v3float %166 %175
        %181 = OpFAdd %v3float %167 %176
        %182 = OpFAdd %v3float %168 %177
        %183 = OpCompositeConstruct %mat4v3float %179 %180 %181 %182
               OpStore %m_1 %183
               OpSelectionMerge %185 None
               OpBranchConditional %152 %184 %185
        %184 = OpLabel
        %189 = OpFOrdEqual %v3bool %179 %187
        %190 = OpAll %bool %189
        %191 = OpFOrdEqual %v3bool %180 %187
        %192 = OpAll %bool %191
        %193 = OpLogicalAnd %bool %190 %192
        %194 = OpFOrdEqual %v3bool %181 %187
        %195 = OpAll %bool %194
        %196 = OpLogicalAnd %bool %193 %195
        %197 = OpFOrdEqual %v3bool %182 %187
        %198 = OpAll %bool %197
        %199 = OpLogicalAnd %bool %196 %198
               OpBranch %185
        %185 = OpLabel
        %200 = OpPhi %bool %false %146 %199 %184
               OpStore %ok %200
               OpStore %m_2 %215
        %221 = OpFSub %v2float %211 %216
        %222 = OpFSub %v2float %212 %217
        %223 = OpFSub %v2float %213 %218
        %224 = OpFSub %v2float %214 %219
        %225 = OpCompositeConstruct %mat4v2float %221 %222 %223 %224
               OpStore %m_2 %225
               OpSelectionMerge %227 None
               OpBranchConditional %200 %226 %227
        %226 = OpLabel
        %240 = OpFOrdEqual %v2bool %221 %235
        %241 = OpAll %bool %240
        %242 = OpFOrdEqual %v2bool %222 %236
        %243 = OpAll %bool %242
        %244 = OpLogicalAnd %bool %241 %243
        %245 = OpFOrdEqual %v2bool %223 %237
        %246 = OpAll %bool %245
        %247 = OpLogicalAnd %bool %244 %246
        %248 = OpFOrdEqual %v2bool %224 %238
        %249 = OpAll %bool %248
        %250 = OpLogicalAnd %bool %247 %249
               OpBranch %227
        %227 = OpLabel
        %251 = OpPhi %bool %false %185 %250 %226
               OpStore %ok %251
               OpStore %m_3 %256
        %260 = OpFDiv %v4float %255 %257
        %261 = OpFDiv %v4float %255 %258
        %262 = OpCompositeConstruct %mat2v4float %260 %261
               OpStore %m_3 %262
               OpSelectionMerge %264 None
               OpBranchConditional %251 %263 %264
        %263 = OpLabel
        %269 = OpFOrdEqual %v4bool %260 %265
        %270 = OpAll %bool %269
        %271 = OpFOrdEqual %v4bool %261 %266
        %272 = OpAll %bool %271
        %273 = OpLogicalAnd %bool %270 %272
               OpBranch %264
        %264 = OpLabel
        %274 = OpPhi %bool %false %227 %273 %263
               OpStore %ok %274
               OpStore %m_4 %278
        %283 = OpMatrixTimesMatrix %mat2v3float %278 %282
               OpStore %m_4 %283
               OpSelectionMerge %285 None
               OpBranchConditional %274 %284 %285
        %284 = OpLabel
        %294 = OpCompositeExtract %v3float %283 0
        %295 = OpFOrdEqual %v3bool %294 %291
        %296 = OpAll %bool %295
        %297 = OpCompositeExtract %v3float %283 1
        %298 = OpFOrdEqual %v3bool %297 %292
        %299 = OpAll %bool %298
        %300 = OpLogicalAnd %bool %296 %299
               OpBranch %285
        %285 = OpLabel
        %301 = OpPhi %bool %false %264 %300 %284
               OpStore %ok %301
               OpReturnValue %301
               OpFunctionEnd
       %main = OpFunction %v4float None %302
        %303 = OpFunctionParameter %_ptr_Function_v2float
        %304 = OpLabel
      %_0_ok = OpVariable %_ptr_Function_bool Function
 %_1_splat_4 = OpVariable %_ptr_Function_mat3v2float Function
       %_2_m = OpVariable %_ptr_Function_mat3v2float Function
 %_3_splat_4 = OpVariable %_ptr_Function_mat2v3float Function
       %_4_m = OpVariable %_ptr_Function_mat2v3float Function
       %_5_m = OpVariable %_ptr_Function_mat4v3float Function
       %_6_m = OpVariable %_ptr_Function_mat4v2float Function
       %_7_m = OpVariable %_ptr_Function_mat2v4float Function
       %_8_m = OpVariable %_ptr_Function_mat2v3float Function
        %456 = OpVariable %_ptr_Function_v4float Function
               OpStore %_0_ok %true
               OpStore %_1_splat_4 %34
               OpStore %_2_m %39
        %308 = OpFAdd %v2float %37 %33
        %309 = OpFAdd %v2float %38 %33
        %310 = OpFAdd %v2float %20 %33
        %311 = OpCompositeConstruct %mat3v2float %308 %309 %310
               OpStore %_2_m %311
               OpSelectionMerge %313 None
               OpBranchConditional %true %312 %313
        %312 = OpLabel
        %314 = OpFOrdEqual %v2bool %308 %48
        %315 = OpAll %bool %314
        %316 = OpFOrdEqual %v2bool %309 %49
        %317 = OpAll %bool %316
        %318 = OpLogicalAnd %bool %315 %317
        %319 = OpFOrdEqual %v2bool %310 %33
        %320 = OpAll %bool %319
        %321 = OpLogicalAnd %bool %318 %320
               OpBranch %313
        %313 = OpLabel
        %322 = OpPhi %bool %false %304 %321 %312
               OpStore %_0_ok %322
               OpStore %_2_m %39
        %323 = OpFSub %v2float %37 %33
        %324 = OpFSub %v2float %38 %33
        %325 = OpFSub %v2float %20 %33
        %326 = OpCompositeConstruct %mat3v2float %323 %324 %325
               OpStore %_2_m %326
               OpSelectionMerge %328 None
               OpBranchConditional %322 %327 %328
        %327 = OpLabel
        %329 = OpFOrdEqual %v2bool %323 %69
        %330 = OpAll %bool %329
        %331 = OpFOrdEqual %v2bool %324 %70
        %332 = OpAll %bool %331
        %333 = OpLogicalAnd %bool %330 %332
        %334 = OpFOrdEqual %v2bool %325 %71
        %335 = OpAll %bool %334
        %336 = OpLogicalAnd %bool %333 %335
               OpBranch %328
        %328 = OpLabel
        %337 = OpPhi %bool %false %313 %336 %327
               OpStore %_0_ok %337
               OpStore %_2_m %39
        %338 = OpFDiv %v2float %37 %33
        %339 = OpFDiv %v2float %38 %33
        %340 = OpFDiv %v2float %20 %33
        %341 = OpCompositeConstruct %mat3v2float %338 %339 %340
               OpStore %_2_m %341
               OpSelectionMerge %343 None
               OpBranchConditional %337 %342 %343
        %342 = OpLabel
        %344 = OpFOrdEqual %v2bool %338 %89
        %345 = OpAll %bool %344
        %346 = OpFOrdEqual %v2bool %339 %90
        %347 = OpAll %bool %346
        %348 = OpLogicalAnd %bool %345 %347
        %349 = OpFOrdEqual %v2bool %340 %20
        %350 = OpAll %bool %349
        %351 = OpLogicalAnd %bool %348 %350
               OpBranch %343
        %343 = OpLabel
        %352 = OpPhi %bool %false %328 %351 %342
               OpStore %_0_ok %352
               OpStore %_3_splat_4 %106
               OpStore %_4_m %106
        %355 = OpFAdd %v3float %105 %108
        %356 = OpFAdd %v3float %105 %109
        %357 = OpCompositeConstruct %mat2v3float %355 %356
               OpStore %_4_m %357
               OpSelectionMerge %359 None
               OpBranchConditional %352 %358 %359
        %358 = OpLabel
        %360 = OpFOrdEqual %v3bool %355 %116
        %361 = OpAll %bool %360
        %362 = OpFOrdEqual %v3bool %356 %117
        %363 = OpAll %bool %362
        %364 = OpLogicalAnd %bool %361 %363
               OpBranch %359
        %359 = OpLabel
        %365 = OpPhi %bool %false %343 %364 %358
               OpStore %_0_ok %365
               OpStore %_4_m %106
        %366 = OpFSub %v3float %105 %108
        %367 = OpFSub %v3float %105 %109
        %368 = OpCompositeConstruct %mat2v3float %366 %367
               OpStore %_4_m %368
               OpSelectionMerge %370 None
               OpBranchConditional %365 %369 %370
        %369 = OpLabel
        %371 = OpFOrdEqual %v3bool %366 %131
        %372 = OpAll %bool %371
        %373 = OpFOrdEqual %v3bool %367 %132
        %374 = OpAll %bool %373
        %375 = OpLogicalAnd %bool %372 %374
               OpBranch %370
        %370 = OpLabel
        %376 = OpPhi %bool %false %359 %375 %369
               OpStore %_0_ok %376
               OpStore %_4_m %106
        %377 = OpFDiv %v3float %105 %140
        %378 = OpFDiv %v3float %105 %140
        %379 = OpCompositeConstruct %mat2v3float %377 %378
               OpStore %_4_m %379
               OpSelectionMerge %381 None
               OpBranchConditional %376 %380 %381
        %380 = OpLabel
        %382 = OpFOrdEqual %v3bool %377 %140
        %383 = OpAll %bool %382
        %384 = OpFOrdEqual %v3bool %378 %140
        %385 = OpAll %bool %384
        %386 = OpLogicalAnd %bool %383 %385
               OpBranch %381
        %381 = OpLabel
        %387 = OpPhi %bool %false %370 %386 %380
               OpStore %_0_ok %387
               OpStore %_5_m %169
        %389 = OpFAdd %v3float %165 %174
        %390 = OpFAdd %v3float %166 %175
        %391 = OpFAdd %v3float %167 %176
        %392 = OpFAdd %v3float %168 %177
        %393 = OpCompositeConstruct %mat4v3float %389 %390 %391 %392
               OpStore %_5_m %393
               OpSelectionMerge %395 None
               OpBranchConditional %387 %394 %395
        %394 = OpLabel
        %396 = OpFOrdEqual %v3bool %389 %187
        %397 = OpAll %bool %396
        %398 = OpFOrdEqual %v3bool %390 %187
        %399 = OpAll %bool %398
        %400 = OpLogicalAnd %bool %397 %399
        %401 = OpFOrdEqual %v3bool %391 %187
        %402 = OpAll %bool %401
        %403 = OpLogicalAnd %bool %400 %402
        %404 = OpFOrdEqual %v3bool %392 %187
        %405 = OpAll %bool %404
        %406 = OpLogicalAnd %bool %403 %405
               OpBranch %395
        %395 = OpLabel
        %407 = OpPhi %bool %false %381 %406 %394
               OpStore %_0_ok %407
               OpStore %_6_m %215
        %409 = OpFSub %v2float %211 %216
        %410 = OpFSub %v2float %212 %217
        %411 = OpFSub %v2float %213 %218
        %412 = OpFSub %v2float %214 %219
        %413 = OpCompositeConstruct %mat4v2float %409 %410 %411 %412
               OpStore %_6_m %413
               OpSelectionMerge %415 None
               OpBranchConditional %407 %414 %415
        %414 = OpLabel
        %416 = OpFOrdEqual %v2bool %409 %235
        %417 = OpAll %bool %416
        %418 = OpFOrdEqual %v2bool %410 %236
        %419 = OpAll %bool %418
        %420 = OpLogicalAnd %bool %417 %419
        %421 = OpFOrdEqual %v2bool %411 %237
        %422 = OpAll %bool %421
        %423 = OpLogicalAnd %bool %420 %422
        %424 = OpFOrdEqual %v2bool %412 %238
        %425 = OpAll %bool %424
        %426 = OpLogicalAnd %bool %423 %425
               OpBranch %415
        %415 = OpLabel
        %427 = OpPhi %bool %false %395 %426 %414
               OpStore %_0_ok %427
               OpStore %_7_m %256
        %429 = OpFDiv %v4float %255 %257
        %430 = OpFDiv %v4float %255 %258
        %431 = OpCompositeConstruct %mat2v4float %429 %430
               OpStore %_7_m %431
               OpSelectionMerge %433 None
               OpBranchConditional %427 %432 %433
        %432 = OpLabel
        %434 = OpFOrdEqual %v4bool %429 %265
        %435 = OpAll %bool %434
        %436 = OpFOrdEqual %v4bool %430 %266
        %437 = OpAll %bool %436
        %438 = OpLogicalAnd %bool %435 %437
               OpBranch %433
        %433 = OpLabel
        %439 = OpPhi %bool %false %415 %438 %432
               OpStore %_0_ok %439
               OpStore %_8_m %278
        %441 = OpMatrixTimesMatrix %mat2v3float %278 %282
               OpStore %_8_m %441
               OpSelectionMerge %443 None
               OpBranchConditional %439 %442 %443
        %442 = OpLabel
        %444 = OpCompositeExtract %v3float %441 0
        %445 = OpFOrdEqual %v3bool %444 %291
        %446 = OpAll %bool %445
        %447 = OpCompositeExtract %v3float %441 1
        %448 = OpFOrdEqual %v3bool %447 %292
        %449 = OpAll %bool %448
        %450 = OpLogicalAnd %bool %446 %449
               OpBranch %443
        %443 = OpLabel
        %451 = OpPhi %bool %false %433 %450 %442
               OpStore %_0_ok %451
               OpSelectionMerge %453 None
               OpBranchConditional %451 %452 %453
        %452 = OpLabel
        %454 = OpFunctionCall %bool %test_matrix_op_matrix_half_b
               OpBranch %453
        %453 = OpLabel
        %455 = OpPhi %bool %false %443 %454 %452
               OpSelectionMerge %460 None
               OpBranchConditional %455 %458 %459
        %458 = OpLabel
        %461 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %465 = OpLoad %v4float %461
               OpStore %456 %465
               OpBranch %460
        %459 = OpLabel
        %466 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %468 = OpLoad %v4float %466
               OpStore %456 %468
               OpBranch %460
        %460 = OpLabel
        %469 = OpLoad %v4float %456
               OpReturnValue %469
               OpFunctionEnd
