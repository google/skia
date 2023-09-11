               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %v "v"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %v RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
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
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
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
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %41 = OpConstantComposite %v4float %float_0 %float_1 %float_1 %float_1
    %v3float = OpTypeVector %float 3
         %59 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
       %true = OpConstantTrue %bool
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
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
          %v = OpVariable %_ptr_Function_v4float Function
         %97 = OpVariable %_ptr_Function_v4float Function
         %28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %32 = OpLoad %v4float %28
               OpStore %v %32
         %33 = OpCompositeExtract %float %32 0
         %35 = OpCompositeConstruct %v4float %33 %float_1 %float_1 %float_1
               OpStore %v %35
         %36 = OpVectorShuffle %v2float %35 %35 0 1
         %37 = OpCompositeExtract %float %36 0
         %38 = OpCompositeExtract %float %36 1
         %39 = OpCompositeConstruct %v4float %37 %38 %float_1 %float_1
               OpStore %v %39
         %40 = OpCompositeConstruct %v4float %37 %float_1 %float_1 %float_1
               OpStore %v %40
               OpStore %v %41
         %42 = OpVectorShuffle %v3float %41 %41 0 1 2
         %44 = OpCompositeExtract %float %42 0
         %45 = OpCompositeExtract %float %42 1
         %46 = OpCompositeExtract %float %42 2
         %47 = OpCompositeConstruct %v4float %44 %45 %46 %float_1
               OpStore %v %47
         %48 = OpVectorShuffle %v2float %47 %47 0 1
         %49 = OpCompositeExtract %float %48 0
         %50 = OpCompositeExtract %float %48 1
         %51 = OpCompositeConstruct %v4float %49 %50 %float_1 %float_1
               OpStore %v %51
         %52 = OpCompositeConstruct %v4float %49 %float_0 %float_1 %float_1
               OpStore %v %52
         %53 = OpCompositeConstruct %v4float %49 %float_1 %float_0 %float_1
               OpStore %v %53
         %54 = OpVectorShuffle %v2float %53 %53 1 2
         %55 = OpCompositeExtract %float %54 0
         %56 = OpCompositeExtract %float %54 1
         %57 = OpCompositeConstruct %v4float %float_1 %55 %56 %float_1
               OpStore %v %57
         %58 = OpCompositeConstruct %v4float %float_0 %55 %float_1 %float_1
               OpStore %v %58
               OpStore %v %59
         %60 = OpVectorShuffle %v3float %59 %59 0 1 2
         %61 = OpCompositeExtract %float %60 0
         %62 = OpCompositeExtract %float %60 1
         %63 = OpCompositeExtract %float %60 2
         %64 = OpCompositeConstruct %v4float %61 %62 %63 %float_1
               OpStore %v %64
         %65 = OpVectorShuffle %v2float %64 %64 0 1
         %66 = OpCompositeExtract %float %65 0
         %67 = OpCompositeExtract %float %65 1
         %68 = OpCompositeConstruct %v4float %66 %67 %float_0 %float_1
               OpStore %v %68
         %69 = OpVectorShuffle %v2float %68 %68 0 1
         %70 = OpCompositeExtract %float %69 0
         %71 = OpCompositeExtract %float %69 1
         %72 = OpCompositeConstruct %v4float %70 %71 %float_1 %float_0
               OpStore %v %72
         %73 = OpVectorShuffle %v2float %72 %72 2 3
         %74 = OpCompositeExtract %float %73 0
         %75 = OpCompositeExtract %float %73 1
         %76 = OpCompositeConstruct %v4float %70 %float_1 %74 %75
               OpStore %v %76
         %77 = OpCompositeConstruct %v4float %70 %float_0 %74 %float_1
               OpStore %v %77
         %78 = OpCompositeConstruct %v4float %70 %float_1 %float_1 %float_1
               OpStore %v %78
         %79 = OpCompositeConstruct %v4float %70 %float_1 %float_0 %float_1
               OpStore %v %79
         %80 = OpVectorShuffle %v3float %79 %79 1 2 3
         %81 = OpCompositeExtract %float %80 0
         %82 = OpCompositeExtract %float %80 1
         %83 = OpCompositeExtract %float %80 2
         %84 = OpCompositeConstruct %v4float %float_1 %81 %82 %83
               OpStore %v %84
         %85 = OpVectorShuffle %v2float %84 %84 1 2
         %86 = OpCompositeExtract %float %85 0
         %87 = OpCompositeExtract %float %85 1
         %88 = OpCompositeConstruct %v4float %float_0 %86 %87 %float_1
               OpStore %v %88
         %89 = OpCompositeConstruct %v4float %float_0 %86 %float_1 %float_1
               OpStore %v %89
         %90 = OpCompositeConstruct %v4float %float_1 %86 %float_1 %float_1
               OpStore %v %90
         %91 = OpVectorShuffle %v2float %90 %90 2 3
         %92 = OpCompositeExtract %float %91 0
         %93 = OpCompositeExtract %float %91 1
         %94 = OpCompositeConstruct %v4float %float_0 %float_0 %92 %93
               OpStore %v %94
         %95 = OpCompositeConstruct %v4float %float_0 %float_0 %92 %float_1
               OpStore %v %95
               OpStore %v %41
               OpSelectionMerge %100 None
               OpBranchConditional %true %98 %99
         %98 = OpLabel
        %101 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %103 = OpLoad %v4float %101
               OpStore %97 %103
               OpBranch %100
         %99 = OpLabel
        %104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %106 = OpLoad %v4float %104
               OpStore %97 %106
               OpBranch %100
        %100 = OpLabel
        %107 = OpLoad %v4float %97
               OpReturnValue %107
               OpFunctionEnd
