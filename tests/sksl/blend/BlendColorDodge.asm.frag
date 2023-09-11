               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %_kGuardedDivideEpsilon "$kGuardedDivideEpsilon"
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %color_dodge_component_Qhh2h2 "color_dodge_component_Qhh2h2"
               OpName %delta "delta"
               OpName %main "main"
               OpDecorate %_kGuardedDivideEpsilon RelaxedPrecision
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %17 Binding 0
               OpDecorate %17 DescriptorSet 0
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %delta RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
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
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
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
               OpDecorate %96 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
      %float = OpTypeFloat 32
%_ptr_Private_float = OpTypePointer Private %float
%_kGuardedDivideEpsilon = OpVariable %_ptr_Private_float Private
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%float_9_99999994en09 = OpConstant %float 9.99999994e-09
    %float_0 = OpConstant %float 0
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %17 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %22 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
    %float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
       %void = OpTypeVoid
         %98 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%color_dodge_component_Qhh2h2 = OpFunction %float None %22
         %23 = OpFunctionParameter %_ptr_Function_v2float
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
      %delta = OpVariable %_ptr_Function_float Function
         %26 = OpLoad %v2float %24
         %27 = OpCompositeExtract %float %26 0
         %28 = OpFOrdEqual %bool %27 %float_0
               OpSelectionMerge %31 None
               OpBranchConditional %28 %29 %30
         %29 = OpLabel
         %32 = OpLoad %v2float %23
         %33 = OpCompositeExtract %float %32 0
         %35 = OpLoad %v2float %24
         %36 = OpCompositeExtract %float %35 1
         %37 = OpFSub %float %float_1 %36
         %38 = OpFMul %float %33 %37
               OpReturnValue %38
         %30 = OpLabel
         %41 = OpLoad %v2float %23
         %42 = OpCompositeExtract %float %41 1
         %43 = OpLoad %v2float %23
         %44 = OpCompositeExtract %float %43 0
         %45 = OpFSub %float %42 %44
               OpStore %delta %45
         %46 = OpFOrdEqual %bool %45 %float_0
               OpSelectionMerge %49 None
               OpBranchConditional %46 %47 %48
         %47 = OpLabel
         %50 = OpLoad %v2float %23
         %51 = OpCompositeExtract %float %50 1
         %52 = OpLoad %v2float %24
         %53 = OpCompositeExtract %float %52 1
         %54 = OpFMul %float %51 %53
         %55 = OpLoad %v2float %23
         %56 = OpCompositeExtract %float %55 0
         %57 = OpLoad %v2float %24
         %58 = OpCompositeExtract %float %57 1
         %59 = OpFSub %float %float_1 %58
         %60 = OpFMul %float %56 %59
         %61 = OpFAdd %float %54 %60
         %62 = OpLoad %v2float %24
         %63 = OpCompositeExtract %float %62 0
         %64 = OpLoad %v2float %23
         %65 = OpCompositeExtract %float %64 1
         %66 = OpFSub %float %float_1 %65
         %67 = OpFMul %float %63 %66
         %68 = OpFAdd %float %61 %67
               OpReturnValue %68
         %48 = OpLabel
         %70 = OpLoad %v2float %24
         %71 = OpCompositeExtract %float %70 1
         %72 = OpLoad %v2float %24
         %73 = OpCompositeExtract %float %72 0
         %74 = OpLoad %v2float %23
         %75 = OpCompositeExtract %float %74 1
         %76 = OpFMul %float %73 %75
         %77 = OpLoad %float %_kGuardedDivideEpsilon
         %78 = OpFAdd %float %45 %77
         %79 = OpFDiv %float %76 %78
         %69 = OpExtInst %float %1 FMin %71 %79
               OpStore %delta %69
         %80 = OpLoad %v2float %23
         %81 = OpCompositeExtract %float %80 1
         %82 = OpFMul %float %69 %81
         %83 = OpLoad %v2float %23
         %84 = OpCompositeExtract %float %83 0
         %85 = OpLoad %v2float %24
         %86 = OpCompositeExtract %float %85 1
         %87 = OpFSub %float %float_1 %86
         %88 = OpFMul %float %84 %87
         %89 = OpFAdd %float %82 %88
         %90 = OpLoad %v2float %24
         %91 = OpCompositeExtract %float %90 0
         %92 = OpLoad %v2float %23
         %93 = OpCompositeExtract %float %92 1
         %94 = OpFSub %float %float_1 %93
         %95 = OpFMul %float %91 %94
         %96 = OpFAdd %float %89 %95
               OpReturnValue %96
         %49 = OpLabel
               OpBranch %31
         %31 = OpLabel
               OpUnreachable
               OpFunctionEnd
       %main = OpFunction %void None %98
         %99 = OpLabel
        %106 = OpVariable %_ptr_Function_v2float Function
        %111 = OpVariable %_ptr_Function_v2float Function
        %116 = OpVariable %_ptr_Function_v2float Function
        %120 = OpVariable %_ptr_Function_v2float Function
        %125 = OpVariable %_ptr_Function_v2float Function
        %129 = OpVariable %_ptr_Function_v2float Function
          %9 = OpSelect %float %false %float_9_99999994en09 %float_0
               OpStore %_kGuardedDivideEpsilon %9
        %100 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
        %104 = OpLoad %v4float %100
        %105 = OpVectorShuffle %v2float %104 %104 0 3
               OpStore %106 %105
        %107 = OpAccessChain %_ptr_Uniform_v4float %17 %int_1
        %109 = OpLoad %v4float %107
        %110 = OpVectorShuffle %v2float %109 %109 0 3
               OpStore %111 %110
        %112 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %106 %111
        %113 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
        %114 = OpLoad %v4float %113
        %115 = OpVectorShuffle %v2float %114 %114 1 3
               OpStore %116 %115
        %117 = OpAccessChain %_ptr_Uniform_v4float %17 %int_1
        %118 = OpLoad %v4float %117
        %119 = OpVectorShuffle %v2float %118 %118 1 3
               OpStore %120 %119
        %121 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %116 %120
        %122 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
        %123 = OpLoad %v4float %122
        %124 = OpVectorShuffle %v2float %123 %123 2 3
               OpStore %125 %124
        %126 = OpAccessChain %_ptr_Uniform_v4float %17 %int_1
        %127 = OpLoad %v4float %126
        %128 = OpVectorShuffle %v2float %127 %127 2 3
               OpStore %129 %128
        %130 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %125 %129
        %131 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
        %132 = OpLoad %v4float %131
        %133 = OpCompositeExtract %float %132 3
        %134 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
        %135 = OpLoad %v4float %134
        %136 = OpCompositeExtract %float %135 3
        %137 = OpFSub %float %float_1 %136
        %138 = OpAccessChain %_ptr_Uniform_v4float %17 %int_1
        %139 = OpLoad %v4float %138
        %140 = OpCompositeExtract %float %139 3
        %141 = OpFMul %float %137 %140
        %142 = OpFAdd %float %133 %141
        %143 = OpCompositeConstruct %v4float %112 %121 %130 %142
               OpStore %sk_FragColor %143
               OpReturn
               OpFunctionEnd
