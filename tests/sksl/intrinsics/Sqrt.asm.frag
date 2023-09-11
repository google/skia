               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testMatrix2x2"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %inputVal "inputVal"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 ColMajor
               OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 1 Offset 32
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 48
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %102 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %mat2v2float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %21 = OpTypeFunction %v4float %_ptr_Function_v2float
   %float_n1 = OpConstant %float -1
   %float_n4 = OpConstant %float -4
  %float_n16 = OpConstant %float -16
  %float_n64 = OpConstant %float -64
         %29 = OpConstantComposite %v4float %float_n1 %float_n4 %float_n16 %float_n64
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
    %float_6 = OpConstant %float 6
   %float_12 = OpConstant %float 12
         %46 = OpConstantComposite %v4float %float_0 %float_2 %float_6 %float_12
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
    %float_1 = OpConstant %float 1
%float_0_0500000007 = OpConstant %float 0.0500000007
         %64 = OpConstantComposite %v2float %float_1 %float_2
         %66 = OpConstantComposite %v2float %float_0_0500000007 %float_0_0500000007
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
    %float_3 = OpConstant %float 3
         %78 = OpConstantComposite %v3float %float_1 %float_2 %float_3
         %80 = OpConstantComposite %v3float %float_0_0500000007 %float_0_0500000007 %float_0_0500000007
     %v3bool = OpTypeVector %bool 3
    %float_4 = OpConstant %float 4
         %90 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
         %92 = OpConstantComposite %v4float %float_0_0500000007 %float_0_0500000007 %float_0_0500000007 %float_0_0500000007
     %v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
   %inputVal = OpVariable %_ptr_Function_v4float Function
         %95 = OpVariable %_ptr_Function_v4float Function
         %24 = OpExtInst %v4float %1 Sqrt %29
         %30 = OpVectorShuffle %v2float %24 %24 0 1
               OpStore %22 %30
         %33 = OpAccessChain %_ptr_Uniform_mat2v2float %7 %int_0
         %37 = OpLoad %mat2v2float %33
         %38 = OpCompositeExtract %float %37 0 0
         %39 = OpCompositeExtract %float %37 0 1
         %40 = OpCompositeExtract %float %37 1 0
         %41 = OpCompositeExtract %float %37 1 1
         %42 = OpCompositeConstruct %v4float %38 %39 %40 %41
         %47 = OpFAdd %v4float %42 %46
               OpStore %inputVal %47
         %52 = OpCompositeExtract %float %47 0
         %51 = OpExtInst %float %1 Sqrt %52
         %54 = OpFSub %float %51 %float_1
         %50 = OpExtInst %float %1 FAbs %54
         %56 = OpFOrdLessThan %bool %50 %float_0_0500000007
               OpSelectionMerge %58 None
               OpBranchConditional %56 %57 %58
         %57 = OpLabel
         %63 = OpVectorShuffle %v2float %47 %47 0 1
         %62 = OpExtInst %v2float %1 Sqrt %63
         %65 = OpFSub %v2float %62 %64
         %61 = OpExtInst %v2float %1 FAbs %65
         %60 = OpFOrdLessThan %v2bool %61 %66
         %59 = OpAll %bool %60
               OpBranch %58
         %58 = OpLabel
         %68 = OpPhi %bool %false %23 %59 %57
               OpSelectionMerge %70 None
               OpBranchConditional %68 %69 %70
         %69 = OpLabel
         %75 = OpVectorShuffle %v3float %47 %47 0 1 2
         %74 = OpExtInst %v3float %1 Sqrt %75
         %79 = OpFSub %v3float %74 %78
         %73 = OpExtInst %v3float %1 FAbs %79
         %72 = OpFOrdLessThan %v3bool %73 %80
         %71 = OpAll %bool %72
               OpBranch %70
         %70 = OpLabel
         %82 = OpPhi %bool %false %58 %71 %69
               OpSelectionMerge %84 None
               OpBranchConditional %82 %83 %84
         %83 = OpLabel
         %88 = OpExtInst %v4float %1 Sqrt %47
         %91 = OpFSub %v4float %88 %90
         %87 = OpExtInst %v4float %1 FAbs %91
         %86 = OpFOrdLessThan %v4bool %87 %92
         %85 = OpAll %bool %86
               OpBranch %84
         %84 = OpLabel
         %94 = OpPhi %bool %false %70 %85 %83
               OpSelectionMerge %98 None
               OpBranchConditional %94 %96 %97
         %96 = OpLabel
         %99 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %102 = OpLoad %v4float %99
               OpStore %95 %102
               OpBranch %98
         %97 = OpLabel
        %103 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %105 = OpLoad %v4float %103
               OpStore %95 %105
               OpBranch %98
         %98 = OpLabel
        %106 = OpLoad %v4float %95
               OpReturnValue %106
               OpFunctionEnd
