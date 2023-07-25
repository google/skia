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
               OpDecorate %143 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
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
%_ptr_Function_v3float = OpTypePointer Function %v3float
    %float_3 = OpConstant %float 3
    %float_2 = OpConstant %float 2
    %float_1 = OpConstant %float 1
         %35 = OpConstantComposite %v3float %float_3 %float_2 %float_1
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_2 = OpConstant %int 2
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
      %false = OpConstantFalse %bool
      %int_1 = OpConstant %int 1
         %79 = OpConstantComposite %v3float %float_3 %float_3 %float_3
       %true = OpConstantTrue %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_4 = OpConstant %float 4
         %88 = OpConstantComposite %v4float %float_4 %float_3 %float_2 %float_1
      %int_4 = OpConstant %int 4
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %126 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
        %130 = OpTypeFunction %v4float %_ptr_Function_v2float
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
   %expected = OpVariable %_ptr_Function_v3float Function
          %c = OpVariable %_ptr_Function_int Function
        %vec = OpVariable %_ptr_Function_v3float Function
          %r = OpVariable %_ptr_Function_int Function
               OpStore %expected %35
               OpStore %c %int_0
               OpBranch %40
         %40 = OpLabel
               OpLoopMerge %44 %43 None
               OpBranch %41
         %41 = OpLabel
         %45 = OpLoad %int %c
         %47 = OpSLessThan %bool %45 %int_3
               OpBranchConditional %47 %42 %44
         %42 = OpLabel
         %49 = OpAccessChain %_ptr_Uniform_mat3v3float %12 %int_2
         %52 = OpLoad %int %c
         %53 = OpAccessChain %_ptr_Uniform_v3float %49 %52
         %55 = OpLoad %v3float %53
               OpStore %vec %55
               OpStore %r %int_0
               OpBranch %57
         %57 = OpLabel
               OpLoopMerge %61 %60 None
               OpBranch %58
         %58 = OpLabel
         %62 = OpLoad %int %r
         %63 = OpSLessThan %bool %62 %int_3
               OpBranchConditional %63 %59 %61
         %59 = OpLabel
         %64 = OpLoad %v3float %vec
         %65 = OpVectorShuffle %v3float %64 %64 2 1 0
         %66 = OpLoad %int %r
         %67 = OpVectorExtractDynamic %float %65 %66
         %68 = OpLoad %v3float %expected
         %69 = OpLoad %int %r
         %70 = OpVectorExtractDynamic %float %68 %69
         %71 = OpFUnordNotEqual %bool %67 %70
               OpSelectionMerge %73 None
               OpBranchConditional %71 %72 %73
         %72 = OpLabel
               OpReturnValue %false
         %73 = OpLabel
               OpBranch %60
         %60 = OpLabel
         %76 = OpLoad %int %r
         %77 = OpIAdd %int %76 %int_1
               OpStore %r %77
               OpBranch %57
         %61 = OpLabel
         %78 = OpLoad %v3float %expected
         %80 = OpFAdd %v3float %78 %79
               OpStore %expected %80
               OpBranch %43
         %43 = OpLabel
         %81 = OpLoad %int %c
         %82 = OpIAdd %int %81 %int_1
               OpStore %c %82
               OpBranch %40
         %44 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
  %test4x4_b = OpFunction %bool None %28
         %84 = OpLabel
 %expected_0 = OpVariable %_ptr_Function_v4float Function
        %c_0 = OpVariable %_ptr_Function_int Function
      %vec_0 = OpVariable %_ptr_Function_v4float Function
        %r_0 = OpVariable %_ptr_Function_int Function
               OpStore %expected_0 %88
               OpStore %c_0 %int_0
               OpBranch %90
         %90 = OpLabel
               OpLoopMerge %94 %93 None
               OpBranch %91
         %91 = OpLabel
         %95 = OpLoad %int %c_0
         %97 = OpSLessThan %bool %95 %int_4
               OpBranchConditional %97 %92 %94
         %92 = OpLabel
         %99 = OpAccessChain %_ptr_Uniform_mat4v4float %12 %int_3
        %101 = OpLoad %int %c_0
        %102 = OpAccessChain %_ptr_Uniform_v4float %99 %101
        %104 = OpLoad %v4float %102
               OpStore %vec_0 %104
               OpStore %r_0 %int_0
               OpBranch %106
        %106 = OpLabel
               OpLoopMerge %110 %109 None
               OpBranch %107
        %107 = OpLabel
        %111 = OpLoad %int %r_0
        %112 = OpSLessThan %bool %111 %int_4
               OpBranchConditional %112 %108 %110
        %108 = OpLabel
        %113 = OpLoad %v4float %vec_0
        %114 = OpVectorShuffle %v4float %113 %113 3 2 1 0
        %115 = OpLoad %int %r_0
        %116 = OpVectorExtractDynamic %float %114 %115
        %117 = OpLoad %v4float %expected_0
        %118 = OpLoad %int %r_0
        %119 = OpVectorExtractDynamic %float %117 %118
        %120 = OpFUnordNotEqual %bool %116 %119
               OpSelectionMerge %122 None
               OpBranchConditional %120 %121 %122
        %121 = OpLabel
               OpReturnValue %false
        %122 = OpLabel
               OpBranch %109
        %109 = OpLabel
        %123 = OpLoad %int %r_0
        %124 = OpIAdd %int %123 %int_1
               OpStore %r_0 %124
               OpBranch %106
        %110 = OpLabel
        %125 = OpLoad %v4float %expected_0
        %127 = OpFAdd %v4float %125 %126
               OpStore %expected_0 %127
               OpBranch %93
         %93 = OpLabel
        %128 = OpLoad %int %c_0
        %129 = OpIAdd %int %128 %int_1
               OpStore %c_0 %129
               OpBranch %90
         %94 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
       %main = OpFunction %v4float None %130
        %131 = OpFunctionParameter %_ptr_Function_v2float
        %132 = OpLabel
        %138 = OpVariable %_ptr_Function_v4float Function
        %133 = OpFunctionCall %bool %test3x3_b
               OpSelectionMerge %135 None
               OpBranchConditional %133 %134 %135
        %134 = OpLabel
        %136 = OpFunctionCall %bool %test4x4_b
               OpBranch %135
        %135 = OpLabel
        %137 = OpPhi %bool %false %132 %136 %134
               OpSelectionMerge %141 None
               OpBranchConditional %137 %139 %140
        %139 = OpLabel
        %142 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %143 = OpLoad %v4float %142
               OpStore %138 %143
               OpBranch %141
        %140 = OpLabel
        %144 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %145 = OpLoad %v4float %144
               OpStore %138 %145
               OpBranch %141
        %141 = OpLabel
        %146 = OpLoad %v4float %138
               OpReturnValue %146
               OpFunctionEnd
