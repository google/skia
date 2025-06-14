               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %matrix2x2 "matrix2x2"            ; id %27
               OpName %inv2x2 "inv2x2"                  ; id %37
               OpName %inv3x3 "inv3x3"                  ; id %44
               OpName %inv4x4 "inv4x4"                  ; id %59
               OpName %Zero "Zero"                      ; id %70

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
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %212 RelaxedPrecision
               OpDecorate %215 RelaxedPrecision
               OpDecorate %216 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %34 = OpConstantComposite %v2float %float_1 %float_2
         %35 = OpConstantComposite %v2float %float_3 %float_4
         %36 = OpConstantComposite %mat2v2float %34 %35
   %float_n2 = OpConstant %float -2
  %float_1_5 = OpConstant %float 1.5
 %float_n0_5 = OpConstant %float -0.5
         %41 = OpConstantComposite %v2float %float_n2 %float_1
         %42 = OpConstantComposite %v2float %float_1_5 %float_n0_5
         %43 = OpConstantComposite %mat2v2float %41 %42
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
  %float_n24 = OpConstant %float -24
   %float_18 = OpConstant %float 18
    %float_5 = OpConstant %float 5
   %float_20 = OpConstant %float 20
  %float_n15 = OpConstant %float -15
   %float_n4 = OpConstant %float -4
   %float_n5 = OpConstant %float -5
         %55 = OpConstantComposite %v3float %float_n24 %float_18 %float_5
         %56 = OpConstantComposite %v3float %float_20 %float_n15 %float_n4
         %57 = OpConstantComposite %v3float %float_n5 %float_4 %float_1
         %58 = OpConstantComposite %mat3v3float %55 %56 %57
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
  %float_0_5 = OpConstant %float 0.5
   %float_n8 = OpConstant %float -8
   %float_n1 = OpConstant %float -1
         %65 = OpConstantComposite %v4float %float_n2 %float_n0_5 %float_1 %float_0_5
         %66 = OpConstantComposite %v4float %float_1 %float_0_5 %float_0 %float_n0_5
         %67 = OpConstantComposite %v4float %float_n8 %float_n1 %float_2 %float_2
         %68 = OpConstantComposite %v4float %float_3 %float_0_5 %float_n1 %float_n0_5
         %69 = OpConstantComposite %mat4v4float %65 %66 %67 %68
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v2bool = OpTypeVector %bool 2
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
    %float_6 = OpConstant %float 6
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
    %float_9 = OpConstant %float 9
        %119 = OpConstantComposite %v3float %float_1 %float_2 %float_3
        %120 = OpConstantComposite %v3float %float_4 %float_5 %float_6
        %121 = OpConstantComposite %v3float %float_7 %float_8 %float_9
        %122 = OpConstantComposite %mat3v3float %119 %120 %121
        %154 = OpConstantComposite %v3float %float_0 %float_1 %float_4
        %155 = OpConstantComposite %v3float %float_5 %float_6 %float_0
        %156 = OpConstantComposite %mat3v3float %119 %154 %155
        %178 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
        %179 = OpConstantComposite %v4float %float_0 %float_2 %float_1 %float_2
        %180 = OpConstantComposite %v4float %float_2 %float_1 %float_0 %float_1
        %181 = OpConstantComposite %v4float %float_2 %float_0 %float_1 %float_4
        %182 = OpConstantComposite %mat4v4float %178 %179 %180 %181
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %16

         %17 = OpLabel
         %21 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %21 %20
         %23 =   OpFunctionCall %v4float %main %21
                 OpStore %sk_FragColor %23
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %24         ; RelaxedPrecision
         %25 = OpFunctionParameter %_ptr_Function_v2float

         %26 = OpLabel
  %matrix2x2 =   OpVariable %_ptr_Function_mat2v2float Function
     %inv2x2 =   OpVariable %_ptr_Function_mat2v2float Function
     %inv3x3 =   OpVariable %_ptr_Function_mat3v3float Function
     %inv4x4 =   OpVariable %_ptr_Function_mat4v4float Function
       %Zero =   OpVariable %_ptr_Function_float Function
        %206 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %matrix2x2 %36
                 OpStore %inv2x2 %43
                 OpStore %inv3x3 %58
                 OpStore %inv4x4 %69
         %72 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %75 =   OpLoad %v4float %72                ; RelaxedPrecision
         %76 =   OpCompositeExtract %float %75 2    ; RelaxedPrecision
                 OpStore %Zero %76
         %80 =   OpFOrdEqual %v2bool %41 %41
         %81 =   OpAll %bool %80
         %82 =   OpFOrdEqual %v2bool %42 %42
         %83 =   OpAll %bool %82
         %84 =   OpLogicalAnd %bool %81 %83
                 OpSelectionMerge %86 None
                 OpBranchConditional %84 %85 %86

         %85 =     OpLabel
         %88 =       OpFOrdEqual %v3bool %55 %55
         %89 =       OpAll %bool %88
         %90 =       OpFOrdEqual %v3bool %56 %56
         %91 =       OpAll %bool %90
         %92 =       OpLogicalAnd %bool %89 %91
         %93 =       OpFOrdEqual %v3bool %57 %57
         %94 =       OpAll %bool %93
         %95 =       OpLogicalAnd %bool %92 %94
                     OpBranch %86

         %86 = OpLabel
         %96 =   OpPhi %bool %false %26 %95 %85
                 OpSelectionMerge %98 None
                 OpBranchConditional %96 %97 %98

         %97 =     OpLabel
        %100 =       OpFOrdEqual %v4bool %65 %65
        %101 =       OpAll %bool %100
        %102 =       OpFOrdEqual %v4bool %66 %66
        %103 =       OpAll %bool %102
        %104 =       OpLogicalAnd %bool %101 %103
        %105 =       OpFOrdEqual %v4bool %67 %67
        %106 =       OpAll %bool %105
        %107 =       OpLogicalAnd %bool %104 %106
        %108 =       OpFOrdEqual %v4bool %68 %68
        %109 =       OpAll %bool %108
        %110 =       OpLogicalAnd %bool %107 %109
                     OpBranch %98

         %98 = OpLabel
        %111 =   OpPhi %bool %false %86 %110 %97
                 OpSelectionMerge %113 None
                 OpBranchConditional %111 %112 %113

        %112 =     OpLabel
        %114 =       OpExtInst %mat3v3float %5 MatrixInverse %122
        %123 =       OpCompositeExtract %v3float %114 0
        %124 =       OpFUnordNotEqual %v3bool %123 %55
        %125 =       OpAny %bool %124
        %126 =       OpCompositeExtract %v3float %114 1
        %127 =       OpFUnordNotEqual %v3bool %126 %56
        %128 =       OpAny %bool %127
        %129 =       OpLogicalOr %bool %125 %128
        %130 =       OpCompositeExtract %v3float %114 2
        %131 =       OpFUnordNotEqual %v3bool %130 %57
        %132 =       OpAny %bool %131
        %133 =       OpLogicalOr %bool %129 %132
                     OpBranch %113

        %113 = OpLabel
        %134 =   OpPhi %bool %false %98 %133 %112
                 OpSelectionMerge %136 None
                 OpBranchConditional %134 %135 %136

        %135 =     OpLabel
        %138 =       OpCompositeConstruct %v2float %76 %76
        %139 =       OpCompositeConstruct %mat2v2float %138 %138
        %140 =       OpFAdd %v2float %34 %138
        %141 =       OpFAdd %v2float %35 %138
        %142 =       OpCompositeConstruct %mat2v2float %140 %141
        %137 =       OpExtInst %mat2v2float %5 MatrixInverse %142
        %143 =       OpCompositeExtract %v2float %137 0
        %144 =       OpFOrdEqual %v2bool %143 %41
        %145 =       OpAll %bool %144
        %146 =       OpCompositeExtract %v2float %137 1
        %147 =       OpFOrdEqual %v2bool %146 %42
        %148 =       OpAll %bool %147
        %149 =       OpLogicalAnd %bool %145 %148
                     OpBranch %136

        %136 = OpLabel
        %150 =   OpPhi %bool %false %113 %149 %135
                 OpSelectionMerge %152 None
                 OpBranchConditional %150 %151 %152

        %151 =     OpLabel
        %157 =       OpCompositeConstruct %v3float %76 %76 %76
        %158 =       OpCompositeConstruct %mat3v3float %157 %157 %157
        %159 =       OpFAdd %v3float %119 %157
        %160 =       OpFAdd %v3float %154 %157
        %161 =       OpFAdd %v3float %155 %157
        %162 =       OpCompositeConstruct %mat3v3float %159 %160 %161
        %153 =       OpExtInst %mat3v3float %5 MatrixInverse %162
        %163 =       OpCompositeExtract %v3float %153 0
        %164 =       OpFOrdEqual %v3bool %163 %55
        %165 =       OpAll %bool %164
        %166 =       OpCompositeExtract %v3float %153 1
        %167 =       OpFOrdEqual %v3bool %166 %56
        %168 =       OpAll %bool %167
        %169 =       OpLogicalAnd %bool %165 %168
        %170 =       OpCompositeExtract %v3float %153 2
        %171 =       OpFOrdEqual %v3bool %170 %57
        %172 =       OpAll %bool %171
        %173 =       OpLogicalAnd %bool %169 %172
                     OpBranch %152

        %152 = OpLabel
        %174 =   OpPhi %bool %false %136 %173 %151
                 OpSelectionMerge %176 None
                 OpBranchConditional %174 %175 %176

        %175 =     OpLabel
        %183 =       OpCompositeConstruct %v4float %76 %76 %76 %76
        %184 =       OpCompositeConstruct %mat4v4float %183 %183 %183 %183
        %185 =       OpFAdd %v4float %178 %183
        %186 =       OpFAdd %v4float %179 %183
        %187 =       OpFAdd %v4float %180 %183
        %188 =       OpFAdd %v4float %181 %183
        %189 =       OpCompositeConstruct %mat4v4float %185 %186 %187 %188
        %177 =       OpExtInst %mat4v4float %5 MatrixInverse %189
        %190 =       OpCompositeExtract %v4float %177 0
        %191 =       OpFOrdEqual %v4bool %190 %65
        %192 =       OpAll %bool %191
        %193 =       OpCompositeExtract %v4float %177 1
        %194 =       OpFOrdEqual %v4bool %193 %66
        %195 =       OpAll %bool %194
        %196 =       OpLogicalAnd %bool %192 %195
        %197 =       OpCompositeExtract %v4float %177 2
        %198 =       OpFOrdEqual %v4bool %197 %67
        %199 =       OpAll %bool %198
        %200 =       OpLogicalAnd %bool %196 %199
        %201 =       OpCompositeExtract %v4float %177 3
        %202 =       OpFOrdEqual %v4bool %201 %68
        %203 =       OpAll %bool %202
        %204 =       OpLogicalAnd %bool %200 %203
                     OpBranch %176

        %176 = OpLabel
        %205 =   OpPhi %bool %false %152 %204 %175
                 OpSelectionMerge %210 None
                 OpBranchConditional %205 %208 %209

        %208 =     OpLabel
        %211 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %212 =       OpLoad %v4float %211           ; RelaxedPrecision
                     OpStore %206 %212
                     OpBranch %210

        %209 =     OpLabel
        %213 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %215 =       OpLoad %v4float %213           ; RelaxedPrecision
                     OpStore %206 %215
                     OpBranch %210

        %210 = OpLabel
        %216 =   OpLoad %v4float %206               ; RelaxedPrecision
                 OpReturnValue %216
               OpFunctionEnd
