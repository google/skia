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
               OpName %color_dodge_component_Qhh2h2 "color_dodge_component_Qhh2h2"
               OpName %dxScale "dxScale"
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
               OpDecorate %dxScale RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %delta RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
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
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
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
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%float_6_10351562en05 = OpConstant %float 6.10351562e-05
    %float_1 = OpConstant %float 1
       %void = OpTypeVoid
         %96 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%color_dodge_component_Qhh2h2 = OpFunction %float None %31
         %32 = OpFunctionParameter %_ptr_Function_v2float
         %33 = OpFunctionParameter %_ptr_Function_v2float
         %34 = OpLabel
    %dxScale = OpVariable %_ptr_Function_float Function
      %delta = OpVariable %_ptr_Function_float Function
         %56 = OpVariable %_ptr_Function_float Function
         %65 = OpVariable %_ptr_Function_float Function
         %71 = OpVariable %_ptr_Function_float Function
         %36 = OpLoad %v2float %33
         %37 = OpCompositeExtract %float %36 0
         %38 = OpFOrdEqual %bool %37 %float_0
         %39 = OpSelect %int %38 %int_0 %int_1
         %43 = OpConvertSToF %float %39
               OpStore %dxScale %43
         %46 = OpLoad %v2float %33
         %47 = OpCompositeExtract %float %46 1
         %49 = OpLoad %v2float %32
         %50 = OpCompositeExtract %float %49 1
         %51 = OpLoad %v2float %32
         %52 = OpCompositeExtract %float %51 0
         %53 = OpFSub %float %50 %52
         %48 = OpExtInst %float %1 FAbs %53
         %55 = OpFOrdGreaterThanEqual %bool %48 %float_6_10351562en05
               OpSelectionMerge %59 None
               OpBranchConditional %55 %57 %58
         %57 = OpLabel
         %60 = OpLoad %v2float %33
         %61 = OpCompositeExtract %float %60 0
         %62 = OpLoad %v2float %32
         %63 = OpCompositeExtract %float %62 1
         %64 = OpFMul %float %61 %63
               OpStore %65 %64
         %66 = OpLoad %v2float %32
         %67 = OpCompositeExtract %float %66 1
         %68 = OpLoad %v2float %32
         %69 = OpCompositeExtract %float %68 0
         %70 = OpFSub %float %67 %69
               OpStore %71 %70
         %72 = OpFunctionCall %float %guarded_divide_Qhhh %65 %71
               OpStore %56 %72
               OpBranch %59
         %58 = OpLabel
         %73 = OpLoad %v2float %33
         %74 = OpCompositeExtract %float %73 1
               OpStore %56 %74
               OpBranch %59
         %59 = OpLabel
         %75 = OpLoad %float %56
         %45 = OpExtInst %float %1 FMin %47 %75
         %76 = OpFMul %float %43 %45
               OpStore %delta %76
         %77 = OpLoad %v2float %32
         %78 = OpCompositeExtract %float %77 1
         %79 = OpFMul %float %76 %78
         %80 = OpLoad %v2float %32
         %81 = OpCompositeExtract %float %80 0
         %83 = OpLoad %v2float %33
         %84 = OpCompositeExtract %float %83 1
         %85 = OpFSub %float %float_1 %84
         %86 = OpFMul %float %81 %85
         %87 = OpFAdd %float %79 %86
         %88 = OpLoad %v2float %33
         %89 = OpCompositeExtract %float %88 0
         %90 = OpLoad %v2float %32
         %91 = OpCompositeExtract %float %90 1
         %92 = OpFSub %float %float_1 %91
         %93 = OpFMul %float %89 %92
         %94 = OpFAdd %float %87 %93
               OpReturnValue %94
               OpFunctionEnd
       %main = OpFunction %void None %96
         %97 = OpLabel
        %102 = OpVariable %_ptr_Function_v2float Function
        %106 = OpVariable %_ptr_Function_v2float Function
        %111 = OpVariable %_ptr_Function_v2float Function
        %115 = OpVariable %_ptr_Function_v2float Function
        %120 = OpVariable %_ptr_Function_v2float Function
        %124 = OpVariable %_ptr_Function_v2float Function
         %10 = OpSelect %float %false %float_9_99999994en09 %float_0
               OpStore %_kGuardedDivideEpsilon %10
         %98 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
        %100 = OpLoad %v4float %98
        %101 = OpVectorShuffle %v2float %100 %100 0 3
               OpStore %102 %101
        %103 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
        %104 = OpLoad %v4float %103
        %105 = OpVectorShuffle %v2float %104 %104 0 3
               OpStore %106 %105
        %107 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %102 %106
        %108 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
        %109 = OpLoad %v4float %108
        %110 = OpVectorShuffle %v2float %109 %109 1 3
               OpStore %111 %110
        %112 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
        %113 = OpLoad %v4float %112
        %114 = OpVectorShuffle %v2float %113 %113 1 3
               OpStore %115 %114
        %116 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %111 %115
        %117 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
        %118 = OpLoad %v4float %117
        %119 = OpVectorShuffle %v2float %118 %118 2 3
               OpStore %120 %119
        %121 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
        %122 = OpLoad %v4float %121
        %123 = OpVectorShuffle %v2float %122 %122 2 3
               OpStore %124 %123
        %125 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %120 %124
        %126 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
        %127 = OpLoad %v4float %126
        %128 = OpCompositeExtract %float %127 3
        %129 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
        %130 = OpLoad %v4float %129
        %131 = OpCompositeExtract %float %130 3
        %132 = OpFSub %float %float_1 %131
        %133 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
        %134 = OpLoad %v4float %133
        %135 = OpCompositeExtract %float %134 3
        %136 = OpFMul %float %132 %135
        %137 = OpFAdd %float %128 %136
        %138 = OpCompositeConstruct %v4float %107 %116 %125 %137
               OpStore %sk_FragColor %138
               OpReturn
               OpFunctionEnd
