               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "inputVal"
               OpMemberName %_UniformBuffer 1 "expected"
               OpMemberName %_UniformBuffer 2 "colorGreen"
               OpMemberName %_UniformBuffer 3 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
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
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %166 RelaxedPrecision
               OpDecorate %167 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
         %92 = OpConstantComposite %v3float %float_0 %float_0 %float_0
        %101 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
    %float_1 = OpConstant %float 1
        %125 = OpConstantComposite %v2float %float_1 %float_1
        %138 = OpConstantComposite %v3float %float_1 %float_1 %float_1
        %150 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
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
        %186 = OpVariable %_ptr_Function_v4float Function
         %26 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %30 = OpLoad %v4float %26
         %31 = OpCompositeExtract %float %30 0
         %25 = OpExtInst %float %1 Atan %31
         %32 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %34 = OpLoad %v4float %32
         %35 = OpCompositeExtract %float %34 0
         %36 = OpFOrdEqual %bool %25 %35
               OpSelectionMerge %38 None
               OpBranchConditional %36 %37 %38
         %37 = OpLabel
         %40 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %41 = OpLoad %v4float %40
         %42 = OpVectorShuffle %v2float %41 %41 0 1
         %39 = OpExtInst %v2float %1 Atan %42
         %43 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %44 = OpLoad %v4float %43
         %45 = OpVectorShuffle %v2float %44 %44 0 1
         %46 = OpFOrdEqual %v2bool %39 %45
         %48 = OpAll %bool %46
               OpBranch %38
         %38 = OpLabel
         %49 = OpPhi %bool %false %22 %48 %37
               OpSelectionMerge %51 None
               OpBranchConditional %49 %50 %51
         %50 = OpLabel
         %53 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %54 = OpLoad %v4float %53
         %55 = OpVectorShuffle %v3float %54 %54 0 1 2
         %52 = OpExtInst %v3float %1 Atan %55
         %57 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %58 = OpLoad %v4float %57
         %59 = OpVectorShuffle %v3float %58 %58 0 1 2
         %60 = OpFOrdEqual %v3bool %52 %59
         %62 = OpAll %bool %60
               OpBranch %51
         %51 = OpLabel
         %63 = OpPhi %bool %false %38 %62 %50
               OpSelectionMerge %65 None
               OpBranchConditional %63 %64 %65
         %64 = OpLabel
         %67 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %68 = OpLoad %v4float %67
         %66 = OpExtInst %v4float %1 Atan %68
         %69 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %70 = OpLoad %v4float %69
         %71 = OpFOrdEqual %v4bool %66 %70
         %73 = OpAll %bool %71
               OpBranch %65
         %65 = OpLabel
         %74 = OpPhi %bool %false %51 %73 %64
               OpSelectionMerge %76 None
               OpBranchConditional %74 %75 %76
         %75 = OpLabel
         %77 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %78 = OpLoad %v4float %77
         %79 = OpCompositeExtract %float %78 0
         %80 = OpFOrdEqual %bool %float_0 %79
               OpBranch %76
         %76 = OpLabel
         %81 = OpPhi %bool %false %65 %80 %75
               OpSelectionMerge %83 None
               OpBranchConditional %81 %82 %83
         %82 = OpLabel
         %84 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %85 = OpLoad %v4float %84
         %86 = OpVectorShuffle %v2float %85 %85 0 1
         %87 = OpFOrdEqual %v2bool %16 %86
         %88 = OpAll %bool %87
               OpBranch %83
         %83 = OpLabel
         %89 = OpPhi %bool %false %76 %88 %82
               OpSelectionMerge %91 None
               OpBranchConditional %89 %90 %91
         %90 = OpLabel
         %93 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %94 = OpLoad %v4float %93
         %95 = OpVectorShuffle %v3float %94 %94 0 1 2
         %96 = OpFOrdEqual %v3bool %92 %95
         %97 = OpAll %bool %96
               OpBranch %91
         %91 = OpLabel
         %98 = OpPhi %bool %false %83 %97 %90
               OpSelectionMerge %100 None
               OpBranchConditional %98 %99 %100
         %99 = OpLabel
        %102 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %103 = OpLoad %v4float %102
        %104 = OpFOrdEqual %v4bool %101 %103
        %105 = OpAll %bool %104
               OpBranch %100
        %100 = OpLabel
        %106 = OpPhi %bool %false %91 %105 %99
               OpSelectionMerge %108 None
               OpBranchConditional %106 %107 %108
        %107 = OpLabel
        %110 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %111 = OpLoad %v4float %110
        %112 = OpCompositeExtract %float %111 0
        %109 = OpExtInst %float %1 Atan2 %112 %float_1
        %114 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %115 = OpLoad %v4float %114
        %116 = OpCompositeExtract %float %115 0
        %117 = OpFOrdEqual %bool %109 %116
               OpBranch %108
        %108 = OpLabel
        %118 = OpPhi %bool %false %100 %117 %107
               OpSelectionMerge %120 None
               OpBranchConditional %118 %119 %120
        %119 = OpLabel
        %122 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %123 = OpLoad %v4float %122
        %124 = OpVectorShuffle %v2float %123 %123 0 1
        %121 = OpExtInst %v2float %1 Atan2 %124 %125
        %126 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %127 = OpLoad %v4float %126
        %128 = OpVectorShuffle %v2float %127 %127 0 1
        %129 = OpFOrdEqual %v2bool %121 %128
        %130 = OpAll %bool %129
               OpBranch %120
        %120 = OpLabel
        %131 = OpPhi %bool %false %108 %130 %119
               OpSelectionMerge %133 None
               OpBranchConditional %131 %132 %133
        %132 = OpLabel
        %135 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %136 = OpLoad %v4float %135
        %137 = OpVectorShuffle %v3float %136 %136 0 1 2
        %134 = OpExtInst %v3float %1 Atan2 %137 %138
        %139 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %140 = OpLoad %v4float %139
        %141 = OpVectorShuffle %v3float %140 %140 0 1 2
        %142 = OpFOrdEqual %v3bool %134 %141
        %143 = OpAll %bool %142
               OpBranch %133
        %133 = OpLabel
        %144 = OpPhi %bool %false %120 %143 %132
               OpSelectionMerge %146 None
               OpBranchConditional %144 %145 %146
        %145 = OpLabel
        %148 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %149 = OpLoad %v4float %148
        %147 = OpExtInst %v4float %1 Atan2 %149 %150
        %151 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %152 = OpLoad %v4float %151
        %153 = OpFOrdEqual %v4bool %147 %152
        %154 = OpAll %bool %153
               OpBranch %146
        %146 = OpLabel
        %155 = OpPhi %bool %false %133 %154 %145
               OpSelectionMerge %157 None
               OpBranchConditional %155 %156 %157
        %156 = OpLabel
        %158 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %159 = OpLoad %v4float %158
        %160 = OpCompositeExtract %float %159 0
        %161 = OpFOrdEqual %bool %float_0 %160
               OpBranch %157
        %157 = OpLabel
        %162 = OpPhi %bool %false %146 %161 %156
               OpSelectionMerge %164 None
               OpBranchConditional %162 %163 %164
        %163 = OpLabel
        %165 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %166 = OpLoad %v4float %165
        %167 = OpVectorShuffle %v2float %166 %166 0 1
        %168 = OpFOrdEqual %v2bool %16 %167
        %169 = OpAll %bool %168
               OpBranch %164
        %164 = OpLabel
        %170 = OpPhi %bool %false %157 %169 %163
               OpSelectionMerge %172 None
               OpBranchConditional %170 %171 %172
        %171 = OpLabel
        %173 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %174 = OpLoad %v4float %173
        %175 = OpVectorShuffle %v3float %174 %174 0 1 2
        %176 = OpFOrdEqual %v3bool %92 %175
        %177 = OpAll %bool %176
               OpBranch %172
        %172 = OpLabel
        %178 = OpPhi %bool %false %164 %177 %171
               OpSelectionMerge %180 None
               OpBranchConditional %178 %179 %180
        %179 = OpLabel
        %181 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %182 = OpLoad %v4float %181
        %183 = OpFOrdEqual %v4bool %101 %182
        %184 = OpAll %bool %183
               OpBranch %180
        %180 = OpLabel
        %185 = OpPhi %bool %false %172 %184 %179
               OpSelectionMerge %190 None
               OpBranchConditional %185 %188 %189
        %188 = OpLabel
        %191 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %193 = OpLoad %v4float %191
               OpStore %186 %193
               OpBranch %190
        %189 = OpLabel
        %194 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %196 = OpLoad %v4float %194
               OpStore %186 %196
               OpBranch %190
        %190 = OpLabel
        %197 = OpLoad %v4float %186
               OpReturnValue %197
               OpFunctionEnd
