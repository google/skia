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
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %f4 "f4"
               OpName %ok "ok"
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
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %135 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
%_ptr_Function_bool = OpTypePointer Function %bool
    %v3float = OpTypeVector %float 3
    %float_4 = OpConstant %float 4
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %53 = OpConstantComposite %v2float %float_1 %float_2
         %54 = OpConstantComposite %v2float %float_3 %float_4
         %55 = OpConstantComposite %mat2v2float %53 %54
     %v2bool = OpTypeVector %bool 2
      %false = OpConstantFalse %bool
%mat3v3float = OpTypeMatrix %v3float 3
         %76 = OpConstantComposite %v3float %float_1 %float_2 %float_3
         %77 = OpConstantComposite %v3float %float_4 %float_1 %float_2
         %78 = OpConstantComposite %v3float %float_3 %float_4 %float_1
         %79 = OpConstantComposite %mat3v3float %76 %77 %78
     %v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
        %113 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
        %114 = OpConstantComposite %mat4v4float %113 %113 %113 %113
     %v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
         %18 = OpLabel
         %21 = OpVariable %_ptr_Function_v2float Function
               OpStore %21 %20
         %23 = OpFunctionCall %v4float %main %21
               OpStore %sk_FragColor %23
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %24
         %25 = OpFunctionParameter %_ptr_Function_v2float
         %26 = OpLabel
         %f4 = OpVariable %_ptr_Function_v4float Function
         %ok = OpVariable %_ptr_Function_bool Function
        %128 = OpVariable %_ptr_Function_v4float Function
         %29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
         %33 = OpLoad %mat2v2float %29
         %34 = OpCompositeExtract %float %33 0 0
         %35 = OpCompositeExtract %float %33 0 1
         %36 = OpCompositeExtract %float %33 1 0
         %37 = OpCompositeExtract %float %33 1 1
         %38 = OpCompositeConstruct %v4float %34 %35 %36 %37
               OpStore %f4 %38
         %41 = OpVectorShuffle %v3float %38 %38 0 1 2
         %44 = OpCompositeExtract %float %41 0
         %45 = OpCompositeExtract %float %41 1
         %46 = OpCompositeConstruct %v2float %44 %45
         %47 = OpCompositeExtract %float %41 2
         %48 = OpCompositeConstruct %v2float %47 %float_4
         %49 = OpCompositeConstruct %mat2v2float %46 %48
         %57 = OpFOrdEqual %v2bool %46 %53
         %58 = OpAll %bool %57
         %59 = OpFOrdEqual %v2bool %48 %54
         %60 = OpAll %bool %59
         %61 = OpLogicalAnd %bool %58 %60
               OpStore %ok %61
               OpSelectionMerge %64 None
               OpBranchConditional %61 %63 %64
         %63 = OpLabel
         %65 = OpVectorShuffle %v2float %38 %38 0 1
         %66 = OpVectorShuffle %v2float %38 %38 2 3
         %67 = OpCompositeExtract %float %65 0
         %68 = OpCompositeExtract %float %65 1
         %69 = OpCompositeExtract %float %66 0
         %70 = OpCompositeConstruct %v3float %67 %68 %69
         %71 = OpCompositeExtract %float %66 1
         %72 = OpCompositeConstruct %v3float %71 %34 %35
         %73 = OpCompositeConstruct %v3float %36 %37 %34
         %75 = OpCompositeConstruct %mat3v3float %70 %72 %73
         %81 = OpFOrdEqual %v3bool %70 %76
         %82 = OpAll %bool %81
         %83 = OpFOrdEqual %v3bool %72 %77
         %84 = OpAll %bool %83
         %85 = OpLogicalAnd %bool %82 %84
         %86 = OpFOrdEqual %v3bool %73 %78
         %87 = OpAll %bool %86
         %88 = OpLogicalAnd %bool %85 %87
               OpBranch %64
         %64 = OpLabel
         %89 = OpPhi %bool %false %26 %88 %63
               OpStore %ok %89
               OpSelectionMerge %91 None
               OpBranchConditional %89 %90 %91
         %90 = OpLabel
         %92 = OpVectorShuffle %v3float %38 %38 0 1 2
         %93 = OpVectorShuffle %v3float %38 %38 3 0 1
         %94 = OpVectorShuffle %v4float %38 %38 2 3 0 1
         %95 = OpVectorShuffle %v2float %38 %38 2 3
         %96 = OpCompositeExtract %float %92 0
         %97 = OpCompositeExtract %float %92 1
         %98 = OpCompositeExtract %float %92 2
         %99 = OpCompositeExtract %float %93 0
        %100 = OpCompositeConstruct %v4float %96 %97 %98 %99
        %101 = OpCompositeExtract %float %93 1
        %102 = OpCompositeExtract %float %93 2
        %103 = OpCompositeExtract %float %94 0
        %104 = OpCompositeExtract %float %94 1
        %105 = OpCompositeConstruct %v4float %101 %102 %103 %104
        %106 = OpCompositeExtract %float %94 2
        %107 = OpCompositeExtract %float %94 3
        %108 = OpCompositeExtract %float %95 0
        %109 = OpCompositeExtract %float %95 1
        %110 = OpCompositeConstruct %v4float %106 %107 %108 %109
        %112 = OpCompositeConstruct %mat4v4float %100 %105 %110 %38
        %116 = OpFOrdEqual %v4bool %100 %113
        %117 = OpAll %bool %116
        %118 = OpFOrdEqual %v4bool %105 %113
        %119 = OpAll %bool %118
        %120 = OpLogicalAnd %bool %117 %119
        %121 = OpFOrdEqual %v4bool %110 %113
        %122 = OpAll %bool %121
        %123 = OpLogicalAnd %bool %120 %122
        %124 = OpFOrdEqual %v4bool %38 %113
        %125 = OpAll %bool %124
        %126 = OpLogicalAnd %bool %123 %125
               OpBranch %91
         %91 = OpLabel
        %127 = OpPhi %bool %false %64 %126 %90
               OpStore %ok %127
               OpSelectionMerge %131 None
               OpBranchConditional %127 %129 %130
        %129 = OpLabel
        %132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %135 = OpLoad %v4float %132
               OpStore %128 %135
               OpBranch %131
        %130 = OpLabel
        %136 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %138 = OpLoad %v4float %136
               OpStore %128 %138
               OpBranch %131
        %131 = OpLabel
        %139 = OpLoad %v4float %128
               OpReturnValue %139
               OpFunctionEnd
