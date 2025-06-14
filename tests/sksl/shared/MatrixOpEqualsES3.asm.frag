               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %8
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "colorRed"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %15
               OpName %test_matrix_op_matrix_half_b "test_matrix_op_matrix_half_b"  ; id %6
               OpName %ok "ok"                                                      ; id %28
               OpName %splat_4 "splat_4"                                            ; id %31
               OpName %m "m"                                                        ; id %37
               OpName %splat_4_0 "splat_4"                                          ; id %103
               OpName %m_0 "m"                                                      ; id %109
               OpName %m_1 "m"                                                      ; id %155
               OpName %m_2 "m"                                                      ; id %203
               OpName %m_3 "m"                                                      ; id %254
               OpName %m_4 "m"                                                      ; id %277
               OpName %main "main"                                                  ; id %7
               OpName %_0_ok "_0_ok"                                                ; id %307
               OpName %_1_splat_4 "_1_splat_4"                                      ; id %308
               OpName %_2_m "_2_m"                                                  ; id %309
               OpName %_3_splat_4 "_3_splat_4"                                      ; id %355
               OpName %_4_m "_4_m"                                                  ; id %356
               OpName %_5_m "_5_m"                                                  ; id %390
               OpName %_6_m "_6_m"                                                  ; id %410
               OpName %_7_m "_7_m"                                                  ; id %430
               OpName %_8_m "_8_m"                                                  ; id %442

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %splat_4 RelaxedPrecision
               OpDecorate %m RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %splat_4_0 RelaxedPrecision
               OpDecorate %m_0 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %m_1 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %m_2 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %224 RelaxedPrecision
               OpDecorate %225 RelaxedPrecision
               OpDecorate %226 RelaxedPrecision
               OpDecorate %227 RelaxedPrecision
               OpDecorate %242 RelaxedPrecision
               OpDecorate %244 RelaxedPrecision
               OpDecorate %247 RelaxedPrecision
               OpDecorate %250 RelaxedPrecision
               OpDecorate %m_3 RelaxedPrecision
               OpDecorate %262 RelaxedPrecision
               OpDecorate %263 RelaxedPrecision
               OpDecorate %264 RelaxedPrecision
               OpDecorate %271 RelaxedPrecision
               OpDecorate %273 RelaxedPrecision
               OpDecorate %m_4 RelaxedPrecision
               OpDecorate %285 RelaxedPrecision
               OpDecorate %296 RelaxedPrecision
               OpDecorate %297 RelaxedPrecision
               OpDecorate %299 RelaxedPrecision
               OpDecorate %300 RelaxedPrecision
               OpDecorate %466 RelaxedPrecision
               OpDecorate %469 RelaxedPrecision
               OpDecorate %470 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %26 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
    %float_4 = OpConstant %float 4
         %35 = OpConstantComposite %v2float %float_4 %float_4
         %36 = OpConstantComposite %mat3v2float %35 %35 %35
    %float_2 = OpConstant %float 2
         %39 = OpConstantComposite %v2float %float_2 %float_0
         %40 = OpConstantComposite %v2float %float_0 %float_2
         %41 = OpConstantComposite %mat3v2float %39 %40 %21
      %false = OpConstantFalse %bool
    %float_6 = OpConstant %float 6
         %50 = OpConstantComposite %v2float %float_6 %float_4
         %51 = OpConstantComposite %v2float %float_4 %float_6
         %52 = OpConstantComposite %mat3v2float %50 %51 %35
     %v2bool = OpTypeVector %bool 2
   %float_n2 = OpConstant %float -2
   %float_n4 = OpConstant %float -4
         %71 = OpConstantComposite %v2float %float_n2 %float_n4
         %72 = OpConstantComposite %v2float %float_n4 %float_n2
         %73 = OpConstantComposite %v2float %float_n4 %float_n4
         %74 = OpConstantComposite %mat3v2float %71 %72 %73
  %float_0_5 = OpConstant %float 0.5
         %91 = OpConstantComposite %v2float %float_0_5 %float_0
         %92 = OpConstantComposite %v2float %float_0 %float_0_5
         %93 = OpConstantComposite %mat3v2float %91 %92 %21
    %v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
        %107 = OpConstantComposite %v3float %float_4 %float_4 %float_4
        %108 = OpConstantComposite %mat2v3float %107 %107
        %110 = OpConstantComposite %v3float %float_2 %float_0 %float_0
        %111 = OpConstantComposite %v3float %float_0 %float_2 %float_0
        %112 = OpConstantComposite %mat2v3float %110 %111
        %118 = OpConstantComposite %v3float %float_6 %float_4 %float_4
        %119 = OpConstantComposite %v3float %float_4 %float_6 %float_4
        %120 = OpConstantComposite %mat2v3float %118 %119
     %v3bool = OpTypeVector %bool 3
        %133 = OpConstantComposite %v3float %float_2 %float_4 %float_4
        %134 = OpConstantComposite %v3float %float_4 %float_2 %float_4
        %135 = OpConstantComposite %mat2v3float %133 %134
        %142 = OpConstantComposite %v3float %float_2 %float_2 %float_2
        %143 = OpConstantComposite %mat2v3float %142 %142
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
        %167 = OpConstantComposite %v3float %float_1 %float_2 %float_3
        %168 = OpConstantComposite %v3float %float_4 %float_5 %float_6
        %169 = OpConstantComposite %v3float %float_7 %float_8 %float_9
        %170 = OpConstantComposite %v3float %float_10 %float_11 %float_12
        %171 = OpConstantComposite %mat4v3float %167 %168 %169 %170
   %float_16 = OpConstant %float 16
   %float_15 = OpConstant %float 15
   %float_14 = OpConstant %float 14
   %float_13 = OpConstant %float 13
        %176 = OpConstantComposite %v3float %float_16 %float_15 %float_14
        %177 = OpConstantComposite %v3float %float_13 %float_12 %float_11
        %178 = OpConstantComposite %v3float %float_10 %float_9 %float_8
        %179 = OpConstantComposite %v3float %float_7 %float_6 %float_5
        %180 = OpConstantComposite %mat4v3float %176 %177 %178 %179
   %float_17 = OpConstant %float 17
        %189 = OpConstantComposite %v3float %float_17 %float_17 %float_17
        %190 = OpConstantComposite %mat4v3float %189 %189 %189 %189
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
   %float_20 = OpConstant %float 20
   %float_30 = OpConstant %float 30
   %float_40 = OpConstant %float 40
   %float_50 = OpConstant %float 50
   %float_60 = OpConstant %float 60
   %float_70 = OpConstant %float 70
   %float_80 = OpConstant %float 80
        %213 = OpConstantComposite %v2float %float_10 %float_20
        %214 = OpConstantComposite %v2float %float_30 %float_40
        %215 = OpConstantComposite %v2float %float_50 %float_60
        %216 = OpConstantComposite %v2float %float_70 %float_80
        %217 = OpConstantComposite %mat4v2float %213 %214 %215 %216
        %218 = OpConstantComposite %v2float %float_1 %float_2
        %219 = OpConstantComposite %v2float %float_3 %float_4
        %220 = OpConstantComposite %v2float %float_5 %float_6
        %221 = OpConstantComposite %v2float %float_7 %float_8
        %222 = OpConstantComposite %mat4v2float %218 %219 %220 %221
   %float_18 = OpConstant %float 18
   %float_27 = OpConstant %float 27
   %float_36 = OpConstant %float 36
   %float_45 = OpConstant %float 45
   %float_54 = OpConstant %float 54
   %float_63 = OpConstant %float 63
   %float_72 = OpConstant %float 72
        %237 = OpConstantComposite %v2float %float_9 %float_18
        %238 = OpConstantComposite %v2float %float_27 %float_36
        %239 = OpConstantComposite %v2float %float_45 %float_54
        %240 = OpConstantComposite %v2float %float_63 %float_72
        %241 = OpConstantComposite %mat4v2float %237 %238 %239 %240
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
        %257 = OpConstantComposite %v4float %float_10 %float_20 %float_30 %float_40
        %258 = OpConstantComposite %mat2v4float %257 %257
        %259 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
        %260 = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
        %261 = OpConstantComposite %mat2v4float %259 %260
        %267 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
        %268 = OpConstantComposite %v4float %float_2 %float_4 %float_6 %float_8
        %269 = OpConstantComposite %mat2v4float %267 %268
     %v4bool = OpTypeVector %bool 4
        %278 = OpConstantComposite %v3float %float_7 %float_9 %float_11
        %279 = OpConstantComposite %v3float %float_8 %float_10 %float_12
        %280 = OpConstantComposite %mat2v3float %278 %279
        %281 = OpConstantComposite %v2float %float_1 %float_4
        %282 = OpConstantComposite %v2float %float_2 %float_5
%mat2v2float = OpTypeMatrix %v2float 2
        %284 = OpConstantComposite %mat2v2float %281 %282
   %float_39 = OpConstant %float 39
   %float_49 = OpConstant %float 49
   %float_59 = OpConstant %float 59
   %float_68 = OpConstant %float 68
   %float_82 = OpConstant %float 82
        %293 = OpConstantComposite %v3float %float_39 %float_49 %float_59
        %294 = OpConstantComposite %v3float %float_54 %float_68 %float_82
        %295 = OpConstantComposite %mat2v3float %293 %294
        %304 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %17

         %18 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function test_matrix_op_matrix_half_b
%test_matrix_op_matrix_half_b = OpFunction %bool None %26

         %27 = OpLabel
         %ok =   OpVariable %_ptr_Function_bool Function
    %splat_4 =   OpVariable %_ptr_Function_mat3v2float Function     ; RelaxedPrecision
          %m =   OpVariable %_ptr_Function_mat3v2float Function     ; RelaxedPrecision
  %splat_4_0 =   OpVariable %_ptr_Function_mat2v3float Function     ; RelaxedPrecision
        %m_0 =   OpVariable %_ptr_Function_mat2v3float Function     ; RelaxedPrecision
        %m_1 =   OpVariable %_ptr_Function_mat4v3float Function     ; RelaxedPrecision
        %m_2 =   OpVariable %_ptr_Function_mat4v2float Function     ; RelaxedPrecision
        %m_3 =   OpVariable %_ptr_Function_mat2v4float Function     ; RelaxedPrecision
        %m_4 =   OpVariable %_ptr_Function_mat2v3float Function     ; RelaxedPrecision
                 OpStore %ok %true
                 OpStore %splat_4 %36
                 OpStore %m %41
         %42 =   OpFAdd %v2float %39 %35            ; RelaxedPrecision
         %43 =   OpFAdd %v2float %40 %35            ; RelaxedPrecision
         %44 =   OpFAdd %v2float %21 %35            ; RelaxedPrecision
         %45 =   OpCompositeConstruct %mat3v2float %42 %43 %44  ; RelaxedPrecision
                 OpStore %m %45
                 OpSelectionMerge %48 None
                 OpBranchConditional %true %47 %48

         %47 =     OpLabel
         %54 =       OpFOrdEqual %v2bool %42 %50    ; RelaxedPrecision
         %55 =       OpAll %bool %54
         %56 =       OpFOrdEqual %v2bool %43 %51    ; RelaxedPrecision
         %57 =       OpAll %bool %56
         %58 =       OpLogicalAnd %bool %55 %57
         %59 =       OpFOrdEqual %v2bool %44 %35    ; RelaxedPrecision
         %60 =       OpAll %bool %59
         %61 =       OpLogicalAnd %bool %58 %60
                     OpBranch %48

         %48 = OpLabel
         %62 =   OpPhi %bool %false %27 %61 %47
                 OpStore %ok %62
                 OpStore %m %41
         %63 =   OpFSub %v2float %39 %35            ; RelaxedPrecision
         %64 =   OpFSub %v2float %40 %35            ; RelaxedPrecision
         %65 =   OpFSub %v2float %21 %35            ; RelaxedPrecision
         %66 =   OpCompositeConstruct %mat3v2float %63 %64 %65  ; RelaxedPrecision
                 OpStore %m %66
                 OpSelectionMerge %68 None
                 OpBranchConditional %62 %67 %68

         %67 =     OpLabel
         %75 =       OpFOrdEqual %v2bool %63 %71    ; RelaxedPrecision
         %76 =       OpAll %bool %75
         %77 =       OpFOrdEqual %v2bool %64 %72    ; RelaxedPrecision
         %78 =       OpAll %bool %77
         %79 =       OpLogicalAnd %bool %76 %78
         %80 =       OpFOrdEqual %v2bool %65 %73    ; RelaxedPrecision
         %81 =       OpAll %bool %80
         %82 =       OpLogicalAnd %bool %79 %81
                     OpBranch %68

         %68 = OpLabel
         %83 =   OpPhi %bool %false %48 %82 %67
                 OpStore %ok %83
                 OpStore %m %41
         %84 =   OpFDiv %v2float %39 %35            ; RelaxedPrecision
         %85 =   OpFDiv %v2float %40 %35            ; RelaxedPrecision
         %86 =   OpFDiv %v2float %21 %35            ; RelaxedPrecision
         %87 =   OpCompositeConstruct %mat3v2float %84 %85 %86  ; RelaxedPrecision
                 OpStore %m %87
                 OpSelectionMerge %89 None
                 OpBranchConditional %83 %88 %89

         %88 =     OpLabel
         %94 =       OpFOrdEqual %v2bool %84 %91    ; RelaxedPrecision
         %95 =       OpAll %bool %94
         %96 =       OpFOrdEqual %v2bool %85 %92    ; RelaxedPrecision
         %97 =       OpAll %bool %96
         %98 =       OpLogicalAnd %bool %95 %97
         %99 =       OpFOrdEqual %v2bool %86 %21    ; RelaxedPrecision
        %100 =       OpAll %bool %99
        %101 =       OpLogicalAnd %bool %98 %100
                     OpBranch %89

         %89 = OpLabel
        %102 =   OpPhi %bool %false %68 %101 %88
                 OpStore %ok %102
                 OpStore %splat_4_0 %108
                 OpStore %m_0 %108
        %113 =   OpFAdd %v3float %107 %110          ; RelaxedPrecision
        %114 =   OpFAdd %v3float %107 %111          ; RelaxedPrecision
        %115 =   OpCompositeConstruct %mat2v3float %113 %114    ; RelaxedPrecision
                 OpStore %m_0 %115
                 OpSelectionMerge %117 None
                 OpBranchConditional %102 %116 %117

        %116 =     OpLabel
        %122 =       OpFOrdEqual %v3bool %113 %118  ; RelaxedPrecision
        %123 =       OpAll %bool %122
        %124 =       OpFOrdEqual %v3bool %114 %119  ; RelaxedPrecision
        %125 =       OpAll %bool %124
        %126 =       OpLogicalAnd %bool %123 %125
                     OpBranch %117

        %117 = OpLabel
        %127 =   OpPhi %bool %false %89 %126 %116
                 OpStore %ok %127
                 OpStore %m_0 %108
        %128 =   OpFSub %v3float %107 %110          ; RelaxedPrecision
        %129 =   OpFSub %v3float %107 %111          ; RelaxedPrecision
        %130 =   OpCompositeConstruct %mat2v3float %128 %129    ; RelaxedPrecision
                 OpStore %m_0 %130
                 OpSelectionMerge %132 None
                 OpBranchConditional %127 %131 %132

        %131 =     OpLabel
        %136 =       OpFOrdEqual %v3bool %128 %133  ; RelaxedPrecision
        %137 =       OpAll %bool %136
        %138 =       OpFOrdEqual %v3bool %129 %134  ; RelaxedPrecision
        %139 =       OpAll %bool %138
        %140 =       OpLogicalAnd %bool %137 %139
                     OpBranch %132

        %132 = OpLabel
        %141 =   OpPhi %bool %false %117 %140 %131
                 OpStore %ok %141
                 OpStore %m_0 %108
        %144 =   OpFDiv %v3float %107 %142          ; RelaxedPrecision
        %145 =   OpFDiv %v3float %107 %142          ; RelaxedPrecision
        %146 =   OpCompositeConstruct %mat2v3float %144 %145    ; RelaxedPrecision
                 OpStore %m_0 %146
                 OpSelectionMerge %148 None
                 OpBranchConditional %141 %147 %148

        %147 =     OpLabel
        %149 =       OpFOrdEqual %v3bool %144 %142  ; RelaxedPrecision
        %150 =       OpAll %bool %149
        %151 =       OpFOrdEqual %v3bool %145 %142  ; RelaxedPrecision
        %152 =       OpAll %bool %151
        %153 =       OpLogicalAnd %bool %150 %152
                     OpBranch %148

        %148 = OpLabel
        %154 =   OpPhi %bool %false %132 %153 %147
                 OpStore %ok %154
                 OpStore %m_1 %171
        %181 =   OpFAdd %v3float %167 %176          ; RelaxedPrecision
        %182 =   OpFAdd %v3float %168 %177          ; RelaxedPrecision
        %183 =   OpFAdd %v3float %169 %178          ; RelaxedPrecision
        %184 =   OpFAdd %v3float %170 %179          ; RelaxedPrecision
        %185 =   OpCompositeConstruct %mat4v3float %181 %182 %183 %184  ; RelaxedPrecision
                 OpStore %m_1 %185
                 OpSelectionMerge %187 None
                 OpBranchConditional %154 %186 %187

        %186 =     OpLabel
        %191 =       OpFOrdEqual %v3bool %181 %189  ; RelaxedPrecision
        %192 =       OpAll %bool %191
        %193 =       OpFOrdEqual %v3bool %182 %189  ; RelaxedPrecision
        %194 =       OpAll %bool %193
        %195 =       OpLogicalAnd %bool %192 %194
        %196 =       OpFOrdEqual %v3bool %183 %189  ; RelaxedPrecision
        %197 =       OpAll %bool %196
        %198 =       OpLogicalAnd %bool %195 %197
        %199 =       OpFOrdEqual %v3bool %184 %189  ; RelaxedPrecision
        %200 =       OpAll %bool %199
        %201 =       OpLogicalAnd %bool %198 %200
                     OpBranch %187

        %187 = OpLabel
        %202 =   OpPhi %bool %false %148 %201 %186
                 OpStore %ok %202
                 OpStore %m_2 %217
        %223 =   OpFSub %v2float %213 %218          ; RelaxedPrecision
        %224 =   OpFSub %v2float %214 %219          ; RelaxedPrecision
        %225 =   OpFSub %v2float %215 %220          ; RelaxedPrecision
        %226 =   OpFSub %v2float %216 %221          ; RelaxedPrecision
        %227 =   OpCompositeConstruct %mat4v2float %223 %224 %225 %226  ; RelaxedPrecision
                 OpStore %m_2 %227
                 OpSelectionMerge %229 None
                 OpBranchConditional %202 %228 %229

        %228 =     OpLabel
        %242 =       OpFOrdEqual %v2bool %223 %237  ; RelaxedPrecision
        %243 =       OpAll %bool %242
        %244 =       OpFOrdEqual %v2bool %224 %238  ; RelaxedPrecision
        %245 =       OpAll %bool %244
        %246 =       OpLogicalAnd %bool %243 %245
        %247 =       OpFOrdEqual %v2bool %225 %239  ; RelaxedPrecision
        %248 =       OpAll %bool %247
        %249 =       OpLogicalAnd %bool %246 %248
        %250 =       OpFOrdEqual %v2bool %226 %240  ; RelaxedPrecision
        %251 =       OpAll %bool %250
        %252 =       OpLogicalAnd %bool %249 %251
                     OpBranch %229

        %229 = OpLabel
        %253 =   OpPhi %bool %false %187 %252 %228
                 OpStore %ok %253
                 OpStore %m_3 %258
        %262 =   OpFDiv %v4float %257 %259          ; RelaxedPrecision
        %263 =   OpFDiv %v4float %257 %260          ; RelaxedPrecision
        %264 =   OpCompositeConstruct %mat2v4float %262 %263    ; RelaxedPrecision
                 OpStore %m_3 %264
                 OpSelectionMerge %266 None
                 OpBranchConditional %253 %265 %266

        %265 =     OpLabel
        %271 =       OpFOrdEqual %v4bool %262 %267  ; RelaxedPrecision
        %272 =       OpAll %bool %271
        %273 =       OpFOrdEqual %v4bool %263 %268  ; RelaxedPrecision
        %274 =       OpAll %bool %273
        %275 =       OpLogicalAnd %bool %272 %274
                     OpBranch %266

        %266 = OpLabel
        %276 =   OpPhi %bool %false %229 %275 %265
                 OpStore %ok %276
                 OpStore %m_4 %280
        %285 =   OpMatrixTimesMatrix %mat2v3float %280 %284     ; RelaxedPrecision
                 OpStore %m_4 %285
                 OpSelectionMerge %287 None
                 OpBranchConditional %276 %286 %287

        %286 =     OpLabel
        %296 =       OpCompositeExtract %v3float %285 0     ; RelaxedPrecision
        %297 =       OpFOrdEqual %v3bool %296 %293          ; RelaxedPrecision
        %298 =       OpAll %bool %297
        %299 =       OpCompositeExtract %v3float %285 1     ; RelaxedPrecision
        %300 =       OpFOrdEqual %v3bool %299 %294          ; RelaxedPrecision
        %301 =       OpAll %bool %300
        %302 =       OpLogicalAnd %bool %298 %301
                     OpBranch %287

        %287 = OpLabel
        %303 =   OpPhi %bool %false %266 %302 %286
                 OpStore %ok %303
                 OpReturnValue %303
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %304        ; RelaxedPrecision
        %305 = OpFunctionParameter %_ptr_Function_v2float

        %306 = OpLabel
      %_0_ok =   OpVariable %_ptr_Function_bool Function
 %_1_splat_4 =   OpVariable %_ptr_Function_mat3v2float Function
       %_2_m =   OpVariable %_ptr_Function_mat3v2float Function
 %_3_splat_4 =   OpVariable %_ptr_Function_mat2v3float Function
       %_4_m =   OpVariable %_ptr_Function_mat2v3float Function
       %_5_m =   OpVariable %_ptr_Function_mat4v3float Function
       %_6_m =   OpVariable %_ptr_Function_mat4v2float Function
       %_7_m =   OpVariable %_ptr_Function_mat2v4float Function
       %_8_m =   OpVariable %_ptr_Function_mat2v3float Function
        %458 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %_0_ok %true
                 OpStore %_1_splat_4 %36
                 OpStore %_2_m %41
        %310 =   OpFAdd %v2float %39 %35
        %311 =   OpFAdd %v2float %40 %35
        %312 =   OpFAdd %v2float %21 %35
        %313 =   OpCompositeConstruct %mat3v2float %310 %311 %312
                 OpStore %_2_m %313
                 OpSelectionMerge %315 None
                 OpBranchConditional %true %314 %315

        %314 =     OpLabel
        %316 =       OpFOrdEqual %v2bool %310 %50
        %317 =       OpAll %bool %316
        %318 =       OpFOrdEqual %v2bool %311 %51
        %319 =       OpAll %bool %318
        %320 =       OpLogicalAnd %bool %317 %319
        %321 =       OpFOrdEqual %v2bool %312 %35
        %322 =       OpAll %bool %321
        %323 =       OpLogicalAnd %bool %320 %322
                     OpBranch %315

        %315 = OpLabel
        %324 =   OpPhi %bool %false %306 %323 %314
                 OpStore %_0_ok %324
                 OpStore %_2_m %41
        %325 =   OpFSub %v2float %39 %35
        %326 =   OpFSub %v2float %40 %35
        %327 =   OpFSub %v2float %21 %35
        %328 =   OpCompositeConstruct %mat3v2float %325 %326 %327
                 OpStore %_2_m %328
                 OpSelectionMerge %330 None
                 OpBranchConditional %324 %329 %330

        %329 =     OpLabel
        %331 =       OpFOrdEqual %v2bool %325 %71
        %332 =       OpAll %bool %331
        %333 =       OpFOrdEqual %v2bool %326 %72
        %334 =       OpAll %bool %333
        %335 =       OpLogicalAnd %bool %332 %334
        %336 =       OpFOrdEqual %v2bool %327 %73
        %337 =       OpAll %bool %336
        %338 =       OpLogicalAnd %bool %335 %337
                     OpBranch %330

        %330 = OpLabel
        %339 =   OpPhi %bool %false %315 %338 %329
                 OpStore %_0_ok %339
                 OpStore %_2_m %41
        %340 =   OpFDiv %v2float %39 %35
        %341 =   OpFDiv %v2float %40 %35
        %342 =   OpFDiv %v2float %21 %35
        %343 =   OpCompositeConstruct %mat3v2float %340 %341 %342
                 OpStore %_2_m %343
                 OpSelectionMerge %345 None
                 OpBranchConditional %339 %344 %345

        %344 =     OpLabel
        %346 =       OpFOrdEqual %v2bool %340 %91
        %347 =       OpAll %bool %346
        %348 =       OpFOrdEqual %v2bool %341 %92
        %349 =       OpAll %bool %348
        %350 =       OpLogicalAnd %bool %347 %349
        %351 =       OpFOrdEqual %v2bool %342 %21
        %352 =       OpAll %bool %351
        %353 =       OpLogicalAnd %bool %350 %352
                     OpBranch %345

        %345 = OpLabel
        %354 =   OpPhi %bool %false %330 %353 %344
                 OpStore %_0_ok %354
                 OpStore %_3_splat_4 %108
                 OpStore %_4_m %108
        %357 =   OpFAdd %v3float %107 %110
        %358 =   OpFAdd %v3float %107 %111
        %359 =   OpCompositeConstruct %mat2v3float %357 %358
                 OpStore %_4_m %359
                 OpSelectionMerge %361 None
                 OpBranchConditional %354 %360 %361

        %360 =     OpLabel
        %362 =       OpFOrdEqual %v3bool %357 %118
        %363 =       OpAll %bool %362
        %364 =       OpFOrdEqual %v3bool %358 %119
        %365 =       OpAll %bool %364
        %366 =       OpLogicalAnd %bool %363 %365
                     OpBranch %361

        %361 = OpLabel
        %367 =   OpPhi %bool %false %345 %366 %360
                 OpStore %_0_ok %367
                 OpStore %_4_m %108
        %368 =   OpFSub %v3float %107 %110
        %369 =   OpFSub %v3float %107 %111
        %370 =   OpCompositeConstruct %mat2v3float %368 %369
                 OpStore %_4_m %370
                 OpSelectionMerge %372 None
                 OpBranchConditional %367 %371 %372

        %371 =     OpLabel
        %373 =       OpFOrdEqual %v3bool %368 %133
        %374 =       OpAll %bool %373
        %375 =       OpFOrdEqual %v3bool %369 %134
        %376 =       OpAll %bool %375
        %377 =       OpLogicalAnd %bool %374 %376
                     OpBranch %372

        %372 = OpLabel
        %378 =   OpPhi %bool %false %361 %377 %371
                 OpStore %_0_ok %378
                 OpStore %_4_m %108
        %379 =   OpFDiv %v3float %107 %142
        %380 =   OpFDiv %v3float %107 %142
        %381 =   OpCompositeConstruct %mat2v3float %379 %380
                 OpStore %_4_m %381
                 OpSelectionMerge %383 None
                 OpBranchConditional %378 %382 %383

        %382 =     OpLabel
        %384 =       OpFOrdEqual %v3bool %379 %142
        %385 =       OpAll %bool %384
        %386 =       OpFOrdEqual %v3bool %380 %142
        %387 =       OpAll %bool %386
        %388 =       OpLogicalAnd %bool %385 %387
                     OpBranch %383

        %383 = OpLabel
        %389 =   OpPhi %bool %false %372 %388 %382
                 OpStore %_0_ok %389
                 OpStore %_5_m %171
        %391 =   OpFAdd %v3float %167 %176
        %392 =   OpFAdd %v3float %168 %177
        %393 =   OpFAdd %v3float %169 %178
        %394 =   OpFAdd %v3float %170 %179
        %395 =   OpCompositeConstruct %mat4v3float %391 %392 %393 %394
                 OpStore %_5_m %395
                 OpSelectionMerge %397 None
                 OpBranchConditional %389 %396 %397

        %396 =     OpLabel
        %398 =       OpFOrdEqual %v3bool %391 %189
        %399 =       OpAll %bool %398
        %400 =       OpFOrdEqual %v3bool %392 %189
        %401 =       OpAll %bool %400
        %402 =       OpLogicalAnd %bool %399 %401
        %403 =       OpFOrdEqual %v3bool %393 %189
        %404 =       OpAll %bool %403
        %405 =       OpLogicalAnd %bool %402 %404
        %406 =       OpFOrdEqual %v3bool %394 %189
        %407 =       OpAll %bool %406
        %408 =       OpLogicalAnd %bool %405 %407
                     OpBranch %397

        %397 = OpLabel
        %409 =   OpPhi %bool %false %383 %408 %396
                 OpStore %_0_ok %409
                 OpStore %_6_m %217
        %411 =   OpFSub %v2float %213 %218
        %412 =   OpFSub %v2float %214 %219
        %413 =   OpFSub %v2float %215 %220
        %414 =   OpFSub %v2float %216 %221
        %415 =   OpCompositeConstruct %mat4v2float %411 %412 %413 %414
                 OpStore %_6_m %415
                 OpSelectionMerge %417 None
                 OpBranchConditional %409 %416 %417

        %416 =     OpLabel
        %418 =       OpFOrdEqual %v2bool %411 %237
        %419 =       OpAll %bool %418
        %420 =       OpFOrdEqual %v2bool %412 %238
        %421 =       OpAll %bool %420
        %422 =       OpLogicalAnd %bool %419 %421
        %423 =       OpFOrdEqual %v2bool %413 %239
        %424 =       OpAll %bool %423
        %425 =       OpLogicalAnd %bool %422 %424
        %426 =       OpFOrdEqual %v2bool %414 %240
        %427 =       OpAll %bool %426
        %428 =       OpLogicalAnd %bool %425 %427
                     OpBranch %417

        %417 = OpLabel
        %429 =   OpPhi %bool %false %397 %428 %416
                 OpStore %_0_ok %429
                 OpStore %_7_m %258
        %431 =   OpFDiv %v4float %257 %259
        %432 =   OpFDiv %v4float %257 %260
        %433 =   OpCompositeConstruct %mat2v4float %431 %432
                 OpStore %_7_m %433
                 OpSelectionMerge %435 None
                 OpBranchConditional %429 %434 %435

        %434 =     OpLabel
        %436 =       OpFOrdEqual %v4bool %431 %267
        %437 =       OpAll %bool %436
        %438 =       OpFOrdEqual %v4bool %432 %268
        %439 =       OpAll %bool %438
        %440 =       OpLogicalAnd %bool %437 %439
                     OpBranch %435

        %435 = OpLabel
        %441 =   OpPhi %bool %false %417 %440 %434
                 OpStore %_0_ok %441
                 OpStore %_8_m %280
        %443 =   OpMatrixTimesMatrix %mat2v3float %280 %284
                 OpStore %_8_m %443
                 OpSelectionMerge %445 None
                 OpBranchConditional %441 %444 %445

        %444 =     OpLabel
        %446 =       OpCompositeExtract %v3float %443 0
        %447 =       OpFOrdEqual %v3bool %446 %293
        %448 =       OpAll %bool %447
        %449 =       OpCompositeExtract %v3float %443 1
        %450 =       OpFOrdEqual %v3bool %449 %294
        %451 =       OpAll %bool %450
        %452 =       OpLogicalAnd %bool %448 %451
                     OpBranch %445

        %445 = OpLabel
        %453 =   OpPhi %bool %false %435 %452 %444
                 OpStore %_0_ok %453
                 OpSelectionMerge %455 None
                 OpBranchConditional %453 %454 %455

        %454 =     OpLabel
        %456 =       OpFunctionCall %bool %test_matrix_op_matrix_half_b
                     OpBranch %455

        %455 = OpLabel
        %457 =   OpPhi %bool %false %445 %456 %454
                 OpSelectionMerge %462 None
                 OpBranchConditional %457 %460 %461

        %460 =     OpLabel
        %463 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %466 =       OpLoad %v4float %463           ; RelaxedPrecision
                     OpStore %458 %466
                     OpBranch %462

        %461 =     OpLabel
        %467 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %469 =       OpLoad %v4float %467           ; RelaxedPrecision
                     OpStore %458 %469
                     OpBranch %462

        %462 = OpLabel
        %470 =   OpLoad %v4float %458               ; RelaxedPrecision
                 OpReturnValue %470
               OpFunctionEnd
