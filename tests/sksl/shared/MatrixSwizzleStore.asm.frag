               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testMatrix3x3"
               OpMemberName %_UniformBuffer 3 "testMatrix4x4"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %test4x4_b "test4x4_b"
               OpName %matrix "matrix"
               OpName %values "values"
               OpName %index "index"
               OpName %main "main"
               OpName %_0_matrix "_0_matrix"
               OpName %_1_values "_1_values"
               OpName %_2_index "_2_index"
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
               OpMemberDecorate %_UniformBuffer 3 Offset 80
               OpMemberDecorate %_UniformBuffer 3 ColMajor
               OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
               OpDecorate %_UniformBuffer Block
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %155 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat3v3float %mat4v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %25 = OpTypeFunction %bool
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_4 = OpConstant %float 4
    %float_3 = OpConstant %float 3
    %float_2 = OpConstant %float 2
    %float_1 = OpConstant %float 1
         %35 = OpConstantComposite %v4float %float_4 %float_3 %float_2 %float_1
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_4 = OpConstant %int 4
         %61 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
      %int_1 = OpConstant %int 1
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
      %int_3 = OpConstant %int 3
     %v4bool = OpTypeVector %bool 4
         %91 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %98 = OpConstantComposite %v3float %float_3 %float_2 %float_1
%_ptr_Function_float = OpTypePointer Function %float
        %120 = OpConstantComposite %v3float %float_3 %float_3 %float_3
      %false = OpConstantFalse %bool
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_2 = OpConstant %int 2
     %v3bool = OpTypeVector %bool 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %16
         %17 = OpLabel
         %21 = OpVariable %_ptr_Function_v2float Function
               OpStore %21 %20
         %23 = OpFunctionCall %v4float %main %21
               OpStore %sk_FragColor %23
               OpReturn
               OpFunctionEnd
  %test4x4_b = OpFunction %bool None %25
         %26 = OpLabel
     %matrix = OpVariable %_ptr_Function_mat4v4float Function
     %values = OpVariable %_ptr_Function_v4float Function
      %index = OpVariable %_ptr_Function_int Function
               OpStore %values %35
               OpStore %index %int_0
               OpBranch %40
         %40 = OpLabel
               OpLoopMerge %44 %43 None
               OpBranch %41
         %41 = OpLabel
         %45 = OpLoad %int %index
         %47 = OpSLessThan %bool %45 %int_4
               OpBranchConditional %47 %42 %44
         %42 = OpLabel
         %48 = OpLoad %v4float %values
         %49 = OpVectorShuffle %v2float %48 %48 0 3
         %50 = OpLoad %int %index
         %51 = OpAccessChain %_ptr_Function_v4float %matrix %50
         %52 = OpLoad %v4float %51
         %53 = OpVectorShuffle %v4float %52 %49 5 1 2 4
               OpStore %51 %53
         %54 = OpLoad %v4float %values
         %55 = OpVectorShuffle %v2float %54 %54 1 2
         %56 = OpLoad %int %index
         %57 = OpAccessChain %_ptr_Function_v4float %matrix %56
         %58 = OpLoad %v4float %57
         %59 = OpVectorShuffle %v4float %58 %55 0 5 4 3
               OpStore %57 %59
         %60 = OpLoad %v4float %values
         %62 = OpFAdd %v4float %60 %61
               OpStore %values %62
               OpBranch %43
         %43 = OpLabel
         %64 = OpLoad %int %index
         %65 = OpIAdd %int %64 %int_1
               OpStore %index %65
               OpBranch %40
         %44 = OpLabel
         %66 = OpLoad %mat4v4float %matrix
         %67 = OpAccessChain %_ptr_Uniform_mat4v4float %8 %int_3
         %70 = OpLoad %mat4v4float %67
         %72 = OpCompositeExtract %v4float %66 0
         %73 = OpCompositeExtract %v4float %70 0
         %74 = OpFOrdEqual %v4bool %72 %73
         %75 = OpAll %bool %74
         %76 = OpCompositeExtract %v4float %66 1
         %77 = OpCompositeExtract %v4float %70 1
         %78 = OpFOrdEqual %v4bool %76 %77
         %79 = OpAll %bool %78
         %80 = OpLogicalAnd %bool %75 %79
         %81 = OpCompositeExtract %v4float %66 2
         %82 = OpCompositeExtract %v4float %70 2
         %83 = OpFOrdEqual %v4bool %81 %82
         %84 = OpAll %bool %83
         %85 = OpLogicalAnd %bool %80 %84
         %86 = OpCompositeExtract %v4float %66 3
         %87 = OpCompositeExtract %v4float %70 3
         %88 = OpFOrdEqual %v4bool %86 %87
         %89 = OpAll %bool %88
         %90 = OpLogicalAnd %bool %85 %89
               OpReturnValue %90
               OpFunctionEnd
       %main = OpFunction %v4float None %91
         %92 = OpFunctionParameter %_ptr_Function_v2float
         %93 = OpLabel
  %_0_matrix = OpVariable %_ptr_Function_mat3v3float Function
  %_1_values = OpVariable %_ptr_Function_v3float Function
   %_2_index = OpVariable %_ptr_Function_int Function
        %149 = OpVariable %_ptr_Function_v4float Function
               OpStore %_1_values %98
               OpStore %_2_index %int_0
               OpBranch %100
        %100 = OpLabel
               OpLoopMerge %104 %103 None
               OpBranch %101
        %101 = OpLabel
        %105 = OpLoad %int %_2_index
        %106 = OpSLessThan %bool %105 %int_3
               OpBranchConditional %106 %102 %104
        %102 = OpLabel
        %107 = OpLoad %v3float %_1_values
        %108 = OpVectorShuffle %v2float %107 %107 0 2
        %109 = OpLoad %int %_2_index
        %110 = OpAccessChain %_ptr_Function_v3float %_0_matrix %109
        %111 = OpLoad %v3float %110
        %112 = OpVectorShuffle %v3float %111 %108 4 1 3
               OpStore %110 %112
        %113 = OpLoad %v3float %_1_values
        %114 = OpCompositeExtract %float %113 1
        %115 = OpLoad %int %_2_index
        %116 = OpAccessChain %_ptr_Function_v3float %_0_matrix %115
        %117 = OpAccessChain %_ptr_Function_float %116 %int_1
               OpStore %117 %114
        %119 = OpLoad %v3float %_1_values
        %121 = OpFAdd %v3float %119 %120
               OpStore %_1_values %121
               OpBranch %103
        %103 = OpLabel
        %122 = OpLoad %int %_2_index
        %123 = OpIAdd %int %122 %int_1
               OpStore %_2_index %123
               OpBranch %100
        %104 = OpLabel
        %125 = OpLoad %mat3v3float %_0_matrix
        %126 = OpAccessChain %_ptr_Uniform_mat3v3float %8 %int_2
        %129 = OpLoad %mat3v3float %126
        %131 = OpCompositeExtract %v3float %125 0
        %132 = OpCompositeExtract %v3float %129 0
        %133 = OpFOrdEqual %v3bool %131 %132
        %134 = OpAll %bool %133
        %135 = OpCompositeExtract %v3float %125 1
        %136 = OpCompositeExtract %v3float %129 1
        %137 = OpFOrdEqual %v3bool %135 %136
        %138 = OpAll %bool %137
        %139 = OpLogicalAnd %bool %134 %138
        %140 = OpCompositeExtract %v3float %125 2
        %141 = OpCompositeExtract %v3float %129 2
        %142 = OpFOrdEqual %v3bool %140 %141
        %143 = OpAll %bool %142
        %144 = OpLogicalAnd %bool %139 %143
               OpSelectionMerge %146 None
               OpBranchConditional %144 %145 %146
        %145 = OpLabel
        %147 = OpFunctionCall %bool %test4x4_b
               OpBranch %146
        %146 = OpLabel
        %148 = OpPhi %bool %false %104 %147 %145
               OpSelectionMerge %152 None
               OpBranchConditional %148 %150 %151
        %150 = OpLabel
        %153 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %155 = OpLoad %v4float %153
               OpStore %149 %155
               OpBranch %152
        %151 = OpLabel
        %156 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %157 = OpLoad %v4float %156
               OpStore %149 %157
               OpBranch %152
        %152 = OpLabel
        %158 = OpLoad %v4float %149
               OpReturnValue %158
               OpFunctionEnd
