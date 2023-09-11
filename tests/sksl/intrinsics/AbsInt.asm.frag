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
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
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
        %int = OpTypeInt 32 1
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
         %30 = OpConstantComposite %v4int %int_1 %int_0 %int_0 %int_2
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %v2int = OpTypeVector %int 2
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
      %v3int = OpTypeVector %int 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %101 = OpConstantComposite %v2int %int_1 %int_0
        %108 = OpConstantComposite %v3int %int_1 %int_0 %int_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
   %expected = OpVariable %_ptr_Function_v4int Function
        %116 = OpVariable %_ptr_Function_v4float Function
               OpStore %expected %30
         %34 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %36 = OpLoad %v4float %34
         %37 = OpCompositeExtract %float %36 0
         %38 = OpConvertFToS %int %37
         %33 = OpExtInst %int %1 SAbs %38
         %39 = OpIEqual %bool %33 %int_1
               OpSelectionMerge %41 None
               OpBranchConditional %39 %40 %41
         %40 = OpLabel
         %43 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %44 = OpLoad %v4float %43
         %45 = OpVectorShuffle %v2float %44 %44 0 1
         %46 = OpCompositeExtract %float %45 0
         %47 = OpConvertFToS %int %46
         %48 = OpCompositeExtract %float %45 1
         %49 = OpConvertFToS %int %48
         %51 = OpCompositeConstruct %v2int %47 %49
         %42 = OpExtInst %v2int %1 SAbs %51
         %52 = OpVectorShuffle %v2int %30 %30 0 1
         %53 = OpIEqual %v2bool %42 %52
         %55 = OpAll %bool %53
               OpBranch %41
         %41 = OpLabel
         %56 = OpPhi %bool %false %22 %55 %40
               OpSelectionMerge %58 None
               OpBranchConditional %56 %57 %58
         %57 = OpLabel
         %60 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %61 = OpLoad %v4float %60
         %62 = OpVectorShuffle %v3float %61 %61 0 1 2
         %64 = OpCompositeExtract %float %62 0
         %65 = OpConvertFToS %int %64
         %66 = OpCompositeExtract %float %62 1
         %67 = OpConvertFToS %int %66
         %68 = OpCompositeExtract %float %62 2
         %69 = OpConvertFToS %int %68
         %71 = OpCompositeConstruct %v3int %65 %67 %69
         %59 = OpExtInst %v3int %1 SAbs %71
         %72 = OpVectorShuffle %v3int %30 %30 0 1 2
         %73 = OpIEqual %v3bool %59 %72
         %75 = OpAll %bool %73
               OpBranch %58
         %58 = OpLabel
         %76 = OpPhi %bool %false %41 %75 %57
               OpSelectionMerge %78 None
               OpBranchConditional %76 %77 %78
         %77 = OpLabel
         %80 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %81 = OpLoad %v4float %80
         %82 = OpCompositeExtract %float %81 0
         %83 = OpConvertFToS %int %82
         %84 = OpCompositeExtract %float %81 1
         %85 = OpConvertFToS %int %84
         %86 = OpCompositeExtract %float %81 2
         %87 = OpConvertFToS %int %86
         %88 = OpCompositeExtract %float %81 3
         %89 = OpConvertFToS %int %88
         %90 = OpCompositeConstruct %v4int %83 %85 %87 %89
         %79 = OpExtInst %v4int %1 SAbs %90
         %91 = OpIEqual %v4bool %79 %30
         %93 = OpAll %bool %91
               OpBranch %78
         %78 = OpLabel
         %94 = OpPhi %bool %false %58 %93 %77
               OpSelectionMerge %96 None
               OpBranchConditional %94 %95 %96
         %95 = OpLabel
               OpBranch %96
         %96 = OpLabel
         %98 = OpPhi %bool %false %78 %true %95
               OpSelectionMerge %100 None
               OpBranchConditional %98 %99 %100
         %99 = OpLabel
        %102 = OpVectorShuffle %v2int %30 %30 0 1
        %103 = OpIEqual %v2bool %101 %102
        %104 = OpAll %bool %103
               OpBranch %100
        %100 = OpLabel
        %105 = OpPhi %bool %false %96 %104 %99
               OpSelectionMerge %107 None
               OpBranchConditional %105 %106 %107
        %106 = OpLabel
        %109 = OpVectorShuffle %v3int %30 %30 0 1 2
        %110 = OpIEqual %v3bool %108 %109
        %111 = OpAll %bool %110
               OpBranch %107
        %107 = OpLabel
        %112 = OpPhi %bool %false %100 %111 %106
               OpSelectionMerge %114 None
               OpBranchConditional %112 %113 %114
        %113 = OpLabel
               OpBranch %114
        %114 = OpLabel
        %115 = OpPhi %bool %false %107 %true %113
               OpSelectionMerge %120 None
               OpBranchConditional %115 %118 %119
        %118 = OpLabel
        %121 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %122 = OpLoad %v4float %121
               OpStore %116 %122
               OpBranch %120
        %119 = OpLabel
        %123 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %124 = OpLoad %v4float %123
               OpStore %116 %124
               OpBranch %120
        %120 = OpLabel
        %125 = OpLoad %v4float %116
               OpReturnValue %125
               OpFunctionEnd
