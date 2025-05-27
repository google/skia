               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testMatrix2x2"
               OpMemberName %_UniformBuffer 3 "testMatrix3x3"
               OpMemberName %_UniformBuffer 4 "testInputs"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %117 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
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
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
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
         %45 = OpConstantComposite %v2float %float_3 %float_6
         %46 = OpConstantComposite %v2float %float_4 %float_8
         %47 = OpConstantComposite %mat2v2float %45 %46
     %v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_3 = OpConstant %int 3
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
   %float_12 = OpConstant %float 12
    %float_5 = OpConstant %float 5
   %float_10 = OpConstant %float 10
   %float_15 = OpConstant %float 15
   %float_18 = OpConstant %float 18
         %73 = OpConstantComposite %v3float %float_4 %float_8 %float_12
         %74 = OpConstantComposite %v3float %float_5 %float_10 %float_15
         %75 = OpConstantComposite %v3float %float_6 %float_12 %float_18
         %76 = OpConstantComposite %mat3v3float %73 %74 %75
     %v3bool = OpTypeVector %bool 3
%mat3v2float = OpTypeMatrix %v2float 3
        %100 = OpConstantComposite %v2float %float_5 %float_10
        %101 = OpConstantComposite %v2float %float_6 %float_12
        %102 = OpConstantComposite %mat3v2float %46 %100 %101
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_4 = OpConstant %int 4
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
        %124 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_2
%mat4v4float = OpTypeMatrix %v4float 4
%float_n1_25 = OpConstant %float -1.25
 %float_0_75 = OpConstant %float 0.75
 %float_2_25 = OpConstant %float 2.25
 %float_n2_5 = OpConstant %float -2.5
  %float_1_5 = OpConstant %float 1.5
  %float_4_5 = OpConstant %float 4.5
        %132 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
        %133 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
        %134 = OpConstantComposite %v4float %float_n2_5 %float_0 %float_1_5 %float_4_5
        %135 = OpConstantComposite %mat4v4float %132 %133 %133 %134
     %v4bool = OpTypeVector %bool 4
        %158 = OpConstantComposite %v2float %float_1 %float_2
%mat2v4float = OpTypeMatrix %v4float 2
        %160 = OpConstantComposite %mat2v4float %132 %134
%mat4v2float = OpTypeMatrix %v2float 4
        %175 = OpConstantComposite %v2float %float_n1_25 %float_n2_5
        %176 = OpConstantComposite %v2float %float_0_75 %float_1_5
        %177 = OpConstantComposite %v2float %float_2_25 %float_4_5
        %178 = OpConstantComposite %mat4v2float %175 %19 %176 %177
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %16
         %17 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
        %195 = OpVariable %_ptr_Function_v4float Function
         %29 = OpAccessChain %_ptr_Uniform_mat2v2float %7 %int_2
         %34 = OpAccessChain %_ptr_Uniform_v2float %29 %int_0
         %36 = OpLoad %v2float %34
         %37 = OpAccessChain %_ptr_Uniform_mat2v2float %7 %int_2
         %39 = OpAccessChain %_ptr_Uniform_v2float %37 %int_1
         %40 = OpLoad %v2float %39
         %28 = OpOuterProduct %mat2v2float %36 %40
         %49 = OpCompositeExtract %v2float %28 0
         %50 = OpFOrdEqual %v2bool %49 %45
         %51 = OpAll %bool %50
         %52 = OpCompositeExtract %v2float %28 1
         %53 = OpFOrdEqual %v2bool %52 %46
         %54 = OpAll %bool %53
         %55 = OpLogicalAnd %bool %51 %54
               OpSelectionMerge %57 None
               OpBranchConditional %55 %56 %57
         %56 = OpLabel
         %59 = OpAccessChain %_ptr_Uniform_mat3v3float %7 %int_3
         %62 = OpAccessChain %_ptr_Uniform_v3float %59 %int_0
         %64 = OpLoad %v3float %62
         %65 = OpAccessChain %_ptr_Uniform_mat3v3float %7 %int_3
         %66 = OpAccessChain %_ptr_Uniform_v3float %65 %int_1
         %67 = OpLoad %v3float %66
         %58 = OpOuterProduct %mat3v3float %64 %67
         %78 = OpCompositeExtract %v3float %58 0
         %79 = OpFOrdEqual %v3bool %78 %73
         %80 = OpAll %bool %79
         %81 = OpCompositeExtract %v3float %58 1
         %82 = OpFOrdEqual %v3bool %81 %74
         %83 = OpAll %bool %82
         %84 = OpLogicalAnd %bool %80 %83
         %85 = OpCompositeExtract %v3float %58 2
         %86 = OpFOrdEqual %v3bool %85 %75
         %87 = OpAll %bool %86
         %88 = OpLogicalAnd %bool %84 %87
               OpBranch %57
         %57 = OpLabel
         %89 = OpPhi %bool %false %25 %88 %56
               OpSelectionMerge %91 None
               OpBranchConditional %89 %90 %91
         %90 = OpLabel
         %93 = OpAccessChain %_ptr_Uniform_mat2v2float %7 %int_2
         %94 = OpAccessChain %_ptr_Uniform_v2float %93 %int_0
         %95 = OpLoad %v2float %94
         %96 = OpAccessChain %_ptr_Uniform_mat3v3float %7 %int_3
         %97 = OpAccessChain %_ptr_Uniform_v3float %96 %int_1
         %98 = OpLoad %v3float %97
         %92 = OpOuterProduct %mat3v2float %95 %98
        %103 = OpCompositeExtract %v2float %92 0
        %104 = OpFOrdEqual %v2bool %103 %46
        %105 = OpAll %bool %104
        %106 = OpCompositeExtract %v2float %92 1
        %107 = OpFOrdEqual %v2bool %106 %100
        %108 = OpAll %bool %107
        %109 = OpLogicalAnd %bool %105 %108
        %110 = OpCompositeExtract %v2float %92 2
        %111 = OpFOrdEqual %v2bool %110 %101
        %112 = OpAll %bool %111
        %113 = OpLogicalAnd %bool %109 %112
               OpBranch %91
         %91 = OpLabel
        %114 = OpPhi %bool %false %57 %113 %90
               OpSelectionMerge %116 None
               OpBranchConditional %114 %115 %116
        %115 = OpLabel
        %118 = OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %121 = OpLoad %v4float %118
        %117 = OpOuterProduct %mat4v4float %121 %124
        %137 = OpCompositeExtract %v4float %117 0
        %138 = OpFOrdEqual %v4bool %137 %132
        %139 = OpAll %bool %138
        %140 = OpCompositeExtract %v4float %117 1
        %141 = OpFOrdEqual %v4bool %140 %133
        %142 = OpAll %bool %141
        %143 = OpLogicalAnd %bool %139 %142
        %144 = OpCompositeExtract %v4float %117 2
        %145 = OpFOrdEqual %v4bool %144 %133
        %146 = OpAll %bool %145
        %147 = OpLogicalAnd %bool %143 %146
        %148 = OpCompositeExtract %v4float %117 3
        %149 = OpFOrdEqual %v4bool %148 %134
        %150 = OpAll %bool %149
        %151 = OpLogicalAnd %bool %147 %150
               OpBranch %116
        %116 = OpLabel
        %152 = OpPhi %bool %false %91 %151 %115
               OpSelectionMerge %154 None
               OpBranchConditional %152 %153 %154
        %153 = OpLabel
        %156 = OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %157 = OpLoad %v4float %156
        %155 = OpOuterProduct %mat2v4float %157 %158
        %161 = OpCompositeExtract %v4float %155 0
        %162 = OpFOrdEqual %v4bool %161 %132
        %163 = OpAll %bool %162
        %164 = OpCompositeExtract %v4float %155 1
        %165 = OpFOrdEqual %v4bool %164 %134
        %166 = OpAll %bool %165
        %167 = OpLogicalAnd %bool %163 %166
               OpBranch %154
        %154 = OpLabel
        %168 = OpPhi %bool %false %116 %167 %153
               OpSelectionMerge %170 None
               OpBranchConditional %168 %169 %170
        %169 = OpLabel
        %172 = OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %173 = OpLoad %v4float %172
        %171 = OpOuterProduct %mat4v2float %158 %173
        %179 = OpCompositeExtract %v2float %171 0
        %180 = OpFOrdEqual %v2bool %179 %175
        %181 = OpAll %bool %180
        %182 = OpCompositeExtract %v2float %171 1
        %183 = OpFOrdEqual %v2bool %182 %19
        %184 = OpAll %bool %183
        %185 = OpLogicalAnd %bool %181 %184
        %186 = OpCompositeExtract %v2float %171 2
        %187 = OpFOrdEqual %v2bool %186 %176
        %188 = OpAll %bool %187
        %189 = OpLogicalAnd %bool %185 %188
        %190 = OpCompositeExtract %v2float %171 3
        %191 = OpFOrdEqual %v2bool %190 %177
        %192 = OpAll %bool %191
        %193 = OpLogicalAnd %bool %189 %192
               OpBranch %170
        %170 = OpLabel
        %194 = OpPhi %bool %false %154 %193 %169
               OpSelectionMerge %199 None
               OpBranchConditional %194 %197 %198
        %197 = OpLabel
        %200 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %201 = OpLoad %v4float %200
               OpStore %195 %201
               OpBranch %199
        %198 = OpLabel
        %202 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %203 = OpLoad %v4float %202
               OpStore %195 %203
               OpBranch %199
        %199 = OpLabel
        %204 = OpLoad %v4float %195
               OpReturnValue %204
               OpFunctionEnd
