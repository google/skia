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
               OpDecorate %33 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
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
%float_n1_5625 = OpConstant %float -1.5625
 %float_0_75 = OpConstant %float 0.75
%float_3_375 = OpConstant %float 3.375
         %31 = OpConstantComposite %v4float %float_n1_5625 %float_0 %float_0_75 %float_3_375
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %49 = OpConstantComposite %v2float %float_2 %float_3
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
    %float_1 = OpConstant %float 1
         %63 = OpConstantComposite %v3float %float_2 %float_3 %float_1
     %v3bool = OpTypeVector %bool 3
  %float_1_5 = OpConstant %float 1.5
         %75 = OpConstantComposite %v4float %float_2 %float_3 %float_1 %float_1_5
     %v4bool = OpTypeVector %bool 4
%float_1_5625 = OpConstant %float 1.5625
         %87 = OpConstantComposite %v2float %float_1_5625 %float_0
         %94 = OpConstantComposite %v3float %float_1_5625 %float_0 %float_0_75
        %101 = OpConstantComposite %v4float %float_1_5625 %float_0 %float_0_75 %float_3_375
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
        %105 = OpVariable %_ptr_Function_v4float Function
               OpStore %expected %31
         %34 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %38 = OpLoad %v4float %34
         %39 = OpCompositeExtract %float %38 0
         %33 = OpExtInst %float %1 Pow %39 %float_2
         %41 = OpFOrdEqual %bool %33 %float_n1_5625
               OpSelectionMerge %43 None
               OpBranchConditional %41 %42 %43
         %42 = OpLabel
         %45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %46 = OpLoad %v4float %45
         %47 = OpVectorShuffle %v2float %46 %46 0 1
         %44 = OpExtInst %v2float %1 Pow %47 %49
         %50 = OpVectorShuffle %v2float %31 %31 0 1
         %51 = OpFOrdEqual %v2bool %44 %50
         %53 = OpAll %bool %51
               OpBranch %43
         %43 = OpLabel
         %54 = OpPhi %bool %false %25 %53 %42
               OpSelectionMerge %56 None
               OpBranchConditional %54 %55 %56
         %55 = OpLabel
         %58 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %59 = OpLoad %v4float %58
         %60 = OpVectorShuffle %v3float %59 %59 0 1 2
         %57 = OpExtInst %v3float %1 Pow %60 %63
         %64 = OpVectorShuffle %v3float %31 %31 0 1 2
         %65 = OpFOrdEqual %v3bool %57 %64
         %67 = OpAll %bool %65
               OpBranch %56
         %56 = OpLabel
         %68 = OpPhi %bool %false %43 %67 %55
               OpSelectionMerge %70 None
               OpBranchConditional %68 %69 %70
         %69 = OpLabel
         %72 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %73 = OpLoad %v4float %72
         %71 = OpExtInst %v4float %1 Pow %73 %75
         %76 = OpFOrdEqual %v4bool %71 %31
         %78 = OpAll %bool %76
               OpBranch %70
         %70 = OpLabel
         %79 = OpPhi %bool %false %56 %78 %69
               OpSelectionMerge %81 None
               OpBranchConditional %79 %80 %81
         %80 = OpLabel
         %83 = OpFOrdEqual %bool %float_1_5625 %float_n1_5625
               OpBranch %81
         %81 = OpLabel
         %84 = OpPhi %bool %false %70 %83 %80
               OpSelectionMerge %86 None
               OpBranchConditional %84 %85 %86
         %85 = OpLabel
         %88 = OpVectorShuffle %v2float %31 %31 0 1
         %89 = OpFOrdEqual %v2bool %87 %88
         %90 = OpAll %bool %89
               OpBranch %86
         %86 = OpLabel
         %91 = OpPhi %bool %false %81 %90 %85
               OpSelectionMerge %93 None
               OpBranchConditional %91 %92 %93
         %92 = OpLabel
         %95 = OpVectorShuffle %v3float %31 %31 0 1 2
         %96 = OpFOrdEqual %v3bool %94 %95
         %97 = OpAll %bool %96
               OpBranch %93
         %93 = OpLabel
         %98 = OpPhi %bool %false %86 %97 %92
               OpSelectionMerge %100 None
               OpBranchConditional %98 %99 %100
         %99 = OpLabel
        %102 = OpFOrdEqual %v4bool %101 %31
        %103 = OpAll %bool %102
               OpBranch %100
        %100 = OpLabel
        %104 = OpPhi %bool %false %93 %103 %99
               OpSelectionMerge %108 None
               OpBranchConditional %104 %106 %107
        %106 = OpLabel
        %109 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %111 = OpLoad %v4float %109
               OpStore %105 %111
               OpBranch %108
        %107 = OpLabel
        %112 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %114 = OpLoad %v4float %112
               OpStore %105 %114
               OpBranch %108
        %108 = OpLabel
        %115 = OpLoad %v4float %105
               OpReturnValue %115
               OpFunctionEnd
