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
               OpName %v "v"
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
               OpDecorate %v RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %38 = OpConstantComposite %v4float %float_0 %float_1 %float_1 %float_1
    %v3float = OpTypeVector %float 3
         %56 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
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
          %v = OpVariable %_ptr_Function_v4float Function
         %95 = OpVariable %_ptr_Function_v4float Function
         %25 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %29 = OpLoad %v4float %25
               OpStore %v %29
         %30 = OpCompositeExtract %float %29 0
         %32 = OpCompositeConstruct %v4float %30 %float_1 %float_1 %float_1
               OpStore %v %32
         %33 = OpVectorShuffle %v2float %32 %32 0 1
         %34 = OpCompositeExtract %float %33 0
         %35 = OpCompositeExtract %float %33 1
         %36 = OpCompositeConstruct %v4float %34 %35 %float_1 %float_1
               OpStore %v %36
         %37 = OpCompositeConstruct %v4float %34 %float_1 %float_1 %float_1
               OpStore %v %37
               OpStore %v %38
         %39 = OpVectorShuffle %v3float %38 %38 0 1 2
         %41 = OpCompositeExtract %float %39 0
         %42 = OpCompositeExtract %float %39 1
         %43 = OpCompositeExtract %float %39 2
         %44 = OpCompositeConstruct %v4float %41 %42 %43 %float_1
               OpStore %v %44
         %45 = OpVectorShuffle %v2float %44 %44 0 1
         %46 = OpCompositeExtract %float %45 0
         %47 = OpCompositeExtract %float %45 1
         %48 = OpCompositeConstruct %v4float %46 %47 %float_1 %float_1
               OpStore %v %48
         %49 = OpCompositeConstruct %v4float %46 %float_0 %float_1 %float_1
               OpStore %v %49
         %50 = OpCompositeConstruct %v4float %46 %float_1 %float_0 %float_1
               OpStore %v %50
         %51 = OpVectorShuffle %v2float %50 %50 1 2
         %52 = OpCompositeExtract %float %51 0
         %53 = OpCompositeExtract %float %51 1
         %54 = OpCompositeConstruct %v4float %float_1 %52 %53 %float_1
               OpStore %v %54
         %55 = OpCompositeConstruct %v4float %float_0 %52 %float_1 %float_1
               OpStore %v %55
               OpStore %v %56
         %57 = OpVectorShuffle %v3float %56 %56 0 1 2
         %58 = OpCompositeExtract %float %57 0
         %59 = OpCompositeExtract %float %57 1
         %60 = OpCompositeExtract %float %57 2
         %61 = OpCompositeConstruct %v4float %58 %59 %60 %float_1
               OpStore %v %61
         %62 = OpVectorShuffle %v2float %61 %61 0 1
         %63 = OpCompositeExtract %float %62 0
         %64 = OpCompositeExtract %float %62 1
         %65 = OpCompositeConstruct %v4float %63 %64 %float_0 %float_1
               OpStore %v %65
         %66 = OpVectorShuffle %v2float %65 %65 0 1
         %67 = OpCompositeExtract %float %66 0
         %68 = OpCompositeExtract %float %66 1
         %69 = OpCompositeConstruct %v4float %67 %68 %float_1 %float_0
               OpStore %v %69
         %70 = OpVectorShuffle %v2float %69 %69 2 3
         %71 = OpCompositeExtract %float %70 0
         %72 = OpCompositeExtract %float %70 1
         %73 = OpCompositeConstruct %v4float %67 %float_1 %71 %72
               OpStore %v %73
         %74 = OpCompositeConstruct %v4float %67 %float_0 %71 %float_1
               OpStore %v %74
         %75 = OpCompositeConstruct %v4float %67 %float_1 %float_1 %float_1
               OpStore %v %75
         %76 = OpCompositeConstruct %v4float %67 %float_1 %float_0 %float_1
               OpStore %v %76
         %77 = OpVectorShuffle %v3float %76 %76 1 2 3
         %78 = OpCompositeExtract %float %77 0
         %79 = OpCompositeExtract %float %77 1
         %80 = OpCompositeExtract %float %77 2
         %81 = OpCompositeConstruct %v4float %float_1 %78 %79 %80
               OpStore %v %81
         %82 = OpVectorShuffle %v2float %81 %81 1 2
         %83 = OpCompositeExtract %float %82 0
         %84 = OpCompositeExtract %float %82 1
         %85 = OpCompositeConstruct %v4float %float_0 %83 %84 %float_1
               OpStore %v %85
         %86 = OpCompositeConstruct %v4float %float_0 %83 %float_1 %float_1
               OpStore %v %86
         %87 = OpCompositeConstruct %v4float %float_1 %83 %float_1 %float_1
               OpStore %v %87
         %88 = OpVectorShuffle %v2float %87 %87 2 3
         %89 = OpCompositeExtract %float %88 0
         %90 = OpCompositeExtract %float %88 1
         %91 = OpCompositeConstruct %v4float %float_0 %float_0 %89 %90
               OpStore %v %91
         %92 = OpCompositeConstruct %v4float %float_0 %float_0 %89 %float_1
               OpStore %v %92
               OpStore %v %38
               OpSelectionMerge %98 None
               OpBranchConditional %true %96 %97
         %96 = OpLabel
         %99 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %101 = OpLoad %v4float %99
               OpStore %95 %101
               OpBranch %98
         %97 = OpLabel
        %102 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %104 = OpLoad %v4float %102
               OpStore %95 %104
               OpBranch %98
         %98 = OpLabel
        %105 = OpLoad %v4float %95
               OpReturnValue %105
               OpFunctionEnd
