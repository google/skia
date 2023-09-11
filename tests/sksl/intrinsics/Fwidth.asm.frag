               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %expected "expected"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %expected RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %28 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
    %float_1 = OpConstant %float 1
         %77 = OpConstantComposite %v2float %float_1 %float_1
         %88 = OpConstantComposite %v2float %float_1 %float_0
        %108 = OpConstantComposite %v2float %float_0 %float_1
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
   %expected = OpVariable %_ptr_Function_v4float Function
        %120 = OpVariable %_ptr_Function_v4float Function
               OpStore %expected %28
         %31 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %35 = OpLoad %v4float %31
         %36 = OpCompositeExtract %float %35 0
         %30 = OpDPdx %float %36
         %37 = OpFOrdEqual %bool %30 %float_0
               OpSelectionMerge %39 None
               OpBranchConditional %37 %38 %39
         %38 = OpLabel
         %41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %42 = OpLoad %v4float %41
         %43 = OpVectorShuffle %v2float %42 %42 0 1
         %40 = OpDPdx %v2float %43
         %44 = OpVectorShuffle %v2float %28 %28 0 1
         %45 = OpFOrdEqual %v2bool %40 %44
         %47 = OpAll %bool %45
               OpBranch %39
         %39 = OpLabel
         %48 = OpPhi %bool %false %25 %47 %38
               OpSelectionMerge %50 None
               OpBranchConditional %48 %49 %50
         %49 = OpLabel
         %52 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %53 = OpLoad %v4float %52
         %54 = OpVectorShuffle %v3float %53 %53 0 1 2
         %51 = OpDPdx %v3float %54
         %56 = OpVectorShuffle %v3float %28 %28 0 1 2
         %57 = OpFOrdEqual %v3bool %51 %56
         %59 = OpAll %bool %57
               OpBranch %50
         %50 = OpLabel
         %60 = OpPhi %bool %false %39 %59 %49
               OpSelectionMerge %62 None
               OpBranchConditional %60 %61 %62
         %61 = OpLabel
         %64 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %65 = OpLoad %v4float %64
         %63 = OpDPdx %v4float %65
         %66 = OpFOrdEqual %v4bool %63 %28
         %68 = OpAll %bool %66
               OpBranch %62
         %62 = OpLabel
         %69 = OpPhi %bool %false %50 %68 %61
               OpSelectionMerge %71 None
               OpBranchConditional %69 %70 %71
         %70 = OpLabel
         %74 = OpLoad %v2float %24
         %75 = OpVectorShuffle %v2float %74 %74 0 0
         %73 = OpFwidth %v2float %75
         %72 = OpExtInst %v2float %1 FSign %73
         %78 = OpFOrdEqual %v2bool %72 %77
         %79 = OpAll %bool %78
               OpBranch %71
         %71 = OpLabel
         %80 = OpPhi %bool %false %62 %79 %70
               OpSelectionMerge %82 None
               OpBranchConditional %80 %81 %82
         %81 = OpLabel
         %85 = OpLoad %v2float %24
         %86 = OpCompositeExtract %float %85 0
         %87 = OpCompositeConstruct %v2float %86 %float_1
         %84 = OpFwidth %v2float %87
         %83 = OpExtInst %v2float %1 FSign %84
         %89 = OpFOrdEqual %v2bool %83 %88
         %90 = OpAll %bool %89
               OpBranch %82
         %82 = OpLabel
         %91 = OpPhi %bool %false %71 %90 %81
               OpSelectionMerge %93 None
               OpBranchConditional %91 %92 %93
         %92 = OpLabel
         %96 = OpLoad %v2float %24
         %97 = OpVectorShuffle %v2float %96 %96 1 1
         %95 = OpFwidth %v2float %97
         %94 = OpExtInst %v2float %1 FSign %95
         %98 = OpFOrdEqual %v2bool %94 %77
         %99 = OpAll %bool %98
               OpBranch %93
         %93 = OpLabel
        %100 = OpPhi %bool %false %82 %99 %92
               OpSelectionMerge %102 None
               OpBranchConditional %100 %101 %102
        %101 = OpLabel
        %105 = OpLoad %v2float %24
        %106 = OpCompositeExtract %float %105 1
        %107 = OpCompositeConstruct %v2float %float_0 %106
        %104 = OpFwidth %v2float %107
        %103 = OpExtInst %v2float %1 FSign %104
        %109 = OpFOrdEqual %v2bool %103 %108
        %110 = OpAll %bool %109
               OpBranch %102
        %102 = OpLabel
        %111 = OpPhi %bool %false %93 %110 %101
               OpSelectionMerge %113 None
               OpBranchConditional %111 %112 %113
        %112 = OpLabel
        %116 = OpLoad %v2float %24
        %115 = OpFwidth %v2float %116
        %114 = OpExtInst %v2float %1 FSign %115
        %117 = OpFOrdEqual %v2bool %114 %77
        %118 = OpAll %bool %117
               OpBranch %113
        %113 = OpLabel
        %119 = OpPhi %bool %false %102 %118 %112
               OpSelectionMerge %123 None
               OpBranchConditional %119 %121 %122
        %121 = OpLabel
        %124 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %126 = OpLoad %v4float %124
               OpStore %120 %126
               OpBranch %123
        %122 = OpLabel
        %127 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %129 = OpLoad %v4float %127
               OpStore %120 %129
               OpBranch %123
        %123 = OpLabel
        %130 = OpLoad %v4float %120
               OpReturnValue %130
               OpFunctionEnd
