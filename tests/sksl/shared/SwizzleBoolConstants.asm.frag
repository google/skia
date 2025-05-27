               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %v "v"
               OpName %result "result"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %true = OpConstantTrue %bool
     %v2bool = OpTypeVector %bool 2
      %false = OpConstantFalse %bool
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
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
          %v = OpVariable %_ptr_Function_v4bool Function
     %result = OpVariable %_ptr_Function_v4bool Function
        %105 = OpVariable %_ptr_Function_v4float Function
         %27 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %31 = OpLoad %v4float %27
         %32 = OpCompositeExtract %float %31 1
         %33 = OpFUnordNotEqual %bool %32 %float_0
         %34 = OpCompositeConstruct %v4bool %33 %33 %33 %33
               OpStore %v %34
         %36 = OpCompositeExtract %bool %34 0
         %38 = OpCompositeConstruct %v4bool %36 %true %true %true
               OpStore %result %38
         %39 = OpVectorShuffle %v2bool %34 %34 0 1
         %41 = OpCompositeExtract %bool %39 0
         %42 = OpCompositeExtract %bool %39 1
         %44 = OpCompositeConstruct %v4bool %41 %42 %false %true
               OpStore %result %44
         %45 = OpCompositeConstruct %v4bool %36 %true %true %false
               OpStore %result %45
         %46 = OpCompositeExtract %bool %34 1
         %47 = OpCompositeConstruct %v4bool %false %46 %true %true
               OpStore %result %47
         %48 = OpVectorShuffle %v3bool %34 %34 0 1 2
         %50 = OpCompositeExtract %bool %48 0
         %51 = OpCompositeExtract %bool %48 1
         %52 = OpCompositeExtract %bool %48 2
         %53 = OpCompositeConstruct %v4bool %50 %51 %52 %true
               OpStore %result %53
         %54 = OpVectorShuffle %v2bool %34 %34 0 1
         %55 = OpCompositeExtract %bool %54 0
         %56 = OpCompositeExtract %bool %54 1
         %57 = OpCompositeConstruct %v4bool %55 %56 %true %true
               OpStore %result %57
         %58 = OpCompositeExtract %bool %34 2
         %59 = OpCompositeConstruct %v4bool %36 %false %58 %true
               OpStore %result %59
         %60 = OpCompositeConstruct %v4bool %36 %true %false %false
               OpStore %result %60
         %61 = OpVectorShuffle %v2bool %34 %34 1 2
         %62 = OpCompositeExtract %bool %61 0
         %63 = OpCompositeExtract %bool %61 1
         %64 = OpCompositeConstruct %v4bool %true %62 %63 %false
               OpStore %result %64
         %65 = OpCompositeConstruct %v4bool %false %46 %true %false
               OpStore %result %65
         %66 = OpCompositeConstruct %v4bool %true %true %58 %false
               OpStore %result %66
               OpStore %result %34
         %67 = OpVectorShuffle %v3bool %34 %34 0 1 2
         %68 = OpCompositeExtract %bool %67 0
         %69 = OpCompositeExtract %bool %67 1
         %70 = OpCompositeExtract %bool %67 2
         %71 = OpCompositeConstruct %v4bool %68 %69 %70 %true
               OpStore %result %71
         %72 = OpVectorShuffle %v2bool %34 %34 0 1
         %73 = OpCompositeExtract %bool %72 0
         %74 = OpCompositeExtract %bool %72 1
         %75 = OpCompositeExtract %bool %34 3
         %76 = OpCompositeConstruct %v4bool %73 %74 %false %75
               OpStore %result %76
         %77 = OpVectorShuffle %v2bool %34 %34 0 1
         %78 = OpCompositeExtract %bool %77 0
         %79 = OpCompositeExtract %bool %77 1
         %80 = OpCompositeConstruct %v4bool %78 %79 %true %false
               OpStore %result %80
         %81 = OpVectorShuffle %v2bool %34 %34 2 3
         %82 = OpCompositeExtract %bool %81 0
         %83 = OpCompositeExtract %bool %81 1
         %84 = OpCompositeConstruct %v4bool %36 %true %82 %83
               OpStore %result %84
               OpStore %result %59
         %85 = OpCompositeConstruct %v4bool %36 %true %true %75
               OpStore %result %85
         %86 = OpCompositeConstruct %v4bool %36 %true %false %true
               OpStore %result %86
         %87 = OpVectorShuffle %v3bool %34 %34 1 2 3
         %88 = OpCompositeExtract %bool %87 0
         %89 = OpCompositeExtract %bool %87 1
         %90 = OpCompositeExtract %bool %87 2
         %91 = OpCompositeConstruct %v4bool %true %88 %89 %90
               OpStore %result %91
         %92 = OpVectorShuffle %v2bool %34 %34 1 2
         %93 = OpCompositeExtract %bool %92 0
         %94 = OpCompositeExtract %bool %92 1
         %95 = OpCompositeConstruct %v4bool %false %93 %94 %true
               OpStore %result %95
         %96 = OpCompositeConstruct %v4bool %false %46 %true %75
               OpStore %result %96
         %97 = OpCompositeConstruct %v4bool %true %46 %true %true
               OpStore %result %97
         %98 = OpVectorShuffle %v2bool %34 %34 2 3
         %99 = OpCompositeExtract %bool %98 0
        %100 = OpCompositeExtract %bool %98 1
        %101 = OpCompositeConstruct %v4bool %false %false %99 %100
               OpStore %result %101
        %102 = OpCompositeConstruct %v4bool %false %false %58 %true
               OpStore %result %102
        %103 = OpCompositeConstruct %v4bool %false %true %true %75
               OpStore %result %103
        %104 = OpAny %bool %103
               OpSelectionMerge %109 None
               OpBranchConditional %104 %107 %108
        %107 = OpLabel
        %110 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %111 = OpLoad %v4float %110
               OpStore %105 %111
               OpBranch %109
        %108 = OpLabel
        %112 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %114 = OpLoad %v4float %112
               OpStore %105 %114
               OpBranch %109
        %109 = OpLabel
        %115 = OpLoad %v4float %105
               OpReturnValue %115
               OpFunctionEnd
