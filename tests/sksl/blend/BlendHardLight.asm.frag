               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %blend_overlay_component_Qhh2h2 "blend_overlay_component_Qhh2h2"
               OpName %blend_overlay_h4h4h4 "blend_overlay_h4h4h4"
               OpName %result "result"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %9 Binding 0
               OpDecorate %9 DescriptorSet 0
               OpDecorate %19 RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
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
               OpDecorate %result RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
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
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %9 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %14 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
    %float_2 = OpConstant %float 2
       %bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %57 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
    %float_1 = OpConstant %float 1
    %v3float = OpTypeVector %float 3
       %void = OpTypeVoid
        %114 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%blend_overlay_component_Qhh2h2 = OpFunction %float None %14
         %15 = OpFunctionParameter %_ptr_Function_v2float
         %16 = OpFunctionParameter %_ptr_Function_v2float
         %17 = OpLabel
         %26 = OpVariable %_ptr_Function_float Function
         %19 = OpLoad %v2float %16
         %20 = OpCompositeExtract %float %19 0
         %21 = OpFMul %float %float_2 %20
         %22 = OpLoad %v2float %16
         %23 = OpCompositeExtract %float %22 1
         %24 = OpFOrdLessThanEqual %bool %21 %23
               OpSelectionMerge %30 None
               OpBranchConditional %24 %28 %29
         %28 = OpLabel
         %31 = OpLoad %v2float %15
         %32 = OpCompositeExtract %float %31 0
         %33 = OpFMul %float %float_2 %32
         %34 = OpLoad %v2float %16
         %35 = OpCompositeExtract %float %34 0
         %36 = OpFMul %float %33 %35
               OpStore %26 %36
               OpBranch %30
         %29 = OpLabel
         %37 = OpLoad %v2float %15
         %38 = OpCompositeExtract %float %37 1
         %39 = OpLoad %v2float %16
         %40 = OpCompositeExtract %float %39 1
         %41 = OpFMul %float %38 %40
         %42 = OpLoad %v2float %16
         %43 = OpCompositeExtract %float %42 1
         %44 = OpLoad %v2float %16
         %45 = OpCompositeExtract %float %44 0
         %46 = OpFSub %float %43 %45
         %47 = OpFMul %float %float_2 %46
         %48 = OpLoad %v2float %15
         %49 = OpCompositeExtract %float %48 1
         %50 = OpLoad %v2float %15
         %51 = OpCompositeExtract %float %50 0
         %52 = OpFSub %float %49 %51
         %53 = OpFMul %float %47 %52
         %54 = OpFSub %float %41 %53
               OpStore %26 %54
               OpBranch %30
         %30 = OpLabel
         %55 = OpLoad %float %26
               OpReturnValue %55
               OpFunctionEnd
%blend_overlay_h4h4h4 = OpFunction %v4float None %57
         %58 = OpFunctionParameter %_ptr_Function_v4float
         %59 = OpFunctionParameter %_ptr_Function_v4float
         %60 = OpLabel
     %result = OpVariable %_ptr_Function_v4float Function
         %64 = OpVariable %_ptr_Function_v2float Function
         %67 = OpVariable %_ptr_Function_v2float Function
         %71 = OpVariable %_ptr_Function_v2float Function
         %74 = OpVariable %_ptr_Function_v2float Function
         %78 = OpVariable %_ptr_Function_v2float Function
         %81 = OpVariable %_ptr_Function_v2float Function
         %62 = OpLoad %v4float %58
         %63 = OpVectorShuffle %v2float %62 %62 0 3
               OpStore %64 %63
         %65 = OpLoad %v4float %59
         %66 = OpVectorShuffle %v2float %65 %65 0 3
               OpStore %67 %66
         %68 = OpFunctionCall %float %blend_overlay_component_Qhh2h2 %64 %67
         %69 = OpLoad %v4float %58
         %70 = OpVectorShuffle %v2float %69 %69 1 3
               OpStore %71 %70
         %72 = OpLoad %v4float %59
         %73 = OpVectorShuffle %v2float %72 %72 1 3
               OpStore %74 %73
         %75 = OpFunctionCall %float %blend_overlay_component_Qhh2h2 %71 %74
         %76 = OpLoad %v4float %58
         %77 = OpVectorShuffle %v2float %76 %76 2 3
               OpStore %78 %77
         %79 = OpLoad %v4float %59
         %80 = OpVectorShuffle %v2float %79 %79 2 3
               OpStore %81 %80
         %82 = OpFunctionCall %float %blend_overlay_component_Qhh2h2 %78 %81
         %83 = OpLoad %v4float %58
         %84 = OpCompositeExtract %float %83 3
         %86 = OpLoad %v4float %58
         %87 = OpCompositeExtract %float %86 3
         %88 = OpFSub %float %float_1 %87
         %89 = OpLoad %v4float %59
         %90 = OpCompositeExtract %float %89 3
         %91 = OpFMul %float %88 %90
         %92 = OpFAdd %float %84 %91
         %93 = OpCompositeConstruct %v4float %68 %75 %82 %92
               OpStore %result %93
         %94 = OpLoad %v4float %result
         %95 = OpVectorShuffle %v3float %94 %94 0 1 2
         %97 = OpLoad %v4float %59
         %98 = OpVectorShuffle %v3float %97 %97 0 1 2
         %99 = OpLoad %v4float %58
        %100 = OpCompositeExtract %float %99 3
        %101 = OpFSub %float %float_1 %100
        %102 = OpVectorTimesScalar %v3float %98 %101
        %103 = OpLoad %v4float %58
        %104 = OpVectorShuffle %v3float %103 %103 0 1 2
        %105 = OpLoad %v4float %59
        %106 = OpCompositeExtract %float %105 3
        %107 = OpFSub %float %float_1 %106
        %108 = OpVectorTimesScalar %v3float %104 %107
        %109 = OpFAdd %v3float %102 %108
        %110 = OpFAdd %v3float %95 %109
        %111 = OpLoad %v4float %result
        %112 = OpVectorShuffle %v4float %111 %110 4 5 6 3
               OpStore %result %112
               OpReturnValue %112
               OpFunctionEnd
       %main = OpFunction %void None %114
        %115 = OpLabel
        %121 = OpVariable %_ptr_Function_v4float Function
        %125 = OpVariable %_ptr_Function_v4float Function
        %116 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
        %120 = OpLoad %v4float %116
               OpStore %121 %120
        %122 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %124 = OpLoad %v4float %122
               OpStore %125 %124
        %126 = OpFunctionCall %v4float %blend_overlay_h4h4h4 %121 %125
               OpStore %sk_FragColor %126
               OpReturn
               OpFunctionEnd
