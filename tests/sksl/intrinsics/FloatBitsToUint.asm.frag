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
               OpDecorate %87 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
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
       %uint = OpTypeInt 32 0
%uint_1065353216 = OpConstant %uint 1065353216
     %v2uint = OpTypeVector %uint 2
%uint_1073741824 = OpConstant %uint 1073741824
         %53 = OpConstantComposite %v2uint %uint_1065353216 %uint_1073741824
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3uint = OpTypeVector %uint 3
%uint_3225419776 = OpConstant %uint 3225419776
         %65 = OpConstantComposite %v3uint %uint_1065353216 %uint_1073741824 %uint_3225419776
     %v3bool = OpTypeVector %bool 3
     %v4uint = OpTypeVector %uint 4
%uint_3229614080 = OpConstant %uint 3229614080
         %75 = OpConstantComposite %v4uint %uint_1065353216 %uint_1073741824 %uint_3225419776 %uint_3229614080
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
         %80 = OpVariable %_ptr_Function_v4float Function
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
         %42 = OpBitcast %uint %43
         %46 = OpIEqual %bool %42 %uint_1065353216
               OpSelectionMerge %48 None
               OpBranchConditional %46 %47 %48
         %47 = OpLabel
         %50 = OpVectorShuffle %v2float %39 %39 0 1
         %49 = OpBitcast %v2uint %50
         %54 = OpIEqual %v2bool %49 %53
         %56 = OpAll %bool %54
               OpBranch %48
         %48 = OpLabel
         %57 = OpPhi %bool %false %23 %56 %47
               OpSelectionMerge %59 None
               OpBranchConditional %57 %58 %59
         %58 = OpLabel
         %61 = OpVectorShuffle %v3float %39 %39 0 1 2
         %60 = OpBitcast %v3uint %61
         %66 = OpIEqual %v3bool %60 %65
         %68 = OpAll %bool %66
               OpBranch %59
         %59 = OpLabel
         %69 = OpPhi %bool %false %48 %68 %58
               OpSelectionMerge %71 None
               OpBranchConditional %69 %70 %71
         %70 = OpLabel
         %72 = OpBitcast %v4uint %39
         %76 = OpIEqual %v4bool %72 %75
         %78 = OpAll %bool %76
               OpBranch %71
         %71 = OpLabel
         %79 = OpPhi %bool %false %59 %78 %70
               OpSelectionMerge %83 None
               OpBranchConditional %79 %81 %82
         %81 = OpLabel
         %84 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %87 = OpLoad %v4float %84
               OpStore %80 %87
               OpBranch %83
         %82 = OpLabel
         %88 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %90 = OpLoad %v4float %88
               OpStore %80 %90
               OpBranch %83
         %83 = OpLabel
         %91 = OpLoad %v4float %80
               OpReturnValue %91
               OpFunctionEnd
