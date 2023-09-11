               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpMemberName %_UniformBuffer 3 "u_skRTFlip"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %expected "expected"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 3 Offset 16384
               OpDecorate %_UniformBuffer Block
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %expected RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %21 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %26 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
    %float_1 = OpConstant %float 1
        %111 = OpConstantComposite %v2float %float_1 %float_1
        %124 = OpConstantComposite %v2float %float_0 %float_1
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
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
   %expected = OpVariable %_ptr_Function_v4float Function
        %128 = OpVariable %_ptr_Function_v4float Function
               OpStore %expected %26
         %30 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %34 = OpLoad %v4float %30
         %35 = OpCompositeExtract %float %34 0
         %29 = OpDPdy %float %35
         %37 = OpAccessChain %_ptr_Uniform_v2float %8 %int_3
         %39 = OpLoad %v2float %37
         %40 = OpCompositeExtract %float %39 1
         %41 = OpFMul %float %29 %40
         %42 = OpFOrdEqual %bool %41 %float_0
               OpSelectionMerge %44 None
               OpBranchConditional %42 %43 %44
         %43 = OpLabel
         %46 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %47 = OpLoad %v4float %46
         %48 = OpVectorShuffle %v2float %47 %47 0 1
         %45 = OpDPdy %v2float %48
         %49 = OpAccessChain %_ptr_Uniform_v2float %8 %int_3
         %50 = OpLoad %v2float %49
         %51 = OpVectorShuffle %v2float %50 %50 1 1
         %52 = OpFMul %v2float %45 %51
         %53 = OpVectorShuffle %v2float %26 %26 0 1
         %54 = OpFOrdEqual %v2bool %52 %53
         %56 = OpAll %bool %54
               OpBranch %44
         %44 = OpLabel
         %57 = OpPhi %bool %false %23 %56 %43
               OpSelectionMerge %59 None
               OpBranchConditional %57 %58 %59
         %58 = OpLabel
         %61 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %62 = OpLoad %v4float %61
         %63 = OpVectorShuffle %v3float %62 %62 0 1 2
         %60 = OpDPdy %v3float %63
         %65 = OpAccessChain %_ptr_Uniform_v2float %8 %int_3
         %66 = OpLoad %v2float %65
         %67 = OpVectorShuffle %v3float %66 %66 1 1 1
         %68 = OpFMul %v3float %60 %67
         %69 = OpVectorShuffle %v3float %26 %26 0 1 2
         %70 = OpFOrdEqual %v3bool %68 %69
         %72 = OpAll %bool %70
               OpBranch %59
         %59 = OpLabel
         %73 = OpPhi %bool %false %44 %72 %58
               OpSelectionMerge %75 None
               OpBranchConditional %73 %74 %75
         %74 = OpLabel
         %77 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %78 = OpLoad %v4float %77
         %76 = OpDPdy %v4float %78
         %79 = OpAccessChain %_ptr_Uniform_v2float %8 %int_3
         %80 = OpLoad %v2float %79
         %81 = OpVectorShuffle %v4float %80 %80 1 1 1 1
         %82 = OpFMul %v4float %76 %81
         %83 = OpFOrdEqual %v4bool %82 %26
         %85 = OpAll %bool %83
               OpBranch %75
         %75 = OpLabel
         %86 = OpPhi %bool %false %59 %85 %74
               OpSelectionMerge %88 None
               OpBranchConditional %86 %87 %88
         %87 = OpLabel
         %91 = OpLoad %v2float %22
         %92 = OpVectorShuffle %v2float %91 %91 0 0
         %90 = OpDPdy %v2float %92
         %93 = OpAccessChain %_ptr_Uniform_v2float %8 %int_3
         %94 = OpLoad %v2float %93
         %95 = OpVectorShuffle %v2float %94 %94 1 1
         %96 = OpFMul %v2float %90 %95
         %89 = OpExtInst %v2float %1 FSign %96
         %97 = OpFOrdEqual %v2bool %89 %17
         %98 = OpAll %bool %97
               OpBranch %88
         %88 = OpLabel
         %99 = OpPhi %bool %false %75 %98 %87
               OpSelectionMerge %101 None
               OpBranchConditional %99 %100 %101
        %100 = OpLabel
        %104 = OpLoad %v2float %22
        %105 = OpVectorShuffle %v2float %104 %104 1 1
        %103 = OpDPdy %v2float %105
        %106 = OpAccessChain %_ptr_Uniform_v2float %8 %int_3
        %107 = OpLoad %v2float %106
        %108 = OpVectorShuffle %v2float %107 %107 1 1
        %109 = OpFMul %v2float %103 %108
        %102 = OpExtInst %v2float %1 FSign %109
        %112 = OpFOrdEqual %v2bool %102 %111
        %113 = OpAll %bool %112
               OpBranch %101
        %101 = OpLabel
        %114 = OpPhi %bool %false %88 %113 %100
               OpSelectionMerge %116 None
               OpBranchConditional %114 %115 %116
        %115 = OpLabel
        %119 = OpLoad %v2float %22
        %118 = OpDPdy %v2float %119
        %120 = OpAccessChain %_ptr_Uniform_v2float %8 %int_3
        %121 = OpLoad %v2float %120
        %122 = OpVectorShuffle %v2float %121 %121 1 1
        %123 = OpFMul %v2float %118 %122
        %117 = OpExtInst %v2float %1 FSign %123
        %125 = OpFOrdEqual %v2bool %117 %124
        %126 = OpAll %bool %125
               OpBranch %116
        %116 = OpLabel
        %127 = OpPhi %bool %false %101 %126 %115
               OpSelectionMerge %131 None
               OpBranchConditional %127 %129 %130
        %129 = OpLabel
        %132 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %134 = OpLoad %v4float %132
               OpStore %128 %134
               OpBranch %131
        %130 = OpLabel
        %135 = OpAccessChain %_ptr_Uniform_v4float %8 %int_2
        %137 = OpLoad %v4float %135
               OpStore %128 %137
               OpBranch %131
        %131 = OpLabel
        %138 = OpLoad %v4float %128
               OpReturnValue %138
               OpFunctionEnd
