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
               OpName %expected "expected"
               OpName %c "c"
               OpName %vec "vec"
               OpName %r "r"
               OpName %test4x4_b "test4x4_b"
               OpName %expected_0 "expected"
               OpName %c_0 "c"
               OpName %vec_0 "vec"
               OpName %r_0 "r"
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
               OpDecorate %141 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
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
%_ptr_Function_v3float = OpTypePointer Function %v3float
    %float_3 = OpConstant %float 3
    %float_2 = OpConstant %float 2
    %float_1 = OpConstant %float 1
         %33 = OpConstantComposite %v3float %float_3 %float_2 %float_1
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_2 = OpConstant %int 2
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
      %false = OpConstantFalse %bool
      %int_1 = OpConstant %int 1
         %77 = OpConstantComposite %v3float %float_3 %float_3 %float_3
       %true = OpConstantTrue %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_4 = OpConstant %float 4
         %86 = OpConstantComposite %v4float %float_4 %float_3 %float_2 %float_1
      %int_4 = OpConstant %int 4
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %124 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
        %128 = OpTypeFunction %v4float %_ptr_Function_v2float
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
   %expected = OpVariable %_ptr_Function_v3float Function
          %c = OpVariable %_ptr_Function_int Function
        %vec = OpVariable %_ptr_Function_v3float Function
          %r = OpVariable %_ptr_Function_int Function
               OpStore %expected %33
               OpStore %c %int_0
               OpBranch %38
         %38 = OpLabel
               OpLoopMerge %42 %41 None
               OpBranch %39
         %39 = OpLabel
         %43 = OpLoad %int %c
         %45 = OpSLessThan %bool %43 %int_3
               OpBranchConditional %45 %40 %42
         %40 = OpLabel
         %47 = OpAccessChain %_ptr_Uniform_mat3v3float %9 %int_2
         %50 = OpLoad %int %c
         %51 = OpAccessChain %_ptr_Uniform_v3float %47 %50
         %53 = OpLoad %v3float %51
               OpStore %vec %53
               OpStore %r %int_0
               OpBranch %55
         %55 = OpLabel
               OpLoopMerge %59 %58 None
               OpBranch %56
         %56 = OpLabel
         %60 = OpLoad %int %r
         %61 = OpSLessThan %bool %60 %int_3
               OpBranchConditional %61 %57 %59
         %57 = OpLabel
         %62 = OpLoad %v3float %vec
         %63 = OpVectorShuffle %v3float %62 %62 2 1 0
         %64 = OpLoad %int %r
         %65 = OpVectorExtractDynamic %float %63 %64
         %66 = OpLoad %v3float %expected
         %67 = OpLoad %int %r
         %68 = OpVectorExtractDynamic %float %66 %67
         %69 = OpFUnordNotEqual %bool %65 %68
               OpSelectionMerge %71 None
               OpBranchConditional %69 %70 %71
         %70 = OpLabel
               OpReturnValue %false
         %71 = OpLabel
               OpBranch %58
         %58 = OpLabel
         %74 = OpLoad %int %r
         %75 = OpIAdd %int %74 %int_1
               OpStore %r %75
               OpBranch %55
         %59 = OpLabel
         %76 = OpLoad %v3float %expected
         %78 = OpFAdd %v3float %76 %77
               OpStore %expected %78
               OpBranch %41
         %41 = OpLabel
         %79 = OpLoad %int %c
         %80 = OpIAdd %int %79 %int_1
               OpStore %c %80
               OpBranch %38
         %42 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
  %test4x4_b = OpFunction %bool None %26
         %82 = OpLabel
 %expected_0 = OpVariable %_ptr_Function_v4float Function
        %c_0 = OpVariable %_ptr_Function_int Function
      %vec_0 = OpVariable %_ptr_Function_v4float Function
        %r_0 = OpVariable %_ptr_Function_int Function
               OpStore %expected_0 %86
               OpStore %c_0 %int_0
               OpBranch %88
         %88 = OpLabel
               OpLoopMerge %92 %91 None
               OpBranch %89
         %89 = OpLabel
         %93 = OpLoad %int %c_0
         %95 = OpSLessThan %bool %93 %int_4
               OpBranchConditional %95 %90 %92
         %90 = OpLabel
         %97 = OpAccessChain %_ptr_Uniform_mat4v4float %9 %int_3
         %99 = OpLoad %int %c_0
        %100 = OpAccessChain %_ptr_Uniform_v4float %97 %99
        %102 = OpLoad %v4float %100
               OpStore %vec_0 %102
               OpStore %r_0 %int_0
               OpBranch %104
        %104 = OpLabel
               OpLoopMerge %108 %107 None
               OpBranch %105
        %105 = OpLabel
        %109 = OpLoad %int %r_0
        %110 = OpSLessThan %bool %109 %int_4
               OpBranchConditional %110 %106 %108
        %106 = OpLabel
        %111 = OpLoad %v4float %vec_0
        %112 = OpVectorShuffle %v4float %111 %111 3 2 1 0
        %113 = OpLoad %int %r_0
        %114 = OpVectorExtractDynamic %float %112 %113
        %115 = OpLoad %v4float %expected_0
        %116 = OpLoad %int %r_0
        %117 = OpVectorExtractDynamic %float %115 %116
        %118 = OpFUnordNotEqual %bool %114 %117
               OpSelectionMerge %120 None
               OpBranchConditional %118 %119 %120
        %119 = OpLabel
               OpReturnValue %false
        %120 = OpLabel
               OpBranch %107
        %107 = OpLabel
        %121 = OpLoad %int %r_0
        %122 = OpIAdd %int %121 %int_1
               OpStore %r_0 %122
               OpBranch %104
        %108 = OpLabel
        %123 = OpLoad %v4float %expected_0
        %125 = OpFAdd %v4float %123 %124
               OpStore %expected_0 %125
               OpBranch %91
         %91 = OpLabel
        %126 = OpLoad %int %c_0
        %127 = OpIAdd %int %126 %int_1
               OpStore %c_0 %127
               OpBranch %88
         %92 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
       %main = OpFunction %v4float None %128
        %129 = OpFunctionParameter %_ptr_Function_v2float
        %130 = OpLabel
        %136 = OpVariable %_ptr_Function_v4float Function
        %131 = OpFunctionCall %bool %test3x3_b
               OpSelectionMerge %133 None
               OpBranchConditional %131 %132 %133
        %132 = OpLabel
        %134 = OpFunctionCall %bool %test4x4_b
               OpBranch %133
        %133 = OpLabel
        %135 = OpPhi %bool %false %130 %134 %132
               OpSelectionMerge %139 None
               OpBranchConditional %135 %137 %138
        %137 = OpLabel
        %140 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %141 = OpLoad %v4float %140
               OpStore %136 %141
               OpBranch %139
        %138 = OpLabel
        %142 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
        %143 = OpLoad %v4float %142
               OpStore %136 %143
               OpBranch %139
        %139 = OpLabel
        %144 = OpLoad %v4float %136
               OpReturnValue %144
               OpFunctionEnd
