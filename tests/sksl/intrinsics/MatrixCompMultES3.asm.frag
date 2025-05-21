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
               OpDecorate %main RelaxedPrecision
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
               OpDecorate %33 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %h42 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
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
               OpDecorate %100 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
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
         %27 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_9
         %28 = OpConstantComposite %mat2v4float %27 %27
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
         %52 = OpConstantComposite %v2float %float_1 %float_2
         %53 = OpConstantComposite %v2float %float_3 %float_4
         %54 = OpConstantComposite %v2float %float_5 %float_6
         %55 = OpConstantComposite %v2float %float_7 %float_8
         %56 = OpConstantComposite %mat4v2float %52 %53 %54 %55
    %v3float = OpTypeVector %float 3
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
   %float_12 = OpConstant %float 12
   %float_22 = OpConstant %float 22
   %float_30 = OpConstant %float 30
   %float_36 = OpConstant %float 36
   %float_40 = OpConstant %float 40
   %float_42 = OpConstant %float 42
         %89 = OpConstantComposite %v3float %float_12 %float_22 %float_30
         %90 = OpConstantComposite %v3float %float_36 %float_40 %float_42
         %91 = OpConstantComposite %v3float %float_42 %float_40 %float_36
         %92 = OpConstantComposite %v3float %float_30 %float_22 %float_12
         %93 = OpConstantComposite %mat4v3float %89 %90 %91 %92
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
         %96 = OpConstantComposite %v4float %float_9 %float_0 %float_0 %float_9
         %97 = OpConstantComposite %v4float %float_0 %float_9 %float_0 %float_9
         %98 = OpConstantComposite %mat2v4float %96 %97
     %v4bool = OpTypeVector %bool 4
        %107 = OpConstantComposite %v2float %float_1 %float_0
        %108 = OpConstantComposite %v2float %float_0 %float_4
        %109 = OpConstantComposite %v2float %float_0 %float_6
        %110 = OpConstantComposite %v2float %float_0 %float_8
        %111 = OpConstantComposite %mat4v2float %107 %108 %109 %110
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
        %140 = OpVariable %_ptr_Function_v4float Function
         %29 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %33 = OpLoad %v4float %29
         %34 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %36 = OpLoad %v4float %34
         %37 = OpCompositeConstruct %mat2v4float %33 %36
         %38 = OpFMul %v4float %27 %33
         %39 = OpFMul %v4float %27 %36
         %40 = OpCompositeConstruct %mat2v4float %38 %39
               OpStore %h24 %40
         %57 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %58 = OpLoad %v4float %57
         %59 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %60 = OpLoad %v4float %59
         %61 = OpCompositeExtract %float %58 0
         %62 = OpCompositeExtract %float %58 1
         %63 = OpCompositeConstruct %v2float %61 %62
         %64 = OpCompositeExtract %float %58 2
         %65 = OpCompositeExtract %float %58 3
         %66 = OpCompositeConstruct %v2float %64 %65
         %67 = OpCompositeExtract %float %60 0
         %68 = OpCompositeExtract %float %60 1
         %69 = OpCompositeConstruct %v2float %67 %68
         %70 = OpCompositeExtract %float %60 2
         %71 = OpCompositeExtract %float %60 3
         %72 = OpCompositeConstruct %v2float %70 %71
         %73 = OpCompositeConstruct %mat4v2float %63 %66 %69 %72
         %74 = OpFMul %v2float %52 %63
         %75 = OpFMul %v2float %53 %66
         %76 = OpFMul %v2float %54 %69
         %77 = OpFMul %v2float %55 %72
         %78 = OpCompositeConstruct %mat4v2float %74 %75 %76 %77
               OpStore %h42 %78
               OpStore %f43 %93
        %100 = OpFOrdEqual %v4bool %38 %96
        %101 = OpAll %bool %100
        %102 = OpFOrdEqual %v4bool %39 %97
        %103 = OpAll %bool %102
        %104 = OpLogicalAnd %bool %101 %103
               OpSelectionMerge %106 None
               OpBranchConditional %104 %105 %106
        %105 = OpLabel
        %113 = OpFOrdEqual %v2bool %74 %107
        %114 = OpAll %bool %113
        %115 = OpFOrdEqual %v2bool %75 %108
        %116 = OpAll %bool %115
        %117 = OpLogicalAnd %bool %114 %116
        %118 = OpFOrdEqual %v2bool %76 %109
        %119 = OpAll %bool %118
        %120 = OpLogicalAnd %bool %117 %119
        %121 = OpFOrdEqual %v2bool %77 %110
        %122 = OpAll %bool %121
        %123 = OpLogicalAnd %bool %120 %122
               OpBranch %106
        %106 = OpLabel
        %124 = OpPhi %bool %false %22 %123 %105
               OpSelectionMerge %126 None
               OpBranchConditional %124 %125 %126
        %125 = OpLabel
        %128 = OpFOrdEqual %v3bool %89 %89
        %129 = OpAll %bool %128
        %130 = OpFOrdEqual %v3bool %90 %90
        %131 = OpAll %bool %130
        %132 = OpLogicalAnd %bool %129 %131
        %133 = OpFOrdEqual %v3bool %91 %91
        %134 = OpAll %bool %133
        %135 = OpLogicalAnd %bool %132 %134
        %136 = OpFOrdEqual %v3bool %92 %92
        %137 = OpAll %bool %136
        %138 = OpLogicalAnd %bool %135 %137
               OpBranch %126
        %126 = OpLabel
        %139 = OpPhi %bool %false %106 %138 %125
               OpSelectionMerge %144 None
               OpBranchConditional %139 %142 %143
        %142 = OpLabel
        %145 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %146 = OpLoad %v4float %145
               OpStore %140 %146
               OpBranch %144
        %143 = OpLabel
        %147 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %148 = OpLoad %v4float %147
               OpStore %140 %148
               OpBranch %144
        %144 = OpLabel
        %149 = OpLoad %v4float %140
               OpReturnValue %149
               OpFunctionEnd
