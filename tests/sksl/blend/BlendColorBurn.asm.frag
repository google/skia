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
               OpName %color_burn_component_Qhh2h2 "color_burn_component_Qhh2h2"
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
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
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
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %delta RelaxedPrecision
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
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
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
        %103 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%color_burn_component_Qhh2h2 = OpFunction %float None %22
         %23 = OpFunctionParameter %_ptr_Function_v2float
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
      %delta = OpVariable %_ptr_Function_float Function
         %26 = OpLoad %v2float %24
         %27 = OpCompositeExtract %float %26 1
         %28 = OpLoad %v2float %24
         %29 = OpCompositeExtract %float %28 0
         %30 = OpFOrdEqual %bool %27 %29
               OpSelectionMerge %33 None
               OpBranchConditional %30 %31 %32
         %31 = OpLabel
         %34 = OpLoad %v2float %23
         %35 = OpCompositeExtract %float %34 1
         %36 = OpLoad %v2float %24
         %37 = OpCompositeExtract %float %36 1
         %38 = OpFMul %float %35 %37
         %39 = OpLoad %v2float %23
         %40 = OpCompositeExtract %float %39 0
         %42 = OpLoad %v2float %24
         %43 = OpCompositeExtract %float %42 1
         %44 = OpFSub %float %float_1 %43
         %45 = OpFMul %float %40 %44
         %46 = OpFAdd %float %38 %45
         %47 = OpLoad %v2float %24
         %48 = OpCompositeExtract %float %47 0
         %49 = OpLoad %v2float %23
         %50 = OpCompositeExtract %float %49 1
         %51 = OpFSub %float %float_1 %50
         %52 = OpFMul %float %48 %51
         %53 = OpFAdd %float %46 %52
               OpReturnValue %53
         %32 = OpLabel
         %54 = OpLoad %v2float %23
         %55 = OpCompositeExtract %float %54 0
         %56 = OpFOrdEqual %bool %55 %float_0
               OpSelectionMerge %59 None
               OpBranchConditional %56 %57 %58
         %57 = OpLabel
         %60 = OpLoad %v2float %24
         %61 = OpCompositeExtract %float %60 0
         %62 = OpLoad %v2float %23
         %63 = OpCompositeExtract %float %62 1
         %64 = OpFSub %float %float_1 %63
         %65 = OpFMul %float %61 %64
               OpReturnValue %65
         %58 = OpLabel
         %69 = OpLoad %v2float %24
         %70 = OpCompositeExtract %float %69 1
         %71 = OpLoad %v2float %24
         %72 = OpCompositeExtract %float %71 1
         %73 = OpLoad %v2float %24
         %74 = OpCompositeExtract %float %73 0
         %75 = OpFSub %float %72 %74
         %76 = OpLoad %v2float %23
         %77 = OpCompositeExtract %float %76 1
         %78 = OpFMul %float %75 %77
         %79 = OpLoad %v2float %23
         %80 = OpCompositeExtract %float %79 0
         %81 = OpLoad %float %_kGuardedDivideEpsilon
         %82 = OpFAdd %float %80 %81
         %83 = OpFDiv %float %78 %82
         %84 = OpFSub %float %70 %83
         %68 = OpExtInst %float %1 FMax %float_0 %84
               OpStore %delta %68
         %85 = OpLoad %v2float %23
         %86 = OpCompositeExtract %float %85 1
         %87 = OpFMul %float %68 %86
         %88 = OpLoad %v2float %23
         %89 = OpCompositeExtract %float %88 0
         %90 = OpLoad %v2float %24
         %91 = OpCompositeExtract %float %90 1
         %92 = OpFSub %float %float_1 %91
         %93 = OpFMul %float %89 %92
         %94 = OpFAdd %float %87 %93
         %95 = OpLoad %v2float %24
         %96 = OpCompositeExtract %float %95 0
         %97 = OpLoad %v2float %23
         %98 = OpCompositeExtract %float %97 1
         %99 = OpFSub %float %float_1 %98
        %100 = OpFMul %float %96 %99
        %101 = OpFAdd %float %94 %100
               OpReturnValue %101
         %59 = OpLabel
               OpBranch %33
         %33 = OpLabel
               OpUnreachable
               OpFunctionEnd
       %main = OpFunction %void None %103
        %104 = OpLabel
        %111 = OpVariable %_ptr_Function_v2float Function
        %116 = OpVariable %_ptr_Function_v2float Function
        %121 = OpVariable %_ptr_Function_v2float Function
        %125 = OpVariable %_ptr_Function_v2float Function
        %130 = OpVariable %_ptr_Function_v2float Function
        %134 = OpVariable %_ptr_Function_v2float Function
          %9 = OpSelect %float %false %float_9_99999994en09 %float_0
               OpStore %_kGuardedDivideEpsilon %9
        %105 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
        %109 = OpLoad %v4float %105
        %110 = OpVectorShuffle %v2float %109 %109 0 3
               OpStore %111 %110
        %112 = OpAccessChain %_ptr_Uniform_v4float %17 %int_1
        %114 = OpLoad %v4float %112
        %115 = OpVectorShuffle %v2float %114 %114 0 3
               OpStore %116 %115
        %117 = OpFunctionCall %float %color_burn_component_Qhh2h2 %111 %116
        %118 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
        %119 = OpLoad %v4float %118
        %120 = OpVectorShuffle %v2float %119 %119 1 3
               OpStore %121 %120
        %122 = OpAccessChain %_ptr_Uniform_v4float %17 %int_1
        %123 = OpLoad %v4float %122
        %124 = OpVectorShuffle %v2float %123 %123 1 3
               OpStore %125 %124
        %126 = OpFunctionCall %float %color_burn_component_Qhh2h2 %121 %125
        %127 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
        %128 = OpLoad %v4float %127
        %129 = OpVectorShuffle %v2float %128 %128 2 3
               OpStore %130 %129
        %131 = OpAccessChain %_ptr_Uniform_v4float %17 %int_1
        %132 = OpLoad %v4float %131
        %133 = OpVectorShuffle %v2float %132 %132 2 3
               OpStore %134 %133
        %135 = OpFunctionCall %float %color_burn_component_Qhh2h2 %130 %134
        %136 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
        %137 = OpLoad %v4float %136
        %138 = OpCompositeExtract %float %137 3
        %139 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
        %140 = OpLoad %v4float %139
        %141 = OpCompositeExtract %float %140 3
        %142 = OpFSub %float %float_1 %141
        %143 = OpAccessChain %_ptr_Uniform_v4float %17 %int_1
        %144 = OpLoad %v4float %143
        %145 = OpCompositeExtract %float %144 3
        %146 = OpFMul %float %142 %145
        %147 = OpFAdd %float %138 %146
        %148 = OpCompositeConstruct %v4float %117 %126 %135 %147
               OpStore %sk_FragColor %148
               OpReturn
               OpFunctionEnd
