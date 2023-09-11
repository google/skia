               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %v "v"
               OpName %result "result"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
          %v = OpVariable %_ptr_Function_v4bool Function
     %result = OpVariable %_ptr_Function_v4bool Function
        %107 = OpVariable %_ptr_Function_v4float Function
         %29 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %33 = OpLoad %v4float %29
         %34 = OpCompositeExtract %float %33 1
         %35 = OpFUnordNotEqual %bool %34 %float_0
         %36 = OpCompositeConstruct %v4bool %35 %35 %35 %35
               OpStore %v %36
         %38 = OpCompositeExtract %bool %36 0
         %40 = OpCompositeConstruct %v4bool %38 %true %true %true
               OpStore %result %40
         %41 = OpVectorShuffle %v2bool %36 %36 0 1
         %43 = OpCompositeExtract %bool %41 0
         %44 = OpCompositeExtract %bool %41 1
         %46 = OpCompositeConstruct %v4bool %43 %44 %false %true
               OpStore %result %46
         %47 = OpCompositeConstruct %v4bool %38 %true %true %false
               OpStore %result %47
         %48 = OpCompositeExtract %bool %36 1
         %49 = OpCompositeConstruct %v4bool %false %48 %true %true
               OpStore %result %49
         %50 = OpVectorShuffle %v3bool %36 %36 0 1 2
         %52 = OpCompositeExtract %bool %50 0
         %53 = OpCompositeExtract %bool %50 1
         %54 = OpCompositeExtract %bool %50 2
         %55 = OpCompositeConstruct %v4bool %52 %53 %54 %true
               OpStore %result %55
         %56 = OpVectorShuffle %v2bool %36 %36 0 1
         %57 = OpCompositeExtract %bool %56 0
         %58 = OpCompositeExtract %bool %56 1
         %59 = OpCompositeConstruct %v4bool %57 %58 %true %true
               OpStore %result %59
         %60 = OpCompositeExtract %bool %36 2
         %61 = OpCompositeConstruct %v4bool %38 %false %60 %true
               OpStore %result %61
         %62 = OpCompositeConstruct %v4bool %38 %true %false %false
               OpStore %result %62
         %63 = OpVectorShuffle %v2bool %36 %36 1 2
         %64 = OpCompositeExtract %bool %63 0
         %65 = OpCompositeExtract %bool %63 1
         %66 = OpCompositeConstruct %v4bool %true %64 %65 %false
               OpStore %result %66
         %67 = OpCompositeConstruct %v4bool %false %48 %true %false
               OpStore %result %67
         %68 = OpCompositeConstruct %v4bool %true %true %60 %false
               OpStore %result %68
               OpStore %result %36
         %69 = OpVectorShuffle %v3bool %36 %36 0 1 2
         %70 = OpCompositeExtract %bool %69 0
         %71 = OpCompositeExtract %bool %69 1
         %72 = OpCompositeExtract %bool %69 2
         %73 = OpCompositeConstruct %v4bool %70 %71 %72 %true
               OpStore %result %73
         %74 = OpVectorShuffle %v2bool %36 %36 0 1
         %75 = OpCompositeExtract %bool %74 0
         %76 = OpCompositeExtract %bool %74 1
         %77 = OpCompositeExtract %bool %36 3
         %78 = OpCompositeConstruct %v4bool %75 %76 %false %77
               OpStore %result %78
         %79 = OpVectorShuffle %v2bool %36 %36 0 1
         %80 = OpCompositeExtract %bool %79 0
         %81 = OpCompositeExtract %bool %79 1
         %82 = OpCompositeConstruct %v4bool %80 %81 %true %false
               OpStore %result %82
         %83 = OpVectorShuffle %v2bool %36 %36 2 3
         %84 = OpCompositeExtract %bool %83 0
         %85 = OpCompositeExtract %bool %83 1
         %86 = OpCompositeConstruct %v4bool %38 %true %84 %85
               OpStore %result %86
               OpStore %result %61
         %87 = OpCompositeConstruct %v4bool %38 %true %true %77
               OpStore %result %87
         %88 = OpCompositeConstruct %v4bool %38 %true %false %true
               OpStore %result %88
         %89 = OpVectorShuffle %v3bool %36 %36 1 2 3
         %90 = OpCompositeExtract %bool %89 0
         %91 = OpCompositeExtract %bool %89 1
         %92 = OpCompositeExtract %bool %89 2
         %93 = OpCompositeConstruct %v4bool %true %90 %91 %92
               OpStore %result %93
         %94 = OpVectorShuffle %v2bool %36 %36 1 2
         %95 = OpCompositeExtract %bool %94 0
         %96 = OpCompositeExtract %bool %94 1
         %97 = OpCompositeConstruct %v4bool %false %95 %96 %true
               OpStore %result %97
         %98 = OpCompositeConstruct %v4bool %false %48 %true %77
               OpStore %result %98
         %99 = OpCompositeConstruct %v4bool %true %48 %true %true
               OpStore %result %99
        %100 = OpVectorShuffle %v2bool %36 %36 2 3
        %101 = OpCompositeExtract %bool %100 0
        %102 = OpCompositeExtract %bool %100 1
        %103 = OpCompositeConstruct %v4bool %false %false %101 %102
               OpStore %result %103
        %104 = OpCompositeConstruct %v4bool %false %false %60 %true
               OpStore %result %104
        %105 = OpCompositeConstruct %v4bool %false %true %true %77
               OpStore %result %105
        %106 = OpAny %bool %105
               OpSelectionMerge %111 None
               OpBranchConditional %106 %109 %110
        %109 = OpLabel
        %112 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %113 = OpLoad %v4float %112
               OpStore %107 %113
               OpBranch %111
        %110 = OpLabel
        %114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %116 = OpLoad %v4float %114
               OpStore %107 %116
               OpBranch %111
        %111 = OpLabel
        %117 = OpLoad %v4float %107
               OpReturnValue %117
               OpFunctionEnd
