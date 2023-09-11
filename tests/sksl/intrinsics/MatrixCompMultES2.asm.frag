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
               OpMemberName %_UniformBuffer 2 "testMatrix2x2"
               OpMemberName %_UniformBuffer 3 "testMatrix3x3"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %h22 "h22"
               OpName %hugeM22 "hugeM22"
               OpName %f22 "f22"
               OpName %h33 "h33"
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
               OpMemberDecorate %_UniformBuffer 3 Offset 64
               OpMemberDecorate %_UniformBuffer 3 ColMajor
               OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %h22 RelaxedPrecision
               OpDecorate %hugeM22 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %h33 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
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
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1000000 = OpConstant %float 1000000
         %32 = OpConstantComposite %v2float %float_1000000 %float_1000000
         %33 = OpConstantComposite %mat2v2float %32 %32
%float_1_00000002e_30 = OpConstant %float 1.00000002e+30
         %36 = OpConstantComposite %v2float %float_1_00000002e_30 %float_1_00000002e_30
         %37 = OpConstantComposite %mat2v2float %36 %36
    %float_5 = OpConstant %float 5
   %float_10 = OpConstant %float 10
   %float_15 = OpConstant %float 15
         %45 = OpConstantComposite %v2float %float_0 %float_5
         %46 = OpConstantComposite %v2float %float_10 %float_15
         %47 = OpConstantComposite %mat2v2float %45 %46
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
         %56 = OpConstantComposite %v2float %float_1 %float_0
         %57 = OpConstantComposite %v2float %float_0 %float_1
         %58 = OpConstantComposite %mat2v2float %56 %57
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
      %int_3 = OpConstant %int 3
    %float_2 = OpConstant %float 2
         %72 = OpConstantComposite %v3float %float_2 %float_2 %float_2
         %73 = OpConstantComposite %mat3v3float %72 %72 %72
      %false = OpConstantFalse %bool
     %v2bool = OpTypeVector %bool 2
    %float_4 = OpConstant %float 4
         %91 = OpConstantComposite %v2float %float_0 %float_4
         %92 = OpConstantComposite %mat2v2float %56 %91
    %float_6 = OpConstant %float 6
    %float_8 = OpConstant %float 8
   %float_12 = OpConstant %float 12
   %float_14 = OpConstant %float 14
   %float_16 = OpConstant %float 16
   %float_18 = OpConstant %float 18
        %107 = OpConstantComposite %v3float %float_2 %float_4 %float_6
        %108 = OpConstantComposite %v3float %float_8 %float_10 %float_12
        %109 = OpConstantComposite %v3float %float_14 %float_16 %float_18
        %110 = OpConstantComposite %mat3v3float %107 %108 %109
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
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
        %h22 = OpVariable %_ptr_Function_mat2v2float Function
    %hugeM22 = OpVariable %_ptr_Function_mat2v2float Function
        %f22 = OpVariable %_ptr_Function_mat2v2float Function
        %h33 = OpVariable %_ptr_Function_mat3v3float Function
        %121 = OpVariable %_ptr_Function_v4float Function
               OpStore %h22 %33
               OpStore %hugeM22 %37
         %39 = OpFMul %v2float %36 %36
         %40 = OpFMul %v2float %36 %36
         %41 = OpCompositeConstruct %mat2v2float %39 %40
               OpStore %h22 %41
               OpStore %h22 %47
         %50 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
         %54 = OpLoad %mat2v2float %50
         %59 = OpCompositeExtract %v2float %54 0
         %60 = OpFMul %v2float %59 %56
         %61 = OpCompositeExtract %v2float %54 1
         %62 = OpFMul %v2float %61 %57
         %63 = OpCompositeConstruct %mat2v2float %60 %62
               OpStore %f22 %63
         %67 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
         %70 = OpLoad %mat3v3float %67
         %74 = OpCompositeExtract %v3float %70 0
         %75 = OpFMul %v3float %74 %72
         %76 = OpCompositeExtract %v3float %70 1
         %77 = OpFMul %v3float %76 %72
         %78 = OpCompositeExtract %v3float %70 2
         %79 = OpFMul %v3float %78 %72
         %80 = OpCompositeConstruct %mat3v3float %75 %77 %79
               OpStore %h33 %80
         %83 = OpFOrdEqual %v2bool %45 %45
         %84 = OpAll %bool %83
         %85 = OpFOrdEqual %v2bool %46 %46
         %86 = OpAll %bool %85
         %87 = OpLogicalAnd %bool %84 %86
               OpSelectionMerge %89 None
               OpBranchConditional %87 %88 %89
         %88 = OpLabel
         %93 = OpFOrdEqual %v2bool %60 %56
         %94 = OpAll %bool %93
         %95 = OpFOrdEqual %v2bool %62 %91
         %96 = OpAll %bool %95
         %97 = OpLogicalAnd %bool %94 %96
               OpBranch %89
         %89 = OpLabel
         %98 = OpPhi %bool %false %28 %97 %88
               OpSelectionMerge %100 None
               OpBranchConditional %98 %99 %100
         %99 = OpLabel
        %112 = OpFOrdEqual %v3bool %75 %107
        %113 = OpAll %bool %112
        %114 = OpFOrdEqual %v3bool %77 %108
        %115 = OpAll %bool %114
        %116 = OpLogicalAnd %bool %113 %115
        %117 = OpFOrdEqual %v3bool %79 %109
        %118 = OpAll %bool %117
        %119 = OpLogicalAnd %bool %116 %118
               OpBranch %100
        %100 = OpLabel
        %120 = OpPhi %bool %false %89 %119 %99
               OpSelectionMerge %125 None
               OpBranchConditional %120 %123 %124
        %123 = OpLabel
        %126 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %129 = OpLoad %v4float %126
               OpStore %121 %129
               OpBranch %125
        %124 = OpLabel
        %130 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %132 = OpLoad %v4float %130
               OpStore %121 %132
               OpBranch %125
        %125 = OpLabel
        %133 = OpLoad %v4float %121
               OpReturnValue %133
               OpFunctionEnd
