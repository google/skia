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
               OpName %test3x3_b "test3x3_b"
               OpName %matrix "matrix"
               OpName %values "values"
               OpName %index "index"
               OpName %test4x4_b "test4x4_b"
               OpName %matrix_0 "matrix"
               OpName %values_0 "values"
               OpName %index_0 "index"
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
               OpMemberDecorate %_UniformBuffer 3 Offset 80
               OpMemberDecorate %_UniformBuffer 3 ColMajor
               OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
               OpDecorate %_UniformBuffer Block
               OpDecorate %9 Binding 0
               OpDecorate %9 DescriptorSet 0
               OpDecorate %140 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat3v3float %mat4v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %9 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %26 = OpTypeFunction %bool
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %35 = OpConstantComposite %v3float %float_1 %float_2 %float_3
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
         %52 = OpConstantComposite %v3float %float_3 %float_3 %float_3
      %int_1 = OpConstant %int 1
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_2 = OpConstant %int 2
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_4 = OpConstant %float 4
         %83 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
      %int_4 = OpConstant %int 4
         %97 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
     %v4bool = OpTypeVector %bool 4
        %125 = OpTypeFunction %v4float %_ptr_Function_v2float
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %17
         %18 = OpLabel
         %22 = OpVariable %_ptr_Function_v2float Function
               OpStore %22 %21
         %24 = OpFunctionCall %v4float %main %22
               OpStore %sk_FragColor %24
               OpReturn
               OpFunctionEnd
  %test3x3_b = OpFunction %bool None %26
         %27 = OpLabel
     %matrix = OpVariable %_ptr_Function_mat3v3float Function
     %values = OpVariable %_ptr_Function_v3float Function
      %index = OpVariable %_ptr_Function_int Function
               OpStore %values %35
               OpStore %index %int_0
               OpBranch %40
         %40 = OpLabel
               OpLoopMerge %44 %43 None
               OpBranch %41
         %41 = OpLabel
         %45 = OpLoad %int %index
         %47 = OpSLessThan %bool %45 %int_3
               OpBranchConditional %47 %42 %44
         %42 = OpLabel
         %48 = OpLoad %v3float %values
         %49 = OpLoad %int %index
         %50 = OpAccessChain %_ptr_Function_v3float %matrix %49
               OpStore %50 %48
         %51 = OpLoad %v3float %values
         %53 = OpFAdd %v3float %51 %52
               OpStore %values %53
               OpBranch %43
         %43 = OpLabel
         %55 = OpLoad %int %index
         %56 = OpIAdd %int %55 %int_1
               OpStore %index %56
               OpBranch %40
         %44 = OpLabel
         %57 = OpLoad %mat3v3float %matrix
         %58 = OpAccessChain %_ptr_Uniform_mat3v3float %9 %int_2
         %61 = OpLoad %mat3v3float %58
         %63 = OpCompositeExtract %v3float %57 0
         %64 = OpCompositeExtract %v3float %61 0
         %65 = OpFOrdEqual %v3bool %63 %64
         %66 = OpAll %bool %65
         %67 = OpCompositeExtract %v3float %57 1
         %68 = OpCompositeExtract %v3float %61 1
         %69 = OpFOrdEqual %v3bool %67 %68
         %70 = OpAll %bool %69
         %71 = OpLogicalAnd %bool %66 %70
         %72 = OpCompositeExtract %v3float %57 2
         %73 = OpCompositeExtract %v3float %61 2
         %74 = OpFOrdEqual %v3bool %72 %73
         %75 = OpAll %bool %74
         %76 = OpLogicalAnd %bool %71 %75
               OpReturnValue %76
               OpFunctionEnd
  %test4x4_b = OpFunction %bool None %26
         %77 = OpLabel
   %matrix_0 = OpVariable %_ptr_Function_mat4v4float Function
   %values_0 = OpVariable %_ptr_Function_v4float Function
    %index_0 = OpVariable %_ptr_Function_int Function
               OpStore %values_0 %83
               OpStore %index_0 %int_0
               OpBranch %85
         %85 = OpLabel
               OpLoopMerge %89 %88 None
               OpBranch %86
         %86 = OpLabel
         %90 = OpLoad %int %index_0
         %92 = OpSLessThan %bool %90 %int_4
               OpBranchConditional %92 %87 %89
         %87 = OpLabel
         %93 = OpLoad %v4float %values_0
         %94 = OpLoad %int %index_0
         %95 = OpAccessChain %_ptr_Function_v4float %matrix_0 %94
               OpStore %95 %93
         %96 = OpLoad %v4float %values_0
         %98 = OpFAdd %v4float %96 %97
               OpStore %values_0 %98
               OpBranch %88
         %88 = OpLabel
         %99 = OpLoad %int %index_0
        %100 = OpIAdd %int %99 %int_1
               OpStore %index_0 %100
               OpBranch %85
         %89 = OpLabel
        %101 = OpLoad %mat4v4float %matrix_0
        %102 = OpAccessChain %_ptr_Uniform_mat4v4float %9 %int_3
        %104 = OpLoad %mat4v4float %102
        %106 = OpCompositeExtract %v4float %101 0
        %107 = OpCompositeExtract %v4float %104 0
        %108 = OpFOrdEqual %v4bool %106 %107
        %109 = OpAll %bool %108
        %110 = OpCompositeExtract %v4float %101 1
        %111 = OpCompositeExtract %v4float %104 1
        %112 = OpFOrdEqual %v4bool %110 %111
        %113 = OpAll %bool %112
        %114 = OpLogicalAnd %bool %109 %113
        %115 = OpCompositeExtract %v4float %101 2
        %116 = OpCompositeExtract %v4float %104 2
        %117 = OpFOrdEqual %v4bool %115 %116
        %118 = OpAll %bool %117
        %119 = OpLogicalAnd %bool %114 %118
        %120 = OpCompositeExtract %v4float %101 3
        %121 = OpCompositeExtract %v4float %104 3
        %122 = OpFOrdEqual %v4bool %120 %121
        %123 = OpAll %bool %122
        %124 = OpLogicalAnd %bool %119 %123
               OpReturnValue %124
               OpFunctionEnd
       %main = OpFunction %v4float None %125
        %126 = OpFunctionParameter %_ptr_Function_v2float
        %127 = OpLabel
        %134 = OpVariable %_ptr_Function_v4float Function
        %129 = OpFunctionCall %bool %test3x3_b
               OpSelectionMerge %131 None
               OpBranchConditional %129 %130 %131
        %130 = OpLabel
        %132 = OpFunctionCall %bool %test4x4_b
               OpBranch %131
        %131 = OpLabel
        %133 = OpPhi %bool %false %127 %132 %130
               OpSelectionMerge %137 None
               OpBranchConditional %133 %135 %136
        %135 = OpLabel
        %138 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %140 = OpLoad %v4float %138
               OpStore %134 %140
               OpBranch %137
        %136 = OpLabel
        %141 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
        %142 = OpLoad %v4float %141
               OpStore %134 %142
               OpBranch %137
        %137 = OpLabel
        %143 = OpLoad %v4float %134
               OpReturnValue %143
               OpFunctionEnd
