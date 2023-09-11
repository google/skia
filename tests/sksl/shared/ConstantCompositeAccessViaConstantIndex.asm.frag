               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
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
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %globalArray RelaxedPrecision
               OpDecorate %_arr_float_int_5 ArrayStride 16
               OpDecorate %16 RelaxedPrecision
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
               OpDecorate %23 Binding 0
               OpDecorate %23 DescriptorSet 0
               OpDecorate %localArray RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %localMatrix RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
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
         %21 = OpConstantComposite %v2float %float_1 %float_1
         %22 = OpConstantComposite %mat2v2float %21 %21
%_UniformBuffer = OpTypeStruct %v4float %mat2v2float %_arr_float_int_5
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %23 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %28 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %31 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %35 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function__arr_float_int_5 = OpTypePointer Function %_arr_float_int_5
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
         %46 = OpConstantComposite %v2float %float_0 %float_1
         %47 = OpConstantComposite %v2float %float_2 %float_3
         %48 = OpConstantComposite %mat2v2float %46 %47
       %true = OpConstantTrue %bool
%_ptr_Uniform__arr_float_int_5 = OpTypePointer Uniform %_arr_float_int_5
      %int_2 = OpConstant %int 2
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
     %v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
      %int_1 = OpConstant %int 1
        %136 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%_entrypoint_v = OpFunction %void None %28
         %29 = OpLabel
         %32 = OpVariable %_ptr_Function_v2float Function
               OpStore %32 %31
         %34 = OpFunctionCall %v4float %main %32
               OpStore %sk_FragColor %34
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %35
         %36 = OpFunctionParameter %_ptr_Function_v2float
         %37 = OpLabel
 %localArray = OpVariable %_ptr_Function__arr_float_int_5 Function
%localMatrix = OpVariable %_ptr_Function_mat2v2float Function
         %16 = OpCompositeConstruct %_arr_float_int_5 %float_1 %float_1 %float_1 %float_1 %float_1
               OpStore %globalArray %16
               OpStore %globalMatrix %22
         %43 = OpCompositeConstruct %_arr_float_int_5 %float_0 %float_1 %float_2 %float_3 %float_4
               OpStore %localArray %43
               OpStore %localMatrix %48
         %50 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %23 %int_2
         %53 = OpLoad %_arr_float_int_5 %50
         %54 = OpCompositeExtract %float %53 0
         %55 = OpFOrdEqual %bool %float_1 %54
         %56 = OpCompositeExtract %float %53 1
         %57 = OpFOrdEqual %bool %float_1 %56
         %58 = OpLogicalAnd %bool %57 %55
         %59 = OpCompositeExtract %float %53 2
         %60 = OpFOrdEqual %bool %float_1 %59
         %61 = OpLogicalAnd %bool %60 %58
         %62 = OpCompositeExtract %float %53 3
         %63 = OpFOrdEqual %bool %float_1 %62
         %64 = OpLogicalAnd %bool %63 %61
         %65 = OpCompositeExtract %float %53 4
         %66 = OpFOrdEqual %bool %float_1 %65
         %67 = OpLogicalAnd %bool %66 %64
               OpSelectionMerge %69 None
               OpBranchConditional %67 %69 %68
         %68 = OpLabel
         %70 = OpAccessChain %_ptr_Uniform_v4float %23 %int_0
         %73 = OpLoad %v4float %70
         %74 = OpVectorShuffle %v2float %73 %73 0 1
         %75 = OpFOrdEqual %v2bool %21 %74
         %77 = OpAll %bool %75
               OpBranch %69
         %69 = OpLabel
         %78 = OpPhi %bool %true %37 %77 %68
               OpSelectionMerge %80 None
               OpBranchConditional %78 %80 %79
         %79 = OpLabel
         %81 = OpAccessChain %_ptr_Uniform_mat2v2float %23 %int_1
         %84 = OpLoad %mat2v2float %81
         %85 = OpCompositeExtract %v2float %84 0
         %86 = OpFOrdEqual %v2bool %21 %85
         %87 = OpAll %bool %86
         %88 = OpCompositeExtract %v2float %84 1
         %89 = OpFOrdEqual %v2bool %21 %88
         %90 = OpAll %bool %89
         %91 = OpLogicalAnd %bool %87 %90
               OpBranch %80
         %80 = OpLabel
         %92 = OpPhi %bool %true %69 %91 %79
               OpSelectionMerge %94 None
               OpBranchConditional %92 %94 %93
         %93 = OpLabel
         %95 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %23 %int_2
         %96 = OpLoad %_arr_float_int_5 %95
         %97 = OpCompositeExtract %float %96 0
         %98 = OpFOrdEqual %bool %float_0 %97
         %99 = OpCompositeExtract %float %96 1
        %100 = OpFOrdEqual %bool %float_1 %99
        %101 = OpLogicalAnd %bool %100 %98
        %102 = OpCompositeExtract %float %96 2
        %103 = OpFOrdEqual %bool %float_2 %102
        %104 = OpLogicalAnd %bool %103 %101
        %105 = OpCompositeExtract %float %96 3
        %106 = OpFOrdEqual %bool %float_3 %105
        %107 = OpLogicalAnd %bool %106 %104
        %108 = OpCompositeExtract %float %96 4
        %109 = OpFOrdEqual %bool %float_4 %108
        %110 = OpLogicalAnd %bool %109 %107
               OpBranch %94
         %94 = OpLabel
        %111 = OpPhi %bool %true %80 %110 %93
               OpSelectionMerge %113 None
               OpBranchConditional %111 %113 %112
        %112 = OpLabel
        %114 = OpAccessChain %_ptr_Uniform_v4float %23 %int_0
        %115 = OpLoad %v4float %114
        %116 = OpVectorShuffle %v2float %115 %115 0 1
        %117 = OpFOrdEqual %v2bool %21 %116
        %118 = OpAll %bool %117
               OpBranch %113
        %113 = OpLabel
        %119 = OpPhi %bool %true %94 %118 %112
               OpSelectionMerge %121 None
               OpBranchConditional %119 %121 %120
        %120 = OpLabel
        %122 = OpAccessChain %_ptr_Uniform_mat2v2float %23 %int_1
        %123 = OpLoad %mat2v2float %122
        %124 = OpCompositeExtract %v2float %123 0
        %125 = OpFOrdEqual %v2bool %46 %124
        %126 = OpAll %bool %125
        %127 = OpCompositeExtract %v2float %123 1
        %128 = OpFOrdEqual %v2bool %47 %127
        %129 = OpAll %bool %128
        %130 = OpLogicalAnd %bool %126 %129
               OpBranch %121
        %121 = OpLabel
        %131 = OpPhi %bool %true %113 %130 %120
               OpSelectionMerge %133 None
               OpBranchConditional %131 %132 %133
        %132 = OpLabel
        %134 = OpAccessChain %_ptr_Uniform_v4float %23 %int_0
        %135 = OpLoad %v4float %134
               OpReturnValue %135
        %133 = OpLabel
               OpReturnValue %136
               OpFunctionEnd
