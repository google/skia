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
               OpName %expected "expected"
               OpName %index "index"
               OpName %test4x4_b "test4x4_b"
               OpName %matrix_0 "matrix"
               OpName %expected_0 "expected"
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
               OpDecorate %116 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
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
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
%_ptr_Function_v3float = OpTypePointer Function %v3float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %40 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
     %v3bool = OpTypeVector %bool 3
      %false = OpConstantFalse %bool
         %63 = OpConstantComposite %v3float %float_3 %float_3 %float_3
      %int_1 = OpConstant %int 1
       %true = OpConstantTrue %bool
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_4 = OpConstant %float 4
         %78 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
      %int_4 = OpConstant %int 4
     %v4bool = OpTypeVector %bool 4
         %98 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
        %102 = OpTypeFunction %v4float %_ptr_Function_v2float
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
   %expected = OpVariable %_ptr_Function_v3float Function
      %index = OpVariable %_ptr_Function_int Function
         %30 = OpAccessChain %_ptr_Uniform_mat3v3float %9 %int_2
         %34 = OpLoad %mat3v3float %30
               OpStore %matrix %34
               OpStore %expected %40
               OpStore %index %int_0
               OpBranch %44
         %44 = OpLabel
               OpLoopMerge %48 %47 None
               OpBranch %45
         %45 = OpLabel
         %49 = OpLoad %int %index
         %51 = OpSLessThan %bool %49 %int_3
               OpBranchConditional %51 %46 %48
         %46 = OpLabel
         %52 = OpLoad %int %index
         %53 = OpAccessChain %_ptr_Function_v3float %matrix %52
         %54 = OpLoad %v3float %53
         %55 = OpLoad %v3float %expected
         %56 = OpFUnordNotEqual %v3bool %54 %55
         %58 = OpAny %bool %56
               OpSelectionMerge %60 None
               OpBranchConditional %58 %59 %60
         %59 = OpLabel
               OpReturnValue %false
         %60 = OpLabel
         %62 = OpLoad %v3float %expected
         %64 = OpFAdd %v3float %62 %63
               OpStore %expected %64
               OpBranch %47
         %47 = OpLabel
         %66 = OpLoad %int %index
         %67 = OpIAdd %int %66 %int_1
               OpStore %index %67
               OpBranch %44
         %48 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
  %test4x4_b = OpFunction %bool None %26
         %69 = OpLabel
   %matrix_0 = OpVariable %_ptr_Function_mat4v4float Function
 %expected_0 = OpVariable %_ptr_Function_v4float Function
    %index_0 = OpVariable %_ptr_Function_int Function
         %72 = OpAccessChain %_ptr_Uniform_mat4v4float %9 %int_3
         %74 = OpLoad %mat4v4float %72
               OpStore %matrix_0 %74
               OpStore %expected_0 %78
               OpStore %index_0 %int_0
               OpBranch %80
         %80 = OpLabel
               OpLoopMerge %84 %83 None
               OpBranch %81
         %81 = OpLabel
         %85 = OpLoad %int %index_0
         %87 = OpSLessThan %bool %85 %int_4
               OpBranchConditional %87 %82 %84
         %82 = OpLabel
         %88 = OpLoad %int %index_0
         %89 = OpAccessChain %_ptr_Function_v4float %matrix_0 %88
         %90 = OpLoad %v4float %89
         %91 = OpLoad %v4float %expected_0
         %92 = OpFUnordNotEqual %v4bool %90 %91
         %94 = OpAny %bool %92
               OpSelectionMerge %96 None
               OpBranchConditional %94 %95 %96
         %95 = OpLabel
               OpReturnValue %false
         %96 = OpLabel
         %97 = OpLoad %v4float %expected_0
         %99 = OpFAdd %v4float %97 %98
               OpStore %expected_0 %99
               OpBranch %83
         %83 = OpLabel
        %100 = OpLoad %int %index_0
        %101 = OpIAdd %int %100 %int_1
               OpStore %index_0 %101
               OpBranch %80
         %84 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
       %main = OpFunction %v4float None %102
        %103 = OpFunctionParameter %_ptr_Function_v2float
        %104 = OpLabel
        %110 = OpVariable %_ptr_Function_v4float Function
        %105 = OpFunctionCall %bool %test3x3_b
               OpSelectionMerge %107 None
               OpBranchConditional %105 %106 %107
        %106 = OpLabel
        %108 = OpFunctionCall %bool %test4x4_b
               OpBranch %107
        %107 = OpLabel
        %109 = OpPhi %bool %false %104 %108 %106
               OpSelectionMerge %113 None
               OpBranchConditional %109 %111 %112
        %111 = OpLabel
        %114 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %116 = OpLoad %v4float %114
               OpStore %110 %116
               OpBranch %113
        %112 = OpLabel
        %117 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
        %118 = OpLoad %v4float %117
               OpStore %110 %118
               OpBranch %113
        %113 = OpLabel
        %119 = OpLoad %v4float %110
               OpReturnValue %119
               OpFunctionEnd
