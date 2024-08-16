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
               OpDecorate %15 Binding 0
               OpDecorate %15 DescriptorSet 0
               OpDecorate %dxScale RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %delta RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
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
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
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
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%float_6_10351562en05 = OpConstant %float 6.10351562e-05
    %float_1 = OpConstant %float 1
       %void = OpTypeVoid
        %104 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%color_dodge_component_Qhh2h2 = OpFunction %float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpFunctionParameter %_ptr_Function_v2float
         %23 = OpLabel
    %dxScale = OpVariable %_ptr_Function_float Function
      %delta = OpVariable %_ptr_Function_float Function
         %26 = OpLoad %v2float %22
         %27 = OpCompositeExtract %float %26 0
         %28 = OpFOrdEqual %bool %27 %float_0
         %29 = OpSelect %int %28 %int_0 %int_1
         %33 = OpConvertSToF %float %29
               OpStore %dxScale %33
         %36 = OpLoad %v2float %22
         %37 = OpCompositeExtract %float %36 1
         %39 = OpLoad %v2float %22
         %40 = OpCompositeExtract %float %39 1
         %41 = OpLoad %v2float %22
         %42 = OpCompositeExtract %float %41 0
         %43 = OpLoad %v2float %21
         %44 = OpCompositeExtract %float %43 1
         %45 = OpFMul %float %42 %44
         %46 = OpLoad %v2float %21
         %47 = OpCompositeExtract %float %46 1
         %48 = OpLoad %v2float %21
         %49 = OpCompositeExtract %float %48 0
         %50 = OpFSub %float %47 %49
         %51 = OpLoad %float %_kGuardedDivideEpsilon
         %52 = OpFAdd %float %50 %51
         %53 = OpFDiv %float %45 %52
         %55 = OpLoad %v2float %21
         %56 = OpCompositeExtract %float %55 1
         %57 = OpLoad %v2float %21
         %58 = OpCompositeExtract %float %57 0
         %59 = OpFSub %float %56 %58
         %54 = OpExtInst %float %1 FAbs %59
         %61 = OpFOrdGreaterThanEqual %bool %54 %float_6_10351562en05
         %62 = OpLoad %v2float %22
         %63 = OpCompositeExtract %float %62 1
         %64 = OpLoad %v2float %22
         %65 = OpCompositeExtract %float %64 0
         %66 = OpLoad %v2float %21
         %67 = OpCompositeExtract %float %66 1
         %68 = OpFMul %float %65 %67
         %69 = OpLoad %v2float %21
         %70 = OpCompositeExtract %float %69 1
         %71 = OpLoad %v2float %21
         %72 = OpCompositeExtract %float %71 0
         %73 = OpFSub %float %70 %72
         %74 = OpLoad %float %_kGuardedDivideEpsilon
         %75 = OpFAdd %float %73 %74
         %76 = OpFDiv %float %68 %75
         %78 = OpLoad %v2float %21
         %79 = OpCompositeExtract %float %78 1
         %80 = OpLoad %v2float %21
         %81 = OpCompositeExtract %float %80 0
         %82 = OpFSub %float %79 %81
         %77 = OpExtInst %float %1 FAbs %82
         %83 = OpFOrdGreaterThanEqual %bool %77 %float_6_10351562en05
         %38 = OpSelect %float %83 %76 %63
         %35 = OpExtInst %float %1 FMin %37 %38
         %84 = OpFMul %float %33 %35
               OpStore %delta %84
         %85 = OpLoad %v2float %21
         %86 = OpCompositeExtract %float %85 1
         %87 = OpFMul %float %84 %86
         %88 = OpLoad %v2float %21
         %89 = OpCompositeExtract %float %88 0
         %91 = OpLoad %v2float %22
         %92 = OpCompositeExtract %float %91 1
         %93 = OpFSub %float %float_1 %92
         %94 = OpFMul %float %89 %93
         %95 = OpFAdd %float %87 %94
         %96 = OpLoad %v2float %22
         %97 = OpCompositeExtract %float %96 0
         %98 = OpLoad %v2float %21
         %99 = OpCompositeExtract %float %98 1
        %100 = OpFSub %float %float_1 %99
        %101 = OpFMul %float %97 %100
        %102 = OpFAdd %float %95 %101
               OpReturnValue %102
               OpFunctionEnd
       %main = OpFunction %void None %104
        %105 = OpLabel
        %110 = OpVariable %_ptr_Function_v2float Function
        %114 = OpVariable %_ptr_Function_v2float Function
        %119 = OpVariable %_ptr_Function_v2float Function
        %123 = OpVariable %_ptr_Function_v2float Function
        %128 = OpVariable %_ptr_Function_v2float Function
        %132 = OpVariable %_ptr_Function_v2float Function
          %9 = OpSelect %float %false %float_9_99999994en09 %float_0
               OpStore %_kGuardedDivideEpsilon %9
        %106 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %108 = OpLoad %v4float %106
        %109 = OpVectorShuffle %v2float %108 %108 0 3
               OpStore %110 %109
        %111 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %112 = OpLoad %v4float %111
        %113 = OpVectorShuffle %v2float %112 %112 0 3
               OpStore %114 %113
        %115 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %110 %114
        %116 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %117 = OpLoad %v4float %116
        %118 = OpVectorShuffle %v2float %117 %117 1 3
               OpStore %119 %118
        %120 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %121 = OpLoad %v4float %120
        %122 = OpVectorShuffle %v2float %121 %121 1 3
               OpStore %123 %122
        %124 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %119 %123
        %125 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %126 = OpLoad %v4float %125
        %127 = OpVectorShuffle %v2float %126 %126 2 3
               OpStore %128 %127
        %129 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %130 = OpLoad %v4float %129
        %131 = OpVectorShuffle %v2float %130 %130 2 3
               OpStore %132 %131
        %133 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %128 %132
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
