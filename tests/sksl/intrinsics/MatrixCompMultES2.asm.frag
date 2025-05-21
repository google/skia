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
               OpDecorate %main RelaxedPrecision
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
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %h33 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
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
         %41 = OpConstantComposite %v2float %float_0 %float_5
         %42 = OpConstantComposite %v2float %float_10 %float_15
         %43 = OpConstantComposite %mat2v2float %41 %42
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
         %51 = OpConstantComposite %v2float %float_1 %float_0
         %52 = OpConstantComposite %v2float %float_0 %float_1
         %53 = OpConstantComposite %mat2v2float %51 %52
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_3 = OpConstant %int 3
    %float_2 = OpConstant %float 2
         %66 = OpConstantComposite %v3float %float_2 %float_2 %float_2
         %67 = OpConstantComposite %mat3v3float %66 %66 %66
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v2bool = OpTypeVector %bool 2
    %float_4 = OpConstant %float 4
         %86 = OpConstantComposite %v2float %float_0 %float_4
         %87 = OpConstantComposite %mat2v2float %51 %86
    %float_6 = OpConstant %float 6
    %float_8 = OpConstant %float 8
   %float_12 = OpConstant %float 12
   %float_14 = OpConstant %float 14
   %float_16 = OpConstant %float 16
   %float_18 = OpConstant %float 18
        %102 = OpConstantComposite %v3float %float_2 %float_4 %float_6
        %103 = OpConstantComposite %v3float %float_8 %float_10 %float_12
        %104 = OpConstantComposite %v3float %float_14 %float_16 %float_18
        %105 = OpConstantComposite %mat3v3float %102 %103 %104
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
        %116 = OpVariable %_ptr_Function_v4float Function
               OpStore %h22 %30
               OpStore %hugeM22 %34
         %35 = OpFMul %v2float %33 %33
         %36 = OpFMul %v2float %33 %33
         %37 = OpCompositeConstruct %mat2v2float %35 %36
               OpStore %h22 %37
               OpStore %h22 %43
         %45 = OpAccessChain %_ptr_Uniform_mat2v2float %7 %int_2
         %49 = OpLoad %mat2v2float %45
         %54 = OpCompositeExtract %v2float %49 0
         %55 = OpFMul %v2float %54 %51
         %56 = OpCompositeExtract %v2float %49 1
         %57 = OpFMul %v2float %56 %52
         %58 = OpCompositeConstruct %mat2v2float %55 %57
               OpStore %f22 %58
         %61 = OpAccessChain %_ptr_Uniform_mat3v3float %7 %int_3
         %64 = OpLoad %mat3v3float %61
         %68 = OpCompositeExtract %v3float %64 0
         %69 = OpFMul %v3float %68 %66
         %70 = OpCompositeExtract %v3float %64 1
         %71 = OpFMul %v3float %70 %66
         %72 = OpCompositeExtract %v3float %64 2
         %73 = OpFMul %v3float %72 %66
         %74 = OpCompositeConstruct %mat3v3float %69 %71 %73
               OpStore %h33 %74
         %78 = OpFOrdEqual %v2bool %41 %41
         %79 = OpAll %bool %78
         %80 = OpFOrdEqual %v2bool %42 %42
         %81 = OpAll %bool %80
         %82 = OpLogicalAnd %bool %79 %81
               OpSelectionMerge %84 None
               OpBranchConditional %82 %83 %84
         %83 = OpLabel
         %88 = OpFOrdEqual %v2bool %55 %51
         %89 = OpAll %bool %88
         %90 = OpFOrdEqual %v2bool %57 %86
         %91 = OpAll %bool %90
         %92 = OpLogicalAnd %bool %89 %91
               OpBranch %84
         %84 = OpLabel
         %93 = OpPhi %bool %false %25 %92 %83
               OpSelectionMerge %95 None
               OpBranchConditional %93 %94 %95
         %94 = OpLabel
        %107 = OpFOrdEqual %v3bool %69 %102
        %108 = OpAll %bool %107
        %109 = OpFOrdEqual %v3bool %71 %103
        %110 = OpAll %bool %109
        %111 = OpLogicalAnd %bool %108 %110
        %112 = OpFOrdEqual %v3bool %73 %104
        %113 = OpAll %bool %112
        %114 = OpLogicalAnd %bool %111 %113
               OpBranch %95
         %95 = OpLabel
        %115 = OpPhi %bool %false %84 %114 %94
               OpSelectionMerge %120 None
               OpBranchConditional %115 %118 %119
        %118 = OpLabel
        %121 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %124 = OpLoad %v4float %121
               OpStore %116 %124
               OpBranch %120
        %119 = OpLabel
        %125 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %127 = OpLoad %v4float %125
               OpStore %116 %127
               OpBranch %120
        %120 = OpLabel
        %128 = OpLoad %v4float %116
               OpReturnValue %128
               OpFunctionEnd
