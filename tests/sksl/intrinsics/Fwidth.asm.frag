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
               OpDecorate %28 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
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
         %25 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
    %float_1 = OpConstant %float 1
         %75 = OpConstantComposite %v2float %float_1 %float_1
         %86 = OpConstantComposite %v2float %float_1 %float_0
        %106 = OpConstantComposite %v2float %float_0 %float_1
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
        %118 = OpVariable %_ptr_Function_v4float Function
               OpStore %expected %25
         %29 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %33 = OpLoad %v4float %29
         %34 = OpCompositeExtract %float %33 0
         %28 = OpDPdx %float %34
         %35 = OpFOrdEqual %bool %28 %float_0
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
         %39 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %40 = OpLoad %v4float %39
         %41 = OpVectorShuffle %v2float %40 %40 0 1
         %38 = OpDPdx %v2float %41
         %42 = OpVectorShuffle %v2float %25 %25 0 1
         %43 = OpFOrdEqual %v2bool %38 %42
         %45 = OpAll %bool %43
               OpBranch %37
         %37 = OpLabel
         %46 = OpPhi %bool %false %22 %45 %36
               OpSelectionMerge %48 None
               OpBranchConditional %46 %47 %48
         %47 = OpLabel
         %50 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %51 = OpLoad %v4float %50
         %52 = OpVectorShuffle %v3float %51 %51 0 1 2
         %49 = OpDPdx %v3float %52
         %54 = OpVectorShuffle %v3float %25 %25 0 1 2
         %55 = OpFOrdEqual %v3bool %49 %54
         %57 = OpAll %bool %55
               OpBranch %48
         %48 = OpLabel
         %58 = OpPhi %bool %false %37 %57 %47
               OpSelectionMerge %60 None
               OpBranchConditional %58 %59 %60
         %59 = OpLabel
         %62 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %63 = OpLoad %v4float %62
         %61 = OpDPdx %v4float %63
         %64 = OpFOrdEqual %v4bool %61 %25
         %66 = OpAll %bool %64
               OpBranch %60
         %60 = OpLabel
         %67 = OpPhi %bool %false %48 %66 %59
               OpSelectionMerge %69 None
               OpBranchConditional %67 %68 %69
         %68 = OpLabel
         %72 = OpLoad %v2float %21
         %73 = OpVectorShuffle %v2float %72 %72 0 0
         %71 = OpFwidth %v2float %73
         %70 = OpExtInst %v2float %1 FSign %71
         %76 = OpFOrdEqual %v2bool %70 %75
         %77 = OpAll %bool %76
               OpBranch %69
         %69 = OpLabel
         %78 = OpPhi %bool %false %60 %77 %68
               OpSelectionMerge %80 None
               OpBranchConditional %78 %79 %80
         %79 = OpLabel
         %83 = OpLoad %v2float %21
         %84 = OpCompositeExtract %float %83 0
         %85 = OpCompositeConstruct %v2float %84 %float_1
         %82 = OpFwidth %v2float %85
         %81 = OpExtInst %v2float %1 FSign %82
         %87 = OpFOrdEqual %v2bool %81 %86
         %88 = OpAll %bool %87
               OpBranch %80
         %80 = OpLabel
         %89 = OpPhi %bool %false %69 %88 %79
               OpSelectionMerge %91 None
               OpBranchConditional %89 %90 %91
         %90 = OpLabel
         %94 = OpLoad %v2float %21
         %95 = OpVectorShuffle %v2float %94 %94 1 1
         %93 = OpFwidth %v2float %95
         %92 = OpExtInst %v2float %1 FSign %93
         %96 = OpFOrdEqual %v2bool %92 %75
         %97 = OpAll %bool %96
               OpBranch %91
         %91 = OpLabel
         %98 = OpPhi %bool %false %80 %97 %90
               OpSelectionMerge %100 None
               OpBranchConditional %98 %99 %100
         %99 = OpLabel
        %103 = OpLoad %v2float %21
        %104 = OpCompositeExtract %float %103 1
        %105 = OpCompositeConstruct %v2float %float_0 %104
        %102 = OpFwidth %v2float %105
        %101 = OpExtInst %v2float %1 FSign %102
        %107 = OpFOrdEqual %v2bool %101 %106
        %108 = OpAll %bool %107
               OpBranch %100
        %100 = OpLabel
        %109 = OpPhi %bool %false %91 %108 %99
               OpSelectionMerge %111 None
               OpBranchConditional %109 %110 %111
        %110 = OpLabel
        %114 = OpLoad %v2float %21
        %113 = OpFwidth %v2float %114
        %112 = OpExtInst %v2float %1 FSign %113
        %115 = OpFOrdEqual %v2bool %112 %75
        %116 = OpAll %bool %115
               OpBranch %111
        %111 = OpLabel
        %117 = OpPhi %bool %false %100 %116 %110
               OpSelectionMerge %121 None
               OpBranchConditional %117 %119 %120
        %119 = OpLabel
        %122 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %124 = OpLoad %v4float %122
               OpStore %118 %124
               OpBranch %121
        %120 = OpLabel
        %125 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %127 = OpLoad %v4float %125
               OpStore %118 %127
               OpBranch %121
        %121 = OpLabel
        %128 = OpLoad %v4float %118
               OpReturnValue %128
               OpFunctionEnd
