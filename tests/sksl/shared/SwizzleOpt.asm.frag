               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorRed"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "testInputs"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %fn_hh4 "fn_hh4"
               OpName %x "x"
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
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %v RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
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
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %22 = OpTypeFunction %float %_ptr_Function_v4float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
       %bool = OpTypeBool
         %42 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
    %v3float = OpTypeVector %float 3
    %float_1 = OpConstant %float 1
  %float_123 = OpConstant %float 123
  %float_456 = OpConstant %float 456
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %99 = OpConstantComposite %v4float %float_1 %float_1 %float_2 %float_3
      %int_0 = OpConstant %int 0
        %128 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
     %v4bool = OpTypeVector %bool 4
%_entrypoint_v = OpFunction %void None %13
         %14 = OpLabel
         %18 = OpVariable %_ptr_Function_v2float Function
               OpStore %18 %17
         %20 = OpFunctionCall %v4float %main %18
               OpStore %sk_FragColor %20
               OpReturn
               OpFunctionEnd
     %fn_hh4 = OpFunction %float None %22
         %23 = OpFunctionParameter %_ptr_Function_v4float
         %24 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
               OpStore %x %int_1
               OpBranch %29
         %29 = OpLabel
               OpLoopMerge %33 %32 None
               OpBranch %30
         %30 = OpLabel
         %34 = OpLoad %int %x
         %36 = OpSLessThanEqual %bool %34 %int_2
               OpBranchConditional %36 %31 %33
         %31 = OpLabel
         %38 = OpLoad %v4float %23
         %39 = OpCompositeExtract %float %38 0
               OpReturnValue %39
         %32 = OpLabel
         %40 = OpLoad %int %x
         %41 = OpIAdd %int %40 %int_1
               OpStore %x %41
               OpBranch %29
         %33 = OpLabel
               OpUnreachable
               OpFunctionEnd
       %main = OpFunction %v4float None %42
         %43 = OpFunctionParameter %_ptr_Function_v2float
         %44 = OpLabel
          %v = OpVariable %_ptr_Function_v4float Function
         %73 = OpVariable %_ptr_Function_v4float Function
         %79 = OpVariable %_ptr_Function_v4float Function
         %83 = OpVariable %_ptr_Function_v4float Function
         %86 = OpVariable %_ptr_Function_v4float Function
         %89 = OpVariable %_ptr_Function_v4float Function
         %93 = OpVariable %_ptr_Function_v4float Function
        %132 = OpVariable %_ptr_Function_v4float Function
         %46 = OpAccessChain %_ptr_Uniform_v4float %8 %int_2
         %48 = OpLoad %v4float %46
               OpStore %v %48
         %49 = OpVectorShuffle %v3float %48 %48 2 1 0
         %51 = OpCompositeExtract %float %49 0
         %52 = OpCompositeExtract %float %49 1
         %53 = OpCompositeExtract %float %49 2
         %54 = OpCompositeConstruct %v4float %float_0 %51 %52 %53
               OpStore %v %54
         %55 = OpVectorShuffle %v2float %54 %54 0 3
         %56 = OpCompositeExtract %float %55 0
         %57 = OpCompositeExtract %float %55 1
         %58 = OpCompositeConstruct %v4float %float_0 %float_0 %56 %57
               OpStore %v %58
         %60 = OpVectorShuffle %v2float %58 %58 3 0
         %61 = OpCompositeExtract %float %60 0
         %62 = OpCompositeExtract %float %60 1
         %63 = OpCompositeConstruct %v4float %float_1 %float_1 %61 %62
               OpStore %v %63
         %64 = OpVectorShuffle %v2float %63 %63 2 1
         %65 = OpCompositeExtract %float %64 0
         %66 = OpCompositeExtract %float %64 1
         %67 = OpCompositeConstruct %v4float %65 %66 %float_1 %float_1
               OpStore %v %67
         %68 = OpVectorShuffle %v2float %67 %67 0 0
         %69 = OpCompositeExtract %float %68 0
         %70 = OpCompositeExtract %float %68 1
         %71 = OpCompositeConstruct %v4float %69 %70 %float_1 %float_1
               OpStore %v %71
         %72 = OpVectorShuffle %v4float %71 %71 3 2 3 2
               OpStore %v %72
               OpStore %73 %72
         %74 = OpFunctionCall %float %fn_hh4 %73
         %77 = OpCompositeConstruct %v3float %74 %float_123 %float_456
         %78 = OpVectorShuffle %v4float %77 %77 1 1 2 2
               OpStore %v %78
               OpStore %79 %78
         %80 = OpFunctionCall %float %fn_hh4 %79
         %81 = OpCompositeConstruct %v3float %80 %float_123 %float_456
         %82 = OpVectorShuffle %v4float %81 %81 1 1 2 2
               OpStore %v %82
               OpStore %83 %82
         %84 = OpFunctionCall %float %fn_hh4 %83
         %85 = OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %84
               OpStore %v %85
               OpStore %86 %85
         %87 = OpFunctionCall %float %fn_hh4 %86
         %88 = OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %87
               OpStore %v %88
               OpStore %89 %88
         %90 = OpFunctionCall %float %fn_hh4 %89
         %91 = OpCompositeConstruct %v3float %90 %float_123 %float_456
         %92 = OpVectorShuffle %v4float %91 %91 1 0 0 2
               OpStore %v %92
               OpStore %93 %92
         %94 = OpFunctionCall %float %fn_hh4 %93
         %95 = OpCompositeConstruct %v3float %94 %float_123 %float_456
         %96 = OpVectorShuffle %v4float %95 %95 1 0 0 2
               OpStore %v %96
               OpStore %v %99
        %100 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %102 = OpLoad %v4float %100
        %103 = OpVectorShuffle %v3float %102 %102 0 1 2
        %104 = OpCompositeExtract %float %103 0
        %105 = OpCompositeExtract %float %103 1
        %106 = OpCompositeExtract %float %103 2
        %107 = OpCompositeConstruct %v4float %104 %105 %106 %float_1
               OpStore %v %107
        %108 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %109 = OpLoad %v4float %108
        %110 = OpCompositeExtract %float %109 0
        %111 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %112 = OpLoad %v4float %111
        %113 = OpVectorShuffle %v2float %112 %112 1 2
        %114 = OpCompositeExtract %float %113 0
        %115 = OpCompositeExtract %float %113 1
        %116 = OpCompositeConstruct %v4float %110 %float_1 %114 %115
               OpStore %v %116
        %117 = OpLoad %v4float %v
        %118 = OpVectorShuffle %v4float %117 %116 7 6 5 4
               OpStore %v %118
        %119 = OpVectorShuffle %v2float %118 %118 1 2
        %120 = OpLoad %v4float %v
        %121 = OpVectorShuffle %v4float %120 %119 4 1 2 5
               OpStore %v %121
        %122 = OpVectorShuffle %v2float %121 %121 3 3
        %123 = OpCompositeExtract %float %122 0
        %124 = OpCompositeExtract %float %122 1
        %125 = OpCompositeConstruct %v3float %123 %124 %float_1
        %126 = OpLoad %v4float %v
        %127 = OpVectorShuffle %v4float %126 %125 6 5 4 3
               OpStore %v %127
        %129 = OpFOrdEqual %v4bool %127 %128
        %131 = OpAll %bool %129
               OpSelectionMerge %135 None
               OpBranchConditional %131 %133 %134
        %133 = OpLabel
        %136 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %137 = OpLoad %v4float %136
               OpStore %132 %137
               OpBranch %135
        %134 = OpLabel
        %138 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %139 = OpLoad %v4float %138
               OpStore %132 %139
               OpBranch %135
        %135 = OpLabel
        %140 = OpLoad %v4float %132
               OpReturnValue %140
               OpFunctionEnd
