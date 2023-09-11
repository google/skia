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
               OpDecorate %77 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
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
   %float_n1 = OpConstant %float -1
    %float_1 = OpConstant %float 1
         %27 = OpConstantComposite %v4float %float_n1 %float_0 %float_1 %float_1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %76 = OpConstantComposite %v2float %float_n1 %float_0
         %83 = OpConstantComposite %v3float %float_n1 %float_0 %float_1
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
         %91 = OpVariable %_ptr_Function_v4float Function
               OpStore %expected %27
         %31 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %35 = OpLoad %v4float %31
         %36 = OpCompositeExtract %float %35 0
         %30 = OpExtInst %float %1 FSign %36
         %37 = OpFOrdEqual %bool %30 %float_n1
               OpSelectionMerge %39 None
               OpBranchConditional %37 %38 %39
         %38 = OpLabel
         %41 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %42 = OpLoad %v4float %41
         %43 = OpVectorShuffle %v2float %42 %42 0 1
         %40 = OpExtInst %v2float %1 FSign %43
         %44 = OpVectorShuffle %v2float %27 %27 0 1
         %45 = OpFOrdEqual %v2bool %40 %44
         %47 = OpAll %bool %45
               OpBranch %39
         %39 = OpLabel
         %48 = OpPhi %bool %false %22 %47 %38
               OpSelectionMerge %50 None
               OpBranchConditional %48 %49 %50
         %49 = OpLabel
         %52 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %53 = OpLoad %v4float %52
         %54 = OpVectorShuffle %v3float %53 %53 0 1 2
         %51 = OpExtInst %v3float %1 FSign %54
         %56 = OpVectorShuffle %v3float %27 %27 0 1 2
         %57 = OpFOrdEqual %v3bool %51 %56
         %59 = OpAll %bool %57
               OpBranch %50
         %50 = OpLabel
         %60 = OpPhi %bool %false %39 %59 %49
               OpSelectionMerge %62 None
               OpBranchConditional %60 %61 %62
         %61 = OpLabel
         %64 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %65 = OpLoad %v4float %64
         %63 = OpExtInst %v4float %1 FSign %65
         %66 = OpFOrdEqual %v4bool %63 %27
         %68 = OpAll %bool %66
               OpBranch %62
         %62 = OpLabel
         %69 = OpPhi %bool %false %50 %68 %61
               OpSelectionMerge %71 None
               OpBranchConditional %69 %70 %71
         %70 = OpLabel
               OpBranch %71
         %71 = OpLabel
         %73 = OpPhi %bool %false %62 %true %70
               OpSelectionMerge %75 None
               OpBranchConditional %73 %74 %75
         %74 = OpLabel
         %77 = OpVectorShuffle %v2float %27 %27 0 1
         %78 = OpFOrdEqual %v2bool %76 %77
         %79 = OpAll %bool %78
               OpBranch %75
         %75 = OpLabel
         %80 = OpPhi %bool %false %71 %79 %74
               OpSelectionMerge %82 None
               OpBranchConditional %80 %81 %82
         %81 = OpLabel
         %84 = OpVectorShuffle %v3float %27 %27 0 1 2
         %85 = OpFOrdEqual %v3bool %83 %84
         %86 = OpAll %bool %85
               OpBranch %82
         %82 = OpLabel
         %87 = OpPhi %bool %false %75 %86 %81
               OpSelectionMerge %89 None
               OpBranchConditional %87 %88 %89
         %88 = OpLabel
               OpBranch %89
         %89 = OpLabel
         %90 = OpPhi %bool %false %82 %true %88
               OpSelectionMerge %94 None
               OpBranchConditional %90 %92 %93
         %92 = OpLabel
         %95 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %97 = OpLoad %v4float %95
               OpStore %91 %97
               OpBranch %94
         %93 = OpLabel
         %98 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %100 = OpLoad %v4float %98
               OpStore %91 %100
               OpBranch %94
         %94 = OpLabel
        %101 = OpLoad %v4float %91
               OpReturnValue %101
               OpFunctionEnd
