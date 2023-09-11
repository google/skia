               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %_kGuardedDivideEpsilon "$kGuardedDivideEpsilon"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %color_dodge_component_Qhh2h2 "color_dodge_component_Qhh2h2"
               OpName %delta "delta"
               OpName %main "main"
               OpDecorate %_kGuardedDivideEpsilon RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %15 Binding 0
               OpDecorate %15 DescriptorSet 0
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %delta RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
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
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
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
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
      %float = OpTypeFloat 32
%_ptr_Private_float = OpTypePointer Private %float
%_kGuardedDivideEpsilon = OpVariable %_ptr_Private_float Private
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%float_9_99999994en09 = OpConstant %float 9.99999994e-09
    %float_0 = OpConstant %float 0
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %15 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
    %float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
       %void = OpTypeVoid
         %96 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%color_dodge_component_Qhh2h2 = OpFunction %float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpFunctionParameter %_ptr_Function_v2float
         %23 = OpLabel
      %delta = OpVariable %_ptr_Function_float Function
         %24 = OpLoad %v2float %22
         %25 = OpCompositeExtract %float %24 0
         %26 = OpFOrdEqual %bool %25 %float_0
               OpSelectionMerge %29 None
               OpBranchConditional %26 %27 %28
         %27 = OpLabel
         %30 = OpLoad %v2float %21
         %31 = OpCompositeExtract %float %30 0
         %33 = OpLoad %v2float %22
         %34 = OpCompositeExtract %float %33 1
         %35 = OpFSub %float %float_1 %34
         %36 = OpFMul %float %31 %35
               OpReturnValue %36
         %28 = OpLabel
         %39 = OpLoad %v2float %21
         %40 = OpCompositeExtract %float %39 1
         %41 = OpLoad %v2float %21
         %42 = OpCompositeExtract %float %41 0
         %43 = OpFSub %float %40 %42
               OpStore %delta %43
         %44 = OpFOrdEqual %bool %43 %float_0
               OpSelectionMerge %47 None
               OpBranchConditional %44 %45 %46
         %45 = OpLabel
         %48 = OpLoad %v2float %21
         %49 = OpCompositeExtract %float %48 1
         %50 = OpLoad %v2float %22
         %51 = OpCompositeExtract %float %50 1
         %52 = OpFMul %float %49 %51
         %53 = OpLoad %v2float %21
         %54 = OpCompositeExtract %float %53 0
         %55 = OpLoad %v2float %22
         %56 = OpCompositeExtract %float %55 1
         %57 = OpFSub %float %float_1 %56
         %58 = OpFMul %float %54 %57
         %59 = OpFAdd %float %52 %58
         %60 = OpLoad %v2float %22
         %61 = OpCompositeExtract %float %60 0
         %62 = OpLoad %v2float %21
         %63 = OpCompositeExtract %float %62 1
         %64 = OpFSub %float %float_1 %63
         %65 = OpFMul %float %61 %64
         %66 = OpFAdd %float %59 %65
               OpReturnValue %66
         %46 = OpLabel
         %68 = OpLoad %v2float %22
         %69 = OpCompositeExtract %float %68 1
         %70 = OpLoad %v2float %22
         %71 = OpCompositeExtract %float %70 0
         %72 = OpLoad %v2float %21
         %73 = OpCompositeExtract %float %72 1
         %74 = OpFMul %float %71 %73
         %75 = OpLoad %float %_kGuardedDivideEpsilon
         %76 = OpFAdd %float %43 %75
         %77 = OpFDiv %float %74 %76
         %67 = OpExtInst %float %1 FMin %69 %77
               OpStore %delta %67
         %78 = OpLoad %v2float %21
         %79 = OpCompositeExtract %float %78 1
         %80 = OpFMul %float %67 %79
         %81 = OpLoad %v2float %21
         %82 = OpCompositeExtract %float %81 0
         %83 = OpLoad %v2float %22
         %84 = OpCompositeExtract %float %83 1
         %85 = OpFSub %float %float_1 %84
         %86 = OpFMul %float %82 %85
         %87 = OpFAdd %float %80 %86
         %88 = OpLoad %v2float %22
         %89 = OpCompositeExtract %float %88 0
         %90 = OpLoad %v2float %21
         %91 = OpCompositeExtract %float %90 1
         %92 = OpFSub %float %float_1 %91
         %93 = OpFMul %float %89 %92
         %94 = OpFAdd %float %87 %93
               OpReturnValue %94
         %47 = OpLabel
               OpBranch %29
         %29 = OpLabel
               OpUnreachable
               OpFunctionEnd
       %main = OpFunction %void None %96
         %97 = OpLabel
        %104 = OpVariable %_ptr_Function_v2float Function
        %109 = OpVariable %_ptr_Function_v2float Function
        %114 = OpVariable %_ptr_Function_v2float Function
        %118 = OpVariable %_ptr_Function_v2float Function
        %123 = OpVariable %_ptr_Function_v2float Function
        %127 = OpVariable %_ptr_Function_v2float Function
          %9 = OpSelect %float %false %float_9_99999994en09 %float_0
               OpStore %_kGuardedDivideEpsilon %9
         %98 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %102 = OpLoad %v4float %98
        %103 = OpVectorShuffle %v2float %102 %102 0 3
               OpStore %104 %103
        %105 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %107 = OpLoad %v4float %105
        %108 = OpVectorShuffle %v2float %107 %107 0 3
               OpStore %109 %108
        %110 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %104 %109
        %111 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %112 = OpLoad %v4float %111
        %113 = OpVectorShuffle %v2float %112 %112 1 3
               OpStore %114 %113
        %115 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %116 = OpLoad %v4float %115
        %117 = OpVectorShuffle %v2float %116 %116 1 3
               OpStore %118 %117
        %119 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %114 %118
        %120 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %121 = OpLoad %v4float %120
        %122 = OpVectorShuffle %v2float %121 %121 2 3
               OpStore %123 %122
        %124 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %125 = OpLoad %v4float %124
        %126 = OpVectorShuffle %v2float %125 %125 2 3
               OpStore %127 %126
        %128 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %123 %127
        %129 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %130 = OpLoad %v4float %129
        %131 = OpCompositeExtract %float %130 3
        %132 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %133 = OpLoad %v4float %132
        %134 = OpCompositeExtract %float %133 3
        %135 = OpFSub %float %float_1 %134
        %136 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %137 = OpLoad %v4float %136
        %138 = OpCompositeExtract %float %137 3
        %139 = OpFMul %float %135 %138
        %140 = OpFAdd %float %131 %139
        %141 = OpCompositeConstruct %v4float %110 %119 %128 %140
               OpStore %sk_FragColor %141
               OpReturn
               OpFunctionEnd
