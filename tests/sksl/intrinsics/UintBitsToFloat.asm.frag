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
               OpName %expectedB "expectedB"
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
               OpDecorate %89 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
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
       %uint = OpTypeInt 32 0
     %v4uint = OpTypeVector %uint 4
%_ptr_Function_v4uint = OpTypePointer Function %v4uint
%uint_1065353216 = OpConstant %uint 1065353216
%uint_1073741824 = OpConstant %uint 1073741824
%uint_3225419776 = OpConstant %uint 3225419776
%uint_3229614080 = OpConstant %uint 3229614080
         %48 = OpConstantComposite %v4uint %uint_1065353216 %uint_1073741824 %uint_3225419776 %uint_3229614080
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v2uint = OpTypeVector %uint 2
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3uint = OpTypeVector %uint 3
     %v3bool = OpTypeVector %bool 3
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
  %expectedB = OpVariable %_ptr_Function_v4uint Function
         %82 = OpVariable %_ptr_Function_v4float Function
         %26 = OpAccessChain %_ptr_Uniform_mat2v2float %7 %int_0
         %30 = OpLoad %mat2v2float %26
         %31 = OpCompositeExtract %float %30 0 0
         %32 = OpCompositeExtract %float %30 0 1
         %33 = OpCompositeExtract %float %30 1 0
         %34 = OpCompositeExtract %float %30 1 1
         %35 = OpCompositeConstruct %v4float %31 %32 %33 %34
         %39 = OpFMul %v4float %35 %38
               OpStore %inputVal %39
               OpStore %expectedB %48
         %51 = OpCompositeExtract %float %39 0
         %52 = OpBitcast %float %uint_1065353216
         %53 = OpFOrdEqual %bool %51 %52
               OpSelectionMerge %55 None
               OpBranchConditional %53 %54 %55
         %54 = OpLabel
         %56 = OpVectorShuffle %v2float %39 %39 0 1
         %58 = OpVectorShuffle %v2uint %48 %48 0 1
         %57 = OpBitcast %v2float %58
         %60 = OpFOrdEqual %v2bool %56 %57
         %62 = OpAll %bool %60
               OpBranch %55
         %55 = OpLabel
         %63 = OpPhi %bool %false %23 %62 %54
               OpSelectionMerge %65 None
               OpBranchConditional %63 %64 %65
         %64 = OpLabel
         %66 = OpVectorShuffle %v3float %39 %39 0 1 2
         %69 = OpVectorShuffle %v3uint %48 %48 0 1 2
         %68 = OpBitcast %v3float %69
         %71 = OpFOrdEqual %v3bool %66 %68
         %73 = OpAll %bool %71
               OpBranch %65
         %65 = OpLabel
         %74 = OpPhi %bool %false %55 %73 %64
               OpSelectionMerge %76 None
               OpBranchConditional %74 %75 %76
         %75 = OpLabel
         %77 = OpBitcast %v4float %48
         %78 = OpFOrdEqual %v4bool %39 %77
         %80 = OpAll %bool %78
               OpBranch %76
         %76 = OpLabel
         %81 = OpPhi %bool %false %65 %80 %75
               OpSelectionMerge %85 None
               OpBranchConditional %81 %83 %84
         %83 = OpLabel
         %86 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %89 = OpLoad %v4float %86
               OpStore %82 %89
               OpBranch %85
         %84 = OpLabel
         %90 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %92 = OpLoad %v4float %90
               OpStore %82 %92
               OpBranch %85
         %85 = OpLabel
         %93 = OpLoad %v4float %82
               OpReturnValue %93
               OpFunctionEnd
