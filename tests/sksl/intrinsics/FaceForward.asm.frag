               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
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
               OpMemberDecorate %_UniformBuffer 3 Offset 48
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 4 Offset 64
               OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %28 RelaxedPrecision
               OpDecorate %expectedPos RelaxedPrecision
               OpDecorate %expectedNeg RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
%float_1_00000002e_30 = OpConstant %float 1.00000002e+30
         %33 = OpConstantComposite %v2float %float_1 %float_1
         %34 = OpConstantComposite %v2float %float_1_00000002e_30 %float_1_00000002e_30
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %39 = OpConstantComposite %v3float %float_1 %float_1 %float_1
         %40 = OpConstantComposite %v3float %float_1_00000002e_30 %float_1_00000002e_30 %float_1_00000002e_30
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %44 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
         %45 = OpConstantComposite %v4float %float_1_00000002e_30 %float_1_00000002e_30 %float_1_00000002e_30 %float_1_00000002e_30
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %57 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
   %float_n1 = OpConstant %float -1
   %float_n2 = OpConstant %float -2
   %float_n3 = OpConstant %float -3
   %float_n4 = OpConstant %float -4
         %62 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n3 %float_n4
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
        %133 = OpConstantComposite %v2float %float_n1 %float_n2
        %140 = OpConstantComposite %v3float %float_1 %float_2 %float_3
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
       %huge = OpVariable %_ptr_Function_float Function
      %huge2 = OpVariable %_ptr_Function_v2float Function
      %huge3 = OpVariable %_ptr_Function_v3float Function
      %huge4 = OpVariable %_ptr_Function_v4float Function
%expectedPos = OpVariable %_ptr_Function_v4float Function
%expectedNeg = OpVariable %_ptr_Function_v4float Function
        %148 = OpVariable %_ptr_Function_v4float Function
         %28 = OpExtInst %float %1 FaceForward %float_1 %float_1_00000002e_30 %float_1_00000002e_30
               OpStore %huge %28
         %32 = OpExtInst %v2float %1 FaceForward %33 %34 %34
               OpStore %huge2 %32
         %38 = OpExtInst %v3float %1 FaceForward %39 %40 %40
               OpStore %huge3 %38
         %43 = OpExtInst %v4float %1 FaceForward %44 %45 %45
               OpStore %huge4 %43
         %47 = OpCompositeConstruct %v4float %28 %28 %28 %28
         %48 = OpVectorShuffle %v4float %32 %32 0 0 0 0
         %49 = OpFAdd %v4float %47 %48
               OpStore %expectedPos %49
         %51 = OpVectorShuffle %v4float %38 %38 0 0 0 0
         %52 = OpVectorShuffle %v4float %43 %43 0 0 0 0
         %53 = OpFAdd %v4float %51 %52
               OpStore %expectedNeg %53
               OpStore %expectedPos %57
               OpStore %expectedNeg %62
         %65 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %69 = OpLoad %v4float %65
         %70 = OpCompositeExtract %float %69 0
         %71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %73 = OpLoad %v4float %71
         %74 = OpCompositeExtract %float %73 0
         %75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %77 = OpLoad %v4float %75
         %78 = OpCompositeExtract %float %77 0
         %64 = OpExtInst %float %1 FaceForward %70 %74 %78
         %79 = OpFOrdEqual %bool %64 %float_n1
               OpSelectionMerge %81 None
               OpBranchConditional %79 %80 %81
         %80 = OpLabel
         %83 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %84 = OpLoad %v4float %83
         %85 = OpVectorShuffle %v2float %84 %84 0 1
         %86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %87 = OpLoad %v4float %86
         %88 = OpVectorShuffle %v2float %87 %87 0 1
         %89 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %90 = OpLoad %v4float %89
         %91 = OpVectorShuffle %v2float %90 %90 0 1
         %82 = OpExtInst %v2float %1 FaceForward %85 %88 %91
         %92 = OpVectorShuffle %v2float %62 %62 0 1
         %93 = OpFOrdEqual %v2bool %82 %92
         %95 = OpAll %bool %93
               OpBranch %81
         %81 = OpLabel
         %96 = OpPhi %bool %false %25 %95 %80
               OpSelectionMerge %98 None
               OpBranchConditional %96 %97 %98
         %97 = OpLabel
        %100 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %101 = OpLoad %v4float %100
        %102 = OpVectorShuffle %v3float %101 %101 0 1 2
        %103 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %104 = OpLoad %v4float %103
        %105 = OpVectorShuffle %v3float %104 %104 0 1 2
        %106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %107 = OpLoad %v4float %106
        %108 = OpVectorShuffle %v3float %107 %107 0 1 2
         %99 = OpExtInst %v3float %1 FaceForward %102 %105 %108
        %109 = OpVectorShuffle %v3float %57 %57 0 1 2
        %110 = OpFOrdEqual %v3bool %99 %109
        %112 = OpAll %bool %110
               OpBranch %98
         %98 = OpLabel
        %113 = OpPhi %bool %false %81 %112 %97
               OpSelectionMerge %115 None
               OpBranchConditional %113 %114 %115
        %114 = OpLabel
        %117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %118 = OpLoad %v4float %117
        %119 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %120 = OpLoad %v4float %119
        %121 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %122 = OpLoad %v4float %121
        %116 = OpExtInst %v4float %1 FaceForward %118 %120 %122
        %123 = OpFOrdEqual %v4bool %116 %57
        %125 = OpAll %bool %123
               OpBranch %115
        %115 = OpLabel
        %126 = OpPhi %bool %false %98 %125 %114
               OpSelectionMerge %128 None
               OpBranchConditional %126 %127 %128
        %127 = OpLabel
               OpBranch %128
        %128 = OpLabel
        %130 = OpPhi %bool %false %115 %true %127
               OpSelectionMerge %132 None
               OpBranchConditional %130 %131 %132
        %131 = OpLabel
        %134 = OpVectorShuffle %v2float %62 %62 0 1
        %135 = OpFOrdEqual %v2bool %133 %134
        %136 = OpAll %bool %135
               OpBranch %132
        %132 = OpLabel
        %137 = OpPhi %bool %false %128 %136 %131
               OpSelectionMerge %139 None
               OpBranchConditional %137 %138 %139
        %138 = OpLabel
        %141 = OpVectorShuffle %v3float %57 %57 0 1 2
        %142 = OpFOrdEqual %v3bool %140 %141
        %143 = OpAll %bool %142
               OpBranch %139
        %139 = OpLabel
        %144 = OpPhi %bool %false %132 %143 %138
               OpSelectionMerge %146 None
               OpBranchConditional %144 %145 %146
        %145 = OpLabel
               OpBranch %146
        %146 = OpLabel
        %147 = OpPhi %bool %false %139 %true %145
               OpSelectionMerge %151 None
               OpBranchConditional %147 %149 %150
        %149 = OpLabel
        %152 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %154 = OpLoad %v4float %152
               OpStore %148 %154
               OpBranch %151
        %150 = OpLabel
        %155 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %157 = OpLoad %v4float %155
               OpStore %148 %157
               OpBranch %151
        %151 = OpLabel
        %158 = OpLoad %v4float %148
               OpReturnValue %158
               OpFunctionEnd
