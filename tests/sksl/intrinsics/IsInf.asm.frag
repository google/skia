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
               OpName %infiniteValue "infiniteValue"
               OpName %finiteValue "finiteValue"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %infiniteValue RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %finiteValue RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
      %false = OpConstantFalse %bool
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
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
%infiniteValue = OpVariable %_ptr_Function_v4float Function
%finiteValue = OpVariable %_ptr_Function_v4float Function
        %110 = OpVariable %_ptr_Function_v4float Function
         %29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_0
         %33 = OpLoad %mat2v2float %29
         %34 = OpCompositeExtract %float %33 0 0
         %35 = OpCompositeExtract %float %33 0 1
         %36 = OpCompositeExtract %float %33 1 0
         %37 = OpCompositeExtract %float %33 1 1
         %38 = OpCompositeConstruct %v4float %34 %35 %36 %37
         %39 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %42 = OpLoad %v4float %39
         %43 = OpCompositeExtract %float %42 0
         %45 = OpFDiv %float %float_1 %43
         %46 = OpVectorTimesScalar %v4float %38 %45
               OpStore %infiniteValue %46
         %48 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_0
         %49 = OpLoad %mat2v2float %48
         %50 = OpCompositeExtract %float %49 0 0
         %51 = OpCompositeExtract %float %49 0 1
         %52 = OpCompositeExtract %float %49 1 0
         %53 = OpCompositeExtract %float %49 1 1
         %54 = OpCompositeConstruct %v4float %50 %51 %52 %53
         %55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %56 = OpLoad %v4float %55
         %57 = OpCompositeExtract %float %56 1
         %58 = OpFDiv %float %float_1 %57
         %59 = OpVectorTimesScalar %v4float %54 %58
               OpStore %finiteValue %59
         %62 = OpCompositeExtract %float %46 0
         %61 = OpIsInf %bool %62
               OpSelectionMerge %64 None
               OpBranchConditional %61 %63 %64
         %63 = OpLabel
         %67 = OpVectorShuffle %v2float %46 %46 0 1
         %66 = OpIsInf %v2bool %67
         %65 = OpAll %bool %66
               OpBranch %64
         %64 = OpLabel
         %69 = OpPhi %bool %false %26 %65 %63
               OpSelectionMerge %71 None
               OpBranchConditional %69 %70 %71
         %70 = OpLabel
         %74 = OpVectorShuffle %v3float %46 %46 0 1 2
         %73 = OpIsInf %v3bool %74
         %72 = OpAll %bool %73
               OpBranch %71
         %71 = OpLabel
         %77 = OpPhi %bool %false %64 %72 %70
               OpSelectionMerge %79 None
               OpBranchConditional %77 %78 %79
         %78 = OpLabel
         %81 = OpIsInf %v4bool %46
         %80 = OpAll %bool %81
               OpBranch %79
         %79 = OpLabel
         %83 = OpPhi %bool %false %71 %80 %78
               OpSelectionMerge %85 None
               OpBranchConditional %83 %84 %85
         %84 = OpLabel
         %88 = OpCompositeExtract %float %59 0
         %87 = OpIsInf %bool %88
         %86 = OpLogicalNot %bool %87
               OpBranch %85
         %85 = OpLabel
         %89 = OpPhi %bool %false %79 %86 %84
               OpSelectionMerge %91 None
               OpBranchConditional %89 %90 %91
         %90 = OpLabel
         %95 = OpVectorShuffle %v2float %59 %59 0 1
         %94 = OpIsInf %v2bool %95
         %93 = OpAny %bool %94
         %92 = OpLogicalNot %bool %93
               OpBranch %91
         %91 = OpLabel
         %96 = OpPhi %bool %false %85 %92 %90
               OpSelectionMerge %98 None
               OpBranchConditional %96 %97 %98
         %97 = OpLabel
        %102 = OpVectorShuffle %v3float %59 %59 0 1 2
        %101 = OpIsInf %v3bool %102
        %100 = OpAny %bool %101
         %99 = OpLogicalNot %bool %100
               OpBranch %98
         %98 = OpLabel
        %103 = OpPhi %bool %false %91 %99 %97
               OpSelectionMerge %105 None
               OpBranchConditional %103 %104 %105
        %104 = OpLabel
        %108 = OpIsInf %v4bool %59
        %107 = OpAny %bool %108
        %106 = OpLogicalNot %bool %107
               OpBranch %105
        %105 = OpLabel
        %109 = OpPhi %bool %false %98 %106 %104
               OpSelectionMerge %113 None
               OpBranchConditional %109 %111 %112
        %111 = OpLabel
        %114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %115 = OpLoad %v4float %114
               OpStore %110 %115
               OpBranch %113
        %112 = OpLabel
        %116 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %118 = OpLoad %v4float %116
               OpStore %110 %118
               OpBranch %113
        %113 = OpLabel
        %119 = OpLoad %v4float %110
               OpReturnValue %119
               OpFunctionEnd
