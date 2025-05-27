               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %h24 "h24"
               OpName %h42 "h42"
               OpName %f43 "f43"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %h24 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %h42 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
    %float_9 = OpConstant %float 9
         %28 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_9
         %29 = OpConstantComposite %mat2v4float %28 %28
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
    %float_5 = OpConstant %float 5
    %float_6 = OpConstant %float 6
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
         %54 = OpConstantComposite %v2float %float_1 %float_2
         %55 = OpConstantComposite %v2float %float_3 %float_4
         %56 = OpConstantComposite %v2float %float_5 %float_6
         %57 = OpConstantComposite %v2float %float_7 %float_8
         %58 = OpConstantComposite %mat4v2float %54 %55 %56 %57
    %v3float = OpTypeVector %float 3
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
   %float_12 = OpConstant %float 12
   %float_22 = OpConstant %float 22
   %float_30 = OpConstant %float 30
   %float_36 = OpConstant %float 36
   %float_40 = OpConstant %float 40
   %float_42 = OpConstant %float 42
         %91 = OpConstantComposite %v3float %float_12 %float_22 %float_30
         %92 = OpConstantComposite %v3float %float_36 %float_40 %float_42
         %93 = OpConstantComposite %v3float %float_42 %float_40 %float_36
         %94 = OpConstantComposite %v3float %float_30 %float_22 %float_12
         %95 = OpConstantComposite %mat4v3float %91 %92 %93 %94
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
         %98 = OpConstantComposite %v4float %float_9 %float_0 %float_0 %float_9
         %99 = OpConstantComposite %v4float %float_0 %float_9 %float_0 %float_9
        %100 = OpConstantComposite %mat2v4float %98 %99
     %v4bool = OpTypeVector %bool 4
        %109 = OpConstantComposite %v2float %float_1 %float_0
        %110 = OpConstantComposite %v2float %float_0 %float_4
        %111 = OpConstantComposite %v2float %float_0 %float_6
        %112 = OpConstantComposite %v2float %float_0 %float_8
        %113 = OpConstantComposite %mat4v2float %109 %110 %111 %112
     %v2bool = OpTypeVector %bool 2
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
        %h24 = OpVariable %_ptr_Function_mat2v4float Function
        %h42 = OpVariable %_ptr_Function_mat4v2float Function
        %f43 = OpVariable %_ptr_Function_mat4v3float Function
        %142 = OpVariable %_ptr_Function_v4float Function
         %30 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %34 = OpLoad %v4float %30
         %35 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %37 = OpLoad %v4float %35
         %38 = OpCompositeConstruct %mat2v4float %34 %37
         %39 = OpFMul %v4float %28 %34
         %40 = OpFMul %v4float %28 %37
         %41 = OpCompositeConstruct %mat2v4float %39 %40
               OpStore %h24 %41
         %59 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %60 = OpLoad %v4float %59
         %61 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %62 = OpLoad %v4float %61
         %63 = OpCompositeExtract %float %60 0
         %64 = OpCompositeExtract %float %60 1
         %65 = OpCompositeConstruct %v2float %63 %64
         %66 = OpCompositeExtract %float %60 2
         %67 = OpCompositeExtract %float %60 3
         %68 = OpCompositeConstruct %v2float %66 %67
         %69 = OpCompositeExtract %float %62 0
         %70 = OpCompositeExtract %float %62 1
         %71 = OpCompositeConstruct %v2float %69 %70
         %72 = OpCompositeExtract %float %62 2
         %73 = OpCompositeExtract %float %62 3
         %74 = OpCompositeConstruct %v2float %72 %73
         %75 = OpCompositeConstruct %mat4v2float %65 %68 %71 %74
         %76 = OpFMul %v2float %54 %65
         %77 = OpFMul %v2float %55 %68
         %78 = OpFMul %v2float %56 %71
         %79 = OpFMul %v2float %57 %74
         %80 = OpCompositeConstruct %mat4v2float %76 %77 %78 %79
               OpStore %h42 %80
               OpStore %f43 %95
        %102 = OpFOrdEqual %v4bool %39 %98
        %103 = OpAll %bool %102
        %104 = OpFOrdEqual %v4bool %40 %99
        %105 = OpAll %bool %104
        %106 = OpLogicalAnd %bool %103 %105
               OpSelectionMerge %108 None
               OpBranchConditional %106 %107 %108
        %107 = OpLabel
        %115 = OpFOrdEqual %v2bool %76 %109
        %116 = OpAll %bool %115
        %117 = OpFOrdEqual %v2bool %77 %110
        %118 = OpAll %bool %117
        %119 = OpLogicalAnd %bool %116 %118
        %120 = OpFOrdEqual %v2bool %78 %111
        %121 = OpAll %bool %120
        %122 = OpLogicalAnd %bool %119 %121
        %123 = OpFOrdEqual %v2bool %79 %112
        %124 = OpAll %bool %123
        %125 = OpLogicalAnd %bool %122 %124
               OpBranch %108
        %108 = OpLabel
        %126 = OpPhi %bool %false %22 %125 %107
               OpSelectionMerge %128 None
               OpBranchConditional %126 %127 %128
        %127 = OpLabel
        %130 = OpFOrdEqual %v3bool %91 %91
        %131 = OpAll %bool %130
        %132 = OpFOrdEqual %v3bool %92 %92
        %133 = OpAll %bool %132
        %134 = OpLogicalAnd %bool %131 %133
        %135 = OpFOrdEqual %v3bool %93 %93
        %136 = OpAll %bool %135
        %137 = OpLogicalAnd %bool %134 %136
        %138 = OpFOrdEqual %v3bool %94 %94
        %139 = OpAll %bool %138
        %140 = OpLogicalAnd %bool %137 %139
               OpBranch %128
        %128 = OpLabel
        %141 = OpPhi %bool %false %108 %140 %127
               OpSelectionMerge %146 None
               OpBranchConditional %141 %144 %145
        %144 = OpLabel
        %147 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %148 = OpLoad %v4float %147
               OpStore %142 %148
               OpBranch %146
        %145 = OpLabel
        %149 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %150 = OpLoad %v4float %149
               OpStore %142 %150
               OpBranch %146
        %146 = OpLabel
        %151 = OpLoad %v4float %142
               OpReturnValue %151
               OpFunctionEnd
