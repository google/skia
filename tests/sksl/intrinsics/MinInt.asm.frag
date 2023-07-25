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
               OpName %intValues "intValues"
               OpName %intGreen "intGreen"
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
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
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
        %int = OpTypeInt 32 1
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
  %float_100 = OpConstant %float 100
      %int_1 = OpConstant %int 1
   %int_n125 = OpConstant %int -125
     %int_50 = OpConstant %int 50
         %62 = OpConstantComposite %v4int %int_n125 %int_0 %int_50 %int_50
    %int_100 = OpConstant %int 100
         %65 = OpConstantComposite %v4int %int_n125 %int_0 %int_0 %int_100
      %false = OpConstantFalse %bool
      %v2int = OpTypeVector %int 2
         %75 = OpConstantComposite %v2int %int_50 %int_50
     %v2bool = OpTypeVector %bool 2
      %v3int = OpTypeVector %int 3
         %86 = OpConstantComposite %v3int %int_50 %int_50 %int_50
     %v3bool = OpTypeVector %bool 3
         %95 = OpConstantComposite %v4int %int_50 %int_50 %int_50 %int_50
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %106 = OpConstantComposite %v2int %int_n125 %int_0
        %113 = OpConstantComposite %v3int %int_n125 %int_0 %int_50
        %162 = OpConstantComposite %v3int %int_n125 %int_0 %int_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
  %intValues = OpVariable %_ptr_Function_v4int Function
   %intGreen = OpVariable %_ptr_Function_v4int Function
  %expectedA = OpVariable %_ptr_Function_v4int Function
  %expectedB = OpVariable %_ptr_Function_v4int Function
        %170 = OpVariable %_ptr_Function_v4float Function
         %30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %33 = OpLoad %v4float %30
         %35 = OpVectorTimesScalar %v4float %33 %float_100
         %36 = OpCompositeExtract %float %35 0
         %37 = OpConvertFToS %int %36
         %38 = OpCompositeExtract %float %35 1
         %39 = OpConvertFToS %int %38
         %40 = OpCompositeExtract %float %35 2
         %41 = OpConvertFToS %int %40
         %42 = OpCompositeExtract %float %35 3
         %43 = OpConvertFToS %int %42
         %44 = OpCompositeConstruct %v4int %37 %39 %41 %43
               OpStore %intValues %44
         %46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %48 = OpLoad %v4float %46
         %49 = OpVectorTimesScalar %v4float %48 %float_100
         %50 = OpCompositeExtract %float %49 0
         %51 = OpConvertFToS %int %50
         %52 = OpCompositeExtract %float %49 1
         %53 = OpConvertFToS %int %52
         %54 = OpCompositeExtract %float %49 2
         %55 = OpConvertFToS %int %54
         %56 = OpCompositeExtract %float %49 3
         %57 = OpConvertFToS %int %56
         %58 = OpCompositeConstruct %v4int %51 %53 %55 %57
               OpStore %intGreen %58
               OpStore %expectedA %62
               OpStore %expectedB %65
         %68 = OpCompositeExtract %int %44 0
         %67 = OpExtInst %int %1 SMin %68 %int_50
         %69 = OpIEqual %bool %67 %int_n125
               OpSelectionMerge %71 None
               OpBranchConditional %69 %70 %71
         %70 = OpLabel
         %73 = OpVectorShuffle %v2int %44 %44 0 1
         %72 = OpExtInst %v2int %1 SMin %73 %75
         %76 = OpVectorShuffle %v2int %62 %62 0 1
         %77 = OpIEqual %v2bool %72 %76
         %79 = OpAll %bool %77
               OpBranch %71
         %71 = OpLabel
         %80 = OpPhi %bool %false %25 %79 %70
               OpSelectionMerge %82 None
               OpBranchConditional %80 %81 %82
         %81 = OpLabel
         %84 = OpVectorShuffle %v3int %44 %44 0 1 2
         %83 = OpExtInst %v3int %1 SMin %84 %86
         %87 = OpVectorShuffle %v3int %62 %62 0 1 2
         %88 = OpIEqual %v3bool %83 %87
         %90 = OpAll %bool %88
               OpBranch %82
         %82 = OpLabel
         %91 = OpPhi %bool %false %71 %90 %81
               OpSelectionMerge %93 None
               OpBranchConditional %91 %92 %93
         %92 = OpLabel
         %94 = OpExtInst %v4int %1 SMin %44 %95
         %96 = OpIEqual %v4bool %94 %62
         %98 = OpAll %bool %96
               OpBranch %93
         %93 = OpLabel
         %99 = OpPhi %bool %false %82 %98 %92
               OpSelectionMerge %101 None
               OpBranchConditional %99 %100 %101
        %100 = OpLabel
               OpBranch %101
        %101 = OpLabel
        %103 = OpPhi %bool %false %93 %true %100
               OpSelectionMerge %105 None
               OpBranchConditional %103 %104 %105
        %104 = OpLabel
        %107 = OpVectorShuffle %v2int %62 %62 0 1
        %108 = OpIEqual %v2bool %106 %107
        %109 = OpAll %bool %108
               OpBranch %105
        %105 = OpLabel
        %110 = OpPhi %bool %false %101 %109 %104
               OpSelectionMerge %112 None
               OpBranchConditional %110 %111 %112
        %111 = OpLabel
        %114 = OpVectorShuffle %v3int %62 %62 0 1 2
        %115 = OpIEqual %v3bool %113 %114
        %116 = OpAll %bool %115
               OpBranch %112
        %112 = OpLabel
        %117 = OpPhi %bool %false %105 %116 %111
               OpSelectionMerge %119 None
               OpBranchConditional %117 %118 %119
        %118 = OpLabel
               OpBranch %119
        %119 = OpLabel
        %120 = OpPhi %bool %false %112 %true %118
               OpSelectionMerge %122 None
               OpBranchConditional %120 %121 %122
        %121 = OpLabel
        %124 = OpCompositeExtract %int %58 0
        %123 = OpExtInst %int %1 SMin %68 %124
        %125 = OpIEqual %bool %123 %int_n125
               OpBranch %122
        %122 = OpLabel
        %126 = OpPhi %bool %false %119 %125 %121
               OpSelectionMerge %128 None
               OpBranchConditional %126 %127 %128
        %127 = OpLabel
        %130 = OpVectorShuffle %v2int %44 %44 0 1
        %131 = OpVectorShuffle %v2int %58 %58 0 1
        %129 = OpExtInst %v2int %1 SMin %130 %131
        %132 = OpVectorShuffle %v2int %65 %65 0 1
        %133 = OpIEqual %v2bool %129 %132
        %134 = OpAll %bool %133
               OpBranch %128
        %128 = OpLabel
        %135 = OpPhi %bool %false %122 %134 %127
               OpSelectionMerge %137 None
               OpBranchConditional %135 %136 %137
        %136 = OpLabel
        %139 = OpVectorShuffle %v3int %44 %44 0 1 2
        %140 = OpVectorShuffle %v3int %58 %58 0 1 2
        %138 = OpExtInst %v3int %1 SMin %139 %140
        %141 = OpVectorShuffle %v3int %65 %65 0 1 2
        %142 = OpIEqual %v3bool %138 %141
        %143 = OpAll %bool %142
               OpBranch %137
        %137 = OpLabel
        %144 = OpPhi %bool %false %128 %143 %136
               OpSelectionMerge %146 None
               OpBranchConditional %144 %145 %146
        %145 = OpLabel
        %147 = OpExtInst %v4int %1 SMin %44 %58
        %148 = OpIEqual %v4bool %147 %65
        %149 = OpAll %bool %148
               OpBranch %146
        %146 = OpLabel
        %150 = OpPhi %bool %false %137 %149 %145
               OpSelectionMerge %152 None
               OpBranchConditional %150 %151 %152
        %151 = OpLabel
               OpBranch %152
        %152 = OpLabel
        %153 = OpPhi %bool %false %146 %true %151
               OpSelectionMerge %155 None
               OpBranchConditional %153 %154 %155
        %154 = OpLabel
        %156 = OpVectorShuffle %v2int %65 %65 0 1
        %157 = OpIEqual %v2bool %106 %156
        %158 = OpAll %bool %157
               OpBranch %155
        %155 = OpLabel
        %159 = OpPhi %bool %false %152 %158 %154
               OpSelectionMerge %161 None
               OpBranchConditional %159 %160 %161
        %160 = OpLabel
        %163 = OpVectorShuffle %v3int %65 %65 0 1 2
        %164 = OpIEqual %v3bool %162 %163
        %165 = OpAll %bool %164
               OpBranch %161
        %161 = OpLabel
        %166 = OpPhi %bool %false %155 %165 %160
               OpSelectionMerge %168 None
               OpBranchConditional %166 %167 %168
        %167 = OpLabel
               OpBranch %168
        %168 = OpLabel
        %169 = OpPhi %bool %false %161 %true %167
               OpSelectionMerge %174 None
               OpBranchConditional %169 %172 %173
        %172 = OpLabel
        %175 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %176 = OpLoad %v4float %175
               OpStore %170 %176
               OpBranch %174
        %173 = OpLabel
        %177 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %179 = OpLoad %v4float %177
               OpStore %170 %179
               OpBranch %174
        %174 = OpLabel
        %180 = OpLoad %v4float %170
               OpReturnValue %180
               OpFunctionEnd
