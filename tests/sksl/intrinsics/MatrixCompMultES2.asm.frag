               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testMatrix2x2"
               OpMemberName %_UniformBuffer 3 "testMatrix3x3"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %h22 "h22"
               OpName %hugeM22 "hugeM22"
               OpName %f22 "f22"
               OpName %h33 "h33"
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
               OpMemberDecorate %_UniformBuffer 3 Offset 64
               OpMemberDecorate %_UniformBuffer 3 ColMajor
               OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %h22 RelaxedPrecision
               OpDecorate %hugeM22 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %h33 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1000000 = OpConstant %float 1000000
         %29 = OpConstantComposite %v2float %float_1000000 %float_1000000
         %30 = OpConstantComposite %mat2v2float %29 %29
%float_1_00000002e_30 = OpConstant %float 1.00000002e+30
         %33 = OpConstantComposite %v2float %float_1_00000002e_30 %float_1_00000002e_30
         %34 = OpConstantComposite %mat2v2float %33 %33
    %float_5 = OpConstant %float 5
   %float_10 = OpConstant %float 10
   %float_15 = OpConstant %float 15
         %42 = OpConstantComposite %v2float %float_0 %float_5
         %43 = OpConstantComposite %v2float %float_10 %float_15
         %44 = OpConstantComposite %mat2v2float %42 %43
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
         %53 = OpConstantComposite %v2float %float_1 %float_0
         %54 = OpConstantComposite %v2float %float_0 %float_1
         %55 = OpConstantComposite %mat2v2float %53 %54
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_3 = OpConstant %int 3
    %float_2 = OpConstant %float 2
         %69 = OpConstantComposite %v3float %float_2 %float_2 %float_2
         %70 = OpConstantComposite %mat3v3float %69 %69 %69
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v2bool = OpTypeVector %bool 2
    %float_4 = OpConstant %float 4
         %89 = OpConstantComposite %v2float %float_0 %float_4
         %90 = OpConstantComposite %mat2v2float %53 %89
    %float_6 = OpConstant %float 6
    %float_8 = OpConstant %float 8
   %float_12 = OpConstant %float 12
   %float_14 = OpConstant %float 14
   %float_16 = OpConstant %float 16
   %float_18 = OpConstant %float 18
        %105 = OpConstantComposite %v3float %float_2 %float_4 %float_6
        %106 = OpConstantComposite %v3float %float_8 %float_10 %float_12
        %107 = OpConstantComposite %v3float %float_14 %float_16 %float_18
        %108 = OpConstantComposite %mat3v3float %105 %106 %107
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
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
        %h22 = OpVariable %_ptr_Function_mat2v2float Function
    %hugeM22 = OpVariable %_ptr_Function_mat2v2float Function
        %f22 = OpVariable %_ptr_Function_mat2v2float Function
        %h33 = OpVariable %_ptr_Function_mat3v3float Function
        %119 = OpVariable %_ptr_Function_v4float Function
               OpStore %h22 %30
               OpStore %hugeM22 %34
         %36 = OpFMul %v2float %33 %33
         %37 = OpFMul %v2float %33 %33
         %38 = OpCompositeConstruct %mat2v2float %36 %37
               OpStore %h22 %38
               OpStore %h22 %44
         %47 = OpAccessChain %_ptr_Uniform_mat2v2float %7 %int_2
         %51 = OpLoad %mat2v2float %47
         %56 = OpCompositeExtract %v2float %51 0
         %57 = OpFMul %v2float %56 %53
         %58 = OpCompositeExtract %v2float %51 1
         %59 = OpFMul %v2float %58 %54
         %60 = OpCompositeConstruct %mat2v2float %57 %59
               OpStore %f22 %60
         %64 = OpAccessChain %_ptr_Uniform_mat3v3float %7 %int_3
         %67 = OpLoad %mat3v3float %64
         %71 = OpCompositeExtract %v3float %67 0
         %72 = OpFMul %v3float %71 %69
         %73 = OpCompositeExtract %v3float %67 1
         %74 = OpFMul %v3float %73 %69
         %75 = OpCompositeExtract %v3float %67 2
         %76 = OpFMul %v3float %75 %69
         %77 = OpCompositeConstruct %mat3v3float %72 %74 %76
               OpStore %h33 %77
         %81 = OpFOrdEqual %v2bool %42 %42
         %82 = OpAll %bool %81
         %83 = OpFOrdEqual %v2bool %43 %43
         %84 = OpAll %bool %83
         %85 = OpLogicalAnd %bool %82 %84
               OpSelectionMerge %87 None
               OpBranchConditional %85 %86 %87
         %86 = OpLabel
         %91 = OpFOrdEqual %v2bool %57 %53
         %92 = OpAll %bool %91
         %93 = OpFOrdEqual %v2bool %59 %89
         %94 = OpAll %bool %93
         %95 = OpLogicalAnd %bool %92 %94
               OpBranch %87
         %87 = OpLabel
         %96 = OpPhi %bool %false %25 %95 %86
               OpSelectionMerge %98 None
               OpBranchConditional %96 %97 %98
         %97 = OpLabel
        %110 = OpFOrdEqual %v3bool %72 %105
        %111 = OpAll %bool %110
        %112 = OpFOrdEqual %v3bool %74 %106
        %113 = OpAll %bool %112
        %114 = OpLogicalAnd %bool %111 %113
        %115 = OpFOrdEqual %v3bool %76 %107
        %116 = OpAll %bool %115
        %117 = OpLogicalAnd %bool %114 %116
               OpBranch %98
         %98 = OpLabel
        %118 = OpPhi %bool %false %87 %117 %97
               OpSelectionMerge %123 None
               OpBranchConditional %118 %121 %122
        %121 = OpLabel
        %124 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %127 = OpLoad %v4float %124
               OpStore %119 %127
               OpBranch %123
        %122 = OpLabel
        %128 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %130 = OpLoad %v4float %128
               OpStore %119 %130
               OpBranch %123
        %123 = OpLabel
        %131 = OpLoad %v4float %119
               OpReturnValue %131
               OpFunctionEnd
