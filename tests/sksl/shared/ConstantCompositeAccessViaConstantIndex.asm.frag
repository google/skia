               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %globalArray "globalArray"
               OpName %globalMatrix "globalMatrix"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorRed"
               OpMemberName %_UniformBuffer 1 "testMatrix2x2"
               OpMemberName %_UniformBuffer 2 "testArray"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %localArray "localArray"
               OpName %localMatrix "localMatrix"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %globalArray RelaxedPrecision
               OpDecorate %_arr_float_int_5 ArrayStride 16
               OpDecorate %13 RelaxedPrecision
               OpDecorate %globalMatrix RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 ColMajor
               OpMemberDecorate %_UniformBuffer 1 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 48
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %20 Binding 0
               OpDecorate %20 DescriptorSet 0
               OpDecorate %localArray RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %localMatrix RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
        %int = OpTypeInt 32 1
      %int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5
%_ptr_Private__arr_float_int_5 = OpTypePointer Private %_arr_float_int_5
%globalArray = OpVariable %_ptr_Private__arr_float_int_5 Private
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Private_mat2v2float = OpTypePointer Private %mat2v2float
%globalMatrix = OpVariable %_ptr_Private_mat2v2float Private
         %18 = OpConstantComposite %v2float %float_1 %float_1
         %19 = OpConstantComposite %mat2v2float %18 %18
%_UniformBuffer = OpTypeStruct %v4float %mat2v2float %_arr_float_int_5
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %20 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %25 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %28 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %32 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function__arr_float_int_5 = OpTypePointer Function %_arr_float_int_5
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
         %43 = OpConstantComposite %v2float %float_0 %float_1
         %44 = OpConstantComposite %v2float %float_2 %float_3
         %45 = OpConstantComposite %mat2v2float %43 %44
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
%_ptr_Uniform__arr_float_int_5 = OpTypePointer Uniform %_arr_float_int_5
      %int_2 = OpConstant %int 2
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
     %v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
      %int_1 = OpConstant %int 1
        %134 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%_entrypoint_v = OpFunction %void None %25
         %26 = OpLabel
         %29 = OpVariable %_ptr_Function_v2float Function
               OpStore %29 %28
         %31 = OpFunctionCall %v4float %main %29
               OpStore %sk_FragColor %31
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %32
         %33 = OpFunctionParameter %_ptr_Function_v2float
         %34 = OpLabel
 %localArray = OpVariable %_ptr_Function__arr_float_int_5 Function
%localMatrix = OpVariable %_ptr_Function_mat2v2float Function
         %13 = OpCompositeConstruct %_arr_float_int_5 %float_1 %float_1 %float_1 %float_1 %float_1
               OpStore %globalArray %13
               OpStore %globalMatrix %19
         %40 = OpCompositeConstruct %_arr_float_int_5 %float_0 %float_1 %float_2 %float_3 %float_4
               OpStore %localArray %40
               OpStore %localMatrix %45
         %48 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %20 %int_2
         %51 = OpLoad %_arr_float_int_5 %48
         %52 = OpCompositeExtract %float %51 0
         %53 = OpFOrdEqual %bool %float_1 %52
         %54 = OpCompositeExtract %float %51 1
         %55 = OpFOrdEqual %bool %float_1 %54
         %56 = OpLogicalAnd %bool %55 %53
         %57 = OpCompositeExtract %float %51 2
         %58 = OpFOrdEqual %bool %float_1 %57
         %59 = OpLogicalAnd %bool %58 %56
         %60 = OpCompositeExtract %float %51 3
         %61 = OpFOrdEqual %bool %float_1 %60
         %62 = OpLogicalAnd %bool %61 %59
         %63 = OpCompositeExtract %float %51 4
         %64 = OpFOrdEqual %bool %float_1 %63
         %65 = OpLogicalAnd %bool %64 %62
               OpSelectionMerge %67 None
               OpBranchConditional %65 %67 %66
         %66 = OpLabel
         %68 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
         %71 = OpLoad %v4float %68
         %72 = OpVectorShuffle %v2float %71 %71 0 1
         %73 = OpFOrdEqual %v2bool %18 %72
         %75 = OpAll %bool %73
               OpBranch %67
         %67 = OpLabel
         %76 = OpPhi %bool %true %34 %75 %66
               OpSelectionMerge %78 None
               OpBranchConditional %76 %78 %77
         %77 = OpLabel
         %79 = OpAccessChain %_ptr_Uniform_mat2v2float %20 %int_1
         %82 = OpLoad %mat2v2float %79
         %83 = OpCompositeExtract %v2float %82 0
         %84 = OpFOrdEqual %v2bool %18 %83
         %85 = OpAll %bool %84
         %86 = OpCompositeExtract %v2float %82 1
         %87 = OpFOrdEqual %v2bool %18 %86
         %88 = OpAll %bool %87
         %89 = OpLogicalAnd %bool %85 %88
               OpBranch %78
         %78 = OpLabel
         %90 = OpPhi %bool %true %67 %89 %77
               OpSelectionMerge %92 None
               OpBranchConditional %90 %92 %91
         %91 = OpLabel
         %93 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %20 %int_2
         %94 = OpLoad %_arr_float_int_5 %93
         %95 = OpCompositeExtract %float %94 0
         %96 = OpFOrdEqual %bool %float_0 %95
         %97 = OpCompositeExtract %float %94 1
         %98 = OpFOrdEqual %bool %float_1 %97
         %99 = OpLogicalAnd %bool %98 %96
        %100 = OpCompositeExtract %float %94 2
        %101 = OpFOrdEqual %bool %float_2 %100
        %102 = OpLogicalAnd %bool %101 %99
        %103 = OpCompositeExtract %float %94 3
        %104 = OpFOrdEqual %bool %float_3 %103
        %105 = OpLogicalAnd %bool %104 %102
        %106 = OpCompositeExtract %float %94 4
        %107 = OpFOrdEqual %bool %float_4 %106
        %108 = OpLogicalAnd %bool %107 %105
               OpBranch %92
         %92 = OpLabel
        %109 = OpPhi %bool %true %78 %108 %91
               OpSelectionMerge %111 None
               OpBranchConditional %109 %111 %110
        %110 = OpLabel
        %112 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
        %113 = OpLoad %v4float %112
        %114 = OpVectorShuffle %v2float %113 %113 0 1
        %115 = OpFOrdEqual %v2bool %18 %114
        %116 = OpAll %bool %115
               OpBranch %111
        %111 = OpLabel
        %117 = OpPhi %bool %true %92 %116 %110
               OpSelectionMerge %119 None
               OpBranchConditional %117 %119 %118
        %118 = OpLabel
        %120 = OpAccessChain %_ptr_Uniform_mat2v2float %20 %int_1
        %121 = OpLoad %mat2v2float %120
        %122 = OpCompositeExtract %v2float %121 0
        %123 = OpFOrdEqual %v2bool %43 %122
        %124 = OpAll %bool %123
        %125 = OpCompositeExtract %v2float %121 1
        %126 = OpFOrdEqual %v2bool %44 %125
        %127 = OpAll %bool %126
        %128 = OpLogicalAnd %bool %124 %127
               OpBranch %119
        %119 = OpLabel
        %129 = OpPhi %bool %true %111 %128 %118
               OpSelectionMerge %131 None
               OpBranchConditional %129 %130 %131
        %130 = OpLabel
        %132 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
        %133 = OpLoad %v4float %132
               OpReturnValue %133
        %131 = OpLabel
               OpReturnValue %134
               OpFunctionEnd
