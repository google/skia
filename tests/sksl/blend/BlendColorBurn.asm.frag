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
               OpName %color_burn_component_Qhh2h2 "color_burn_component_Qhh2h2"
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
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
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
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %delta RelaxedPrecision
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
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
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
        %101 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%color_burn_component_Qhh2h2 = OpFunction %float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpFunctionParameter %_ptr_Function_v2float
         %23 = OpLabel
      %delta = OpVariable %_ptr_Function_float Function
         %24 = OpLoad %v2float %22
         %25 = OpCompositeExtract %float %24 1
         %26 = OpLoad %v2float %22
         %27 = OpCompositeExtract %float %26 0
         %28 = OpFOrdEqual %bool %25 %27
               OpSelectionMerge %31 None
               OpBranchConditional %28 %29 %30
         %29 = OpLabel
         %32 = OpLoad %v2float %21
         %33 = OpCompositeExtract %float %32 1
         %34 = OpLoad %v2float %22
         %35 = OpCompositeExtract %float %34 1
         %36 = OpFMul %float %33 %35
         %37 = OpLoad %v2float %21
         %38 = OpCompositeExtract %float %37 0
         %40 = OpLoad %v2float %22
         %41 = OpCompositeExtract %float %40 1
         %42 = OpFSub %float %float_1 %41
         %43 = OpFMul %float %38 %42
         %44 = OpFAdd %float %36 %43
         %45 = OpLoad %v2float %22
         %46 = OpCompositeExtract %float %45 0
         %47 = OpLoad %v2float %21
         %48 = OpCompositeExtract %float %47 1
         %49 = OpFSub %float %float_1 %48
         %50 = OpFMul %float %46 %49
         %51 = OpFAdd %float %44 %50
               OpReturnValue %51
         %30 = OpLabel
         %52 = OpLoad %v2float %21
         %53 = OpCompositeExtract %float %52 0
         %54 = OpFOrdEqual %bool %53 %float_0
               OpSelectionMerge %57 None
               OpBranchConditional %54 %55 %56
         %55 = OpLabel
         %58 = OpLoad %v2float %22
         %59 = OpCompositeExtract %float %58 0
         %60 = OpLoad %v2float %21
         %61 = OpCompositeExtract %float %60 1
         %62 = OpFSub %float %float_1 %61
         %63 = OpFMul %float %59 %62
               OpReturnValue %63
         %56 = OpLabel
         %67 = OpLoad %v2float %22
         %68 = OpCompositeExtract %float %67 1
         %69 = OpLoad %v2float %22
         %70 = OpCompositeExtract %float %69 1
         %71 = OpLoad %v2float %22
         %72 = OpCompositeExtract %float %71 0
         %73 = OpFSub %float %70 %72
         %74 = OpLoad %v2float %21
         %75 = OpCompositeExtract %float %74 1
         %76 = OpFMul %float %73 %75
         %77 = OpLoad %v2float %21
         %78 = OpCompositeExtract %float %77 0
         %79 = OpLoad %float %_kGuardedDivideEpsilon
         %80 = OpFAdd %float %78 %79
         %81 = OpFDiv %float %76 %80
         %82 = OpFSub %float %68 %81
         %66 = OpExtInst %float %1 FMax %float_0 %82
               OpStore %delta %66
         %83 = OpLoad %v2float %21
         %84 = OpCompositeExtract %float %83 1
         %85 = OpFMul %float %66 %84
         %86 = OpLoad %v2float %21
         %87 = OpCompositeExtract %float %86 0
         %88 = OpLoad %v2float %22
         %89 = OpCompositeExtract %float %88 1
         %90 = OpFSub %float %float_1 %89
         %91 = OpFMul %float %87 %90
         %92 = OpFAdd %float %85 %91
         %93 = OpLoad %v2float %22
         %94 = OpCompositeExtract %float %93 0
         %95 = OpLoad %v2float %21
         %96 = OpCompositeExtract %float %95 1
         %97 = OpFSub %float %float_1 %96
         %98 = OpFMul %float %94 %97
         %99 = OpFAdd %float %92 %98
               OpReturnValue %99
         %57 = OpLabel
               OpBranch %31
         %31 = OpLabel
               OpUnreachable
               OpFunctionEnd
       %main = OpFunction %void None %101
        %102 = OpLabel
        %109 = OpVariable %_ptr_Function_v2float Function
        %114 = OpVariable %_ptr_Function_v2float Function
        %119 = OpVariable %_ptr_Function_v2float Function
        %123 = OpVariable %_ptr_Function_v2float Function
        %128 = OpVariable %_ptr_Function_v2float Function
        %132 = OpVariable %_ptr_Function_v2float Function
          %9 = OpSelect %float %false %float_9_99999994en09 %float_0
               OpStore %_kGuardedDivideEpsilon %9
        %103 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %107 = OpLoad %v4float %103
        %108 = OpVectorShuffle %v2float %107 %107 0 3
               OpStore %109 %108
        %110 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %112 = OpLoad %v4float %110
        %113 = OpVectorShuffle %v2float %112 %112 0 3
               OpStore %114 %113
        %115 = OpFunctionCall %float %color_burn_component_Qhh2h2 %109 %114
        %116 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %117 = OpLoad %v4float %116
        %118 = OpVectorShuffle %v2float %117 %117 1 3
               OpStore %119 %118
        %120 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %121 = OpLoad %v4float %120
        %122 = OpVectorShuffle %v2float %121 %121 1 3
               OpStore %123 %122
        %124 = OpFunctionCall %float %color_burn_component_Qhh2h2 %119 %123
        %125 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %126 = OpLoad %v4float %125
        %127 = OpVectorShuffle %v2float %126 %126 2 3
               OpStore %128 %127
        %129 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %130 = OpLoad %v4float %129
        %131 = OpVectorShuffle %v2float %130 %130 2 3
               OpStore %132 %131
        %133 = OpFunctionCall %float %color_burn_component_Qhh2h2 %128 %132
        %134 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %135 = OpLoad %v4float %134
        %136 = OpCompositeExtract %float %135 3
        %137 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %138 = OpLoad %v4float %137
        %139 = OpCompositeExtract %float %138 3
        %140 = OpFSub %float %float_1 %139
        %141 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %142 = OpLoad %v4float %141
        %143 = OpCompositeExtract %float %142 3
        %144 = OpFMul %float %140 %143
        %145 = OpFAdd %float %136 %144
        %146 = OpCompositeConstruct %v4float %115 %124 %133 %145
               OpStore %sk_FragColor %146
               OpReturn
               OpFunctionEnd
