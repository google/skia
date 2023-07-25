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
               OpName %expected "expected"
               OpName %index "index"
               OpName %test4x4_b "test4x4_b"
               OpName %matrix_0 "matrix"
               OpName %expected_0 "expected"
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
               OpDecorate %118 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
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
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
%_ptr_Function_v3float = OpTypePointer Function %v3float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %42 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
     %v3bool = OpTypeVector %bool 3
      %false = OpConstantFalse %bool
         %65 = OpConstantComposite %v3float %float_3 %float_3 %float_3
      %int_1 = OpConstant %int 1
       %true = OpConstantTrue %bool
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_4 = OpConstant %float 4
         %80 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
      %int_4 = OpConstant %int 4
     %v4bool = OpTypeVector %bool 4
        %100 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
        %104 = OpTypeFunction %v4float %_ptr_Function_v2float
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
   %expected = OpVariable %_ptr_Function_v3float Function
      %index = OpVariable %_ptr_Function_int Function
         %32 = OpAccessChain %_ptr_Uniform_mat3v3float %12 %int_2
         %36 = OpLoad %mat3v3float %32
               OpStore %matrix %36
               OpStore %expected %42
               OpStore %index %int_0
               OpBranch %46
         %46 = OpLabel
               OpLoopMerge %50 %49 None
               OpBranch %47
         %47 = OpLabel
         %51 = OpLoad %int %index
         %53 = OpSLessThan %bool %51 %int_3
               OpBranchConditional %53 %48 %50
         %48 = OpLabel
         %54 = OpLoad %int %index
         %55 = OpAccessChain %_ptr_Function_v3float %matrix %54
         %56 = OpLoad %v3float %55
         %57 = OpLoad %v3float %expected
         %58 = OpFUnordNotEqual %v3bool %56 %57
         %60 = OpAny %bool %58
               OpSelectionMerge %62 None
               OpBranchConditional %60 %61 %62
         %61 = OpLabel
               OpReturnValue %false
         %62 = OpLabel
         %64 = OpLoad %v3float %expected
         %66 = OpFAdd %v3float %64 %65
               OpStore %expected %66
               OpBranch %49
         %49 = OpLabel
         %68 = OpLoad %int %index
         %69 = OpIAdd %int %68 %int_1
               OpStore %index %69
               OpBranch %46
         %50 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
  %test4x4_b = OpFunction %bool None %28
         %71 = OpLabel
   %matrix_0 = OpVariable %_ptr_Function_mat4v4float Function
 %expected_0 = OpVariable %_ptr_Function_v4float Function
    %index_0 = OpVariable %_ptr_Function_int Function
         %74 = OpAccessChain %_ptr_Uniform_mat4v4float %12 %int_3
         %76 = OpLoad %mat4v4float %74
               OpStore %matrix_0 %76
               OpStore %expected_0 %80
               OpStore %index_0 %int_0
               OpBranch %82
         %82 = OpLabel
               OpLoopMerge %86 %85 None
               OpBranch %83
         %83 = OpLabel
         %87 = OpLoad %int %index_0
         %89 = OpSLessThan %bool %87 %int_4
               OpBranchConditional %89 %84 %86
         %84 = OpLabel
         %90 = OpLoad %int %index_0
         %91 = OpAccessChain %_ptr_Function_v4float %matrix_0 %90
         %92 = OpLoad %v4float %91
         %93 = OpLoad %v4float %expected_0
         %94 = OpFUnordNotEqual %v4bool %92 %93
         %96 = OpAny %bool %94
               OpSelectionMerge %98 None
               OpBranchConditional %96 %97 %98
         %97 = OpLabel
               OpReturnValue %false
         %98 = OpLabel
         %99 = OpLoad %v4float %expected_0
        %101 = OpFAdd %v4float %99 %100
               OpStore %expected_0 %101
               OpBranch %85
         %85 = OpLabel
        %102 = OpLoad %int %index_0
        %103 = OpIAdd %int %102 %int_1
               OpStore %index_0 %103
               OpBranch %82
         %86 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
       %main = OpFunction %v4float None %104
        %105 = OpFunctionParameter %_ptr_Function_v2float
        %106 = OpLabel
        %112 = OpVariable %_ptr_Function_v4float Function
        %107 = OpFunctionCall %bool %test3x3_b
               OpSelectionMerge %109 None
               OpBranchConditional %107 %108 %109
        %108 = OpLabel
        %110 = OpFunctionCall %bool %test4x4_b
               OpBranch %109
        %109 = OpLabel
        %111 = OpPhi %bool %false %106 %110 %108
               OpSelectionMerge %115 None
               OpBranchConditional %111 %113 %114
        %113 = OpLabel
        %116 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %118 = OpLoad %v4float %116
               OpStore %112 %118
               OpBranch %115
        %114 = OpLabel
        %119 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %120 = OpLoad %v4float %119
               OpStore %112 %120
               OpBranch %115
        %115 = OpLabel
        %121 = OpLoad %v4float %112
               OpReturnValue %121
               OpFunctionEnd
