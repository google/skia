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
               OpMemberDecorate %_UniformBuffer 3 Offset 80
               OpMemberDecorate %_UniformBuffer 3 ColMajor
               OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %157 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat3v3float %mat4v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %27 = OpTypeFunction %bool
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_4 = OpConstant %float 4
    %float_3 = OpConstant %float 3
    %float_2 = OpConstant %float 2
    %float_1 = OpConstant %float 1
         %37 = OpConstantComposite %v4float %float_4 %float_3 %float_2 %float_1
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_4 = OpConstant %int 4
         %63 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
      %int_1 = OpConstant %int 1
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
      %int_3 = OpConstant %int 3
     %v4bool = OpTypeVector %bool 4
         %93 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
        %100 = OpConstantComposite %v3float %float_3 %float_2 %float_1
%_ptr_Function_float = OpTypePointer Function %float
        %122 = OpConstantComposite %v3float %float_3 %float_3 %float_3
      %false = OpConstantFalse %bool
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_2 = OpConstant %int 2
     %v3bool = OpTypeVector %bool 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %19
         %20 = OpLabel
         %24 = OpVariable %_ptr_Function_v2float Function
               OpStore %24 %23
         %26 = OpFunctionCall %v4float %main %24
               OpStore %sk_FragColor %26
               OpReturn
               OpFunctionEnd
  %test4x4_b = OpFunction %bool None %27
         %28 = OpLabel
     %matrix = OpVariable %_ptr_Function_mat4v4float Function
     %values = OpVariable %_ptr_Function_v4float Function
      %index = OpVariable %_ptr_Function_int Function
               OpStore %values %37
               OpStore %index %int_0
               OpBranch %42
         %42 = OpLabel
               OpLoopMerge %46 %45 None
               OpBranch %43
         %43 = OpLabel
         %47 = OpLoad %int %index
         %49 = OpSLessThan %bool %47 %int_4
               OpBranchConditional %49 %44 %46
         %44 = OpLabel
         %50 = OpLoad %v4float %values
         %51 = OpVectorShuffle %v2float %50 %50 0 3
         %52 = OpLoad %int %index
         %53 = OpAccessChain %_ptr_Function_v4float %matrix %52
         %54 = OpLoad %v4float %53
         %55 = OpVectorShuffle %v4float %54 %51 5 1 2 4
               OpStore %53 %55
         %56 = OpLoad %v4float %values
         %57 = OpVectorShuffle %v2float %56 %56 1 2
         %58 = OpLoad %int %index
         %59 = OpAccessChain %_ptr_Function_v4float %matrix %58
         %60 = OpLoad %v4float %59
         %61 = OpVectorShuffle %v4float %60 %57 0 5 4 3
               OpStore %59 %61
         %62 = OpLoad %v4float %values
         %64 = OpFAdd %v4float %62 %63
               OpStore %values %64
               OpBranch %45
         %45 = OpLabel
         %66 = OpLoad %int %index
         %67 = OpIAdd %int %66 %int_1
               OpStore %index %67
               OpBranch %42
         %46 = OpLabel
         %68 = OpLoad %mat4v4float %matrix
         %69 = OpAccessChain %_ptr_Uniform_mat4v4float %11 %int_3
         %72 = OpLoad %mat4v4float %69
         %74 = OpCompositeExtract %v4float %68 0
         %75 = OpCompositeExtract %v4float %72 0
         %76 = OpFOrdEqual %v4bool %74 %75
         %77 = OpAll %bool %76
         %78 = OpCompositeExtract %v4float %68 1
         %79 = OpCompositeExtract %v4float %72 1
         %80 = OpFOrdEqual %v4bool %78 %79
         %81 = OpAll %bool %80
         %82 = OpLogicalAnd %bool %77 %81
         %83 = OpCompositeExtract %v4float %68 2
         %84 = OpCompositeExtract %v4float %72 2
         %85 = OpFOrdEqual %v4bool %83 %84
         %86 = OpAll %bool %85
         %87 = OpLogicalAnd %bool %82 %86
         %88 = OpCompositeExtract %v4float %68 3
         %89 = OpCompositeExtract %v4float %72 3
         %90 = OpFOrdEqual %v4bool %88 %89
         %91 = OpAll %bool %90
         %92 = OpLogicalAnd %bool %87 %91
               OpReturnValue %92
               OpFunctionEnd
       %main = OpFunction %v4float None %93
         %94 = OpFunctionParameter %_ptr_Function_v2float
         %95 = OpLabel
  %_0_matrix = OpVariable %_ptr_Function_mat3v3float Function
  %_1_values = OpVariable %_ptr_Function_v3float Function
   %_2_index = OpVariable %_ptr_Function_int Function
        %151 = OpVariable %_ptr_Function_v4float Function
               OpStore %_1_values %100
               OpStore %_2_index %int_0
               OpBranch %102
        %102 = OpLabel
               OpLoopMerge %106 %105 None
               OpBranch %103
        %103 = OpLabel
        %107 = OpLoad %int %_2_index
        %108 = OpSLessThan %bool %107 %int_3
               OpBranchConditional %108 %104 %106
        %104 = OpLabel
        %109 = OpLoad %v3float %_1_values
        %110 = OpVectorShuffle %v2float %109 %109 0 2
        %111 = OpLoad %int %_2_index
        %112 = OpAccessChain %_ptr_Function_v3float %_0_matrix %111
        %113 = OpLoad %v3float %112
        %114 = OpVectorShuffle %v3float %113 %110 4 1 3
               OpStore %112 %114
        %115 = OpLoad %v3float %_1_values
        %116 = OpCompositeExtract %float %115 1
        %117 = OpLoad %int %_2_index
        %118 = OpAccessChain %_ptr_Function_v3float %_0_matrix %117
        %119 = OpAccessChain %_ptr_Function_float %118 %int_1
               OpStore %119 %116
        %121 = OpLoad %v3float %_1_values
        %123 = OpFAdd %v3float %121 %122
               OpStore %_1_values %123
               OpBranch %105
        %105 = OpLabel
        %124 = OpLoad %int %_2_index
        %125 = OpIAdd %int %124 %int_1
               OpStore %_2_index %125
               OpBranch %102
        %106 = OpLabel
        %127 = OpLoad %mat3v3float %_0_matrix
        %128 = OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_2
        %131 = OpLoad %mat3v3float %128
        %133 = OpCompositeExtract %v3float %127 0
        %134 = OpCompositeExtract %v3float %131 0
        %135 = OpFOrdEqual %v3bool %133 %134
        %136 = OpAll %bool %135
        %137 = OpCompositeExtract %v3float %127 1
        %138 = OpCompositeExtract %v3float %131 1
        %139 = OpFOrdEqual %v3bool %137 %138
        %140 = OpAll %bool %139
        %141 = OpLogicalAnd %bool %136 %140
        %142 = OpCompositeExtract %v3float %127 2
        %143 = OpCompositeExtract %v3float %131 2
        %144 = OpFOrdEqual %v3bool %142 %143
        %145 = OpAll %bool %144
        %146 = OpLogicalAnd %bool %141 %145
               OpSelectionMerge %148 None
               OpBranchConditional %146 %147 %148
        %147 = OpLabel
        %149 = OpFunctionCall %bool %test4x4_b
               OpBranch %148
        %148 = OpLabel
        %150 = OpPhi %bool %false %106 %149 %147
               OpSelectionMerge %154 None
               OpBranchConditional %150 %152 %153
        %152 = OpLabel
        %155 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %157 = OpLoad %v4float %155
               OpStore %151 %157
               OpBranch %154
        %153 = OpLabel
        %158 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %159 = OpLoad %v4float %158
               OpStore %151 %159
               OpBranch %154
        %154 = OpLabel
        %160 = OpLoad %v4float %151
               OpReturnValue %160
               OpFunctionEnd
