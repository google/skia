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
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %expected RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1_5625 = OpConstant %float -1.5625
 %float_0_75 = OpConstant %float 0.75
%float_3_375 = OpConstant %float 3.375
         %28 = OpConstantComposite %v4float %float_n1_5625 %float_0 %float_0_75 %float_3_375
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %47 = OpConstantComposite %v2float %float_2 %float_3
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
    %float_1 = OpConstant %float 1
         %61 = OpConstantComposite %v3float %float_2 %float_3 %float_1
     %v3bool = OpTypeVector %bool 3
  %float_1_5 = OpConstant %float 1.5
         %73 = OpConstantComposite %v4float %float_2 %float_3 %float_1 %float_1_5
     %v4bool = OpTypeVector %bool 4
%float_1_5625 = OpConstant %float 1.5625
         %85 = OpConstantComposite %v2float %float_1_5625 %float_0
         %92 = OpConstantComposite %v3float %float_1_5625 %float_0 %float_0_75
         %99 = OpConstantComposite %v4float %float_1_5625 %float_0 %float_0_75 %float_3_375
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
   %expected = OpVariable %_ptr_Function_v4float Function
        %103 = OpVariable %_ptr_Function_v4float Function
               OpStore %expected %28
         %32 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %36 = OpLoad %v4float %32
         %37 = OpCompositeExtract %float %36 0
         %31 = OpExtInst %float %1 Pow %37 %float_2
         %39 = OpFOrdEqual %bool %31 %float_n1_5625
               OpSelectionMerge %41 None
               OpBranchConditional %39 %40 %41
         %40 = OpLabel
         %43 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %44 = OpLoad %v4float %43
         %45 = OpVectorShuffle %v2float %44 %44 0 1
         %42 = OpExtInst %v2float %1 Pow %45 %47
         %48 = OpVectorShuffle %v2float %28 %28 0 1
         %49 = OpFOrdEqual %v2bool %42 %48
         %51 = OpAll %bool %49
               OpBranch %41
         %41 = OpLabel
         %52 = OpPhi %bool %false %22 %51 %40
               OpSelectionMerge %54 None
               OpBranchConditional %52 %53 %54
         %53 = OpLabel
         %56 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %57 = OpLoad %v4float %56
         %58 = OpVectorShuffle %v3float %57 %57 0 1 2
         %55 = OpExtInst %v3float %1 Pow %58 %61
         %62 = OpVectorShuffle %v3float %28 %28 0 1 2
         %63 = OpFOrdEqual %v3bool %55 %62
         %65 = OpAll %bool %63
               OpBranch %54
         %54 = OpLabel
         %66 = OpPhi %bool %false %41 %65 %53
               OpSelectionMerge %68 None
               OpBranchConditional %66 %67 %68
         %67 = OpLabel
         %70 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %71 = OpLoad %v4float %70
         %69 = OpExtInst %v4float %1 Pow %71 %73
         %74 = OpFOrdEqual %v4bool %69 %28
         %76 = OpAll %bool %74
               OpBranch %68
         %68 = OpLabel
         %77 = OpPhi %bool %false %54 %76 %67
               OpSelectionMerge %79 None
               OpBranchConditional %77 %78 %79
         %78 = OpLabel
         %81 = OpFOrdEqual %bool %float_1_5625 %float_n1_5625
               OpBranch %79
         %79 = OpLabel
         %82 = OpPhi %bool %false %68 %81 %78
               OpSelectionMerge %84 None
               OpBranchConditional %82 %83 %84
         %83 = OpLabel
         %86 = OpVectorShuffle %v2float %28 %28 0 1
         %87 = OpFOrdEqual %v2bool %85 %86
         %88 = OpAll %bool %87
               OpBranch %84
         %84 = OpLabel
         %89 = OpPhi %bool %false %79 %88 %83
               OpSelectionMerge %91 None
               OpBranchConditional %89 %90 %91
         %90 = OpLabel
         %93 = OpVectorShuffle %v3float %28 %28 0 1 2
         %94 = OpFOrdEqual %v3bool %92 %93
         %95 = OpAll %bool %94
               OpBranch %91
         %91 = OpLabel
         %96 = OpPhi %bool %false %84 %95 %90
               OpSelectionMerge %98 None
               OpBranchConditional %96 %97 %98
         %97 = OpLabel
        %100 = OpFOrdEqual %v4bool %99 %28
        %101 = OpAll %bool %100
               OpBranch %98
         %98 = OpLabel
        %102 = OpPhi %bool %false %91 %101 %97
               OpSelectionMerge %106 None
               OpBranchConditional %102 %104 %105
        %104 = OpLabel
        %107 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %109 = OpLoad %v4float %107
               OpStore %103 %109
               OpBranch %106
        %105 = OpLabel
        %110 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %112 = OpLoad %v4float %110
               OpStore %103 %112
               OpBranch %106
        %106 = OpLabel
        %113 = OpLoad %v4float %103
               OpReturnValue %113
               OpFunctionEnd
