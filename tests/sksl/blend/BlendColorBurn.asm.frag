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
               OpName %guarded_divide_Qhhh "guarded_divide_Qhhh"
               OpName %color_burn_component_Qhh2h2 "color_burn_component_Qhh2h2"
               OpName %dyTerm "dyTerm"
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
               OpDecorate %16 Binding 0
               OpDecorate %16 DescriptorSet 0
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %dyTerm RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %delta RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
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
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
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
               OpDecorate %104 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
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
         %16 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_float = OpTypePointer Function %float
         %20 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %31 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_6_10351562en05 = OpConstant %float 6.10351562e-05
    %float_1 = OpConstant %float 1
       %void = OpTypeVoid
         %97 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%guarded_divide_Qhhh = OpFunction %float None %20
         %21 = OpFunctionParameter %_ptr_Function_float
         %22 = OpFunctionParameter %_ptr_Function_float
         %23 = OpLabel
         %24 = OpLoad %float %21
         %25 = OpLoad %float %22
         %26 = OpLoad %float %_kGuardedDivideEpsilon
         %27 = OpFAdd %float %25 %26
         %28 = OpFDiv %float %24 %27
               OpReturnValue %28
               OpFunctionEnd
%color_burn_component_Qhh2h2 = OpFunction %float None %31
         %32 = OpFunctionParameter %_ptr_Function_v2float
         %33 = OpFunctionParameter %_ptr_Function_v2float
         %34 = OpLabel
     %dyTerm = OpVariable %_ptr_Function_float Function
         %41 = OpVariable %_ptr_Function_float Function
      %delta = OpVariable %_ptr_Function_float Function
         %54 = OpVariable %_ptr_Function_float Function
         %71 = OpVariable %_ptr_Function_float Function
         %74 = OpVariable %_ptr_Function_float Function
         %36 = OpLoad %v2float %33
         %37 = OpCompositeExtract %float %36 1
         %38 = OpLoad %v2float %33
         %39 = OpCompositeExtract %float %38 0
         %40 = OpFOrdEqual %bool %37 %39
               OpSelectionMerge %44 None
               OpBranchConditional %40 %42 %43
         %42 = OpLabel
         %45 = OpLoad %v2float %33
         %46 = OpCompositeExtract %float %45 1
               OpStore %41 %46
               OpBranch %44
         %43 = OpLabel
               OpStore %41 %float_0
               OpBranch %44
         %44 = OpLabel
         %47 = OpLoad %float %41
               OpStore %dyTerm %47
         %50 = OpLoad %v2float %32
         %51 = OpCompositeExtract %float %50 0
         %49 = OpExtInst %float %1 FAbs %51
         %53 = OpFOrdGreaterThanEqual %bool %49 %float_6_10351562en05
               OpSelectionMerge %57 None
               OpBranchConditional %53 %55 %56
         %55 = OpLabel
         %58 = OpLoad %v2float %33
         %59 = OpCompositeExtract %float %58 1
         %61 = OpLoad %v2float %33
         %62 = OpCompositeExtract %float %61 1
         %63 = OpLoad %v2float %33
         %64 = OpCompositeExtract %float %63 1
         %65 = OpLoad %v2float %33
         %66 = OpCompositeExtract %float %65 0
         %67 = OpFSub %float %64 %66
         %68 = OpLoad %v2float %32
         %69 = OpCompositeExtract %float %68 1
         %70 = OpFMul %float %67 %69
               OpStore %71 %70
         %72 = OpLoad %v2float %32
         %73 = OpCompositeExtract %float %72 0
               OpStore %74 %73
         %75 = OpFunctionCall %float %guarded_divide_Qhhh %71 %74
         %60 = OpExtInst %float %1 FMin %62 %75
         %76 = OpFSub %float %59 %60
               OpStore %54 %76
               OpBranch %57
         %56 = OpLabel
               OpStore %54 %47
               OpBranch %57
         %57 = OpLabel
         %77 = OpLoad %float %54
               OpStore %delta %77
         %78 = OpLoad %v2float %32
         %79 = OpCompositeExtract %float %78 1
         %80 = OpFMul %float %77 %79
         %81 = OpLoad %v2float %32
         %82 = OpCompositeExtract %float %81 0
         %84 = OpLoad %v2float %33
         %85 = OpCompositeExtract %float %84 1
         %86 = OpFSub %float %float_1 %85
         %87 = OpFMul %float %82 %86
         %88 = OpFAdd %float %80 %87
         %89 = OpLoad %v2float %33
         %90 = OpCompositeExtract %float %89 0
         %91 = OpLoad %v2float %32
         %92 = OpCompositeExtract %float %91 1
         %93 = OpFSub %float %float_1 %92
         %94 = OpFMul %float %90 %93
         %95 = OpFAdd %float %88 %94
               OpReturnValue %95
               OpFunctionEnd
       %main = OpFunction %void None %97
         %98 = OpLabel
        %105 = OpVariable %_ptr_Function_v2float Function
        %110 = OpVariable %_ptr_Function_v2float Function
        %115 = OpVariable %_ptr_Function_v2float Function
        %119 = OpVariable %_ptr_Function_v2float Function
        %124 = OpVariable %_ptr_Function_v2float Function
        %128 = OpVariable %_ptr_Function_v2float Function
         %10 = OpSelect %float %false %float_9_99999994en09 %float_0
               OpStore %_kGuardedDivideEpsilon %10
         %99 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
        %103 = OpLoad %v4float %99
        %104 = OpVectorShuffle %v2float %103 %103 0 3
               OpStore %105 %104
        %106 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
        %108 = OpLoad %v4float %106
        %109 = OpVectorShuffle %v2float %108 %108 0 3
               OpStore %110 %109
        %111 = OpFunctionCall %float %color_burn_component_Qhh2h2 %105 %110
        %112 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
        %113 = OpLoad %v4float %112
        %114 = OpVectorShuffle %v2float %113 %113 1 3
               OpStore %115 %114
        %116 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
        %117 = OpLoad %v4float %116
        %118 = OpVectorShuffle %v2float %117 %117 1 3
               OpStore %119 %118
        %120 = OpFunctionCall %float %color_burn_component_Qhh2h2 %115 %119
        %121 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
        %122 = OpLoad %v4float %121
        %123 = OpVectorShuffle %v2float %122 %122 2 3
               OpStore %124 %123
        %125 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
        %126 = OpLoad %v4float %125
        %127 = OpVectorShuffle %v2float %126 %126 2 3
               OpStore %128 %127
        %129 = OpFunctionCall %float %color_burn_component_Qhh2h2 %124 %128
        %130 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
        %131 = OpLoad %v4float %130
        %132 = OpCompositeExtract %float %131 3
        %133 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
        %134 = OpLoad %v4float %133
        %135 = OpCompositeExtract %float %134 3
        %136 = OpFSub %float %float_1 %135
        %137 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
        %138 = OpLoad %v4float %137
        %139 = OpCompositeExtract %float %138 3
        %140 = OpFMul %float %136 %139
        %141 = OpFAdd %float %132 %140
        %142 = OpCompositeConstruct %v4float %111 %120 %129 %141
               OpStore %sk_FragColor %142
               OpReturn
               OpFunctionEnd
