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
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %88 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
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
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%float_n0_021816615 = OpConstant %float -0.021816615
%float_0_000500000024 = OpConstant %float 0.000500000024
         %46 = OpConstantComposite %v2float %float_n0_021816615 %float_0
         %48 = OpConstantComposite %v2float %float_0_000500000024 %float_0_000500000024
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
%float_0_0130899698 = OpConstant %float 0.0130899698
         %62 = OpConstantComposite %v3float %float_n0_021816615 %float_0 %float_0_0130899698
         %64 = OpConstantComposite %v3float %float_0_000500000024 %float_0_000500000024 %float_0_000500000024
     %v3bool = OpTypeVector %bool 3
%float_0_0392699093 = OpConstant %float 0.0392699093
         %76 = OpConstantComposite %v4float %float_n0_021816615 %float_0 %float_0_0130899698 %float_0_0392699093
         %78 = OpConstantComposite %v4float %float_0_000500000024 %float_0_000500000024 %float_0_000500000024 %float_0_000500000024
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
         %81 = OpVariable %_ptr_Function_v4float Function
         %27 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %31 = OpLoad %v4float %27
         %32 = OpCompositeExtract %float %31 0
         %26 = OpExtInst %float %1 Radians %32
         %34 = OpFSub %float %26 %float_n0_021816615
         %25 = OpExtInst %float %1 FAbs %34
         %36 = OpFOrdLessThan %bool %25 %float_0_000500000024
               OpSelectionMerge %38 None
               OpBranchConditional %36 %37 %38
         %37 = OpLabel
         %43 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %44 = OpLoad %v4float %43
         %45 = OpVectorShuffle %v2float %44 %44 0 1
         %42 = OpExtInst %v2float %1 Radians %45
         %47 = OpFSub %v2float %42 %46
         %41 = OpExtInst %v2float %1 FAbs %47
         %40 = OpFOrdLessThan %v2bool %41 %48
         %39 = OpAll %bool %40
               OpBranch %38
         %38 = OpLabel
         %50 = OpPhi %bool %false %22 %39 %37
               OpSelectionMerge %52 None
               OpBranchConditional %50 %51 %52
         %51 = OpLabel
         %57 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %58 = OpLoad %v4float %57
         %59 = OpVectorShuffle %v3float %58 %58 0 1 2
         %56 = OpExtInst %v3float %1 Radians %59
         %63 = OpFSub %v3float %56 %62
         %55 = OpExtInst %v3float %1 FAbs %63
         %54 = OpFOrdLessThan %v3bool %55 %64
         %53 = OpAll %bool %54
               OpBranch %52
         %52 = OpLabel
         %66 = OpPhi %bool %false %38 %53 %51
               OpSelectionMerge %68 None
               OpBranchConditional %66 %67 %68
         %67 = OpLabel
         %73 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %74 = OpLoad %v4float %73
         %72 = OpExtInst %v4float %1 Radians %74
         %77 = OpFSub %v4float %72 %76
         %71 = OpExtInst %v4float %1 FAbs %77
         %70 = OpFOrdLessThan %v4bool %71 %78
         %69 = OpAll %bool %70
               OpBranch %68
         %68 = OpLabel
         %80 = OpPhi %bool %false %52 %69 %67
               OpSelectionMerge %85 None
               OpBranchConditional %80 %83 %84
         %83 = OpLabel
         %86 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %88 = OpLoad %v4float %86
               OpStore %81 %88
               OpBranch %85
         %84 = OpLabel
         %89 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %91 = OpLoad %v4float %89
               OpStore %81 %91
               OpBranch %85
         %85 = OpLabel
         %92 = OpLoad %v4float %81
               OpReturnValue %92
               OpFunctionEnd
