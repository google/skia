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
               OpName %test3x3_b "test3x3_b"
               OpName %matrix "matrix"
               OpName %values "values"
               OpName %index "index"
               OpName %test4x4_b "test4x4_b"
               OpName %matrix_0 "matrix"
               OpName %values_0 "values"
               OpName %index_0 "index"
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
               OpMemberDecorate %_UniformBuffer 3 Offset 80
               OpMemberDecorate %_UniformBuffer 3 ColMajor
               OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %142 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
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
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %24 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %28 = OpTypeFunction %bool
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %37 = OpConstantComposite %v3float %float_1 %float_2 %float_3
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
         %54 = OpConstantComposite %v3float %float_3 %float_3 %float_3
      %int_1 = OpConstant %int 1
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_2 = OpConstant %int 2
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_4 = OpConstant %float 4
         %85 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
      %int_4 = OpConstant %int 4
         %99 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
     %v4bool = OpTypeVector %bool 4
        %127 = OpTypeFunction %v4float %_ptr_Function_v2float
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %20
         %21 = OpLabel
         %25 = OpVariable %_ptr_Function_v2float Function
               OpStore %25 %24
         %27 = OpFunctionCall %v4float %main %25
               OpStore %sk_FragColor %27
               OpReturn
               OpFunctionEnd
  %test3x3_b = OpFunction %bool None %28
         %29 = OpLabel
     %matrix = OpVariable %_ptr_Function_mat3v3float Function
     %values = OpVariable %_ptr_Function_v3float Function
      %index = OpVariable %_ptr_Function_int Function
               OpStore %values %37
               OpStore %index %int_0
               OpBranch %42
         %42 = OpLabel
               OpLoopMerge %46 %45 None
               OpBranch %43
         %43 = OpLabel
         %47 = OpLoad %int %index
         %49 = OpSLessThan %bool %47 %int_3
               OpBranchConditional %49 %44 %46
         %44 = OpLabel
         %50 = OpLoad %v3float %values
         %51 = OpLoad %int %index
         %52 = OpAccessChain %_ptr_Function_v3float %matrix %51
               OpStore %52 %50
         %53 = OpLoad %v3float %values
         %55 = OpFAdd %v3float %53 %54
               OpStore %values %55
               OpBranch %45
         %45 = OpLabel
         %57 = OpLoad %int %index
         %58 = OpIAdd %int %57 %int_1
               OpStore %index %58
               OpBranch %42
         %46 = OpLabel
         %59 = OpLoad %mat3v3float %matrix
         %60 = OpAccessChain %_ptr_Uniform_mat3v3float %12 %int_2
         %63 = OpLoad %mat3v3float %60
         %65 = OpCompositeExtract %v3float %59 0
         %66 = OpCompositeExtract %v3float %63 0
         %67 = OpFOrdEqual %v3bool %65 %66
         %68 = OpAll %bool %67
         %69 = OpCompositeExtract %v3float %59 1
         %70 = OpCompositeExtract %v3float %63 1
         %71 = OpFOrdEqual %v3bool %69 %70
         %72 = OpAll %bool %71
         %73 = OpLogicalAnd %bool %68 %72
         %74 = OpCompositeExtract %v3float %59 2
         %75 = OpCompositeExtract %v3float %63 2
         %76 = OpFOrdEqual %v3bool %74 %75
         %77 = OpAll %bool %76
         %78 = OpLogicalAnd %bool %73 %77
               OpReturnValue %78
               OpFunctionEnd
  %test4x4_b = OpFunction %bool None %28
         %79 = OpLabel
   %matrix_0 = OpVariable %_ptr_Function_mat4v4float Function
   %values_0 = OpVariable %_ptr_Function_v4float Function
    %index_0 = OpVariable %_ptr_Function_int Function
               OpStore %values_0 %85
               OpStore %index_0 %int_0
               OpBranch %87
         %87 = OpLabel
               OpLoopMerge %91 %90 None
               OpBranch %88
         %88 = OpLabel
         %92 = OpLoad %int %index_0
         %94 = OpSLessThan %bool %92 %int_4
               OpBranchConditional %94 %89 %91
         %89 = OpLabel
         %95 = OpLoad %v4float %values_0
         %96 = OpLoad %int %index_0
         %97 = OpAccessChain %_ptr_Function_v4float %matrix_0 %96
               OpStore %97 %95
         %98 = OpLoad %v4float %values_0
        %100 = OpFAdd %v4float %98 %99
               OpStore %values_0 %100
               OpBranch %90
         %90 = OpLabel
        %101 = OpLoad %int %index_0
        %102 = OpIAdd %int %101 %int_1
               OpStore %index_0 %102
               OpBranch %87
         %91 = OpLabel
        %103 = OpLoad %mat4v4float %matrix_0
        %104 = OpAccessChain %_ptr_Uniform_mat4v4float %12 %int_3
        %106 = OpLoad %mat4v4float %104
        %108 = OpCompositeExtract %v4float %103 0
        %109 = OpCompositeExtract %v4float %106 0
        %110 = OpFOrdEqual %v4bool %108 %109
        %111 = OpAll %bool %110
        %112 = OpCompositeExtract %v4float %103 1
        %113 = OpCompositeExtract %v4float %106 1
        %114 = OpFOrdEqual %v4bool %112 %113
        %115 = OpAll %bool %114
        %116 = OpLogicalAnd %bool %111 %115
        %117 = OpCompositeExtract %v4float %103 2
        %118 = OpCompositeExtract %v4float %106 2
        %119 = OpFOrdEqual %v4bool %117 %118
        %120 = OpAll %bool %119
        %121 = OpLogicalAnd %bool %116 %120
        %122 = OpCompositeExtract %v4float %103 3
        %123 = OpCompositeExtract %v4float %106 3
        %124 = OpFOrdEqual %v4bool %122 %123
        %125 = OpAll %bool %124
        %126 = OpLogicalAnd %bool %121 %125
               OpReturnValue %126
               OpFunctionEnd
       %main = OpFunction %v4float None %127
        %128 = OpFunctionParameter %_ptr_Function_v2float
        %129 = OpLabel
        %136 = OpVariable %_ptr_Function_v4float Function
        %131 = OpFunctionCall %bool %test3x3_b
               OpSelectionMerge %133 None
               OpBranchConditional %131 %132 %133
        %132 = OpLabel
        %134 = OpFunctionCall %bool %test4x4_b
               OpBranch %133
        %133 = OpLabel
        %135 = OpPhi %bool %false %129 %134 %132
               OpSelectionMerge %139 None
               OpBranchConditional %135 %137 %138
        %137 = OpLabel
        %140 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %142 = OpLoad %v4float %140
               OpStore %136 %142
               OpBranch %139
        %138 = OpLabel
        %143 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %144 = OpLoad %v4float %143
               OpStore %136 %144
               OpBranch %139
        %139 = OpLabel
        %145 = OpLoad %v4float %136
               OpReturnValue %145
               OpFunctionEnd
