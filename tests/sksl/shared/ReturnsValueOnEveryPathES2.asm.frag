               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %return_on_both_sides_b "return_on_both_sides_b"
               OpName %for_inside_body_b "for_inside_body_b"
               OpName %x "x"
               OpName %after_for_body_b "after_for_body_b"
               OpName %x_0 "x"
               OpName %for_with_double_sided_conditional_return_b "for_with_double_sided_conditional_return_b"
               OpName %x_1 "x"
               OpName %if_else_chain_b "if_else_chain_b"
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
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %32 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %26 = OpTypeFunction %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
       %true = OpConstantTrue %bool
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
     %int_10 = OpConstant %int 10
      %int_1 = OpConstant %int 1
    %float_2 = OpConstant %float 2
      %false = OpConstantFalse %bool
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
        %111 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %17
         %18 = OpLabel
         %22 = OpVariable %_ptr_Function_v2float Function
               OpStore %22 %21
         %24 = OpFunctionCall %v4float %main %22
               OpStore %sk_FragColor %24
               OpReturn
               OpFunctionEnd
%return_on_both_sides_b = OpFunction %bool None %26
         %27 = OpLabel
         %28 = OpAccessChain %_ptr_Uniform_float %12 %int_2
         %32 = OpLoad %float %28
         %34 = OpFOrdEqual %bool %32 %float_1
               OpSelectionMerge %37 None
               OpBranchConditional %34 %35 %36
         %35 = OpLabel
               OpReturnValue %true
         %36 = OpLabel
               OpReturnValue %true
         %37 = OpLabel
               OpUnreachable
               OpFunctionEnd
%for_inside_body_b = OpFunction %bool None %26
         %39 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
               OpStore %x %int_0
               OpBranch %43
         %43 = OpLabel
               OpLoopMerge %47 %46 None
               OpBranch %44
         %44 = OpLabel
         %48 = OpLoad %int %x
         %50 = OpSLessThanEqual %bool %48 %int_10
               OpBranchConditional %50 %45 %47
         %45 = OpLabel
               OpReturnValue %true
         %46 = OpLabel
         %52 = OpLoad %int %x
         %53 = OpIAdd %int %52 %int_1
               OpStore %x %53
               OpBranch %43
         %47 = OpLabel
               OpUnreachable
               OpFunctionEnd
%after_for_body_b = OpFunction %bool None %26
         %54 = OpLabel
        %x_0 = OpVariable %_ptr_Function_int Function
               OpStore %x_0 %int_0
               OpBranch %56
         %56 = OpLabel
               OpLoopMerge %60 %59 None
               OpBranch %57
         %57 = OpLabel
         %61 = OpLoad %int %x_0
         %62 = OpSLessThanEqual %bool %61 %int_10
               OpBranchConditional %62 %58 %60
         %58 = OpLabel
               OpBranch %59
         %59 = OpLabel
         %63 = OpLoad %int %x_0
         %64 = OpIAdd %int %63 %int_1
               OpStore %x_0 %64
               OpBranch %56
         %60 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%for_with_double_sided_conditional_return_b = OpFunction %bool None %26
         %65 = OpLabel
        %x_1 = OpVariable %_ptr_Function_int Function
               OpStore %x_1 %int_0
               OpBranch %67
         %67 = OpLabel
               OpLoopMerge %71 %70 None
               OpBranch %68
         %68 = OpLabel
         %72 = OpLoad %int %x_1
         %73 = OpSLessThanEqual %bool %72 %int_10
               OpBranchConditional %73 %69 %71
         %69 = OpLabel
         %74 = OpAccessChain %_ptr_Uniform_float %12 %int_2
         %75 = OpLoad %float %74
         %76 = OpFOrdEqual %bool %75 %float_1
               OpSelectionMerge %79 None
               OpBranchConditional %76 %77 %78
         %77 = OpLabel
               OpReturnValue %true
         %78 = OpLabel
               OpReturnValue %true
         %79 = OpLabel
               OpBranch %70
         %70 = OpLabel
         %80 = OpLoad %int %x_1
         %81 = OpIAdd %int %80 %int_1
               OpStore %x_1 %81
               OpBranch %67
         %71 = OpLabel
               OpUnreachable
               OpFunctionEnd
%if_else_chain_b = OpFunction %bool None %26
         %82 = OpLabel
         %83 = OpAccessChain %_ptr_Uniform_float %12 %int_2
         %84 = OpLoad %float %83
         %85 = OpFOrdEqual %bool %84 %float_1
               OpSelectionMerge %88 None
               OpBranchConditional %85 %86 %87
         %86 = OpLabel
               OpReturnValue %true
         %87 = OpLabel
         %89 = OpAccessChain %_ptr_Uniform_float %12 %int_2
         %90 = OpLoad %float %89
         %92 = OpFOrdEqual %bool %90 %float_2
               OpSelectionMerge %95 None
               OpBranchConditional %92 %93 %94
         %93 = OpLabel
               OpReturnValue %false
         %94 = OpLabel
         %97 = OpAccessChain %_ptr_Uniform_float %12 %int_2
         %98 = OpLoad %float %97
        %100 = OpFOrdEqual %bool %98 %float_3
               OpSelectionMerge %103 None
               OpBranchConditional %100 %101 %102
        %101 = OpLabel
               OpReturnValue %true
        %102 = OpLabel
        %104 = OpAccessChain %_ptr_Uniform_float %12 %int_2
        %105 = OpLoad %float %104
        %107 = OpFOrdEqual %bool %105 %float_4
               OpSelectionMerge %110 None
               OpBranchConditional %107 %108 %109
        %108 = OpLabel
               OpReturnValue %false
        %109 = OpLabel
               OpReturnValue %true
        %110 = OpLabel
               OpBranch %103
        %103 = OpLabel
               OpBranch %95
         %95 = OpLabel
               OpBranch %88
         %88 = OpLabel
               OpUnreachable
               OpFunctionEnd
       %main = OpFunction %v4float None %111
        %112 = OpFunctionParameter %_ptr_Function_v2float
        %113 = OpLabel
        %134 = OpVariable %_ptr_Function_v4float Function
               OpSelectionMerge %115 None
               OpBranchConditional %true %114 %115
        %114 = OpLabel
        %116 = OpFunctionCall %bool %return_on_both_sides_b
               OpBranch %115
        %115 = OpLabel
        %117 = OpPhi %bool %false %113 %116 %114
               OpSelectionMerge %119 None
               OpBranchConditional %117 %118 %119
        %118 = OpLabel
        %120 = OpFunctionCall %bool %for_inside_body_b
               OpBranch %119
        %119 = OpLabel
        %121 = OpPhi %bool %false %115 %120 %118
               OpSelectionMerge %123 None
               OpBranchConditional %121 %122 %123
        %122 = OpLabel
        %124 = OpFunctionCall %bool %after_for_body_b
               OpBranch %123
        %123 = OpLabel
        %125 = OpPhi %bool %false %119 %124 %122
               OpSelectionMerge %127 None
               OpBranchConditional %125 %126 %127
        %126 = OpLabel
        %128 = OpFunctionCall %bool %for_with_double_sided_conditional_return_b
               OpBranch %127
        %127 = OpLabel
        %129 = OpPhi %bool %false %123 %128 %126
               OpSelectionMerge %131 None
               OpBranchConditional %129 %130 %131
        %130 = OpLabel
        %132 = OpFunctionCall %bool %if_else_chain_b
               OpBranch %131
        %131 = OpLabel
        %133 = OpPhi %bool %false %127 %132 %130
               OpSelectionMerge %138 None
               OpBranchConditional %133 %136 %137
        %136 = OpLabel
        %139 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %141 = OpLoad %v4float %139
               OpStore %134 %141
               OpBranch %138
        %137 = OpLabel
        %142 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %143 = OpLoad %v4float %142
               OpStore %134 %143
               OpBranch %138
        %138 = OpLabel
        %144 = OpLoad %v4float %134
               OpReturnValue %144
               OpFunctionEnd
