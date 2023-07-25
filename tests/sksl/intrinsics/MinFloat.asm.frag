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
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %expectedA "expectedA"
               OpName %expectedB "expectedB"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1_25 = OpConstant %float -1.25
  %float_0_5 = OpConstant %float 0.5
         %30 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_5 %float_0_5
    %float_1 = OpConstant %float 1
         %33 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0 %float_1
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %49 = OpConstantComposite %v2float %float_0_5 %float_0_5
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %62 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
     %v3bool = OpTypeVector %bool 3
         %73 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %84 = OpConstantComposite %v2float %float_n1_25 %float_0
         %91 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0_5
      %int_1 = OpConstant %int 1
        %158 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0
      %int_2 = OpConstant %int 2
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
  %expectedA = OpVariable %_ptr_Function_v4float Function
  %expectedB = OpVariable %_ptr_Function_v4float Function
        %166 = OpVariable %_ptr_Function_v4float Function
               OpStore %expectedA %30
               OpStore %expectedB %33
         %36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %40 = OpLoad %v4float %36
         %41 = OpCompositeExtract %float %40 0
         %35 = OpExtInst %float %1 FMin %41 %float_0_5
         %42 = OpFOrdEqual %bool %35 %float_n1_25
               OpSelectionMerge %44 None
               OpBranchConditional %42 %43 %44
         %43 = OpLabel
         %46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %47 = OpLoad %v4float %46
         %48 = OpVectorShuffle %v2float %47 %47 0 1
         %45 = OpExtInst %v2float %1 FMin %48 %49
         %50 = OpVectorShuffle %v2float %30 %30 0 1
         %51 = OpFOrdEqual %v2bool %45 %50
         %53 = OpAll %bool %51
               OpBranch %44
         %44 = OpLabel
         %54 = OpPhi %bool %false %25 %53 %43
               OpSelectionMerge %56 None
               OpBranchConditional %54 %55 %56
         %55 = OpLabel
         %58 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %59 = OpLoad %v4float %58
         %60 = OpVectorShuffle %v3float %59 %59 0 1 2
         %57 = OpExtInst %v3float %1 FMin %60 %62
         %63 = OpVectorShuffle %v3float %30 %30 0 1 2
         %64 = OpFOrdEqual %v3bool %57 %63
         %66 = OpAll %bool %64
               OpBranch %56
         %56 = OpLabel
         %67 = OpPhi %bool %false %44 %66 %55
               OpSelectionMerge %69 None
               OpBranchConditional %67 %68 %69
         %68 = OpLabel
         %71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %72 = OpLoad %v4float %71
         %70 = OpExtInst %v4float %1 FMin %72 %73
         %74 = OpFOrdEqual %v4bool %70 %30
         %76 = OpAll %bool %74
               OpBranch %69
         %69 = OpLabel
         %77 = OpPhi %bool %false %56 %76 %68
               OpSelectionMerge %79 None
               OpBranchConditional %77 %78 %79
         %78 = OpLabel
               OpBranch %79
         %79 = OpLabel
         %81 = OpPhi %bool %false %69 %true %78
               OpSelectionMerge %83 None
               OpBranchConditional %81 %82 %83
         %82 = OpLabel
         %85 = OpVectorShuffle %v2float %30 %30 0 1
         %86 = OpFOrdEqual %v2bool %84 %85
         %87 = OpAll %bool %86
               OpBranch %83
         %83 = OpLabel
         %88 = OpPhi %bool %false %79 %87 %82
               OpSelectionMerge %90 None
               OpBranchConditional %88 %89 %90
         %89 = OpLabel
         %92 = OpVectorShuffle %v3float %30 %30 0 1 2
         %93 = OpFOrdEqual %v3bool %91 %92
         %94 = OpAll %bool %93
               OpBranch %90
         %90 = OpLabel
         %95 = OpPhi %bool %false %83 %94 %89
               OpSelectionMerge %97 None
               OpBranchConditional %95 %96 %97
         %96 = OpLabel
               OpBranch %97
         %97 = OpLabel
         %98 = OpPhi %bool %false %90 %true %96
               OpSelectionMerge %100 None
               OpBranchConditional %98 %99 %100
         %99 = OpLabel
        %102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %103 = OpLoad %v4float %102
        %104 = OpCompositeExtract %float %103 0
        %105 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %107 = OpLoad %v4float %105
        %108 = OpCompositeExtract %float %107 0
        %101 = OpExtInst %float %1 FMin %104 %108
        %109 = OpFOrdEqual %bool %101 %float_n1_25
               OpBranch %100
        %100 = OpLabel
        %110 = OpPhi %bool %false %97 %109 %99
               OpSelectionMerge %112 None
               OpBranchConditional %110 %111 %112
        %111 = OpLabel
        %114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %115 = OpLoad %v4float %114
        %116 = OpVectorShuffle %v2float %115 %115 0 1
        %117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %118 = OpLoad %v4float %117
        %119 = OpVectorShuffle %v2float %118 %118 0 1
        %113 = OpExtInst %v2float %1 FMin %116 %119
        %120 = OpVectorShuffle %v2float %33 %33 0 1
        %121 = OpFOrdEqual %v2bool %113 %120
        %122 = OpAll %bool %121
               OpBranch %112
        %112 = OpLabel
        %123 = OpPhi %bool %false %100 %122 %111
               OpSelectionMerge %125 None
               OpBranchConditional %123 %124 %125
        %124 = OpLabel
        %127 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %128 = OpLoad %v4float %127
        %129 = OpVectorShuffle %v3float %128 %128 0 1 2
        %130 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %131 = OpLoad %v4float %130
        %132 = OpVectorShuffle %v3float %131 %131 0 1 2
        %126 = OpExtInst %v3float %1 FMin %129 %132
        %133 = OpVectorShuffle %v3float %33 %33 0 1 2
        %134 = OpFOrdEqual %v3bool %126 %133
        %135 = OpAll %bool %134
               OpBranch %125
        %125 = OpLabel
        %136 = OpPhi %bool %false %112 %135 %124
               OpSelectionMerge %138 None
               OpBranchConditional %136 %137 %138
        %137 = OpLabel
        %140 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %141 = OpLoad %v4float %140
        %142 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %143 = OpLoad %v4float %142
        %139 = OpExtInst %v4float %1 FMin %141 %143
        %144 = OpFOrdEqual %v4bool %139 %33
        %145 = OpAll %bool %144
               OpBranch %138
        %138 = OpLabel
        %146 = OpPhi %bool %false %125 %145 %137
               OpSelectionMerge %148 None
               OpBranchConditional %146 %147 %148
        %147 = OpLabel
               OpBranch %148
        %148 = OpLabel
        %149 = OpPhi %bool %false %138 %true %147
               OpSelectionMerge %151 None
               OpBranchConditional %149 %150 %151
        %150 = OpLabel
        %152 = OpVectorShuffle %v2float %33 %33 0 1
        %153 = OpFOrdEqual %v2bool %84 %152
        %154 = OpAll %bool %153
               OpBranch %151
        %151 = OpLabel
        %155 = OpPhi %bool %false %148 %154 %150
               OpSelectionMerge %157 None
               OpBranchConditional %155 %156 %157
        %156 = OpLabel
        %159 = OpVectorShuffle %v3float %33 %33 0 1 2
        %160 = OpFOrdEqual %v3bool %158 %159
        %161 = OpAll %bool %160
               OpBranch %157
        %157 = OpLabel
        %162 = OpPhi %bool %false %151 %161 %156
               OpSelectionMerge %164 None
               OpBranchConditional %162 %163 %164
        %163 = OpLabel
               OpBranch %164
        %164 = OpLabel
        %165 = OpPhi %bool %false %157 %true %163
               OpSelectionMerge %169 None
               OpBranchConditional %165 %167 %168
        %167 = OpLabel
        %170 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %171 = OpLoad %v4float %170
               OpStore %166 %171
               OpBranch %169
        %168 = OpLabel
        %172 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %174 = OpLoad %v4float %172
               OpStore %166 %174
               OpBranch %169
        %169 = OpLabel
        %175 = OpLoad %v4float %166
               OpReturnValue %175
               OpFunctionEnd
