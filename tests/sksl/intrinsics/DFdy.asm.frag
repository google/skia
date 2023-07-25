               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpMemberName %_UniformBuffer 3 "u_skRTFlip"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %expected "expected"
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
               OpMemberDecorate %_UniformBuffer 3 Offset 16384
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %expected RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %29 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
    %float_1 = OpConstant %float 1
        %113 = OpConstantComposite %v2float %float_1 %float_1
        %126 = OpConstantComposite %v2float %float_0 %float_1
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %17
         %18 = OpLabel
         %21 = OpVariable %_ptr_Function_v2float Function
               OpStore %21 %20
         %23 = OpFunctionCall %v4float %main %21
               OpStore %sk_FragColor %23
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %24
         %25 = OpFunctionParameter %_ptr_Function_v2float
         %26 = OpLabel
   %expected = OpVariable %_ptr_Function_v4float Function
        %130 = OpVariable %_ptr_Function_v4float Function
               OpStore %expected %29
         %32 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %36 = OpLoad %v4float %32
         %37 = OpCompositeExtract %float %36 0
         %31 = OpDPdy %float %37
         %39 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
         %41 = OpLoad %v2float %39
         %42 = OpCompositeExtract %float %41 1
         %43 = OpFMul %float %31 %42
         %44 = OpFOrdEqual %bool %43 %float_0
               OpSelectionMerge %46 None
               OpBranchConditional %44 %45 %46
         %45 = OpLabel
         %48 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %49 = OpLoad %v4float %48
         %50 = OpVectorShuffle %v2float %49 %49 0 1
         %47 = OpDPdy %v2float %50
         %51 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
         %52 = OpLoad %v2float %51
         %53 = OpVectorShuffle %v2float %52 %52 1 1
         %54 = OpFMul %v2float %47 %53
         %55 = OpVectorShuffle %v2float %29 %29 0 1
         %56 = OpFOrdEqual %v2bool %54 %55
         %58 = OpAll %bool %56
               OpBranch %46
         %46 = OpLabel
         %59 = OpPhi %bool %false %26 %58 %45
               OpSelectionMerge %61 None
               OpBranchConditional %59 %60 %61
         %60 = OpLabel
         %63 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %64 = OpLoad %v4float %63
         %65 = OpVectorShuffle %v3float %64 %64 0 1 2
         %62 = OpDPdy %v3float %65
         %67 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
         %68 = OpLoad %v2float %67
         %69 = OpVectorShuffle %v3float %68 %68 1 1 1
         %70 = OpFMul %v3float %62 %69
         %71 = OpVectorShuffle %v3float %29 %29 0 1 2
         %72 = OpFOrdEqual %v3bool %70 %71
         %74 = OpAll %bool %72
               OpBranch %61
         %61 = OpLabel
         %75 = OpPhi %bool %false %46 %74 %60
               OpSelectionMerge %77 None
               OpBranchConditional %75 %76 %77
         %76 = OpLabel
         %79 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %80 = OpLoad %v4float %79
         %78 = OpDPdy %v4float %80
         %81 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
         %82 = OpLoad %v2float %81
         %83 = OpVectorShuffle %v4float %82 %82 1 1 1 1
         %84 = OpFMul %v4float %78 %83
         %85 = OpFOrdEqual %v4bool %84 %29
         %87 = OpAll %bool %85
               OpBranch %77
         %77 = OpLabel
         %88 = OpPhi %bool %false %61 %87 %76
               OpSelectionMerge %90 None
               OpBranchConditional %88 %89 %90
         %89 = OpLabel
         %93 = OpLoad %v2float %25
         %94 = OpVectorShuffle %v2float %93 %93 0 0
         %92 = OpDPdy %v2float %94
         %95 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
         %96 = OpLoad %v2float %95
         %97 = OpVectorShuffle %v2float %96 %96 1 1
         %98 = OpFMul %v2float %92 %97
         %91 = OpExtInst %v2float %1 FSign %98
         %99 = OpFOrdEqual %v2bool %91 %20
        %100 = OpAll %bool %99
               OpBranch %90
         %90 = OpLabel
        %101 = OpPhi %bool %false %77 %100 %89
               OpSelectionMerge %103 None
               OpBranchConditional %101 %102 %103
        %102 = OpLabel
        %106 = OpLoad %v2float %25
        %107 = OpVectorShuffle %v2float %106 %106 1 1
        %105 = OpDPdy %v2float %107
        %108 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
        %109 = OpLoad %v2float %108
        %110 = OpVectorShuffle %v2float %109 %109 1 1
        %111 = OpFMul %v2float %105 %110
        %104 = OpExtInst %v2float %1 FSign %111
        %114 = OpFOrdEqual %v2bool %104 %113
        %115 = OpAll %bool %114
               OpBranch %103
        %103 = OpLabel
        %116 = OpPhi %bool %false %90 %115 %102
               OpSelectionMerge %118 None
               OpBranchConditional %116 %117 %118
        %117 = OpLabel
        %121 = OpLoad %v2float %25
        %120 = OpDPdy %v2float %121
        %122 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
        %123 = OpLoad %v2float %122
        %124 = OpVectorShuffle %v2float %123 %123 1 1
        %125 = OpFMul %v2float %120 %124
        %119 = OpExtInst %v2float %1 FSign %125
        %127 = OpFOrdEqual %v2bool %119 %126
        %128 = OpAll %bool %127
               OpBranch %118
        %118 = OpLabel
        %129 = OpPhi %bool %false %103 %128 %117
               OpSelectionMerge %133 None
               OpBranchConditional %129 %131 %132
        %131 = OpLabel
        %134 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %136 = OpLoad %v4float %134
               OpStore %130 %136
               OpBranch %133
        %132 = OpLabel
        %137 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %139 = OpLoad %v4float %137
               OpStore %130 %139
               OpBranch %133
        %133 = OpLabel
        %140 = OpLoad %v4float %130
               OpReturnValue %140
               OpFunctionEnd
