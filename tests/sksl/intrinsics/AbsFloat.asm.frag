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
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %98 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
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
 %float_1_25 = OpConstant %float 1.25
 %float_0_75 = OpConstant %float 0.75
 %float_2_25 = OpConstant %float 2.25
         %28 = OpConstantComposite %v4float %float_1_25 %float_0 %float_0_75 %float_2_25
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
         %77 = OpConstantComposite %v2float %float_1_25 %float_0
         %84 = OpConstantComposite %v3float %float_1_25 %float_0 %float_0_75
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
         %92 = OpVariable %_ptr_Function_v4float Function
               OpStore %expected %28
         %32 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %36 = OpLoad %v4float %32
         %37 = OpCompositeExtract %float %36 0
         %31 = OpExtInst %float %1 FAbs %37
         %38 = OpFOrdEqual %bool %31 %float_1_25
               OpSelectionMerge %40 None
               OpBranchConditional %38 %39 %40
         %39 = OpLabel
         %42 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %43 = OpLoad %v4float %42
         %44 = OpVectorShuffle %v2float %43 %43 0 1
         %41 = OpExtInst %v2float %1 FAbs %44
         %45 = OpVectorShuffle %v2float %28 %28 0 1
         %46 = OpFOrdEqual %v2bool %41 %45
         %48 = OpAll %bool %46
               OpBranch %40
         %40 = OpLabel
         %49 = OpPhi %bool %false %22 %48 %39
               OpSelectionMerge %51 None
               OpBranchConditional %49 %50 %51
         %50 = OpLabel
         %53 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %54 = OpLoad %v4float %53
         %55 = OpVectorShuffle %v3float %54 %54 0 1 2
         %52 = OpExtInst %v3float %1 FAbs %55
         %57 = OpVectorShuffle %v3float %28 %28 0 1 2
         %58 = OpFOrdEqual %v3bool %52 %57
         %60 = OpAll %bool %58
               OpBranch %51
         %51 = OpLabel
         %61 = OpPhi %bool %false %40 %60 %50
               OpSelectionMerge %63 None
               OpBranchConditional %61 %62 %63
         %62 = OpLabel
         %65 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %66 = OpLoad %v4float %65
         %64 = OpExtInst %v4float %1 FAbs %66
         %67 = OpFOrdEqual %v4bool %64 %28
         %69 = OpAll %bool %67
               OpBranch %63
         %63 = OpLabel
         %70 = OpPhi %bool %false %51 %69 %62
               OpSelectionMerge %72 None
               OpBranchConditional %70 %71 %72
         %71 = OpLabel
               OpBranch %72
         %72 = OpLabel
         %74 = OpPhi %bool %false %63 %true %71
               OpSelectionMerge %76 None
               OpBranchConditional %74 %75 %76
         %75 = OpLabel
         %78 = OpVectorShuffle %v2float %28 %28 0 1
         %79 = OpFOrdEqual %v2bool %77 %78
         %80 = OpAll %bool %79
               OpBranch %76
         %76 = OpLabel
         %81 = OpPhi %bool %false %72 %80 %75
               OpSelectionMerge %83 None
               OpBranchConditional %81 %82 %83
         %82 = OpLabel
         %85 = OpVectorShuffle %v3float %28 %28 0 1 2
         %86 = OpFOrdEqual %v3bool %84 %85
         %87 = OpAll %bool %86
               OpBranch %83
         %83 = OpLabel
         %88 = OpPhi %bool %false %76 %87 %82
               OpSelectionMerge %90 None
               OpBranchConditional %88 %89 %90
         %89 = OpLabel
               OpBranch %90
         %90 = OpLabel
         %91 = OpPhi %bool %false %83 %true %89
               OpSelectionMerge %95 None
               OpBranchConditional %91 %93 %94
         %93 = OpLabel
         %96 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %98 = OpLoad %v4float %96
               OpStore %92 %98
               OpBranch %95
         %94 = OpLabel
         %99 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %101 = OpLoad %v4float %99
               OpStore %92 %101
               OpBranch %95
         %95 = OpLabel
        %102 = OpLoad %v4float %92
               OpReturnValue %102
               OpFunctionEnd
