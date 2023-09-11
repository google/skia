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
               OpName %expectedB "expectedB"
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
   %float_n1 = OpConstant %float -1
         %41 = OpConstantComposite %v4float %float_1 %float_1 %float_n1 %float_n1
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1065353216 = OpConstant %int 1065353216
%int_1073741824 = OpConstant %int 1073741824
%int_n1069547520 = OpConstant %int -1069547520
%int_n1065353216 = OpConstant %int -1065353216
         %50 = OpConstantComposite %v4int %int_1065353216 %int_1073741824 %int_n1069547520 %int_n1065353216
      %false = OpConstantFalse %bool
      %v2int = OpTypeVector %int 2
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
      %v3int = OpTypeVector %int 3
     %v3bool = OpTypeVector %bool 3
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
  %expectedB = OpVariable %_ptr_Function_v4int Function
         %83 = OpVariable %_ptr_Function_v4float Function
         %29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_0
         %33 = OpLoad %mat2v2float %29
         %34 = OpCompositeExtract %float %33 0 0
         %35 = OpCompositeExtract %float %33 0 1
         %36 = OpCompositeExtract %float %33 1 0
         %37 = OpCompositeExtract %float %33 1 1
         %38 = OpCompositeConstruct %v4float %34 %35 %36 %37
         %42 = OpFMul %v4float %38 %41
               OpStore %inputVal %42
               OpStore %expectedB %50
         %52 = OpCompositeExtract %float %42 0
         %53 = OpBitcast %float %int_1065353216
         %54 = OpFOrdEqual %bool %52 %53
               OpSelectionMerge %56 None
               OpBranchConditional %54 %55 %56
         %55 = OpLabel
         %57 = OpVectorShuffle %v2float %42 %42 0 1
         %59 = OpVectorShuffle %v2int %50 %50 0 1
         %58 = OpBitcast %v2float %59
         %61 = OpFOrdEqual %v2bool %57 %58
         %63 = OpAll %bool %61
               OpBranch %56
         %56 = OpLabel
         %64 = OpPhi %bool %false %26 %63 %55
               OpSelectionMerge %66 None
               OpBranchConditional %64 %65 %66
         %65 = OpLabel
         %67 = OpVectorShuffle %v3float %42 %42 0 1 2
         %70 = OpVectorShuffle %v3int %50 %50 0 1 2
         %69 = OpBitcast %v3float %70
         %72 = OpFOrdEqual %v3bool %67 %69
         %74 = OpAll %bool %72
               OpBranch %66
         %66 = OpLabel
         %75 = OpPhi %bool %false %56 %74 %65
               OpSelectionMerge %77 None
               OpBranchConditional %75 %76 %77
         %76 = OpLabel
         %78 = OpBitcast %v4float %50
         %79 = OpFOrdEqual %v4bool %42 %78
         %81 = OpAll %bool %79
               OpBranch %77
         %77 = OpLabel
         %82 = OpPhi %bool %false %66 %81 %76
               OpSelectionMerge %86 None
               OpBranchConditional %82 %84 %85
         %84 = OpLabel
         %87 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %90 = OpLoad %v4float %87
               OpStore %83 %90
               OpBranch %86
         %85 = OpLabel
         %91 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %93 = OpLoad %v4float %91
               OpStore %83 %93
               OpBranch %86
         %86 = OpLabel
         %94 = OpLoad %v4float %83
               OpReturnValue %94
               OpFunctionEnd
