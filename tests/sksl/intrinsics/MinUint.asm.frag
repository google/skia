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
               OpName %uintValues "uintValues"
               OpName %uintGreen "uintGreen"
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
               OpDecorate %30 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
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
       %uint = OpTypeInt 32 0
     %v4uint = OpTypeVector %uint 4
%_ptr_Function_v4uint = OpTypePointer Function %v4uint
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
  %float_100 = OpConstant %float 100
      %int_1 = OpConstant %int 1
    %uint_50 = OpConstant %uint 50
     %uint_0 = OpConstant %uint 0
         %64 = OpConstantComposite %v4uint %uint_50 %uint_0 %uint_50 %uint_50
   %uint_100 = OpConstant %uint 100
         %67 = OpConstantComposite %v4uint %uint_0 %uint_0 %uint_0 %uint_100
      %false = OpConstantFalse %bool
     %v2uint = OpTypeVector %uint 2
         %77 = OpConstantComposite %v2uint %uint_50 %uint_50
     %v2bool = OpTypeVector %bool 2
     %v3uint = OpTypeVector %uint 3
         %88 = OpConstantComposite %v3uint %uint_50 %uint_50 %uint_50
     %v3bool = OpTypeVector %bool 3
         %97 = OpConstantComposite %v4uint %uint_50 %uint_50 %uint_50 %uint_50
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %108 = OpConstantComposite %v2uint %uint_50 %uint_0
        %115 = OpConstantComposite %v3uint %uint_50 %uint_0 %uint_50
        %158 = OpConstantComposite %v2uint %uint_0 %uint_0
        %165 = OpConstantComposite %v3uint %uint_0 %uint_0 %uint_0
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
 %uintValues = OpVariable %_ptr_Function_v4uint Function
  %uintGreen = OpVariable %_ptr_Function_v4uint Function
  %expectedA = OpVariable %_ptr_Function_v4uint Function
  %expectedB = OpVariable %_ptr_Function_v4uint Function
        %173 = OpVariable %_ptr_Function_v4float Function
         %31 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %35 = OpLoad %v4float %31
         %30 = OpExtInst %v4float %1 FAbs %35
         %37 = OpVectorTimesScalar %v4float %30 %float_100
         %38 = OpCompositeExtract %float %37 0
         %39 = OpConvertFToU %uint %38
         %40 = OpCompositeExtract %float %37 1
         %41 = OpConvertFToU %uint %40
         %42 = OpCompositeExtract %float %37 2
         %43 = OpConvertFToU %uint %42
         %44 = OpCompositeExtract %float %37 3
         %45 = OpConvertFToU %uint %44
         %46 = OpCompositeConstruct %v4uint %39 %41 %43 %45
               OpStore %uintValues %46
         %48 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %50 = OpLoad %v4float %48
         %51 = OpVectorTimesScalar %v4float %50 %float_100
         %52 = OpCompositeExtract %float %51 0
         %53 = OpConvertFToU %uint %52
         %54 = OpCompositeExtract %float %51 1
         %55 = OpConvertFToU %uint %54
         %56 = OpCompositeExtract %float %51 2
         %57 = OpConvertFToU %uint %56
         %58 = OpCompositeExtract %float %51 3
         %59 = OpConvertFToU %uint %58
         %60 = OpCompositeConstruct %v4uint %53 %55 %57 %59
               OpStore %uintGreen %60
               OpStore %expectedA %64
               OpStore %expectedB %67
         %70 = OpCompositeExtract %uint %46 0
         %69 = OpExtInst %uint %1 UMin %70 %uint_50
         %71 = OpIEqual %bool %69 %uint_50
               OpSelectionMerge %73 None
               OpBranchConditional %71 %72 %73
         %72 = OpLabel
         %75 = OpVectorShuffle %v2uint %46 %46 0 1
         %74 = OpExtInst %v2uint %1 UMin %75 %77
         %78 = OpVectorShuffle %v2uint %64 %64 0 1
         %79 = OpIEqual %v2bool %74 %78
         %81 = OpAll %bool %79
               OpBranch %73
         %73 = OpLabel
         %82 = OpPhi %bool %false %25 %81 %72
               OpSelectionMerge %84 None
               OpBranchConditional %82 %83 %84
         %83 = OpLabel
         %86 = OpVectorShuffle %v3uint %46 %46 0 1 2
         %85 = OpExtInst %v3uint %1 UMin %86 %88
         %89 = OpVectorShuffle %v3uint %64 %64 0 1 2
         %90 = OpIEqual %v3bool %85 %89
         %92 = OpAll %bool %90
               OpBranch %84
         %84 = OpLabel
         %93 = OpPhi %bool %false %73 %92 %83
               OpSelectionMerge %95 None
               OpBranchConditional %93 %94 %95
         %94 = OpLabel
         %96 = OpExtInst %v4uint %1 UMin %46 %97
         %98 = OpIEqual %v4bool %96 %64
        %100 = OpAll %bool %98
               OpBranch %95
         %95 = OpLabel
        %101 = OpPhi %bool %false %84 %100 %94
               OpSelectionMerge %103 None
               OpBranchConditional %101 %102 %103
        %102 = OpLabel
               OpBranch %103
        %103 = OpLabel
        %105 = OpPhi %bool %false %95 %true %102
               OpSelectionMerge %107 None
               OpBranchConditional %105 %106 %107
        %106 = OpLabel
        %109 = OpVectorShuffle %v2uint %64 %64 0 1
        %110 = OpIEqual %v2bool %108 %109
        %111 = OpAll %bool %110
               OpBranch %107
        %107 = OpLabel
        %112 = OpPhi %bool %false %103 %111 %106
               OpSelectionMerge %114 None
               OpBranchConditional %112 %113 %114
        %113 = OpLabel
        %116 = OpVectorShuffle %v3uint %64 %64 0 1 2
        %117 = OpIEqual %v3bool %115 %116
        %118 = OpAll %bool %117
               OpBranch %114
        %114 = OpLabel
        %119 = OpPhi %bool %false %107 %118 %113
               OpSelectionMerge %121 None
               OpBranchConditional %119 %120 %121
        %120 = OpLabel
               OpBranch %121
        %121 = OpLabel
        %122 = OpPhi %bool %false %114 %true %120
               OpSelectionMerge %124 None
               OpBranchConditional %122 %123 %124
        %123 = OpLabel
        %126 = OpCompositeExtract %uint %60 0
        %125 = OpExtInst %uint %1 UMin %70 %126
        %127 = OpIEqual %bool %125 %uint_0
               OpBranch %124
        %124 = OpLabel
        %128 = OpPhi %bool %false %121 %127 %123
               OpSelectionMerge %130 None
               OpBranchConditional %128 %129 %130
        %129 = OpLabel
        %132 = OpVectorShuffle %v2uint %46 %46 0 1
        %133 = OpVectorShuffle %v2uint %60 %60 0 1
        %131 = OpExtInst %v2uint %1 UMin %132 %133
        %134 = OpVectorShuffle %v2uint %67 %67 0 1
        %135 = OpIEqual %v2bool %131 %134
        %136 = OpAll %bool %135
               OpBranch %130
        %130 = OpLabel
        %137 = OpPhi %bool %false %124 %136 %129
               OpSelectionMerge %139 None
               OpBranchConditional %137 %138 %139
        %138 = OpLabel
        %141 = OpVectorShuffle %v3uint %46 %46 0 1 2
        %142 = OpVectorShuffle %v3uint %60 %60 0 1 2
        %140 = OpExtInst %v3uint %1 UMin %141 %142
        %143 = OpVectorShuffle %v3uint %67 %67 0 1 2
        %144 = OpIEqual %v3bool %140 %143
        %145 = OpAll %bool %144
               OpBranch %139
        %139 = OpLabel
        %146 = OpPhi %bool %false %130 %145 %138
               OpSelectionMerge %148 None
               OpBranchConditional %146 %147 %148
        %147 = OpLabel
        %149 = OpExtInst %v4uint %1 UMin %46 %60
        %150 = OpIEqual %v4bool %149 %67
        %151 = OpAll %bool %150
               OpBranch %148
        %148 = OpLabel
        %152 = OpPhi %bool %false %139 %151 %147
               OpSelectionMerge %154 None
               OpBranchConditional %152 %153 %154
        %153 = OpLabel
               OpBranch %154
        %154 = OpLabel
        %155 = OpPhi %bool %false %148 %true %153
               OpSelectionMerge %157 None
               OpBranchConditional %155 %156 %157
        %156 = OpLabel
        %159 = OpVectorShuffle %v2uint %67 %67 0 1
        %160 = OpIEqual %v2bool %158 %159
        %161 = OpAll %bool %160
               OpBranch %157
        %157 = OpLabel
        %162 = OpPhi %bool %false %154 %161 %156
               OpSelectionMerge %164 None
               OpBranchConditional %162 %163 %164
        %163 = OpLabel
        %166 = OpVectorShuffle %v3uint %67 %67 0 1 2
        %167 = OpIEqual %v3bool %165 %166
        %168 = OpAll %bool %167
               OpBranch %164
        %164 = OpLabel
        %169 = OpPhi %bool %false %157 %168 %163
               OpSelectionMerge %171 None
               OpBranchConditional %169 %170 %171
        %170 = OpLabel
               OpBranch %171
        %171 = OpLabel
        %172 = OpPhi %bool %false %164 %true %170
               OpSelectionMerge %177 None
               OpBranchConditional %172 %175 %176
        %175 = OpLabel
        %178 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %179 = OpLoad %v4float %178
               OpStore %173 %179
               OpBranch %177
        %176 = OpLabel
        %180 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %182 = OpLoad %v4float %180
               OpStore %173 %182
               OpBranch %177
        %177 = OpLabel
        %183 = OpLoad %v4float %173
               OpReturnValue %183
               OpFunctionEnd
