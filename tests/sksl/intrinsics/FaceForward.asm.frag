               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "N"
               OpMemberName %_UniformBuffer 1 "I"
               OpMemberName %_UniformBuffer 2 "NRef"
               OpMemberName %_UniformBuffer 3 "colorGreen"
               OpMemberName %_UniformBuffer 4 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %huge "huge"
               OpName %huge2 "huge2"
               OpName %huge3 "huge3"
               OpName %huge4 "huge4"
               OpName %expectedPos "expectedPos"
               OpName %expectedNeg "expectedNeg"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 3 Offset 48
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 4 Offset 64
               OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %25 RelaxedPrecision
               OpDecorate %expectedPos RelaxedPrecision
               OpDecorate %expectedNeg RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
%float_1_00000002e_30 = OpConstant %float 1.00000002e+30
         %30 = OpConstantComposite %v2float %float_1 %float_1
         %31 = OpConstantComposite %v2float %float_1_00000002e_30 %float_1_00000002e_30
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %36 = OpConstantComposite %v3float %float_1 %float_1 %float_1
         %37 = OpConstantComposite %v3float %float_1_00000002e_30 %float_1_00000002e_30 %float_1_00000002e_30
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %41 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
         %42 = OpConstantComposite %v4float %float_1_00000002e_30 %float_1_00000002e_30 %float_1_00000002e_30 %float_1_00000002e_30
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %54 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
   %float_n1 = OpConstant %float -1
   %float_n2 = OpConstant %float -2
   %float_n3 = OpConstant %float -3
   %float_n4 = OpConstant %float -4
         %59 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n3 %float_n4
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
     %v2bool = OpTypeVector %bool 2
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %131 = OpConstantComposite %v2float %float_n1 %float_n2
        %138 = OpConstantComposite %v3float %float_1 %float_2 %float_3
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
       %huge = OpVariable %_ptr_Function_float Function
      %huge2 = OpVariable %_ptr_Function_v2float Function
      %huge3 = OpVariable %_ptr_Function_v3float Function
      %huge4 = OpVariable %_ptr_Function_v4float Function
%expectedPos = OpVariable %_ptr_Function_v4float Function
%expectedNeg = OpVariable %_ptr_Function_v4float Function
        %146 = OpVariable %_ptr_Function_v4float Function
         %25 = OpExtInst %float %1 FaceForward %float_1 %float_1_00000002e_30 %float_1_00000002e_30
               OpStore %huge %25
         %29 = OpExtInst %v2float %1 FaceForward %30 %31 %31
               OpStore %huge2 %29
         %35 = OpExtInst %v3float %1 FaceForward %36 %37 %37
               OpStore %huge3 %35
         %40 = OpExtInst %v4float %1 FaceForward %41 %42 %42
               OpStore %huge4 %40
         %44 = OpCompositeConstruct %v4float %25 %25 %25 %25
         %45 = OpVectorShuffle %v4float %29 %29 0 0 0 0
         %46 = OpFAdd %v4float %44 %45
               OpStore %expectedPos %46
         %48 = OpVectorShuffle %v4float %35 %35 0 0 0 0
         %49 = OpVectorShuffle %v4float %40 %40 0 0 0 0
         %50 = OpFAdd %v4float %48 %49
               OpStore %expectedNeg %50
               OpStore %expectedPos %54
               OpStore %expectedNeg %59
         %63 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %67 = OpLoad %v4float %63
         %68 = OpCompositeExtract %float %67 0
         %69 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %71 = OpLoad %v4float %69
         %72 = OpCompositeExtract %float %71 0
         %73 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %75 = OpLoad %v4float %73
         %76 = OpCompositeExtract %float %75 0
         %62 = OpExtInst %float %1 FaceForward %68 %72 %76
         %77 = OpFOrdEqual %bool %62 %float_n1
               OpSelectionMerge %79 None
               OpBranchConditional %77 %78 %79
         %78 = OpLabel
         %81 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %82 = OpLoad %v4float %81
         %83 = OpVectorShuffle %v2float %82 %82 0 1
         %84 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %85 = OpLoad %v4float %84
         %86 = OpVectorShuffle %v2float %85 %85 0 1
         %87 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %88 = OpLoad %v4float %87
         %89 = OpVectorShuffle %v2float %88 %88 0 1
         %80 = OpExtInst %v2float %1 FaceForward %83 %86 %89
         %90 = OpVectorShuffle %v2float %59 %59 0 1
         %91 = OpFOrdEqual %v2bool %80 %90
         %93 = OpAll %bool %91
               OpBranch %79
         %79 = OpLabel
         %94 = OpPhi %bool %false %22 %93 %78
               OpSelectionMerge %96 None
               OpBranchConditional %94 %95 %96
         %95 = OpLabel
         %98 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %99 = OpLoad %v4float %98
        %100 = OpVectorShuffle %v3float %99 %99 0 1 2
        %101 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %102 = OpLoad %v4float %101
        %103 = OpVectorShuffle %v3float %102 %102 0 1 2
        %104 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %105 = OpLoad %v4float %104
        %106 = OpVectorShuffle %v3float %105 %105 0 1 2
         %97 = OpExtInst %v3float %1 FaceForward %100 %103 %106
        %107 = OpVectorShuffle %v3float %54 %54 0 1 2
        %108 = OpFOrdEqual %v3bool %97 %107
        %110 = OpAll %bool %108
               OpBranch %96
         %96 = OpLabel
        %111 = OpPhi %bool %false %79 %110 %95
               OpSelectionMerge %113 None
               OpBranchConditional %111 %112 %113
        %112 = OpLabel
        %115 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %116 = OpLoad %v4float %115
        %117 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %118 = OpLoad %v4float %117
        %119 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %120 = OpLoad %v4float %119
        %114 = OpExtInst %v4float %1 FaceForward %116 %118 %120
        %121 = OpFOrdEqual %v4bool %114 %54
        %123 = OpAll %bool %121
               OpBranch %113
        %113 = OpLabel
        %124 = OpPhi %bool %false %96 %123 %112
               OpSelectionMerge %126 None
               OpBranchConditional %124 %125 %126
        %125 = OpLabel
               OpBranch %126
        %126 = OpLabel
        %128 = OpPhi %bool %false %113 %true %125
               OpSelectionMerge %130 None
               OpBranchConditional %128 %129 %130
        %129 = OpLabel
        %132 = OpVectorShuffle %v2float %59 %59 0 1
        %133 = OpFOrdEqual %v2bool %131 %132
        %134 = OpAll %bool %133
               OpBranch %130
        %130 = OpLabel
        %135 = OpPhi %bool %false %126 %134 %129
               OpSelectionMerge %137 None
               OpBranchConditional %135 %136 %137
        %136 = OpLabel
        %139 = OpVectorShuffle %v3float %54 %54 0 1 2
        %140 = OpFOrdEqual %v3bool %138 %139
        %141 = OpAll %bool %140
               OpBranch %137
        %137 = OpLabel
        %142 = OpPhi %bool %false %130 %141 %136
               OpSelectionMerge %144 None
               OpBranchConditional %142 %143 %144
        %143 = OpLabel
               OpBranch %144
        %144 = OpLabel
        %145 = OpPhi %bool %false %137 %true %143
               OpSelectionMerge %149 None
               OpBranchConditional %145 %147 %148
        %147 = OpLabel
        %150 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %152 = OpLoad %v4float %150
               OpStore %146 %152
               OpBranch %149
        %148 = OpLabel
        %153 = OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %155 = OpLoad %v4float %153
               OpStore %146 %155
               OpBranch %149
        %149 = OpLabel
        %156 = OpLoad %v4float %146
               OpReturnValue %156
               OpFunctionEnd
