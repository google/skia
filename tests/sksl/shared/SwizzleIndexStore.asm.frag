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
               OpName %vec "vec"
               OpName %c "c"
               OpName %r "r"
               OpName %test4x4_b "test4x4_b"
               OpName %expected_0 "expected"
               OpName %vec_0 "vec"
               OpName %c_0 "c"
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
               OpDecorate %152 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
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
      %int_1 = OpConstant %int 1
      %v3int = OpTypeVector %int 3
         %68 = OpConstantComposite %v3int %int_2 %int_1 %int_0
%_ptr_Function_float = OpTypePointer Function %float
     %v3bool = OpTypeVector %bool 3
      %false = OpConstantFalse %bool
         %84 = OpConstantComposite %v3float %float_3 %float_3 %float_3
       %true = OpConstantTrue %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_4 = OpConstant %float 4
         %93 = OpConstantComposite %v4float %float_4 %float_3 %float_2 %float_1
      %int_4 = OpConstant %int 4
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %v4int = OpTypeVector %int 4
        %121 = OpConstantComposite %v4int %int_3 %int_2 %int_1 %int_0
     %v4bool = OpTypeVector %bool 4
        %135 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
        %139 = OpTypeFunction %v4float %_ptr_Function_v2float
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
        %vec = OpVariable %_ptr_Function_v3float Function
          %c = OpVariable %_ptr_Function_int Function
          %r = OpVariable %_ptr_Function_int Function
               OpStore %expected %35
               OpStore %c %int_0
               OpBranch %41
         %41 = OpLabel
               OpLoopMerge %45 %44 None
               OpBranch %42
         %42 = OpLabel
         %46 = OpLoad %int %c
         %48 = OpSLessThan %bool %46 %int_3
               OpBranchConditional %48 %43 %45
         %43 = OpLabel
               OpStore %r %int_0
               OpBranch %50
         %50 = OpLabel
               OpLoopMerge %54 %53 None
               OpBranch %51
         %51 = OpLabel
         %55 = OpLoad %int %r
         %56 = OpSLessThan %bool %55 %int_3
               OpBranchConditional %56 %52 %54
         %52 = OpLabel
         %57 = OpAccessChain %_ptr_Uniform_mat3v3float %12 %int_2
         %60 = OpLoad %int %c
         %61 = OpAccessChain %_ptr_Uniform_v3float %57 %60
         %63 = OpLoad %v3float %61
         %64 = OpLoad %int %r
         %65 = OpVectorExtractDynamic %float %63 %64
         %69 = OpLoad %int %r
         %70 = OpVectorExtractDynamic %int %68 %69
         %71 = OpAccessChain %_ptr_Function_float %vec %70
               OpStore %71 %65
               OpBranch %53
         %53 = OpLabel
         %73 = OpLoad %int %r
         %74 = OpIAdd %int %73 %int_1
               OpStore %r %74
               OpBranch %50
         %54 = OpLabel
         %75 = OpLoad %v3float %vec
         %76 = OpLoad %v3float %expected
         %77 = OpFUnordNotEqual %v3bool %75 %76
         %79 = OpAny %bool %77
               OpSelectionMerge %81 None
               OpBranchConditional %79 %80 %81
         %80 = OpLabel
               OpReturnValue %false
         %81 = OpLabel
         %83 = OpLoad %v3float %expected
         %85 = OpFAdd %v3float %83 %84
               OpStore %expected %85
               OpBranch %44
         %44 = OpLabel
         %86 = OpLoad %int %c
         %87 = OpIAdd %int %86 %int_1
               OpStore %c %87
               OpBranch %41
         %45 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
  %test4x4_b = OpFunction %bool None %28
         %89 = OpLabel
 %expected_0 = OpVariable %_ptr_Function_v4float Function
      %vec_0 = OpVariable %_ptr_Function_v4float Function
        %c_0 = OpVariable %_ptr_Function_int Function
        %r_0 = OpVariable %_ptr_Function_int Function
               OpStore %expected_0 %93
               OpStore %c_0 %int_0
               OpBranch %96
         %96 = OpLabel
               OpLoopMerge %100 %99 None
               OpBranch %97
         %97 = OpLabel
        %101 = OpLoad %int %c_0
        %103 = OpSLessThan %bool %101 %int_4
               OpBranchConditional %103 %98 %100
         %98 = OpLabel
               OpStore %r_0 %int_0
               OpBranch %105
        %105 = OpLabel
               OpLoopMerge %109 %108 None
               OpBranch %106
        %106 = OpLabel
        %110 = OpLoad %int %r_0
        %111 = OpSLessThan %bool %110 %int_4
               OpBranchConditional %111 %107 %109
        %107 = OpLabel
        %112 = OpAccessChain %_ptr_Uniform_mat4v4float %12 %int_3
        %114 = OpLoad %int %c_0
        %115 = OpAccessChain %_ptr_Uniform_v4float %112 %114
        %117 = OpLoad %v4float %115
        %118 = OpLoad %int %r_0
        %119 = OpVectorExtractDynamic %float %117 %118
        %122 = OpLoad %int %r_0
        %123 = OpVectorExtractDynamic %int %121 %122
        %124 = OpAccessChain %_ptr_Function_float %vec_0 %123
               OpStore %124 %119
               OpBranch %108
        %108 = OpLabel
        %125 = OpLoad %int %r_0
        %126 = OpIAdd %int %125 %int_1
               OpStore %r_0 %126
               OpBranch %105
        %109 = OpLabel
        %127 = OpLoad %v4float %vec_0
        %128 = OpLoad %v4float %expected_0
        %129 = OpFUnordNotEqual %v4bool %127 %128
        %131 = OpAny %bool %129
               OpSelectionMerge %133 None
               OpBranchConditional %131 %132 %133
        %132 = OpLabel
               OpReturnValue %false
        %133 = OpLabel
        %134 = OpLoad %v4float %expected_0
        %136 = OpFAdd %v4float %134 %135
               OpStore %expected_0 %136
               OpBranch %99
         %99 = OpLabel
        %137 = OpLoad %int %c_0
        %138 = OpIAdd %int %137 %int_1
               OpStore %c_0 %138
               OpBranch %96
        %100 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
       %main = OpFunction %v4float None %139
        %140 = OpFunctionParameter %_ptr_Function_v2float
        %141 = OpLabel
        %147 = OpVariable %_ptr_Function_v4float Function
        %142 = OpFunctionCall %bool %test3x3_b
               OpSelectionMerge %144 None
               OpBranchConditional %142 %143 %144
        %143 = OpLabel
        %145 = OpFunctionCall %bool %test4x4_b
               OpBranch %144
        %144 = OpLabel
        %146 = OpPhi %bool %false %141 %145 %143
               OpSelectionMerge %150 None
               OpBranchConditional %146 %148 %149
        %148 = OpLabel
        %151 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %152 = OpLoad %v4float %151
               OpStore %147 %152
               OpBranch %150
        %149 = OpLabel
        %153 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %154 = OpLoad %v4float %153
               OpStore %147 %154
               OpBranch %150
        %150 = OpLabel
        %155 = OpLoad %v4float %147
               OpReturnValue %155
               OpFunctionEnd
