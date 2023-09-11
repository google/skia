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
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %expectedA RelaxedPrecision
               OpDecorate %expectedB RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
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
  %float_0_5 = OpConstant %float 0.5
 %float_0_75 = OpConstant %float 0.75
 %float_2_25 = OpConstant %float 2.25
         %31 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_75 %float_2_25
    %float_1 = OpConstant %float 1
         %34 = OpConstantComposite %v4float %float_0 %float_1 %float_0_75 %float_2_25
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %50 = OpConstantComposite %v2float %float_0_5 %float_0_5
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %63 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
     %v3bool = OpTypeVector %bool 3
         %74 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %91 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_75
      %int_1 = OpConstant %int 1
        %152 = OpConstantComposite %v2float %float_0 %float_1
        %159 = OpConstantComposite %v3float %float_0 %float_1 %float_0_75
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
        %167 = OpVariable %_ptr_Function_v4float Function
               OpStore %expectedA %31
               OpStore %expectedB %34
         %37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %41 = OpLoad %v4float %37
         %42 = OpCompositeExtract %float %41 0
         %36 = OpExtInst %float %1 FMax %42 %float_0_5
         %43 = OpFOrdEqual %bool %36 %float_0_5
               OpSelectionMerge %45 None
               OpBranchConditional %43 %44 %45
         %44 = OpLabel
         %47 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %48 = OpLoad %v4float %47
         %49 = OpVectorShuffle %v2float %48 %48 0 1
         %46 = OpExtInst %v2float %1 FMax %49 %50
         %51 = OpVectorShuffle %v2float %31 %31 0 1
         %52 = OpFOrdEqual %v2bool %46 %51
         %54 = OpAll %bool %52
               OpBranch %45
         %45 = OpLabel
         %55 = OpPhi %bool %false %25 %54 %44
               OpSelectionMerge %57 None
               OpBranchConditional %55 %56 %57
         %56 = OpLabel
         %59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %60 = OpLoad %v4float %59
         %61 = OpVectorShuffle %v3float %60 %60 0 1 2
         %58 = OpExtInst %v3float %1 FMax %61 %63
         %64 = OpVectorShuffle %v3float %31 %31 0 1 2
         %65 = OpFOrdEqual %v3bool %58 %64
         %67 = OpAll %bool %65
               OpBranch %57
         %57 = OpLabel
         %68 = OpPhi %bool %false %45 %67 %56
               OpSelectionMerge %70 None
               OpBranchConditional %68 %69 %70
         %69 = OpLabel
         %72 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %73 = OpLoad %v4float %72
         %71 = OpExtInst %v4float %1 FMax %73 %74
         %75 = OpFOrdEqual %v4bool %71 %31
         %77 = OpAll %bool %75
               OpBranch %70
         %70 = OpLabel
         %78 = OpPhi %bool %false %57 %77 %69
               OpSelectionMerge %80 None
               OpBranchConditional %78 %79 %80
         %79 = OpLabel
               OpBranch %80
         %80 = OpLabel
         %82 = OpPhi %bool %false %70 %true %79
               OpSelectionMerge %84 None
               OpBranchConditional %82 %83 %84
         %83 = OpLabel
         %85 = OpVectorShuffle %v2float %31 %31 0 1
         %86 = OpFOrdEqual %v2bool %50 %85
         %87 = OpAll %bool %86
               OpBranch %84
         %84 = OpLabel
         %88 = OpPhi %bool %false %80 %87 %83
               OpSelectionMerge %90 None
               OpBranchConditional %88 %89 %90
         %89 = OpLabel
         %92 = OpVectorShuffle %v3float %31 %31 0 1 2
         %93 = OpFOrdEqual %v3bool %91 %92
         %94 = OpAll %bool %93
               OpBranch %90
         %90 = OpLabel
         %95 = OpPhi %bool %false %84 %94 %89
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
        %101 = OpExtInst %float %1 FMax %104 %108
        %109 = OpFOrdEqual %bool %101 %float_0
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
        %113 = OpExtInst %v2float %1 FMax %116 %119
        %120 = OpVectorShuffle %v2float %34 %34 0 1
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
        %126 = OpExtInst %v3float %1 FMax %129 %132
        %133 = OpVectorShuffle %v3float %34 %34 0 1 2
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
        %139 = OpExtInst %v4float %1 FMax %141 %143
        %144 = OpFOrdEqual %v4bool %139 %34
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
        %153 = OpVectorShuffle %v2float %34 %34 0 1
        %154 = OpFOrdEqual %v2bool %152 %153
        %155 = OpAll %bool %154
               OpBranch %151
        %151 = OpLabel
        %156 = OpPhi %bool %false %148 %155 %150
               OpSelectionMerge %158 None
               OpBranchConditional %156 %157 %158
        %157 = OpLabel
        %160 = OpVectorShuffle %v3float %34 %34 0 1 2
        %161 = OpFOrdEqual %v3bool %159 %160
        %162 = OpAll %bool %161
               OpBranch %158
        %158 = OpLabel
        %163 = OpPhi %bool %false %151 %162 %157
               OpSelectionMerge %165 None
               OpBranchConditional %163 %164 %165
        %164 = OpLabel
               OpBranch %165
        %165 = OpLabel
        %166 = OpPhi %bool %false %158 %true %164
               OpSelectionMerge %170 None
               OpBranchConditional %166 %168 %169
        %168 = OpLabel
        %171 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %172 = OpLoad %v4float %171
               OpStore %167 %172
               OpBranch %170
        %169 = OpLabel
        %173 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %175 = OpLoad %v4float %173
               OpStore %167 %175
               OpBranch %170
        %170 = OpLabel
        %176 = OpLoad %v4float %167
               OpReturnValue %176
               OpFunctionEnd
