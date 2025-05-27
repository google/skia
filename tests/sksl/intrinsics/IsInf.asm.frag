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
               OpName %infiniteValue "infiniteValue"
               OpName %finiteValue "finiteValue"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 ColMajor
               OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 32
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 48
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %infiniteValue RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %finiteValue RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
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
%infiniteValue = OpVariable %_ptr_Function_v4float Function
%finiteValue = OpVariable %_ptr_Function_v4float Function
        %108 = OpVariable %_ptr_Function_v4float Function
         %26 = OpAccessChain %_ptr_Uniform_mat2v2float %7 %int_0
         %30 = OpLoad %mat2v2float %26
         %31 = OpCompositeExtract %float %30 0 0
         %32 = OpCompositeExtract %float %30 0 1
         %33 = OpCompositeExtract %float %30 1 0
         %34 = OpCompositeExtract %float %30 1 1
         %35 = OpCompositeConstruct %v4float %31 %32 %33 %34
         %36 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %39 = OpLoad %v4float %36
         %40 = OpCompositeExtract %float %39 0
         %42 = OpFDiv %float %float_1 %40
         %43 = OpVectorTimesScalar %v4float %35 %42
               OpStore %infiniteValue %43
         %45 = OpAccessChain %_ptr_Uniform_mat2v2float %7 %int_0
         %46 = OpLoad %mat2v2float %45
         %47 = OpCompositeExtract %float %46 0 0
         %48 = OpCompositeExtract %float %46 0 1
         %49 = OpCompositeExtract %float %46 1 0
         %50 = OpCompositeExtract %float %46 1 1
         %51 = OpCompositeConstruct %v4float %47 %48 %49 %50
         %52 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %53 = OpLoad %v4float %52
         %54 = OpCompositeExtract %float %53 1
         %55 = OpFDiv %float %float_1 %54
         %56 = OpVectorTimesScalar %v4float %51 %55
               OpStore %finiteValue %56
         %60 = OpCompositeExtract %float %43 0
         %59 = OpIsInf %bool %60
               OpSelectionMerge %62 None
               OpBranchConditional %59 %61 %62
         %61 = OpLabel
         %65 = OpVectorShuffle %v2float %43 %43 0 1
         %64 = OpIsInf %v2bool %65
         %63 = OpAll %bool %64
               OpBranch %62
         %62 = OpLabel
         %67 = OpPhi %bool %false %23 %63 %61
               OpSelectionMerge %69 None
               OpBranchConditional %67 %68 %69
         %68 = OpLabel
         %72 = OpVectorShuffle %v3float %43 %43 0 1 2
         %71 = OpIsInf %v3bool %72
         %70 = OpAll %bool %71
               OpBranch %69
         %69 = OpLabel
         %75 = OpPhi %bool %false %62 %70 %68
               OpSelectionMerge %77 None
               OpBranchConditional %75 %76 %77
         %76 = OpLabel
         %79 = OpIsInf %v4bool %43
         %78 = OpAll %bool %79
               OpBranch %77
         %77 = OpLabel
         %81 = OpPhi %bool %false %69 %78 %76
               OpSelectionMerge %83 None
               OpBranchConditional %81 %82 %83
         %82 = OpLabel
         %86 = OpCompositeExtract %float %56 0
         %85 = OpIsInf %bool %86
         %84 = OpLogicalNot %bool %85
               OpBranch %83
         %83 = OpLabel
         %87 = OpPhi %bool %false %77 %84 %82
               OpSelectionMerge %89 None
               OpBranchConditional %87 %88 %89
         %88 = OpLabel
         %93 = OpVectorShuffle %v2float %56 %56 0 1
         %92 = OpIsInf %v2bool %93
         %91 = OpAny %bool %92
         %90 = OpLogicalNot %bool %91
               OpBranch %89
         %89 = OpLabel
         %94 = OpPhi %bool %false %83 %90 %88
               OpSelectionMerge %96 None
               OpBranchConditional %94 %95 %96
         %95 = OpLabel
        %100 = OpVectorShuffle %v3float %56 %56 0 1 2
         %99 = OpIsInf %v3bool %100
         %98 = OpAny %bool %99
         %97 = OpLogicalNot %bool %98
               OpBranch %96
         %96 = OpLabel
        %101 = OpPhi %bool %false %89 %97 %95
               OpSelectionMerge %103 None
               OpBranchConditional %101 %102 %103
        %102 = OpLabel
        %106 = OpIsInf %v4bool %56
        %105 = OpAny %bool %106
        %104 = OpLogicalNot %bool %105
               OpBranch %103
        %103 = OpLabel
        %107 = OpPhi %bool %false %96 %104 %102
               OpSelectionMerge %111 None
               OpBranchConditional %107 %109 %110
        %109 = OpLabel
        %112 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %113 = OpLoad %v4float %112
               OpStore %108 %113
               OpBranch %111
        %110 = OpLabel
        %114 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %116 = OpLoad %v4float %114
               OpStore %108 %116
               OpBranch %111
        %111 = OpLabel
        %117 = OpLoad %v4float %108
               OpReturnValue %117
               OpFunctionEnd
