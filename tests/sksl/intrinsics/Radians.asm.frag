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
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %90 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
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
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%float_n0_021816615 = OpConstant %float -0.021816615
%float_0_000500000024 = OpConstant %float 0.000500000024
         %48 = OpConstantComposite %v2float %float_n0_021816615 %float_0
         %50 = OpConstantComposite %v2float %float_0_000500000024 %float_0_000500000024
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
%float_0_0130899698 = OpConstant %float 0.0130899698
         %64 = OpConstantComposite %v3float %float_n0_021816615 %float_0 %float_0_0130899698
         %66 = OpConstantComposite %v3float %float_0_000500000024 %float_0_000500000024 %float_0_000500000024
     %v3bool = OpTypeVector %bool 3
%float_0_0392699093 = OpConstant %float 0.0392699093
         %78 = OpConstantComposite %v4float %float_n0_021816615 %float_0 %float_0_0130899698 %float_0_0392699093
         %80 = OpConstantComposite %v4float %float_0_000500000024 %float_0_000500000024 %float_0_000500000024 %float_0_000500000024
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
         %83 = OpVariable %_ptr_Function_v4float Function
         %29 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %33 = OpLoad %v4float %29
         %34 = OpCompositeExtract %float %33 0
         %28 = OpExtInst %float %1 Radians %34
         %36 = OpFSub %float %28 %float_n0_021816615
         %27 = OpExtInst %float %1 FAbs %36
         %38 = OpFOrdLessThan %bool %27 %float_0_000500000024
               OpSelectionMerge %40 None
               OpBranchConditional %38 %39 %40
         %39 = OpLabel
         %45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %46 = OpLoad %v4float %45
         %47 = OpVectorShuffle %v2float %46 %46 0 1
         %44 = OpExtInst %v2float %1 Radians %47
         %49 = OpFSub %v2float %44 %48
         %43 = OpExtInst %v2float %1 FAbs %49
         %42 = OpFOrdLessThan %v2bool %43 %50
         %41 = OpAll %bool %42
               OpBranch %40
         %40 = OpLabel
         %52 = OpPhi %bool %false %25 %41 %39
               OpSelectionMerge %54 None
               OpBranchConditional %52 %53 %54
         %53 = OpLabel
         %59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %60 = OpLoad %v4float %59
         %61 = OpVectorShuffle %v3float %60 %60 0 1 2
         %58 = OpExtInst %v3float %1 Radians %61
         %65 = OpFSub %v3float %58 %64
         %57 = OpExtInst %v3float %1 FAbs %65
         %56 = OpFOrdLessThan %v3bool %57 %66
         %55 = OpAll %bool %56
               OpBranch %54
         %54 = OpLabel
         %68 = OpPhi %bool %false %40 %55 %53
               OpSelectionMerge %70 None
               OpBranchConditional %68 %69 %70
         %69 = OpLabel
         %75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %76 = OpLoad %v4float %75
         %74 = OpExtInst %v4float %1 Radians %76
         %79 = OpFSub %v4float %74 %78
         %73 = OpExtInst %v4float %1 FAbs %79
         %72 = OpFOrdLessThan %v4bool %73 %80
         %71 = OpAll %bool %72
               OpBranch %70
         %70 = OpLabel
         %82 = OpPhi %bool %false %54 %71 %69
               OpSelectionMerge %87 None
               OpBranchConditional %82 %85 %86
         %85 = OpLabel
         %88 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %90 = OpLoad %v4float %88
               OpStore %83 %90
               OpBranch %87
         %86 = OpLabel
         %91 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %93 = OpLoad %v4float %91
               OpStore %83 %93
               OpBranch %87
         %87 = OpLabel
         %94 = OpLoad %v4float %83
               OpReturnValue %94
               OpFunctionEnd
