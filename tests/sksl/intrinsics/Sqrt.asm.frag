               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testMatrix2x2"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %inputVal "inputVal"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %104 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %mat2v2float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
   %float_n1 = OpConstant %float -1
   %float_n4 = OpConstant %float -4
  %float_n16 = OpConstant %float -16
  %float_n64 = OpConstant %float -64
         %32 = OpConstantComposite %v4float %float_n1 %float_n4 %float_n16 %float_n64
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
    %float_6 = OpConstant %float 6
   %float_12 = OpConstant %float 12
         %49 = OpConstantComposite %v4float %float_0 %float_2 %float_6 %float_12
      %false = OpConstantFalse %bool
    %float_1 = OpConstant %float 1
%float_0_0500000007 = OpConstant %float 0.0500000007
         %66 = OpConstantComposite %v2float %float_1 %float_2
         %68 = OpConstantComposite %v2float %float_0_0500000007 %float_0_0500000007
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
    %float_3 = OpConstant %float 3
         %80 = OpConstantComposite %v3float %float_1 %float_2 %float_3
         %82 = OpConstantComposite %v3float %float_0_0500000007 %float_0_0500000007 %float_0_0500000007
     %v3bool = OpTypeVector %bool 3
    %float_4 = OpConstant %float 4
         %92 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
         %94 = OpConstantComposite %v4float %float_0_0500000007 %float_0_0500000007 %float_0_0500000007 %float_0_0500000007
     %v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %17
         %18 = OpLabel
         %21 = OpVariable %_ptr_Function_v2float Function
               OpStore %21 %20
         %23 = OpFunctionCall %v4float %main %21
               OpStore %sk_FragColor %23
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %24
         %25 = OpFunctionParameter %_ptr_Function_v2float
         %26 = OpLabel
   %inputVal = OpVariable %_ptr_Function_v4float Function
         %97 = OpVariable %_ptr_Function_v4float Function
         %27 = OpExtInst %v4float %1 Sqrt %32
         %33 = OpVectorShuffle %v2float %27 %27 0 1
               OpStore %25 %33
         %36 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_0
         %40 = OpLoad %mat2v2float %36
         %41 = OpCompositeExtract %float %40 0 0
         %42 = OpCompositeExtract %float %40 0 1
         %43 = OpCompositeExtract %float %40 1 0
         %44 = OpCompositeExtract %float %40 1 1
         %45 = OpCompositeConstruct %v4float %41 %42 %43 %44
         %50 = OpFAdd %v4float %45 %49
               OpStore %inputVal %50
         %54 = OpCompositeExtract %float %50 0
         %53 = OpExtInst %float %1 Sqrt %54
         %56 = OpFSub %float %53 %float_1
         %52 = OpExtInst %float %1 FAbs %56
         %58 = OpFOrdLessThan %bool %52 %float_0_0500000007
               OpSelectionMerge %60 None
               OpBranchConditional %58 %59 %60
         %59 = OpLabel
         %65 = OpVectorShuffle %v2float %50 %50 0 1
         %64 = OpExtInst %v2float %1 Sqrt %65
         %67 = OpFSub %v2float %64 %66
         %63 = OpExtInst %v2float %1 FAbs %67
         %62 = OpFOrdLessThan %v2bool %63 %68
         %61 = OpAll %bool %62
               OpBranch %60
         %60 = OpLabel
         %70 = OpPhi %bool %false %26 %61 %59
               OpSelectionMerge %72 None
               OpBranchConditional %70 %71 %72
         %71 = OpLabel
         %77 = OpVectorShuffle %v3float %50 %50 0 1 2
         %76 = OpExtInst %v3float %1 Sqrt %77
         %81 = OpFSub %v3float %76 %80
         %75 = OpExtInst %v3float %1 FAbs %81
         %74 = OpFOrdLessThan %v3bool %75 %82
         %73 = OpAll %bool %74
               OpBranch %72
         %72 = OpLabel
         %84 = OpPhi %bool %false %60 %73 %71
               OpSelectionMerge %86 None
               OpBranchConditional %84 %85 %86
         %85 = OpLabel
         %90 = OpExtInst %v4float %1 Sqrt %50
         %93 = OpFSub %v4float %90 %92
         %89 = OpExtInst %v4float %1 FAbs %93
         %88 = OpFOrdLessThan %v4bool %89 %94
         %87 = OpAll %bool %88
               OpBranch %86
         %86 = OpLabel
         %96 = OpPhi %bool %false %72 %87 %85
               OpSelectionMerge %100 None
               OpBranchConditional %96 %98 %99
         %98 = OpLabel
        %101 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %104 = OpLoad %v4float %101
               OpStore %97 %104
               OpBranch %100
         %99 = OpLabel
        %105 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %107 = OpLoad %v4float %105
               OpStore %97 %107
               OpBranch %100
        %100 = OpLabel
        %108 = OpLoad %v4float %97
               OpReturnValue %108
               OpFunctionEnd
