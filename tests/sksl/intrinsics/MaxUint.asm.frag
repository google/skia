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
               OpDecorate %181 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
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
   %uint_125 = OpConstant %uint 125
    %uint_80 = OpConstant %uint 80
   %uint_225 = OpConstant %uint 225
         %65 = OpConstantComposite %v4uint %uint_125 %uint_80 %uint_80 %uint_225
   %uint_100 = OpConstant %uint 100
    %uint_75 = OpConstant %uint 75
         %69 = OpConstantComposite %v4uint %uint_125 %uint_100 %uint_75 %uint_225
      %false = OpConstantFalse %bool
     %v2uint = OpTypeVector %uint 2
         %79 = OpConstantComposite %v2uint %uint_80 %uint_80
     %v2bool = OpTypeVector %bool 2
     %v3uint = OpTypeVector %uint 3
         %90 = OpConstantComposite %v3uint %uint_80 %uint_80 %uint_80
     %v3bool = OpTypeVector %bool 3
         %99 = OpConstantComposite %v4uint %uint_80 %uint_80 %uint_80 %uint_80
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %110 = OpConstantComposite %v2uint %uint_125 %uint_80
        %117 = OpConstantComposite %v3uint %uint_125 %uint_80 %uint_80
        %160 = OpConstantComposite %v2uint %uint_125 %uint_100
        %167 = OpConstantComposite %v3uint %uint_125 %uint_100 %uint_75
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
        %175 = OpVariable %_ptr_Function_v4float Function
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
               OpStore %expectedA %65
               OpStore %expectedB %69
         %72 = OpCompositeExtract %uint %46 0
         %71 = OpExtInst %uint %1 UMax %72 %uint_80
         %73 = OpIEqual %bool %71 %uint_125
               OpSelectionMerge %75 None
               OpBranchConditional %73 %74 %75
         %74 = OpLabel
         %77 = OpVectorShuffle %v2uint %46 %46 0 1
         %76 = OpExtInst %v2uint %1 UMax %77 %79
         %80 = OpVectorShuffle %v2uint %65 %65 0 1
         %81 = OpIEqual %v2bool %76 %80
         %83 = OpAll %bool %81
               OpBranch %75
         %75 = OpLabel
         %84 = OpPhi %bool %false %25 %83 %74
               OpSelectionMerge %86 None
               OpBranchConditional %84 %85 %86
         %85 = OpLabel
         %88 = OpVectorShuffle %v3uint %46 %46 0 1 2
         %87 = OpExtInst %v3uint %1 UMax %88 %90
         %91 = OpVectorShuffle %v3uint %65 %65 0 1 2
         %92 = OpIEqual %v3bool %87 %91
         %94 = OpAll %bool %92
               OpBranch %86
         %86 = OpLabel
         %95 = OpPhi %bool %false %75 %94 %85
               OpSelectionMerge %97 None
               OpBranchConditional %95 %96 %97
         %96 = OpLabel
         %98 = OpExtInst %v4uint %1 UMax %46 %99
        %100 = OpIEqual %v4bool %98 %65
        %102 = OpAll %bool %100
               OpBranch %97
         %97 = OpLabel
        %103 = OpPhi %bool %false %86 %102 %96
               OpSelectionMerge %105 None
               OpBranchConditional %103 %104 %105
        %104 = OpLabel
               OpBranch %105
        %105 = OpLabel
        %107 = OpPhi %bool %false %97 %true %104
               OpSelectionMerge %109 None
               OpBranchConditional %107 %108 %109
        %108 = OpLabel
        %111 = OpVectorShuffle %v2uint %65 %65 0 1
        %112 = OpIEqual %v2bool %110 %111
        %113 = OpAll %bool %112
               OpBranch %109
        %109 = OpLabel
        %114 = OpPhi %bool %false %105 %113 %108
               OpSelectionMerge %116 None
               OpBranchConditional %114 %115 %116
        %115 = OpLabel
        %118 = OpVectorShuffle %v3uint %65 %65 0 1 2
        %119 = OpIEqual %v3bool %117 %118
        %120 = OpAll %bool %119
               OpBranch %116
        %116 = OpLabel
        %121 = OpPhi %bool %false %109 %120 %115
               OpSelectionMerge %123 None
               OpBranchConditional %121 %122 %123
        %122 = OpLabel
               OpBranch %123
        %123 = OpLabel
        %124 = OpPhi %bool %false %116 %true %122
               OpSelectionMerge %126 None
               OpBranchConditional %124 %125 %126
        %125 = OpLabel
        %128 = OpCompositeExtract %uint %60 0
        %127 = OpExtInst %uint %1 UMax %72 %128
        %129 = OpIEqual %bool %127 %uint_125
               OpBranch %126
        %126 = OpLabel
        %130 = OpPhi %bool %false %123 %129 %125
               OpSelectionMerge %132 None
               OpBranchConditional %130 %131 %132
        %131 = OpLabel
        %134 = OpVectorShuffle %v2uint %46 %46 0 1
        %135 = OpVectorShuffle %v2uint %60 %60 0 1
        %133 = OpExtInst %v2uint %1 UMax %134 %135
        %136 = OpVectorShuffle %v2uint %69 %69 0 1
        %137 = OpIEqual %v2bool %133 %136
        %138 = OpAll %bool %137
               OpBranch %132
        %132 = OpLabel
        %139 = OpPhi %bool %false %126 %138 %131
               OpSelectionMerge %141 None
               OpBranchConditional %139 %140 %141
        %140 = OpLabel
        %143 = OpVectorShuffle %v3uint %46 %46 0 1 2
        %144 = OpVectorShuffle %v3uint %60 %60 0 1 2
        %142 = OpExtInst %v3uint %1 UMax %143 %144
        %145 = OpVectorShuffle %v3uint %69 %69 0 1 2
        %146 = OpIEqual %v3bool %142 %145
        %147 = OpAll %bool %146
               OpBranch %141
        %141 = OpLabel
        %148 = OpPhi %bool %false %132 %147 %140
               OpSelectionMerge %150 None
               OpBranchConditional %148 %149 %150
        %149 = OpLabel
        %151 = OpExtInst %v4uint %1 UMax %46 %60
        %152 = OpIEqual %v4bool %151 %69
        %153 = OpAll %bool %152
               OpBranch %150
        %150 = OpLabel
        %154 = OpPhi %bool %false %141 %153 %149
               OpSelectionMerge %156 None
               OpBranchConditional %154 %155 %156
        %155 = OpLabel
               OpBranch %156
        %156 = OpLabel
        %157 = OpPhi %bool %false %150 %true %155
               OpSelectionMerge %159 None
               OpBranchConditional %157 %158 %159
        %158 = OpLabel
        %161 = OpVectorShuffle %v2uint %69 %69 0 1
        %162 = OpIEqual %v2bool %160 %161
        %163 = OpAll %bool %162
               OpBranch %159
        %159 = OpLabel
        %164 = OpPhi %bool %false %156 %163 %158
               OpSelectionMerge %166 None
               OpBranchConditional %164 %165 %166
        %165 = OpLabel
        %168 = OpVectorShuffle %v3uint %69 %69 0 1 2
        %169 = OpIEqual %v3bool %167 %168
        %170 = OpAll %bool %169
               OpBranch %166
        %166 = OpLabel
        %171 = OpPhi %bool %false %159 %170 %165
               OpSelectionMerge %173 None
               OpBranchConditional %171 %172 %173
        %172 = OpLabel
               OpBranch %173
        %173 = OpLabel
        %174 = OpPhi %bool %false %166 %true %172
               OpSelectionMerge %179 None
               OpBranchConditional %174 %177 %178
        %177 = OpLabel
        %180 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %181 = OpLoad %v4float %180
               OpStore %175 %181
               OpBranch %179
        %178 = OpLabel
        %182 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %184 = OpLoad %v4float %182
               OpStore %175 %184
               OpBranch %179
        %179 = OpLabel
        %185 = OpLoad %v4float %175
               OpReturnValue %185
               OpFunctionEnd
