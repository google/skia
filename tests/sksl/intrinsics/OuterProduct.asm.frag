               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testMatrix2x2"
               OpMemberName %_UniformBuffer 3 "testMatrix3x3"
               OpMemberName %_UniformBuffer 4 "testInputs"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %119 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %206 RelaxedPrecision
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
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %26 = OpTypeFunction %v4float %_ptr_Function_v2float
      %false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
      %int_1 = OpConstant %int 1
    %float_3 = OpConstant %float 3
    %float_6 = OpConstant %float 6
    %float_4 = OpConstant %float 4
    %float_8 = OpConstant %float 8
         %47 = OpConstantComposite %v2float %float_3 %float_6
         %48 = OpConstantComposite %v2float %float_4 %float_8
         %49 = OpConstantComposite %mat2v2float %47 %48
     %v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_3 = OpConstant %int 3
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
   %float_12 = OpConstant %float 12
    %float_5 = OpConstant %float 5
   %float_10 = OpConstant %float 10
   %float_15 = OpConstant %float 15
   %float_18 = OpConstant %float 18
         %75 = OpConstantComposite %v3float %float_4 %float_8 %float_12
         %76 = OpConstantComposite %v3float %float_5 %float_10 %float_15
         %77 = OpConstantComposite %v3float %float_6 %float_12 %float_18
         %78 = OpConstantComposite %mat3v3float %75 %76 %77
     %v3bool = OpTypeVector %bool 3
%mat3v2float = OpTypeMatrix %v2float 3
        %102 = OpConstantComposite %v2float %float_5 %float_10
        %103 = OpConstantComposite %v2float %float_6 %float_12
        %104 = OpConstantComposite %mat3v2float %48 %102 %103
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_4 = OpConstant %int 4
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
        %126 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_2
%mat4v4float = OpTypeMatrix %v4float 4
%float_n1_25 = OpConstant %float -1.25
 %float_0_75 = OpConstant %float 0.75
 %float_2_25 = OpConstant %float 2.25
 %float_n2_5 = OpConstant %float -2.5
  %float_1_5 = OpConstant %float 1.5
  %float_4_5 = OpConstant %float 4.5
        %134 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
        %135 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
        %136 = OpConstantComposite %v4float %float_n2_5 %float_0 %float_1_5 %float_4_5
        %137 = OpConstantComposite %mat4v4float %134 %135 %135 %136
     %v4bool = OpTypeVector %bool 4
        %160 = OpConstantComposite %v2float %float_1 %float_2
%mat2v4float = OpTypeMatrix %v4float 2
        %162 = OpConstantComposite %mat2v4float %134 %136
%mat4v2float = OpTypeMatrix %v2float 4
        %177 = OpConstantComposite %v2float %float_n1_25 %float_n2_5
        %178 = OpConstantComposite %v2float %float_0_75 %float_1_5
        %179 = OpConstantComposite %v2float %float_2_25 %float_4_5
        %180 = OpConstantComposite %mat4v2float %177 %22 %178 %179
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %19
         %20 = OpLabel
         %23 = OpVariable %_ptr_Function_v2float Function
               OpStore %23 %22
         %25 = OpFunctionCall %v4float %main %23
               OpStore %sk_FragColor %25
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %26
         %27 = OpFunctionParameter %_ptr_Function_v2float
         %28 = OpLabel
        %197 = OpVariable %_ptr_Function_v4float Function
         %31 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
         %36 = OpAccessChain %_ptr_Uniform_v2float %31 %int_0
         %38 = OpLoad %v2float %36
         %39 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
         %41 = OpAccessChain %_ptr_Uniform_v2float %39 %int_1
         %42 = OpLoad %v2float %41
         %30 = OpOuterProduct %mat2v2float %38 %42
         %51 = OpCompositeExtract %v2float %30 0
         %52 = OpFOrdEqual %v2bool %51 %47
         %53 = OpAll %bool %52
         %54 = OpCompositeExtract %v2float %30 1
         %55 = OpFOrdEqual %v2bool %54 %48
         %56 = OpAll %bool %55
         %57 = OpLogicalAnd %bool %53 %56
               OpSelectionMerge %59 None
               OpBranchConditional %57 %58 %59
         %58 = OpLabel
         %61 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
         %64 = OpAccessChain %_ptr_Uniform_v3float %61 %int_0
         %66 = OpLoad %v3float %64
         %67 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
         %68 = OpAccessChain %_ptr_Uniform_v3float %67 %int_1
         %69 = OpLoad %v3float %68
         %60 = OpOuterProduct %mat3v3float %66 %69
         %80 = OpCompositeExtract %v3float %60 0
         %81 = OpFOrdEqual %v3bool %80 %75
         %82 = OpAll %bool %81
         %83 = OpCompositeExtract %v3float %60 1
         %84 = OpFOrdEqual %v3bool %83 %76
         %85 = OpAll %bool %84
         %86 = OpLogicalAnd %bool %82 %85
         %87 = OpCompositeExtract %v3float %60 2
         %88 = OpFOrdEqual %v3bool %87 %77
         %89 = OpAll %bool %88
         %90 = OpLogicalAnd %bool %86 %89
               OpBranch %59
         %59 = OpLabel
         %91 = OpPhi %bool %false %28 %90 %58
               OpSelectionMerge %93 None
               OpBranchConditional %91 %92 %93
         %92 = OpLabel
         %95 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
         %96 = OpAccessChain %_ptr_Uniform_v2float %95 %int_0
         %97 = OpLoad %v2float %96
         %98 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
         %99 = OpAccessChain %_ptr_Uniform_v3float %98 %int_1
        %100 = OpLoad %v3float %99
         %94 = OpOuterProduct %mat3v2float %97 %100
        %105 = OpCompositeExtract %v2float %94 0
        %106 = OpFOrdEqual %v2bool %105 %48
        %107 = OpAll %bool %106
        %108 = OpCompositeExtract %v2float %94 1
        %109 = OpFOrdEqual %v2bool %108 %102
        %110 = OpAll %bool %109
        %111 = OpLogicalAnd %bool %107 %110
        %112 = OpCompositeExtract %v2float %94 2
        %113 = OpFOrdEqual %v2bool %112 %103
        %114 = OpAll %bool %113
        %115 = OpLogicalAnd %bool %111 %114
               OpBranch %93
         %93 = OpLabel
        %116 = OpPhi %bool %false %59 %115 %92
               OpSelectionMerge %118 None
               OpBranchConditional %116 %117 %118
        %117 = OpLabel
        %120 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %123 = OpLoad %v4float %120
        %119 = OpOuterProduct %mat4v4float %123 %126
        %139 = OpCompositeExtract %v4float %119 0
        %140 = OpFOrdEqual %v4bool %139 %134
        %141 = OpAll %bool %140
        %142 = OpCompositeExtract %v4float %119 1
        %143 = OpFOrdEqual %v4bool %142 %135
        %144 = OpAll %bool %143
        %145 = OpLogicalAnd %bool %141 %144
        %146 = OpCompositeExtract %v4float %119 2
        %147 = OpFOrdEqual %v4bool %146 %135
        %148 = OpAll %bool %147
        %149 = OpLogicalAnd %bool %145 %148
        %150 = OpCompositeExtract %v4float %119 3
        %151 = OpFOrdEqual %v4bool %150 %136
        %152 = OpAll %bool %151
        %153 = OpLogicalAnd %bool %149 %152
               OpBranch %118
        %118 = OpLabel
        %154 = OpPhi %bool %false %93 %153 %117
               OpSelectionMerge %156 None
               OpBranchConditional %154 %155 %156
        %155 = OpLabel
        %158 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %159 = OpLoad %v4float %158
        %157 = OpOuterProduct %mat2v4float %159 %160
        %163 = OpCompositeExtract %v4float %157 0
        %164 = OpFOrdEqual %v4bool %163 %134
        %165 = OpAll %bool %164
        %166 = OpCompositeExtract %v4float %157 1
        %167 = OpFOrdEqual %v4bool %166 %136
        %168 = OpAll %bool %167
        %169 = OpLogicalAnd %bool %165 %168
               OpBranch %156
        %156 = OpLabel
        %170 = OpPhi %bool %false %118 %169 %155
               OpSelectionMerge %172 None
               OpBranchConditional %170 %171 %172
        %171 = OpLabel
        %174 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %175 = OpLoad %v4float %174
        %173 = OpOuterProduct %mat4v2float %160 %175
        %181 = OpCompositeExtract %v2float %173 0
        %182 = OpFOrdEqual %v2bool %181 %177
        %183 = OpAll %bool %182
        %184 = OpCompositeExtract %v2float %173 1
        %185 = OpFOrdEqual %v2bool %184 %22
        %186 = OpAll %bool %185
        %187 = OpLogicalAnd %bool %183 %186
        %188 = OpCompositeExtract %v2float %173 2
        %189 = OpFOrdEqual %v2bool %188 %178
        %190 = OpAll %bool %189
        %191 = OpLogicalAnd %bool %187 %190
        %192 = OpCompositeExtract %v2float %173 3
        %193 = OpFOrdEqual %v2bool %192 %179
        %194 = OpAll %bool %193
        %195 = OpLogicalAnd %bool %191 %194
               OpBranch %172
        %172 = OpLabel
        %196 = OpPhi %bool %false %156 %195 %171
               OpSelectionMerge %201 None
               OpBranchConditional %196 %199 %200
        %199 = OpLabel
        %202 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %203 = OpLoad %v4float %202
               OpStore %197 %203
               OpBranch %201
        %200 = OpLabel
        %204 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %205 = OpLoad %v4float %204
               OpStore %197 %205
               OpBranch %201
        %201 = OpLabel
        %206 = OpLoad %v4float %197
               OpReturnValue %206
               OpFunctionEnd
