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
               OpDecorate %37 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision
               OpDecorate %163 RelaxedPrecision
               OpDecorate %165 RelaxedPrecision
               OpDecorate %166 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %186 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision
               OpDecorate %212 RelaxedPrecision
               OpDecorate %213 RelaxedPrecision
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
%float_0_84375 = OpConstant %float 0.84375
    %float_1 = OpConstant %float 1
         %30 = OpConstantComposite %v4float %float_0 %float_0 %float_0_84375 %float_1
         %32 = OpConstantComposite %v4float %float_1 %float_0 %float_1 %float_1
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %45 = OpConstantComposite %v3float %float_0 %float_0 %float_0_84375
     %v3bool = OpTypeVector %bool 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
      %int_1 = OpConstant %int 1
%float_n1_25 = OpConstant %float -1.25
         %99 = OpConstantComposite %v2float %float_n1_25 %float_0
 %float_0_75 = OpConstant %float 0.75
        %116 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0_75
 %float_2_25 = OpConstant %float 2.25
        %133 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
     %v4bool = OpTypeVector %bool 4
        %143 = OpConstantComposite %v2float %float_1 %float_0
        %150 = OpConstantComposite %v3float %float_1 %float_0 %float_1
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
        %205 = OpVariable %_ptr_Function_v4float Function
               OpStore %expectedA %30
               OpStore %expectedB %32
               OpSelectionMerge %36 None
               OpBranchConditional %true %35 %36
         %35 = OpLabel
         %37 = OpVectorShuffle %v2float %30 %30 0 1
         %38 = OpFOrdEqual %v2bool %19 %37
         %40 = OpAll %bool %38
               OpBranch %36
         %36 = OpLabel
         %41 = OpPhi %bool %false %25 %40 %35
               OpSelectionMerge %43 None
               OpBranchConditional %41 %42 %43
         %42 = OpLabel
         %46 = OpVectorShuffle %v3float %30 %30 0 1 2
         %47 = OpFOrdEqual %v3bool %45 %46
         %49 = OpAll %bool %47
               OpBranch %43
         %43 = OpLabel
         %50 = OpPhi %bool %false %36 %49 %42
               OpSelectionMerge %52 None
               OpBranchConditional %50 %51 %52
         %51 = OpLabel
               OpBranch %52
         %52 = OpLabel
         %53 = OpPhi %bool %false %43 %true %51
               OpSelectionMerge %55 None
               OpBranchConditional %53 %54 %55
         %54 = OpLabel
               OpBranch %55
         %55 = OpLabel
         %56 = OpPhi %bool %false %52 %true %54
               OpSelectionMerge %58 None
               OpBranchConditional %56 %57 %58
         %57 = OpLabel
         %59 = OpVectorShuffle %v2float %30 %30 0 1
         %60 = OpFOrdEqual %v2bool %19 %59
         %61 = OpAll %bool %60
               OpBranch %58
         %58 = OpLabel
         %62 = OpPhi %bool %false %55 %61 %57
               OpSelectionMerge %64 None
               OpBranchConditional %62 %63 %64
         %63 = OpLabel
         %65 = OpVectorShuffle %v3float %30 %30 0 1 2
         %66 = OpFOrdEqual %v3bool %45 %65
         %67 = OpAll %bool %66
               OpBranch %64
         %64 = OpLabel
         %68 = OpPhi %bool %false %58 %67 %63
               OpSelectionMerge %70 None
               OpBranchConditional %68 %69 %70
         %69 = OpLabel
               OpBranch %70
         %70 = OpLabel
         %71 = OpPhi %bool %false %64 %true %69
               OpSelectionMerge %73 None
               OpBranchConditional %71 %72 %73
         %72 = OpLabel
         %75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %79 = OpLoad %v4float %75
         %80 = OpCompositeExtract %float %79 1
         %81 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %83 = OpLoad %v4float %81
         %84 = OpCompositeExtract %float %83 1
         %74 = OpExtInst %float %1 SmoothStep %80 %84 %float_n1_25
         %86 = OpFOrdEqual %bool %74 %float_0
               OpBranch %73
         %73 = OpLabel
         %87 = OpPhi %bool %false %70 %86 %72
               OpSelectionMerge %89 None
               OpBranchConditional %87 %88 %89
         %88 = OpLabel
         %91 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %92 = OpLoad %v4float %91
         %93 = OpCompositeExtract %float %92 1
         %94 = OpCompositeConstruct %v2float %93 %93
         %95 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %96 = OpLoad %v4float %95
         %97 = OpCompositeExtract %float %96 1
         %98 = OpCompositeConstruct %v2float %97 %97
         %90 = OpExtInst %v2float %1 SmoothStep %94 %98 %99
        %100 = OpVectorShuffle %v2float %30 %30 0 1
        %101 = OpFOrdEqual %v2bool %90 %100
        %102 = OpAll %bool %101
               OpBranch %89
         %89 = OpLabel
        %103 = OpPhi %bool %false %73 %102 %88
               OpSelectionMerge %105 None
               OpBranchConditional %103 %104 %105
        %104 = OpLabel
        %107 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %108 = OpLoad %v4float %107
        %109 = OpCompositeExtract %float %108 1
        %110 = OpCompositeConstruct %v3float %109 %109 %109
        %111 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %112 = OpLoad %v4float %111
        %113 = OpCompositeExtract %float %112 1
        %114 = OpCompositeConstruct %v3float %113 %113 %113
        %106 = OpExtInst %v3float %1 SmoothStep %110 %114 %116
        %117 = OpVectorShuffle %v3float %30 %30 0 1 2
        %118 = OpFOrdEqual %v3bool %106 %117
        %119 = OpAll %bool %118
               OpBranch %105
        %105 = OpLabel
        %120 = OpPhi %bool %false %89 %119 %104
               OpSelectionMerge %122 None
               OpBranchConditional %120 %121 %122
        %121 = OpLabel
        %124 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %125 = OpLoad %v4float %124
        %126 = OpCompositeExtract %float %125 1
        %127 = OpCompositeConstruct %v4float %126 %126 %126 %126
        %128 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %129 = OpLoad %v4float %128
        %130 = OpCompositeExtract %float %129 1
        %131 = OpCompositeConstruct %v4float %130 %130 %130 %130
        %123 = OpExtInst %v4float %1 SmoothStep %127 %131 %133
        %134 = OpFOrdEqual %v4bool %123 %30
        %136 = OpAll %bool %134
               OpBranch %122
        %122 = OpLabel
        %137 = OpPhi %bool %false %105 %136 %121
               OpSelectionMerge %139 None
               OpBranchConditional %137 %138 %139
        %138 = OpLabel
               OpBranch %139
        %139 = OpLabel
        %140 = OpPhi %bool %false %122 %true %138
               OpSelectionMerge %142 None
               OpBranchConditional %140 %141 %142
        %141 = OpLabel
        %144 = OpVectorShuffle %v2float %32 %32 0 1
        %145 = OpFOrdEqual %v2bool %143 %144
        %146 = OpAll %bool %145
               OpBranch %142
        %142 = OpLabel
        %147 = OpPhi %bool %false %139 %146 %141
               OpSelectionMerge %149 None
               OpBranchConditional %147 %148 %149
        %148 = OpLabel
        %151 = OpVectorShuffle %v3float %32 %32 0 1 2
        %152 = OpFOrdEqual %v3bool %150 %151
        %153 = OpAll %bool %152
               OpBranch %149
        %149 = OpLabel
        %154 = OpPhi %bool %false %142 %153 %148
               OpSelectionMerge %156 None
               OpBranchConditional %154 %155 %156
        %155 = OpLabel
               OpBranch %156
        %156 = OpLabel
        %157 = OpPhi %bool %false %149 %true %155
               OpSelectionMerge %159 None
               OpBranchConditional %157 %158 %159
        %158 = OpLabel
        %161 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %162 = OpLoad %v4float %161
        %163 = OpCompositeExtract %float %162 0
        %164 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %165 = OpLoad %v4float %164
        %166 = OpCompositeExtract %float %165 0
        %160 = OpExtInst %float %1 SmoothStep %163 %166 %float_n1_25
        %167 = OpFOrdEqual %bool %160 %float_1
               OpBranch %159
        %159 = OpLabel
        %168 = OpPhi %bool %false %156 %167 %158
               OpSelectionMerge %170 None
               OpBranchConditional %168 %169 %170
        %169 = OpLabel
        %172 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %173 = OpLoad %v4float %172
        %174 = OpVectorShuffle %v2float %173 %173 0 1
        %175 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %176 = OpLoad %v4float %175
        %177 = OpVectorShuffle %v2float %176 %176 0 1
        %171 = OpExtInst %v2float %1 SmoothStep %174 %177 %99
        %178 = OpVectorShuffle %v2float %32 %32 0 1
        %179 = OpFOrdEqual %v2bool %171 %178
        %180 = OpAll %bool %179
               OpBranch %170
        %170 = OpLabel
        %181 = OpPhi %bool %false %159 %180 %169
               OpSelectionMerge %183 None
               OpBranchConditional %181 %182 %183
        %182 = OpLabel
        %185 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %186 = OpLoad %v4float %185
        %187 = OpVectorShuffle %v3float %186 %186 0 1 2
        %188 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %189 = OpLoad %v4float %188
        %190 = OpVectorShuffle %v3float %189 %189 0 1 2
        %184 = OpExtInst %v3float %1 SmoothStep %187 %190 %116
        %191 = OpVectorShuffle %v3float %32 %32 0 1 2
        %192 = OpFOrdEqual %v3bool %184 %191
        %193 = OpAll %bool %192
               OpBranch %183
        %183 = OpLabel
        %194 = OpPhi %bool %false %170 %193 %182
               OpSelectionMerge %196 None
               OpBranchConditional %194 %195 %196
        %195 = OpLabel
        %198 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %199 = OpLoad %v4float %198
        %200 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %201 = OpLoad %v4float %200
        %197 = OpExtInst %v4float %1 SmoothStep %199 %201 %133
        %202 = OpFOrdEqual %v4bool %197 %32
        %203 = OpAll %bool %202
               OpBranch %196
        %196 = OpLabel
        %204 = OpPhi %bool %false %183 %203 %195
               OpSelectionMerge %208 None
               OpBranchConditional %204 %206 %207
        %206 = OpLabel
        %209 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %210 = OpLoad %v4float %209
               OpStore %205 %210
               OpBranch %208
        %207 = OpLabel
        %211 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %212 = OpLoad %v4float %211
               OpStore %205 %212
               OpBranch %208
        %208 = OpLabel
        %213 = OpLoad %v4float %205
               OpReturnValue %213
               OpFunctionEnd
