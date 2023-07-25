               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testMatrix2x2"
               OpMemberName %_UniformBuffer 1 "testMatrix3x3"
               OpMemberName %_UniformBuffer 2 "colorGreen"
               OpMemberName %_UniformBuffer 3 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %testMatrix2x3 "testMatrix2x3"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 ColMajor
               OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 1 Offset 32
               OpMemberDecorate %_UniformBuffer 1 ColMajor
               OpMemberDecorate %_UniformBuffer 1 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 2 Offset 80
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 3 Offset 96
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %114 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %mat2v2float %mat3v3float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %26 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
    %float_5 = OpConstant %float 5
    %float_6 = OpConstant %float 6
         %38 = OpConstantComposite %v3float %float_1 %float_2 %float_3
         %39 = OpConstantComposite %v3float %float_4 %float_5 %float_6
         %40 = OpConstantComposite %mat2v3float %38 %39
      %false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %48 = OpConstantComposite %v2float %float_1 %float_3
         %49 = OpConstantComposite %v2float %float_2 %float_4
         %50 = OpConstantComposite %mat2v2float %48 %49
     %v2bool = OpTypeVector %bool 2
%mat3v2float = OpTypeMatrix %v2float 3
         %63 = OpConstantComposite %v2float %float_1 %float_4
         %64 = OpConstantComposite %v2float %float_2 %float_5
         %65 = OpConstantComposite %v2float %float_3 %float_6
         %66 = OpConstantComposite %mat3v2float %63 %64 %65
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_1 = OpConstant %int 1
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
    %float_9 = OpConstant %float 9
         %89 = OpConstantComposite %v3float %float_1 %float_4 %float_7
         %90 = OpConstantComposite %v3float %float_2 %float_5 %float_8
         %91 = OpConstantComposite %v3float %float_3 %float_6 %float_9
         %92 = OpConstantComposite %mat3v3float %89 %90 %91
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
%_entrypoint_v = OpFunction %void None %19
         %20 = OpLabel
         %23 = OpVariable %_ptr_Function_v2float Function
               OpStore %23 %22
         %25 = OpFunctionCall %v4float %main %23
               OpStore %sk_FragColor %25
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %26
         %27 = OpFunctionParameter %_ptr_Function_v2float
         %28 = OpLabel
%testMatrix2x3 = OpVariable %_ptr_Function_mat2v3float Function
        %106 = OpVariable %_ptr_Function_v4float Function
               OpStore %testMatrix2x3 %40
         %43 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_0
         %47 = OpLoad %mat2v2float %43
         %42 = OpTranspose %mat2v2float %47
         %52 = OpCompositeExtract %v2float %42 0
         %53 = OpFOrdEqual %v2bool %52 %48
         %54 = OpAll %bool %53
         %55 = OpCompositeExtract %v2float %42 1
         %56 = OpFOrdEqual %v2bool %55 %49
         %57 = OpAll %bool %56
         %58 = OpLogicalAnd %bool %54 %57
               OpSelectionMerge %60 None
               OpBranchConditional %58 %59 %60
         %59 = OpLabel
         %61 = OpTranspose %mat3v2float %40
         %67 = OpCompositeExtract %v2float %61 0
         %68 = OpFOrdEqual %v2bool %67 %63
         %69 = OpAll %bool %68
         %70 = OpCompositeExtract %v2float %61 1
         %71 = OpFOrdEqual %v2bool %70 %64
         %72 = OpAll %bool %71
         %73 = OpLogicalAnd %bool %69 %72
         %74 = OpCompositeExtract %v2float %61 2
         %75 = OpFOrdEqual %v2bool %74 %65
         %76 = OpAll %bool %75
         %77 = OpLogicalAnd %bool %73 %76
               OpBranch %60
         %60 = OpLabel
         %78 = OpPhi %bool %false %28 %77 %59
               OpSelectionMerge %80 None
               OpBranchConditional %78 %79 %80
         %79 = OpLabel
         %82 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_1
         %85 = OpLoad %mat3v3float %82
         %81 = OpTranspose %mat3v3float %85
         %94 = OpCompositeExtract %v3float %81 0
         %95 = OpFOrdEqual %v3bool %94 %89
         %96 = OpAll %bool %95
         %97 = OpCompositeExtract %v3float %81 1
         %98 = OpFOrdEqual %v3bool %97 %90
         %99 = OpAll %bool %98
        %100 = OpLogicalAnd %bool %96 %99
        %101 = OpCompositeExtract %v3float %81 2
        %102 = OpFOrdEqual %v3bool %101 %91
        %103 = OpAll %bool %102
        %104 = OpLogicalAnd %bool %100 %103
               OpBranch %80
         %80 = OpLabel
        %105 = OpPhi %bool %false %60 %104 %79
               OpSelectionMerge %110 None
               OpBranchConditional %105 %108 %109
        %108 = OpLabel
        %111 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %114 = OpLoad %v4float %111
               OpStore %106 %114
               OpBranch %110
        %109 = OpLabel
        %115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %117 = OpLoad %v4float %115
               OpStore %106 %117
               OpBranch %110
        %110 = OpLabel
        %118 = OpLoad %v4float %106
               OpReturnValue %118
               OpFunctionEnd
