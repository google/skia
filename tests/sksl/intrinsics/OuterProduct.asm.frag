               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %16
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testMatrix2x2"
               OpMemberName %_UniformBuffer 3 "testMatrix3x3"
               OpMemberName %_UniformBuffer 4 "testInputs"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %18
               OpName %main "main"                      ; id %6

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
               OpMemberDecorate %_UniformBuffer 2 ColMajor
               OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 3 Offset 64
               OpMemberDecorate %_UniformBuffer 3 ColMajor
               OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 4 Offset 112
               OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %120 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %206 RelaxedPrecision
               OpDecorate %207 RelaxedPrecision

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
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float %v4float     ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %27 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
      %int_2 = OpConstant %int 2
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
      %int_1 = OpConstant %int 1
    %float_3 = OpConstant %float 3
    %float_6 = OpConstant %float 6
    %float_4 = OpConstant %float 4
    %float_8 = OpConstant %float 8
         %48 = OpConstantComposite %v2float %float_3 %float_6
         %49 = OpConstantComposite %v2float %float_4 %float_8
         %50 = OpConstantComposite %mat2v2float %48 %49
     %v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_3 = OpConstant %int 3
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
   %float_12 = OpConstant %float 12
    %float_5 = OpConstant %float 5
   %float_10 = OpConstant %float 10
   %float_15 = OpConstant %float 15
   %float_18 = OpConstant %float 18
         %76 = OpConstantComposite %v3float %float_4 %float_8 %float_12
         %77 = OpConstantComposite %v3float %float_5 %float_10 %float_15
         %78 = OpConstantComposite %v3float %float_6 %float_12 %float_18
         %79 = OpConstantComposite %mat3v3float %76 %77 %78
     %v3bool = OpTypeVector %bool 3
%mat3v2float = OpTypeMatrix %v2float 3
        %103 = OpConstantComposite %v2float %float_5 %float_10
        %104 = OpConstantComposite %v2float %float_6 %float_12
        %105 = OpConstantComposite %mat3v2float %49 %103 %104
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_4 = OpConstant %int 4
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
        %127 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_2
%mat4v4float = OpTypeMatrix %v4float 4
%float_n1_25 = OpConstant %float -1.25
 %float_0_75 = OpConstant %float 0.75
 %float_2_25 = OpConstant %float 2.25
 %float_n2_5 = OpConstant %float -2.5
  %float_1_5 = OpConstant %float 1.5
  %float_4_5 = OpConstant %float 4.5
        %135 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
        %136 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
        %137 = OpConstantComposite %v4float %float_n2_5 %float_0 %float_1_5 %float_4_5
        %138 = OpConstantComposite %mat4v4float %135 %136 %136 %137
     %v4bool = OpTypeVector %bool 4
        %161 = OpConstantComposite %v2float %float_1 %float_2
%mat2v4float = OpTypeMatrix %v4float 2
        %163 = OpConstantComposite %mat2v4float %135 %137
%mat4v2float = OpTypeMatrix %v2float 4
        %178 = OpConstantComposite %v2float %float_n1_25 %float_n2_5
        %179 = OpConstantComposite %v2float %float_0_75 %float_1_5
        %180 = OpConstantComposite %v2float %float_2_25 %float_4_5
        %181 = OpConstantComposite %mat4v2float %178 %23 %179 %180
%_ptr_Function_v4float = OpTypePointer Function %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %20

         %21 = OpLabel
         %24 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %24 %23
         %26 =   OpFunctionCall %v4float %main %24
                 OpStore %sk_FragColor %26
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %27         ; RelaxedPrecision
         %28 = OpFunctionParameter %_ptr_Function_v2float

         %29 = OpLabel
        %198 =   OpVariable %_ptr_Function_v4float Function
         %33 =   OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
         %37 =   OpAccessChain %_ptr_Uniform_v2float %33 %int_0
         %39 =   OpLoad %v2float %37
         %40 =   OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
         %42 =   OpAccessChain %_ptr_Uniform_v2float %40 %int_1
         %43 =   OpLoad %v2float %42
         %32 =   OpOuterProduct %mat2v2float %39 %43
         %52 =   OpCompositeExtract %v2float %32 0
         %53 =   OpFOrdEqual %v2bool %52 %48
         %54 =   OpAll %bool %53
         %55 =   OpCompositeExtract %v2float %32 1
         %56 =   OpFOrdEqual %v2bool %55 %49
         %57 =   OpAll %bool %56
         %58 =   OpLogicalAnd %bool %54 %57
                 OpSelectionMerge %60 None
                 OpBranchConditional %58 %59 %60

         %59 =     OpLabel
         %62 =       OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_3
         %65 =       OpAccessChain %_ptr_Uniform_v3float %62 %int_0
         %67 =       OpLoad %v3float %65
         %68 =       OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_3
         %69 =       OpAccessChain %_ptr_Uniform_v3float %68 %int_1
         %70 =       OpLoad %v3float %69
         %61 =       OpOuterProduct %mat3v3float %67 %70
         %81 =       OpCompositeExtract %v3float %61 0
         %82 =       OpFOrdEqual %v3bool %81 %76
         %83 =       OpAll %bool %82
         %84 =       OpCompositeExtract %v3float %61 1
         %85 =       OpFOrdEqual %v3bool %84 %77
         %86 =       OpAll %bool %85
         %87 =       OpLogicalAnd %bool %83 %86
         %88 =       OpCompositeExtract %v3float %61 2
         %89 =       OpFOrdEqual %v3bool %88 %78
         %90 =       OpAll %bool %89
         %91 =       OpLogicalAnd %bool %87 %90
                     OpBranch %60

         %60 = OpLabel
         %92 =   OpPhi %bool %false %29 %91 %59
                 OpSelectionMerge %94 None
                 OpBranchConditional %92 %93 %94

         %93 =     OpLabel
         %96 =       OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
         %97 =       OpAccessChain %_ptr_Uniform_v2float %96 %int_0
         %98 =       OpLoad %v2float %97
         %99 =       OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_3
        %100 =       OpAccessChain %_ptr_Uniform_v3float %99 %int_1
        %101 =       OpLoad %v3float %100
         %95 =       OpOuterProduct %mat3v2float %98 %101
        %106 =       OpCompositeExtract %v2float %95 0
        %107 =       OpFOrdEqual %v2bool %106 %49
        %108 =       OpAll %bool %107
        %109 =       OpCompositeExtract %v2float %95 1
        %110 =       OpFOrdEqual %v2bool %109 %103
        %111 =       OpAll %bool %110
        %112 =       OpLogicalAnd %bool %108 %111
        %113 =       OpCompositeExtract %v2float %95 2
        %114 =       OpFOrdEqual %v2bool %113 %104
        %115 =       OpAll %bool %114
        %116 =       OpLogicalAnd %bool %112 %115
                     OpBranch %94

         %94 = OpLabel
        %117 =   OpPhi %bool %false %60 %116 %93
                 OpSelectionMerge %119 None
                 OpBranchConditional %117 %118 %119

        %118 =     OpLabel
        %121 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %124 =       OpLoad %v4float %121           ; RelaxedPrecision
        %120 =       OpOuterProduct %mat4v4float %124 %127  ; RelaxedPrecision
        %140 =       OpCompositeExtract %v4float %120 0
        %141 =       OpFOrdEqual %v4bool %140 %135
        %142 =       OpAll %bool %141
        %143 =       OpCompositeExtract %v4float %120 1
        %144 =       OpFOrdEqual %v4bool %143 %136
        %145 =       OpAll %bool %144
        %146 =       OpLogicalAnd %bool %142 %145
        %147 =       OpCompositeExtract %v4float %120 2
        %148 =       OpFOrdEqual %v4bool %147 %136
        %149 =       OpAll %bool %148
        %150 =       OpLogicalAnd %bool %146 %149
        %151 =       OpCompositeExtract %v4float %120 3
        %152 =       OpFOrdEqual %v4bool %151 %137
        %153 =       OpAll %bool %152
        %154 =       OpLogicalAnd %bool %150 %153
                     OpBranch %119

        %119 = OpLabel
        %155 =   OpPhi %bool %false %94 %154 %118
                 OpSelectionMerge %157 None
                 OpBranchConditional %155 %156 %157

        %156 =     OpLabel
        %159 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %160 =       OpLoad %v4float %159           ; RelaxedPrecision
        %158 =       OpOuterProduct %mat2v4float %160 %161
        %164 =       OpCompositeExtract %v4float %158 0
        %165 =       OpFOrdEqual %v4bool %164 %135
        %166 =       OpAll %bool %165
        %167 =       OpCompositeExtract %v4float %158 1
        %168 =       OpFOrdEqual %v4bool %167 %137
        %169 =       OpAll %bool %168
        %170 =       OpLogicalAnd %bool %166 %169
                     OpBranch %157

        %157 = OpLabel
        %171 =   OpPhi %bool %false %119 %170 %156
                 OpSelectionMerge %173 None
                 OpBranchConditional %171 %172 %173

        %172 =     OpLabel
        %175 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %176 =       OpLoad %v4float %175           ; RelaxedPrecision
        %174 =       OpOuterProduct %mat4v2float %161 %176
        %182 =       OpCompositeExtract %v2float %174 0
        %183 =       OpFOrdEqual %v2bool %182 %178
        %184 =       OpAll %bool %183
        %185 =       OpCompositeExtract %v2float %174 1
        %186 =       OpFOrdEqual %v2bool %185 %23
        %187 =       OpAll %bool %186
        %188 =       OpLogicalAnd %bool %184 %187
        %189 =       OpCompositeExtract %v2float %174 2
        %190 =       OpFOrdEqual %v2bool %189 %179
        %191 =       OpAll %bool %190
        %192 =       OpLogicalAnd %bool %188 %191
        %193 =       OpCompositeExtract %v2float %174 3
        %194 =       OpFOrdEqual %v2bool %193 %180
        %195 =       OpAll %bool %194
        %196 =       OpLogicalAnd %bool %192 %195
                     OpBranch %173

        %173 = OpLabel
        %197 =   OpPhi %bool %false %157 %196 %172
                 OpSelectionMerge %202 None
                 OpBranchConditional %197 %200 %201

        %200 =     OpLabel
        %203 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %204 =       OpLoad %v4float %203           ; RelaxedPrecision
                     OpStore %198 %204
                     OpBranch %202

        %201 =     OpLabel
        %205 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %206 =       OpLoad %v4float %205           ; RelaxedPrecision
                     OpStore %198 %206
                     OpBranch %202

        %202 = OpLabel
        %207 =   OpLoad %v4float %198               ; RelaxedPrecision
                 OpReturnValue %207
               OpFunctionEnd
