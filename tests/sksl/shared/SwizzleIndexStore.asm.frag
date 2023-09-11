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
               OpName %vec "vec"
               OpName %c "c"
               OpName %r "r"
               OpName %test4x4_b "test4x4_b"
               OpName %expected_0 "expected"
               OpName %vec_0 "vec"
               OpName %c_0 "c"
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
               OpDecorate %150 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
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
      %int_1 = OpConstant %int 1
      %v3int = OpTypeVector %int 3
         %66 = OpConstantComposite %v3int %int_2 %int_1 %int_0
%_ptr_Function_float = OpTypePointer Function %float
     %v3bool = OpTypeVector %bool 3
      %false = OpConstantFalse %bool
         %82 = OpConstantComposite %v3float %float_3 %float_3 %float_3
       %true = OpConstantTrue %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_4 = OpConstant %float 4
         %91 = OpConstantComposite %v4float %float_4 %float_3 %float_2 %float_1
      %int_4 = OpConstant %int 4
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %v4int = OpTypeVector %int 4
        %119 = OpConstantComposite %v4int %int_3 %int_2 %int_1 %int_0
     %v4bool = OpTypeVector %bool 4
        %133 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
        %137 = OpTypeFunction %v4float %_ptr_Function_v2float
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
        %vec = OpVariable %_ptr_Function_v3float Function
          %c = OpVariable %_ptr_Function_int Function
          %r = OpVariable %_ptr_Function_int Function
               OpStore %expected %33
               OpStore %c %int_0
               OpBranch %39
         %39 = OpLabel
               OpLoopMerge %43 %42 None
               OpBranch %40
         %40 = OpLabel
         %44 = OpLoad %int %c
         %46 = OpSLessThan %bool %44 %int_3
               OpBranchConditional %46 %41 %43
         %41 = OpLabel
               OpStore %r %int_0
               OpBranch %48
         %48 = OpLabel
               OpLoopMerge %52 %51 None
               OpBranch %49
         %49 = OpLabel
         %53 = OpLoad %int %r
         %54 = OpSLessThan %bool %53 %int_3
               OpBranchConditional %54 %50 %52
         %50 = OpLabel
         %55 = OpAccessChain %_ptr_Uniform_mat3v3float %9 %int_2
         %58 = OpLoad %int %c
         %59 = OpAccessChain %_ptr_Uniform_v3float %55 %58
         %61 = OpLoad %v3float %59
         %62 = OpLoad %int %r
         %63 = OpVectorExtractDynamic %float %61 %62
         %67 = OpLoad %int %r
         %68 = OpVectorExtractDynamic %int %66 %67
         %69 = OpAccessChain %_ptr_Function_float %vec %68
               OpStore %69 %63
               OpBranch %51
         %51 = OpLabel
         %71 = OpLoad %int %r
         %72 = OpIAdd %int %71 %int_1
               OpStore %r %72
               OpBranch %48
         %52 = OpLabel
         %73 = OpLoad %v3float %vec
         %74 = OpLoad %v3float %expected
         %75 = OpFUnordNotEqual %v3bool %73 %74
         %77 = OpAny %bool %75
               OpSelectionMerge %79 None
               OpBranchConditional %77 %78 %79
         %78 = OpLabel
               OpReturnValue %false
         %79 = OpLabel
         %81 = OpLoad %v3float %expected
         %83 = OpFAdd %v3float %81 %82
               OpStore %expected %83
               OpBranch %42
         %42 = OpLabel
         %84 = OpLoad %int %c
         %85 = OpIAdd %int %84 %int_1
               OpStore %c %85
               OpBranch %39
         %43 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
  %test4x4_b = OpFunction %bool None %26
         %87 = OpLabel
 %expected_0 = OpVariable %_ptr_Function_v4float Function
      %vec_0 = OpVariable %_ptr_Function_v4float Function
        %c_0 = OpVariable %_ptr_Function_int Function
        %r_0 = OpVariable %_ptr_Function_int Function
               OpStore %expected_0 %91
               OpStore %c_0 %int_0
               OpBranch %94
         %94 = OpLabel
               OpLoopMerge %98 %97 None
               OpBranch %95
         %95 = OpLabel
         %99 = OpLoad %int %c_0
        %101 = OpSLessThan %bool %99 %int_4
               OpBranchConditional %101 %96 %98
         %96 = OpLabel
               OpStore %r_0 %int_0
               OpBranch %103
        %103 = OpLabel
               OpLoopMerge %107 %106 None
               OpBranch %104
        %104 = OpLabel
        %108 = OpLoad %int %r_0
        %109 = OpSLessThan %bool %108 %int_4
               OpBranchConditional %109 %105 %107
        %105 = OpLabel
        %110 = OpAccessChain %_ptr_Uniform_mat4v4float %9 %int_3
        %112 = OpLoad %int %c_0
        %113 = OpAccessChain %_ptr_Uniform_v4float %110 %112
        %115 = OpLoad %v4float %113
        %116 = OpLoad %int %r_0
        %117 = OpVectorExtractDynamic %float %115 %116
        %120 = OpLoad %int %r_0
        %121 = OpVectorExtractDynamic %int %119 %120
        %122 = OpAccessChain %_ptr_Function_float %vec_0 %121
               OpStore %122 %117
               OpBranch %106
        %106 = OpLabel
        %123 = OpLoad %int %r_0
        %124 = OpIAdd %int %123 %int_1
               OpStore %r_0 %124
               OpBranch %103
        %107 = OpLabel
        %125 = OpLoad %v4float %vec_0
        %126 = OpLoad %v4float %expected_0
        %127 = OpFUnordNotEqual %v4bool %125 %126
        %129 = OpAny %bool %127
               OpSelectionMerge %131 None
               OpBranchConditional %129 %130 %131
        %130 = OpLabel
               OpReturnValue %false
        %131 = OpLabel
        %132 = OpLoad %v4float %expected_0
        %134 = OpFAdd %v4float %132 %133
               OpStore %expected_0 %134
               OpBranch %97
         %97 = OpLabel
        %135 = OpLoad %int %c_0
        %136 = OpIAdd %int %135 %int_1
               OpStore %c_0 %136
               OpBranch %94
         %98 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
       %main = OpFunction %v4float None %137
        %138 = OpFunctionParameter %_ptr_Function_v2float
        %139 = OpLabel
        %145 = OpVariable %_ptr_Function_v4float Function
        %140 = OpFunctionCall %bool %test3x3_b
               OpSelectionMerge %142 None
               OpBranchConditional %140 %141 %142
        %141 = OpLabel
        %143 = OpFunctionCall %bool %test4x4_b
               OpBranch %142
        %142 = OpLabel
        %144 = OpPhi %bool %false %139 %143 %141
               OpSelectionMerge %148 None
               OpBranchConditional %144 %146 %147
        %146 = OpLabel
        %149 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %150 = OpLoad %v4float %149
               OpStore %145 %150
               OpBranch %148
        %147 = OpLabel
        %151 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
        %152 = OpLoad %v4float %151
               OpStore %145 %152
               OpBranch %148
        %148 = OpLabel
        %153 = OpLoad %v4float %145
               OpReturnValue %153
               OpFunctionEnd
