               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testMatrix2x2"
               OpMemberName %_UniformBuffer 1 "testMatrix3x3"
               OpMemberName %_UniformBuffer 2 "colorGreen"
               OpMemberName %_UniformBuffer 3 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %testMatrix2x3 "testMatrix2x3"
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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %112 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
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
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
    %float_5 = OpConstant %float 5
    %float_6 = OpConstant %float 6
         %35 = OpConstantComposite %v3float %float_1 %float_2 %float_3
         %36 = OpConstantComposite %v3float %float_4 %float_5 %float_6
         %37 = OpConstantComposite %mat2v3float %35 %36
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %46 = OpConstantComposite %v2float %float_1 %float_3
         %47 = OpConstantComposite %v2float %float_2 %float_4
         %48 = OpConstantComposite %mat2v2float %46 %47
     %v2bool = OpTypeVector %bool 2
%mat3v2float = OpTypeMatrix %v2float 3
         %61 = OpConstantComposite %v2float %float_1 %float_4
         %62 = OpConstantComposite %v2float %float_2 %float_5
         %63 = OpConstantComposite %v2float %float_3 %float_6
         %64 = OpConstantComposite %mat3v2float %61 %62 %63
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_1 = OpConstant %int 1
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
    %float_9 = OpConstant %float 9
         %87 = OpConstantComposite %v3float %float_1 %float_4 %float_7
         %88 = OpConstantComposite %v3float %float_2 %float_5 %float_8
         %89 = OpConstantComposite %v3float %float_3 %float_6 %float_9
         %90 = OpConstantComposite %mat3v3float %87 %88 %89
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
%_entrypoint_v = OpFunction %void None %16
         %17 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
%testMatrix2x3 = OpVariable %_ptr_Function_mat2v3float Function
        %104 = OpVariable %_ptr_Function_v4float Function
               OpStore %testMatrix2x3 %37
         %41 = OpAccessChain %_ptr_Uniform_mat2v2float %7 %int_0
         %45 = OpLoad %mat2v2float %41
         %40 = OpTranspose %mat2v2float %45
         %50 = OpCompositeExtract %v2float %40 0
         %51 = OpFOrdEqual %v2bool %50 %46
         %52 = OpAll %bool %51
         %53 = OpCompositeExtract %v2float %40 1
         %54 = OpFOrdEqual %v2bool %53 %47
         %55 = OpAll %bool %54
         %56 = OpLogicalAnd %bool %52 %55
               OpSelectionMerge %58 None
               OpBranchConditional %56 %57 %58
         %57 = OpLabel
         %59 = OpTranspose %mat3v2float %37
         %65 = OpCompositeExtract %v2float %59 0
         %66 = OpFOrdEqual %v2bool %65 %61
         %67 = OpAll %bool %66
         %68 = OpCompositeExtract %v2float %59 1
         %69 = OpFOrdEqual %v2bool %68 %62
         %70 = OpAll %bool %69
         %71 = OpLogicalAnd %bool %67 %70
         %72 = OpCompositeExtract %v2float %59 2
         %73 = OpFOrdEqual %v2bool %72 %63
         %74 = OpAll %bool %73
         %75 = OpLogicalAnd %bool %71 %74
               OpBranch %58
         %58 = OpLabel
         %76 = OpPhi %bool %false %25 %75 %57
               OpSelectionMerge %78 None
               OpBranchConditional %76 %77 %78
         %77 = OpLabel
         %80 = OpAccessChain %_ptr_Uniform_mat3v3float %7 %int_1
         %83 = OpLoad %mat3v3float %80
         %79 = OpTranspose %mat3v3float %83
         %92 = OpCompositeExtract %v3float %79 0
         %93 = OpFOrdEqual %v3bool %92 %87
         %94 = OpAll %bool %93
         %95 = OpCompositeExtract %v3float %79 1
         %96 = OpFOrdEqual %v3bool %95 %88
         %97 = OpAll %bool %96
         %98 = OpLogicalAnd %bool %94 %97
         %99 = OpCompositeExtract %v3float %79 2
        %100 = OpFOrdEqual %v3bool %99 %89
        %101 = OpAll %bool %100
        %102 = OpLogicalAnd %bool %98 %101
               OpBranch %78
         %78 = OpLabel
        %103 = OpPhi %bool %false %58 %102 %77
               OpSelectionMerge %108 None
               OpBranchConditional %103 %106 %107
        %106 = OpLabel
        %109 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %112 = OpLoad %v4float %109
               OpStore %104 %112
               OpBranch %108
        %107 = OpLabel
        %113 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %115 = OpLoad %v4float %113
               OpStore %104 %115
               OpBranch %108
        %108 = OpLabel
        %116 = OpLoad %v4float %104
               OpReturnValue %116
               OpFunctionEnd
