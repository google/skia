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
               OpDecorate %86 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
   %float_n1 = OpConstant %float -1
         %38 = OpConstantComposite %v4float %float_1 %float_1 %float_n1 %float_n1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%int_1065353216 = OpConstant %int 1065353216
      %v2int = OpTypeVector %int 2
%int_1073741824 = OpConstant %int 1073741824
         %52 = OpConstantComposite %v2int %int_1065353216 %int_1073741824
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
      %v3int = OpTypeVector %int 3
%int_n1069547520 = OpConstant %int -1069547520
         %64 = OpConstantComposite %v3int %int_1065353216 %int_1073741824 %int_n1069547520
     %v3bool = OpTypeVector %bool 3
      %v4int = OpTypeVector %int 4
%int_n1065353216 = OpConstant %int -1065353216
         %74 = OpConstantComposite %v4int %int_1065353216 %int_1073741824 %int_n1069547520 %int_n1065353216
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
         %79 = OpVariable %_ptr_Function_v4float Function
         %26 = OpAccessChain %_ptr_Uniform_mat2v2float %7 %int_0
         %30 = OpLoad %mat2v2float %26
         %31 = OpCompositeExtract %float %30 0 0
         %32 = OpCompositeExtract %float %30 0 1
         %33 = OpCompositeExtract %float %30 1 0
         %34 = OpCompositeExtract %float %30 1 1
         %35 = OpCompositeConstruct %v4float %31 %32 %33 %34
         %39 = OpFMul %v4float %35 %38
               OpStore %inputVal %39
         %43 = OpCompositeExtract %float %39 0
         %42 = OpBitcast %int %43
         %45 = OpIEqual %bool %42 %int_1065353216
               OpSelectionMerge %47 None
               OpBranchConditional %45 %46 %47
         %46 = OpLabel
         %49 = OpVectorShuffle %v2float %39 %39 0 1
         %48 = OpBitcast %v2int %49
         %53 = OpIEqual %v2bool %48 %52
         %55 = OpAll %bool %53
               OpBranch %47
         %47 = OpLabel
         %56 = OpPhi %bool %false %23 %55 %46
               OpSelectionMerge %58 None
               OpBranchConditional %56 %57 %58
         %57 = OpLabel
         %60 = OpVectorShuffle %v3float %39 %39 0 1 2
         %59 = OpBitcast %v3int %60
         %65 = OpIEqual %v3bool %59 %64
         %67 = OpAll %bool %65
               OpBranch %58
         %58 = OpLabel
         %68 = OpPhi %bool %false %47 %67 %57
               OpSelectionMerge %70 None
               OpBranchConditional %68 %69 %70
         %69 = OpLabel
         %71 = OpBitcast %v4int %39
         %75 = OpIEqual %v4bool %71 %74
         %77 = OpAll %bool %75
               OpBranch %70
         %70 = OpLabel
         %78 = OpPhi %bool %false %58 %77 %69
               OpSelectionMerge %82 None
               OpBranchConditional %78 %80 %81
         %80 = OpLabel
         %83 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %86 = OpLoad %v4float %83
               OpStore %79 %86
               OpBranch %82
         %81 = OpLabel
         %87 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %89 = OpLoad %v4float %87
               OpStore %79 %89
               OpBranch %82
         %82 = OpLabel
         %90 = OpLoad %v4float %79
               OpReturnValue %90
               OpFunctionEnd
