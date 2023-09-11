               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
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
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %v RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
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
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
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
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %25 = OpTypeFunction %float %_ptr_Function_v4float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
         %44 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
    %v3float = OpTypeVector %float 3
    %float_1 = OpConstant %float 1
  %float_123 = OpConstant %float 123
  %float_456 = OpConstant %float 456
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
        %101 = OpConstantComposite %v4float %float_1 %float_1 %float_2 %float_3
      %int_0 = OpConstant %int 0
        %130 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
     %v4bool = OpTypeVector %bool 4
%_entrypoint_v = OpFunction %void None %16
         %17 = OpLabel
         %21 = OpVariable %_ptr_Function_v2float Function
               OpStore %21 %20
         %23 = OpFunctionCall %v4float %main %21
               OpStore %sk_FragColor %23
               OpReturn
               OpFunctionEnd
     %fn_hh4 = OpFunction %float None %25
         %26 = OpFunctionParameter %_ptr_Function_v4float
         %27 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
               OpStore %x %int_1
               OpBranch %32
         %32 = OpLabel
               OpLoopMerge %36 %35 None
               OpBranch %33
         %33 = OpLabel
         %37 = OpLoad %int %x
         %39 = OpSLessThanEqual %bool %37 %int_2
               OpBranchConditional %39 %34 %36
         %34 = OpLabel
         %40 = OpLoad %v4float %26
         %41 = OpCompositeExtract %float %40 0
               OpReturnValue %41
         %35 = OpLabel
         %42 = OpLoad %int %x
         %43 = OpIAdd %int %42 %int_1
               OpStore %x %43
               OpBranch %32
         %36 = OpLabel
               OpUnreachable
               OpFunctionEnd
       %main = OpFunction %v4float None %44
         %45 = OpFunctionParameter %_ptr_Function_v2float
         %46 = OpLabel
          %v = OpVariable %_ptr_Function_v4float Function
         %75 = OpVariable %_ptr_Function_v4float Function
         %81 = OpVariable %_ptr_Function_v4float Function
         %85 = OpVariable %_ptr_Function_v4float Function
         %88 = OpVariable %_ptr_Function_v4float Function
         %91 = OpVariable %_ptr_Function_v4float Function
         %95 = OpVariable %_ptr_Function_v4float Function
        %134 = OpVariable %_ptr_Function_v4float Function
         %48 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %50 = OpLoad %v4float %48
               OpStore %v %50
         %51 = OpVectorShuffle %v3float %50 %50 2 1 0
         %53 = OpCompositeExtract %float %51 0
         %54 = OpCompositeExtract %float %51 1
         %55 = OpCompositeExtract %float %51 2
         %56 = OpCompositeConstruct %v4float %float_0 %53 %54 %55
               OpStore %v %56
         %57 = OpVectorShuffle %v2float %56 %56 0 3
         %58 = OpCompositeExtract %float %57 0
         %59 = OpCompositeExtract %float %57 1
         %60 = OpCompositeConstruct %v4float %float_0 %float_0 %58 %59
               OpStore %v %60
         %62 = OpVectorShuffle %v2float %60 %60 3 0
         %63 = OpCompositeExtract %float %62 0
         %64 = OpCompositeExtract %float %62 1
         %65 = OpCompositeConstruct %v4float %float_1 %float_1 %63 %64
               OpStore %v %65
         %66 = OpVectorShuffle %v2float %65 %65 2 1
         %67 = OpCompositeExtract %float %66 0
         %68 = OpCompositeExtract %float %66 1
         %69 = OpCompositeConstruct %v4float %67 %68 %float_1 %float_1
               OpStore %v %69
         %70 = OpVectorShuffle %v2float %69 %69 0 0
         %71 = OpCompositeExtract %float %70 0
         %72 = OpCompositeExtract %float %70 1
         %73 = OpCompositeConstruct %v4float %71 %72 %float_1 %float_1
               OpStore %v %73
         %74 = OpVectorShuffle %v4float %73 %73 3 2 3 2
               OpStore %v %74
               OpStore %75 %74
         %76 = OpFunctionCall %float %fn_hh4 %75
         %79 = OpCompositeConstruct %v3float %76 %float_123 %float_456
         %80 = OpVectorShuffle %v4float %79 %79 1 1 2 2
               OpStore %v %80
               OpStore %81 %80
         %82 = OpFunctionCall %float %fn_hh4 %81
         %83 = OpCompositeConstruct %v3float %82 %float_123 %float_456
         %84 = OpVectorShuffle %v4float %83 %83 1 1 2 2
               OpStore %v %84
               OpStore %85 %84
         %86 = OpFunctionCall %float %fn_hh4 %85
         %87 = OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %86
               OpStore %v %87
               OpStore %88 %87
         %89 = OpFunctionCall %float %fn_hh4 %88
         %90 = OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %89
               OpStore %v %90
               OpStore %91 %90
         %92 = OpFunctionCall %float %fn_hh4 %91
         %93 = OpCompositeConstruct %v3float %92 %float_123 %float_456
         %94 = OpVectorShuffle %v4float %93 %93 1 0 0 2
               OpStore %v %94
               OpStore %95 %94
         %96 = OpFunctionCall %float %fn_hh4 %95
         %97 = OpCompositeConstruct %v3float %96 %float_123 %float_456
         %98 = OpVectorShuffle %v4float %97 %97 1 0 0 2
               OpStore %v %98
               OpStore %v %101
        %102 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %104 = OpLoad %v4float %102
        %105 = OpVectorShuffle %v3float %104 %104 0 1 2
        %106 = OpCompositeExtract %float %105 0
        %107 = OpCompositeExtract %float %105 1
        %108 = OpCompositeExtract %float %105 2
        %109 = OpCompositeConstruct %v4float %106 %107 %108 %float_1
               OpStore %v %109
        %110 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %111 = OpLoad %v4float %110
        %112 = OpCompositeExtract %float %111 0
        %113 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %114 = OpLoad %v4float %113
        %115 = OpVectorShuffle %v2float %114 %114 1 2
        %116 = OpCompositeExtract %float %115 0
        %117 = OpCompositeExtract %float %115 1
        %118 = OpCompositeConstruct %v4float %112 %float_1 %116 %117
               OpStore %v %118
        %119 = OpLoad %v4float %v
        %120 = OpVectorShuffle %v4float %119 %118 7 6 5 4
               OpStore %v %120
        %121 = OpVectorShuffle %v2float %120 %120 1 2
        %122 = OpLoad %v4float %v
        %123 = OpVectorShuffle %v4float %122 %121 4 1 2 5
               OpStore %v %123
        %124 = OpVectorShuffle %v2float %123 %123 3 3
        %125 = OpCompositeExtract %float %124 0
        %126 = OpCompositeExtract %float %124 1
        %127 = OpCompositeConstruct %v3float %125 %126 %float_1
        %128 = OpLoad %v4float %v
        %129 = OpVectorShuffle %v4float %128 %127 6 5 4 3
               OpStore %v %129
        %131 = OpFOrdEqual %v4bool %129 %130
        %133 = OpAll %bool %131
               OpSelectionMerge %137 None
               OpBranchConditional %133 %135 %136
        %135 = OpLabel
        %138 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %139 = OpLoad %v4float %138
               OpStore %134 %139
               OpBranch %137
        %136 = OpLabel
        %140 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %141 = OpLoad %v4float %140
               OpStore %134 %141
               OpBranch %137
        %137 = OpLabel
        %142 = OpLoad %v4float %134
               OpReturnValue %142
               OpFunctionEnd
