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
               OpName %splat_2 "splat_2"                                            ; id %38
               OpName %m "m"                                                        ; id %42
               OpName %m_0 "m"                                                      ; id %159
               OpName %m_1 "m"                                                      ; id %208
               OpName %m_2 "m"                                                      ; id %238
               OpName %m_3 "m"                                                      ; id %257
               OpName %m_4 "m"                                                      ; id %278
               OpName %main "main"                                                  ; id %7
               OpName %_0_ok "_0_ok"                                                ; id %318
               OpName %_1_splat_4 "_1_splat_4"                                      ; id %319
               OpName %_2_splat_2 "_2_splat_2"                                      ; id %320
               OpName %_3_m "_3_m"                                                  ; id %321
               OpName %_4_m "_4_m"                                                  ; id %412
               OpName %_5_m "_5_m"                                                  ; id %432
               OpName %_6_m "_6_m"                                                  ; id %444
               OpName %_7_m "_7_m"                                                  ; id %456
               OpName %_8_m "_8_m"                                                  ; id %468

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
               OpDecorate %splat_2 RelaxedPrecision
               OpDecorate %m RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %m_0 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %186 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %m_1 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %221 RelaxedPrecision
               OpDecorate %222 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %234 RelaxedPrecision
               OpDecorate %m_2 RelaxedPrecision
               OpDecorate %244 RelaxedPrecision
               OpDecorate %245 RelaxedPrecision
               OpDecorate %246 RelaxedPrecision
               OpDecorate %251 RelaxedPrecision
               OpDecorate %253 RelaxedPrecision
               OpDecorate %m_3 RelaxedPrecision
               OpDecorate %262 RelaxedPrecision
               OpDecorate %270 RelaxedPrecision
               OpDecorate %271 RelaxedPrecision
               OpDecorate %273 RelaxedPrecision
               OpDecorate %274 RelaxedPrecision
               OpDecorate %m_4 RelaxedPrecision
               OpDecorate %287 RelaxedPrecision
               OpDecorate %303 RelaxedPrecision
               OpDecorate %304 RelaxedPrecision
               OpDecorate %306 RelaxedPrecision
               OpDecorate %307 RelaxedPrecision
               OpDecorate %310 RelaxedPrecision
               OpDecorate %311 RelaxedPrecision
               OpDecorate %496 RelaxedPrecision
               OpDecorate %499 RelaxedPrecision
               OpDecorate %500 RelaxedPrecision

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
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
    %float_4 = OpConstant %float 4
         %36 = OpConstantComposite %v3float %float_4 %float_4 %float_4
         %37 = OpConstantComposite %mat3v3float %36 %36 %36
    %float_2 = OpConstant %float 2
         %40 = OpConstantComposite %v3float %float_2 %float_2 %float_2
         %41 = OpConstantComposite %mat3v3float %40 %40 %40
         %43 = OpConstantComposite %v3float %float_2 %float_0 %float_0
         %44 = OpConstantComposite %v3float %float_0 %float_2 %float_0
         %45 = OpConstantComposite %v3float %float_0 %float_0 %float_2
         %46 = OpConstantComposite %mat3v3float %43 %44 %45
      %false = OpConstantFalse %bool
    %float_6 = OpConstant %float 6
         %55 = OpConstantComposite %v3float %float_6 %float_4 %float_4
         %56 = OpConstantComposite %v3float %float_4 %float_6 %float_4
         %57 = OpConstantComposite %v3float %float_4 %float_4 %float_6
         %58 = OpConstantComposite %mat3v3float %55 %56 %57
     %v3bool = OpTypeVector %bool 3
   %float_n2 = OpConstant %float -2
   %float_n4 = OpConstant %float -4
         %77 = OpConstantComposite %v3float %float_n2 %float_n4 %float_n4
         %78 = OpConstantComposite %v3float %float_n4 %float_n2 %float_n4
         %79 = OpConstantComposite %v3float %float_n4 %float_n4 %float_n2
         %80 = OpConstantComposite %mat3v3float %77 %78 %79
  %float_0_5 = OpConstant %float 0.5
         %97 = OpConstantComposite %v3float %float_0_5 %float_0 %float_0
         %98 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
         %99 = OpConstantComposite %v3float %float_0 %float_0 %float_0_5
        %100 = OpConstantComposite %mat3v3float %97 %98 %99
        %131 = OpConstantComposite %v3float %float_2 %float_4 %float_4
        %132 = OpConstantComposite %v3float %float_4 %float_2 %float_4
        %133 = OpConstantComposite %v3float %float_4 %float_4 %float_2
        %134 = OpConstantComposite %mat3v3float %131 %132 %133
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
        %175 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
        %176 = OpConstantComposite %v4float %float_5 %float_6 %float_7 %float_8
        %177 = OpConstantComposite %v4float %float_9 %float_10 %float_11 %float_12
        %178 = OpConstantComposite %v4float %float_13 %float_14 %float_15 %float_16
        %179 = OpConstantComposite %mat4v4float %175 %176 %177 %178
        %180 = OpConstantComposite %v4float %float_16 %float_15 %float_14 %float_13
        %181 = OpConstantComposite %v4float %float_12 %float_11 %float_10 %float_9
        %182 = OpConstantComposite %v4float %float_8 %float_7 %float_6 %float_5
        %183 = OpConstantComposite %v4float %float_4 %float_3 %float_2 %float_1
        %184 = OpConstantComposite %mat4v4float %180 %181 %182 %183
   %float_17 = OpConstant %float 17
        %193 = OpConstantComposite %v4float %float_17 %float_17 %float_17 %float_17
        %194 = OpConstantComposite %mat4v4float %193 %193 %193 %193
     %v4bool = OpTypeVector %bool 4
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
   %float_20 = OpConstant %float 20
   %float_30 = OpConstant %float 30
   %float_40 = OpConstant %float 40
        %214 = OpConstantComposite %v2float %float_10 %float_20
        %215 = OpConstantComposite %v2float %float_30 %float_40
        %216 = OpConstantComposite %mat2v2float %214 %215
        %217 = OpConstantComposite %v2float %float_1 %float_2
        %218 = OpConstantComposite %v2float %float_3 %float_4
        %219 = OpConstantComposite %mat2v2float %217 %218
   %float_18 = OpConstant %float 18
   %float_27 = OpConstant %float 27
   %float_36 = OpConstant %float 36
        %228 = OpConstantComposite %v2float %float_9 %float_18
        %229 = OpConstantComposite %v2float %float_27 %float_36
        %230 = OpConstantComposite %mat2v2float %228 %229
     %v2bool = OpTypeVector %bool 2
        %239 = OpConstantComposite %v2float %float_2 %float_4
        %240 = OpConstantComposite %v2float %float_6 %float_8
        %241 = OpConstantComposite %mat2v2float %239 %240
        %242 = OpConstantComposite %v2float %float_2 %float_2
        %243 = OpConstantComposite %mat2v2float %242 %239
        %249 = OpConstantComposite %v2float %float_3 %float_2
        %250 = OpConstantComposite %mat2v2float %217 %249
        %258 = OpConstantComposite %v2float %float_7 %float_4
        %259 = OpConstantComposite %mat2v2float %217 %258
        %260 = OpConstantComposite %v2float %float_3 %float_5
        %261 = OpConstantComposite %mat2v2float %260 %249
   %float_38 = OpConstant %float 38
   %float_26 = OpConstant %float 26
        %267 = OpConstantComposite %v2float %float_38 %float_26
        %268 = OpConstantComposite %v2float %float_17 %float_14
        %269 = OpConstantComposite %mat2v2float %267 %268
        %279 = OpConstantComposite %v3float %float_10 %float_4 %float_2
        %280 = OpConstantComposite %v3float %float_20 %float_5 %float_3
        %281 = OpConstantComposite %v3float %float_10 %float_6 %float_5
        %282 = OpConstantComposite %mat3v3float %279 %280 %281
        %283 = OpConstantComposite %v3float %float_3 %float_3 %float_4
        %284 = OpConstantComposite %v3float %float_2 %float_3 %float_4
        %285 = OpConstantComposite %v3float %float_4 %float_9 %float_2
        %286 = OpConstantComposite %mat3v3float %283 %284 %285
  %float_130 = OpConstant %float 130
   %float_51 = OpConstant %float 51
   %float_35 = OpConstant %float 35
  %float_120 = OpConstant %float 120
   %float_47 = OpConstant %float 47
   %float_33 = OpConstant %float 33
  %float_240 = OpConstant %float 240
   %float_73 = OpConstant %float 73
   %float_45 = OpConstant %float 45
        %299 = OpConstantComposite %v3float %float_130 %float_51 %float_35
        %300 = OpConstantComposite %v3float %float_120 %float_47 %float_33
        %301 = OpConstantComposite %v3float %float_240 %float_73 %float_45
        %302 = OpConstantComposite %mat3v3float %299 %300 %301
        %315 = OpTypeFunction %v4float %_ptr_Function_v2float
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
    %splat_4 =   OpVariable %_ptr_Function_mat3v3float Function     ; RelaxedPrecision
    %splat_2 =   OpVariable %_ptr_Function_mat3v3float Function     ; RelaxedPrecision
          %m =   OpVariable %_ptr_Function_mat3v3float Function     ; RelaxedPrecision
        %m_0 =   OpVariable %_ptr_Function_mat4v4float Function     ; RelaxedPrecision
        %m_1 =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
        %m_2 =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
        %m_3 =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
        %m_4 =   OpVariable %_ptr_Function_mat3v3float Function     ; RelaxedPrecision
                 OpStore %ok %true
                 OpStore %splat_4 %37
                 OpStore %splat_2 %41
                 OpStore %m %46
         %47 =   OpFAdd %v3float %43 %36            ; RelaxedPrecision
         %48 =   OpFAdd %v3float %44 %36            ; RelaxedPrecision
         %49 =   OpFAdd %v3float %45 %36            ; RelaxedPrecision
         %50 =   OpCompositeConstruct %mat3v3float %47 %48 %49  ; RelaxedPrecision
                 OpStore %m %50
                 OpSelectionMerge %53 None
                 OpBranchConditional %true %52 %53

         %52 =     OpLabel
         %60 =       OpFOrdEqual %v3bool %47 %55    ; RelaxedPrecision
         %61 =       OpAll %bool %60
         %62 =       OpFOrdEqual %v3bool %48 %56    ; RelaxedPrecision
         %63 =       OpAll %bool %62
         %64 =       OpLogicalAnd %bool %61 %63
         %65 =       OpFOrdEqual %v3bool %49 %57    ; RelaxedPrecision
         %66 =       OpAll %bool %65
         %67 =       OpLogicalAnd %bool %64 %66
                     OpBranch %53

         %53 = OpLabel
         %68 =   OpPhi %bool %false %27 %67 %52
                 OpStore %ok %68
                 OpStore %m %46
         %69 =   OpFSub %v3float %43 %36            ; RelaxedPrecision
         %70 =   OpFSub %v3float %44 %36            ; RelaxedPrecision
         %71 =   OpFSub %v3float %45 %36            ; RelaxedPrecision
         %72 =   OpCompositeConstruct %mat3v3float %69 %70 %71  ; RelaxedPrecision
                 OpStore %m %72
                 OpSelectionMerge %74 None
                 OpBranchConditional %68 %73 %74

         %73 =     OpLabel
         %81 =       OpFOrdEqual %v3bool %69 %77    ; RelaxedPrecision
         %82 =       OpAll %bool %81
         %83 =       OpFOrdEqual %v3bool %70 %78    ; RelaxedPrecision
         %84 =       OpAll %bool %83
         %85 =       OpLogicalAnd %bool %82 %84
         %86 =       OpFOrdEqual %v3bool %71 %79    ; RelaxedPrecision
         %87 =       OpAll %bool %86
         %88 =       OpLogicalAnd %bool %85 %87
                     OpBranch %74

         %74 = OpLabel
         %89 =   OpPhi %bool %false %53 %88 %73
                 OpStore %ok %89
                 OpStore %m %46
         %90 =   OpFDiv %v3float %43 %36            ; RelaxedPrecision
         %91 =   OpFDiv %v3float %44 %36            ; RelaxedPrecision
         %92 =   OpFDiv %v3float %45 %36            ; RelaxedPrecision
         %93 =   OpCompositeConstruct %mat3v3float %90 %91 %92  ; RelaxedPrecision
                 OpStore %m %93
                 OpSelectionMerge %95 None
                 OpBranchConditional %89 %94 %95

         %94 =     OpLabel
        %101 =       OpFOrdEqual %v3bool %90 %97    ; RelaxedPrecision
        %102 =       OpAll %bool %101
        %103 =       OpFOrdEqual %v3bool %91 %98    ; RelaxedPrecision
        %104 =       OpAll %bool %103
        %105 =       OpLogicalAnd %bool %102 %104
        %106 =       OpFOrdEqual %v3bool %92 %99    ; RelaxedPrecision
        %107 =       OpAll %bool %106
        %108 =       OpLogicalAnd %bool %105 %107
                     OpBranch %95

         %95 = OpLabel
        %109 =   OpPhi %bool %false %74 %108 %94
                 OpStore %ok %109
                 OpStore %m %37
        %110 =   OpFAdd %v3float %36 %43            ; RelaxedPrecision
        %111 =   OpFAdd %v3float %36 %44            ; RelaxedPrecision
        %112 =   OpFAdd %v3float %36 %45            ; RelaxedPrecision
        %113 =   OpCompositeConstruct %mat3v3float %110 %111 %112   ; RelaxedPrecision
                 OpStore %m %113
                 OpSelectionMerge %115 None
                 OpBranchConditional %109 %114 %115

        %114 =     OpLabel
        %116 =       OpFOrdEqual %v3bool %110 %55   ; RelaxedPrecision
        %117 =       OpAll %bool %116
        %118 =       OpFOrdEqual %v3bool %111 %56   ; RelaxedPrecision
        %119 =       OpAll %bool %118
        %120 =       OpLogicalAnd %bool %117 %119
        %121 =       OpFOrdEqual %v3bool %112 %57   ; RelaxedPrecision
        %122 =       OpAll %bool %121
        %123 =       OpLogicalAnd %bool %120 %122
                     OpBranch %115

        %115 = OpLabel
        %124 =   OpPhi %bool %false %95 %123 %114
                 OpStore %ok %124
                 OpStore %m %37
        %125 =   OpFSub %v3float %36 %43            ; RelaxedPrecision
        %126 =   OpFSub %v3float %36 %44            ; RelaxedPrecision
        %127 =   OpFSub %v3float %36 %45            ; RelaxedPrecision
        %128 =   OpCompositeConstruct %mat3v3float %125 %126 %127   ; RelaxedPrecision
                 OpStore %m %128
                 OpSelectionMerge %130 None
                 OpBranchConditional %124 %129 %130

        %129 =     OpLabel
        %135 =       OpFOrdEqual %v3bool %125 %131  ; RelaxedPrecision
        %136 =       OpAll %bool %135
        %137 =       OpFOrdEqual %v3bool %126 %132  ; RelaxedPrecision
        %138 =       OpAll %bool %137
        %139 =       OpLogicalAnd %bool %136 %138
        %140 =       OpFOrdEqual %v3bool %127 %133  ; RelaxedPrecision
        %141 =       OpAll %bool %140
        %142 =       OpLogicalAnd %bool %139 %141
                     OpBranch %130

        %130 = OpLabel
        %143 =   OpPhi %bool %false %115 %142 %129
                 OpStore %ok %143
                 OpStore %m %37
        %144 =   OpFDiv %v3float %36 %40            ; RelaxedPrecision
        %145 =   OpFDiv %v3float %36 %40            ; RelaxedPrecision
        %146 =   OpFDiv %v3float %36 %40            ; RelaxedPrecision
        %147 =   OpCompositeConstruct %mat3v3float %144 %145 %146   ; RelaxedPrecision
                 OpStore %m %147
                 OpSelectionMerge %149 None
                 OpBranchConditional %143 %148 %149

        %148 =     OpLabel
        %150 =       OpFOrdEqual %v3bool %144 %40   ; RelaxedPrecision
        %151 =       OpAll %bool %150
        %152 =       OpFOrdEqual %v3bool %145 %40   ; RelaxedPrecision
        %153 =       OpAll %bool %152
        %154 =       OpLogicalAnd %bool %151 %153
        %155 =       OpFOrdEqual %v3bool %146 %40   ; RelaxedPrecision
        %156 =       OpAll %bool %155
        %157 =       OpLogicalAnd %bool %154 %156
                     OpBranch %149

        %149 = OpLabel
        %158 =   OpPhi %bool %false %130 %157 %148
                 OpStore %ok %158
                 OpStore %m_0 %179
        %185 =   OpFAdd %v4float %175 %180          ; RelaxedPrecision
        %186 =   OpFAdd %v4float %176 %181          ; RelaxedPrecision
        %187 =   OpFAdd %v4float %177 %182          ; RelaxedPrecision
        %188 =   OpFAdd %v4float %178 %183          ; RelaxedPrecision
        %189 =   OpCompositeConstruct %mat4v4float %185 %186 %187 %188  ; RelaxedPrecision
                 OpStore %m_0 %189
                 OpSelectionMerge %191 None
                 OpBranchConditional %158 %190 %191

        %190 =     OpLabel
        %196 =       OpFOrdEqual %v4bool %185 %193  ; RelaxedPrecision
        %197 =       OpAll %bool %196
        %198 =       OpFOrdEqual %v4bool %186 %193  ; RelaxedPrecision
        %199 =       OpAll %bool %198
        %200 =       OpLogicalAnd %bool %197 %199
        %201 =       OpFOrdEqual %v4bool %187 %193  ; RelaxedPrecision
        %202 =       OpAll %bool %201
        %203 =       OpLogicalAnd %bool %200 %202
        %204 =       OpFOrdEqual %v4bool %188 %193  ; RelaxedPrecision
        %205 =       OpAll %bool %204
        %206 =       OpLogicalAnd %bool %203 %205
                     OpBranch %191

        %191 = OpLabel
        %207 =   OpPhi %bool %false %149 %206 %190
                 OpStore %ok %207
                 OpStore %m_1 %216
        %220 =   OpFSub %v2float %214 %217          ; RelaxedPrecision
        %221 =   OpFSub %v2float %215 %218          ; RelaxedPrecision
        %222 =   OpCompositeConstruct %mat2v2float %220 %221    ; RelaxedPrecision
                 OpStore %m_1 %222
                 OpSelectionMerge %224 None
                 OpBranchConditional %207 %223 %224

        %223 =     OpLabel
        %232 =       OpFOrdEqual %v2bool %220 %228  ; RelaxedPrecision
        %233 =       OpAll %bool %232
        %234 =       OpFOrdEqual %v2bool %221 %229  ; RelaxedPrecision
        %235 =       OpAll %bool %234
        %236 =       OpLogicalAnd %bool %233 %235
                     OpBranch %224

        %224 = OpLabel
        %237 =   OpPhi %bool %false %191 %236 %223
                 OpStore %ok %237
                 OpStore %m_2 %241
        %244 =   OpFDiv %v2float %239 %242          ; RelaxedPrecision
        %245 =   OpFDiv %v2float %240 %239          ; RelaxedPrecision
        %246 =   OpCompositeConstruct %mat2v2float %244 %245    ; RelaxedPrecision
                 OpStore %m_2 %246
                 OpSelectionMerge %248 None
                 OpBranchConditional %237 %247 %248

        %247 =     OpLabel
        %251 =       OpFOrdEqual %v2bool %244 %217  ; RelaxedPrecision
        %252 =       OpAll %bool %251
        %253 =       OpFOrdEqual %v2bool %245 %249  ; RelaxedPrecision
        %254 =       OpAll %bool %253
        %255 =       OpLogicalAnd %bool %252 %254
                     OpBranch %248

        %248 = OpLabel
        %256 =   OpPhi %bool %false %224 %255 %247
                 OpStore %ok %256
                 OpStore %m_3 %259
        %262 =   OpMatrixTimesMatrix %mat2v2float %259 %261     ; RelaxedPrecision
                 OpStore %m_3 %262
                 OpSelectionMerge %264 None
                 OpBranchConditional %256 %263 %264

        %263 =     OpLabel
        %270 =       OpCompositeExtract %v2float %262 0     ; RelaxedPrecision
        %271 =       OpFOrdEqual %v2bool %270 %267          ; RelaxedPrecision
        %272 =       OpAll %bool %271
        %273 =       OpCompositeExtract %v2float %262 1     ; RelaxedPrecision
        %274 =       OpFOrdEqual %v2bool %273 %268          ; RelaxedPrecision
        %275 =       OpAll %bool %274
        %276 =       OpLogicalAnd %bool %272 %275
                     OpBranch %264

        %264 = OpLabel
        %277 =   OpPhi %bool %false %248 %276 %263
                 OpStore %ok %277
                 OpStore %m_4 %282
        %287 =   OpMatrixTimesMatrix %mat3v3float %282 %286     ; RelaxedPrecision
                 OpStore %m_4 %287
                 OpSelectionMerge %289 None
                 OpBranchConditional %277 %288 %289

        %288 =     OpLabel
        %303 =       OpCompositeExtract %v3float %287 0     ; RelaxedPrecision
        %304 =       OpFOrdEqual %v3bool %303 %299          ; RelaxedPrecision
        %305 =       OpAll %bool %304
        %306 =       OpCompositeExtract %v3float %287 1     ; RelaxedPrecision
        %307 =       OpFOrdEqual %v3bool %306 %300          ; RelaxedPrecision
        %308 =       OpAll %bool %307
        %309 =       OpLogicalAnd %bool %305 %308
        %310 =       OpCompositeExtract %v3float %287 2     ; RelaxedPrecision
        %311 =       OpFOrdEqual %v3bool %310 %301          ; RelaxedPrecision
        %312 =       OpAll %bool %311
        %313 =       OpLogicalAnd %bool %309 %312
                     OpBranch %289

        %289 = OpLabel
        %314 =   OpPhi %bool %false %264 %313 %288
                 OpStore %ok %314
                 OpReturnValue %314
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %315        ; RelaxedPrecision
        %316 = OpFunctionParameter %_ptr_Function_v2float

        %317 = OpLabel
      %_0_ok =   OpVariable %_ptr_Function_bool Function
 %_1_splat_4 =   OpVariable %_ptr_Function_mat3v3float Function
 %_2_splat_2 =   OpVariable %_ptr_Function_mat3v3float Function
       %_3_m =   OpVariable %_ptr_Function_mat3v3float Function
       %_4_m =   OpVariable %_ptr_Function_mat4v4float Function
       %_5_m =   OpVariable %_ptr_Function_mat2v2float Function
       %_6_m =   OpVariable %_ptr_Function_mat2v2float Function
       %_7_m =   OpVariable %_ptr_Function_mat2v2float Function
       %_8_m =   OpVariable %_ptr_Function_mat3v3float Function
        %488 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %_0_ok %true
                 OpStore %_1_splat_4 %37
                 OpStore %_2_splat_2 %41
                 OpStore %_3_m %46
        %322 =   OpFAdd %v3float %43 %36
        %323 =   OpFAdd %v3float %44 %36
        %324 =   OpFAdd %v3float %45 %36
        %325 =   OpCompositeConstruct %mat3v3float %322 %323 %324
                 OpStore %_3_m %325
                 OpSelectionMerge %327 None
                 OpBranchConditional %true %326 %327

        %326 =     OpLabel
        %328 =       OpFOrdEqual %v3bool %322 %55
        %329 =       OpAll %bool %328
        %330 =       OpFOrdEqual %v3bool %323 %56
        %331 =       OpAll %bool %330
        %332 =       OpLogicalAnd %bool %329 %331
        %333 =       OpFOrdEqual %v3bool %324 %57
        %334 =       OpAll %bool %333
        %335 =       OpLogicalAnd %bool %332 %334
                     OpBranch %327

        %327 = OpLabel
        %336 =   OpPhi %bool %false %317 %335 %326
                 OpStore %_0_ok %336
                 OpStore %_3_m %46
        %337 =   OpFSub %v3float %43 %36
        %338 =   OpFSub %v3float %44 %36
        %339 =   OpFSub %v3float %45 %36
        %340 =   OpCompositeConstruct %mat3v3float %337 %338 %339
                 OpStore %_3_m %340
                 OpSelectionMerge %342 None
                 OpBranchConditional %336 %341 %342

        %341 =     OpLabel
        %343 =       OpFOrdEqual %v3bool %337 %77
        %344 =       OpAll %bool %343
        %345 =       OpFOrdEqual %v3bool %338 %78
        %346 =       OpAll %bool %345
        %347 =       OpLogicalAnd %bool %344 %346
        %348 =       OpFOrdEqual %v3bool %339 %79
        %349 =       OpAll %bool %348
        %350 =       OpLogicalAnd %bool %347 %349
                     OpBranch %342

        %342 = OpLabel
        %351 =   OpPhi %bool %false %327 %350 %341
                 OpStore %_0_ok %351
                 OpStore %_3_m %46
        %352 =   OpFDiv %v3float %43 %36
        %353 =   OpFDiv %v3float %44 %36
        %354 =   OpFDiv %v3float %45 %36
        %355 =   OpCompositeConstruct %mat3v3float %352 %353 %354
                 OpStore %_3_m %355
                 OpSelectionMerge %357 None
                 OpBranchConditional %351 %356 %357

        %356 =     OpLabel
        %358 =       OpFOrdEqual %v3bool %352 %97
        %359 =       OpAll %bool %358
        %360 =       OpFOrdEqual %v3bool %353 %98
        %361 =       OpAll %bool %360
        %362 =       OpLogicalAnd %bool %359 %361
        %363 =       OpFOrdEqual %v3bool %354 %99
        %364 =       OpAll %bool %363
        %365 =       OpLogicalAnd %bool %362 %364
                     OpBranch %357

        %357 = OpLabel
        %366 =   OpPhi %bool %false %342 %365 %356
                 OpStore %_0_ok %366
                 OpStore %_3_m %37
        %367 =   OpFAdd %v3float %36 %43
        %368 =   OpFAdd %v3float %36 %44
        %369 =   OpFAdd %v3float %36 %45
        %370 =   OpCompositeConstruct %mat3v3float %367 %368 %369
                 OpStore %_3_m %370
                 OpSelectionMerge %372 None
                 OpBranchConditional %366 %371 %372

        %371 =     OpLabel
        %373 =       OpFOrdEqual %v3bool %367 %55
        %374 =       OpAll %bool %373
        %375 =       OpFOrdEqual %v3bool %368 %56
        %376 =       OpAll %bool %375
        %377 =       OpLogicalAnd %bool %374 %376
        %378 =       OpFOrdEqual %v3bool %369 %57
        %379 =       OpAll %bool %378
        %380 =       OpLogicalAnd %bool %377 %379
                     OpBranch %372

        %372 = OpLabel
        %381 =   OpPhi %bool %false %357 %380 %371
                 OpStore %_0_ok %381
                 OpStore %_3_m %37
        %382 =   OpFSub %v3float %36 %43
        %383 =   OpFSub %v3float %36 %44
        %384 =   OpFSub %v3float %36 %45
        %385 =   OpCompositeConstruct %mat3v3float %382 %383 %384
                 OpStore %_3_m %385
                 OpSelectionMerge %387 None
                 OpBranchConditional %381 %386 %387

        %386 =     OpLabel
        %388 =       OpFOrdEqual %v3bool %382 %131
        %389 =       OpAll %bool %388
        %390 =       OpFOrdEqual %v3bool %383 %132
        %391 =       OpAll %bool %390
        %392 =       OpLogicalAnd %bool %389 %391
        %393 =       OpFOrdEqual %v3bool %384 %133
        %394 =       OpAll %bool %393
        %395 =       OpLogicalAnd %bool %392 %394
                     OpBranch %387

        %387 = OpLabel
        %396 =   OpPhi %bool %false %372 %395 %386
                 OpStore %_0_ok %396
                 OpStore %_3_m %37
        %397 =   OpFDiv %v3float %36 %40
        %398 =   OpFDiv %v3float %36 %40
        %399 =   OpFDiv %v3float %36 %40
        %400 =   OpCompositeConstruct %mat3v3float %397 %398 %399
                 OpStore %_3_m %400
                 OpSelectionMerge %402 None
                 OpBranchConditional %396 %401 %402

        %401 =     OpLabel
        %403 =       OpFOrdEqual %v3bool %397 %40
        %404 =       OpAll %bool %403
        %405 =       OpFOrdEqual %v3bool %398 %40
        %406 =       OpAll %bool %405
        %407 =       OpLogicalAnd %bool %404 %406
        %408 =       OpFOrdEqual %v3bool %399 %40
        %409 =       OpAll %bool %408
        %410 =       OpLogicalAnd %bool %407 %409
                     OpBranch %402

        %402 = OpLabel
        %411 =   OpPhi %bool %false %387 %410 %401
                 OpStore %_0_ok %411
                 OpStore %_4_m %179
        %413 =   OpFAdd %v4float %175 %180
        %414 =   OpFAdd %v4float %176 %181
        %415 =   OpFAdd %v4float %177 %182
        %416 =   OpFAdd %v4float %178 %183
        %417 =   OpCompositeConstruct %mat4v4float %413 %414 %415 %416
                 OpStore %_4_m %417
                 OpSelectionMerge %419 None
                 OpBranchConditional %411 %418 %419

        %418 =     OpLabel
        %420 =       OpFOrdEqual %v4bool %413 %193
        %421 =       OpAll %bool %420
        %422 =       OpFOrdEqual %v4bool %414 %193
        %423 =       OpAll %bool %422
        %424 =       OpLogicalAnd %bool %421 %423
        %425 =       OpFOrdEqual %v4bool %415 %193
        %426 =       OpAll %bool %425
        %427 =       OpLogicalAnd %bool %424 %426
        %428 =       OpFOrdEqual %v4bool %416 %193
        %429 =       OpAll %bool %428
        %430 =       OpLogicalAnd %bool %427 %429
                     OpBranch %419

        %419 = OpLabel
        %431 =   OpPhi %bool %false %402 %430 %418
                 OpStore %_0_ok %431
                 OpStore %_5_m %216
        %433 =   OpFSub %v2float %214 %217
        %434 =   OpFSub %v2float %215 %218
        %435 =   OpCompositeConstruct %mat2v2float %433 %434
                 OpStore %_5_m %435
                 OpSelectionMerge %437 None
                 OpBranchConditional %431 %436 %437

        %436 =     OpLabel
        %438 =       OpFOrdEqual %v2bool %433 %228
        %439 =       OpAll %bool %438
        %440 =       OpFOrdEqual %v2bool %434 %229
        %441 =       OpAll %bool %440
        %442 =       OpLogicalAnd %bool %439 %441
                     OpBranch %437

        %437 = OpLabel
        %443 =   OpPhi %bool %false %419 %442 %436
                 OpStore %_0_ok %443
                 OpStore %_6_m %241
        %445 =   OpFDiv %v2float %239 %242
        %446 =   OpFDiv %v2float %240 %239
        %447 =   OpCompositeConstruct %mat2v2float %445 %446
                 OpStore %_6_m %447
                 OpSelectionMerge %449 None
                 OpBranchConditional %443 %448 %449

        %448 =     OpLabel
        %450 =       OpFOrdEqual %v2bool %445 %217
        %451 =       OpAll %bool %450
        %452 =       OpFOrdEqual %v2bool %446 %249
        %453 =       OpAll %bool %452
        %454 =       OpLogicalAnd %bool %451 %453
                     OpBranch %449

        %449 = OpLabel
        %455 =   OpPhi %bool %false %437 %454 %448
                 OpStore %_0_ok %455
                 OpStore %_7_m %259
        %457 =   OpMatrixTimesMatrix %mat2v2float %259 %261
                 OpStore %_7_m %457
                 OpSelectionMerge %459 None
                 OpBranchConditional %455 %458 %459

        %458 =     OpLabel
        %460 =       OpCompositeExtract %v2float %457 0
        %461 =       OpFOrdEqual %v2bool %460 %267
        %462 =       OpAll %bool %461
        %463 =       OpCompositeExtract %v2float %457 1
        %464 =       OpFOrdEqual %v2bool %463 %268
        %465 =       OpAll %bool %464
        %466 =       OpLogicalAnd %bool %462 %465
                     OpBranch %459

        %459 = OpLabel
        %467 =   OpPhi %bool %false %449 %466 %458
                 OpStore %_0_ok %467
                 OpStore %_8_m %282
        %469 =   OpMatrixTimesMatrix %mat3v3float %282 %286
                 OpStore %_8_m %469
                 OpSelectionMerge %471 None
                 OpBranchConditional %467 %470 %471

        %470 =     OpLabel
        %472 =       OpCompositeExtract %v3float %469 0
        %473 =       OpFOrdEqual %v3bool %472 %299
        %474 =       OpAll %bool %473
        %475 =       OpCompositeExtract %v3float %469 1
        %476 =       OpFOrdEqual %v3bool %475 %300
        %477 =       OpAll %bool %476
        %478 =       OpLogicalAnd %bool %474 %477
        %479 =       OpCompositeExtract %v3float %469 2
        %480 =       OpFOrdEqual %v3bool %479 %301
        %481 =       OpAll %bool %480
        %482 =       OpLogicalAnd %bool %478 %481
                     OpBranch %471

        %471 = OpLabel
        %483 =   OpPhi %bool %false %459 %482 %470
                 OpStore %_0_ok %483
                 OpSelectionMerge %485 None
                 OpBranchConditional %483 %484 %485

        %484 =     OpLabel
        %486 =       OpFunctionCall %bool %test_matrix_op_matrix_half_b
                     OpBranch %485

        %485 = OpLabel
        %487 =   OpPhi %bool %false %471 %486 %484
                 OpSelectionMerge %492 None
                 OpBranchConditional %487 %490 %491

        %490 =     OpLabel
        %493 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %496 =       OpLoad %v4float %493           ; RelaxedPrecision
                     OpStore %488 %496
                     OpBranch %492

        %491 =     OpLabel
        %497 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %499 =       OpLoad %v4float %497           ; RelaxedPrecision
                     OpStore %488 %499
                     OpBranch %492

        %492 = OpLabel
        %500 =   OpLoad %v4float %488               ; RelaxedPrecision
                 OpReturnValue %500
               OpFunctionEnd
