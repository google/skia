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
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %f4 "f4"
               OpName %ok "ok"
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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %133 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %21 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
    %v3float = OpTypeVector %float 3
    %float_4 = OpConstant %float 4
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %51 = OpConstantComposite %v2float %float_1 %float_2
         %52 = OpConstantComposite %v2float %float_3 %float_4
         %53 = OpConstantComposite %mat2v2float %51 %52
     %v2bool = OpTypeVector %bool 2
      %false = OpConstantFalse %bool
%mat3v3float = OpTypeMatrix %v3float 3
         %74 = OpConstantComposite %v3float %float_1 %float_2 %float_3
         %75 = OpConstantComposite %v3float %float_4 %float_1 %float_2
         %76 = OpConstantComposite %v3float %float_3 %float_4 %float_1
         %77 = OpConstantComposite %mat3v3float %74 %75 %76
     %v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
        %111 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
        %112 = OpConstantComposite %mat4v4float %111 %111 %111 %111
     %v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %14
         %15 = OpLabel
         %18 = OpVariable %_ptr_Function_v2float Function
               OpStore %18 %17
         %20 = OpFunctionCall %v4float %main %18
               OpStore %sk_FragColor %20
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %21
         %22 = OpFunctionParameter %_ptr_Function_v2float
         %23 = OpLabel
         %f4 = OpVariable %_ptr_Function_v4float Function
         %ok = OpVariable %_ptr_Function_bool Function
        %126 = OpVariable %_ptr_Function_v4float Function
         %26 = OpAccessChain %_ptr_Uniform_mat2v2float %7 %int_2
         %30 = OpLoad %mat2v2float %26
         %31 = OpCompositeExtract %float %30 0 0
         %32 = OpCompositeExtract %float %30 0 1
         %33 = OpCompositeExtract %float %30 1 0
         %34 = OpCompositeExtract %float %30 1 1
         %35 = OpCompositeConstruct %v4float %31 %32 %33 %34
               OpStore %f4 %35
         %39 = OpVectorShuffle %v3float %35 %35 0 1 2
         %42 = OpCompositeExtract %float %39 0
         %43 = OpCompositeExtract %float %39 1
         %44 = OpCompositeConstruct %v2float %42 %43
         %45 = OpCompositeExtract %float %39 2
         %46 = OpCompositeConstruct %v2float %45 %float_4
         %47 = OpCompositeConstruct %mat2v2float %44 %46
         %55 = OpFOrdEqual %v2bool %44 %51
         %56 = OpAll %bool %55
         %57 = OpFOrdEqual %v2bool %46 %52
         %58 = OpAll %bool %57
         %59 = OpLogicalAnd %bool %56 %58
               OpStore %ok %59
               OpSelectionMerge %62 None
               OpBranchConditional %59 %61 %62
         %61 = OpLabel
         %63 = OpVectorShuffle %v2float %35 %35 0 1
         %64 = OpVectorShuffle %v2float %35 %35 2 3
         %65 = OpCompositeExtract %float %63 0
         %66 = OpCompositeExtract %float %63 1
         %67 = OpCompositeExtract %float %64 0
         %68 = OpCompositeConstruct %v3float %65 %66 %67
         %69 = OpCompositeExtract %float %64 1
         %70 = OpCompositeConstruct %v3float %69 %31 %32
         %71 = OpCompositeConstruct %v3float %33 %34 %31
         %73 = OpCompositeConstruct %mat3v3float %68 %70 %71
         %79 = OpFOrdEqual %v3bool %68 %74
         %80 = OpAll %bool %79
         %81 = OpFOrdEqual %v3bool %70 %75
         %82 = OpAll %bool %81
         %83 = OpLogicalAnd %bool %80 %82
         %84 = OpFOrdEqual %v3bool %71 %76
         %85 = OpAll %bool %84
         %86 = OpLogicalAnd %bool %83 %85
               OpBranch %62
         %62 = OpLabel
         %87 = OpPhi %bool %false %23 %86 %61
               OpStore %ok %87
               OpSelectionMerge %89 None
               OpBranchConditional %87 %88 %89
         %88 = OpLabel
         %90 = OpVectorShuffle %v3float %35 %35 0 1 2
         %91 = OpVectorShuffle %v3float %35 %35 3 0 1
         %92 = OpVectorShuffle %v4float %35 %35 2 3 0 1
         %93 = OpVectorShuffle %v2float %35 %35 2 3
         %94 = OpCompositeExtract %float %90 0
         %95 = OpCompositeExtract %float %90 1
         %96 = OpCompositeExtract %float %90 2
         %97 = OpCompositeExtract %float %91 0
         %98 = OpCompositeConstruct %v4float %94 %95 %96 %97
         %99 = OpCompositeExtract %float %91 1
        %100 = OpCompositeExtract %float %91 2
        %101 = OpCompositeExtract %float %92 0
        %102 = OpCompositeExtract %float %92 1
        %103 = OpCompositeConstruct %v4float %99 %100 %101 %102
        %104 = OpCompositeExtract %float %92 2
        %105 = OpCompositeExtract %float %92 3
        %106 = OpCompositeExtract %float %93 0
        %107 = OpCompositeExtract %float %93 1
        %108 = OpCompositeConstruct %v4float %104 %105 %106 %107
        %110 = OpCompositeConstruct %mat4v4float %98 %103 %108 %35
        %114 = OpFOrdEqual %v4bool %98 %111
        %115 = OpAll %bool %114
        %116 = OpFOrdEqual %v4bool %103 %111
        %117 = OpAll %bool %116
        %118 = OpLogicalAnd %bool %115 %117
        %119 = OpFOrdEqual %v4bool %108 %111
        %120 = OpAll %bool %119
        %121 = OpLogicalAnd %bool %118 %120
        %122 = OpFOrdEqual %v4bool %35 %111
        %123 = OpAll %bool %122
        %124 = OpLogicalAnd %bool %121 %123
               OpBranch %89
         %89 = OpLabel
        %125 = OpPhi %bool %false %62 %124 %88
               OpStore %ok %125
               OpSelectionMerge %129 None
               OpBranchConditional %125 %127 %128
        %127 = OpLabel
        %130 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %133 = OpLoad %v4float %130
               OpStore %126 %133
               OpBranch %129
        %128 = OpLabel
        %134 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %136 = OpLoad %v4float %134
               OpStore %126 %136
               OpBranch %129
        %129 = OpLabel
        %137 = OpLoad %v4float %126
               OpReturnValue %137
               OpFunctionEnd
