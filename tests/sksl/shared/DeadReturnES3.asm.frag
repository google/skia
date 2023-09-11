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
               OpName %test_return_b "test_return_b"
               OpName %test_break_b "test_break_b"
               OpName %test_continue_b "test_continue_b"
               OpName %test_if_return_b "test_if_return_b"
               OpName %test_if_break_b "test_if_break_b"
               OpName %test_else_b "test_else_b"
               OpName %test_loop_return_b "test_loop_return_b"
               OpName %test_loop_break_b "test_loop_break_b"
               OpName %x "x"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %15 Binding 0
               OpDecorate %15 DescriptorSet 0
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %15 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %24 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %29 = OpTypeFunction %bool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
      %int_1 = OpConstant %int 1
        %106 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %20
         %21 = OpLabel
         %25 = OpVariable %_ptr_Function_v2float Function
               OpStore %25 %24
         %27 = OpFunctionCall %v4float %main %25
               OpStore %sk_FragColor %27
               OpReturn
               OpFunctionEnd
%test_return_b = OpFunction %bool None %29
         %30 = OpLabel
               OpBranch %31
         %31 = OpLabel
               OpLoopMerge %35 %34 None
               OpBranch %32
         %32 = OpLabel
               OpReturnValue %true
         %34 = OpLabel
               OpBranchConditional %false %31 %35
         %35 = OpLabel
               OpUnreachable
               OpFunctionEnd
%test_break_b = OpFunction %bool None %29
         %38 = OpLabel
               OpBranch %39
         %39 = OpLabel
               OpLoopMerge %43 %42 None
               OpBranch %40
         %40 = OpLabel
               OpBranch %43
         %42 = OpLabel
               OpBranchConditional %false %39 %43
         %43 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%test_continue_b = OpFunction %bool None %29
         %44 = OpLabel
               OpBranch %45
         %45 = OpLabel
               OpLoopMerge %49 %48 None
               OpBranch %46
         %46 = OpLabel
               OpBranch %48
         %48 = OpLabel
               OpBranchConditional %false %45 %49
         %49 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%test_if_return_b = OpFunction %bool None %29
         %50 = OpLabel
               OpBranch %51
         %51 = OpLabel
               OpLoopMerge %55 %54 None
               OpBranch %52
         %52 = OpLabel
         %56 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %60 = OpLoad %v4float %56
         %61 = OpCompositeExtract %float %60 1
         %62 = OpFOrdGreaterThan %bool %61 %float_0
               OpSelectionMerge %65 None
               OpBranchConditional %62 %63 %64
         %63 = OpLabel
               OpReturnValue %true
         %64 = OpLabel
               OpBranch %55
         %65 = OpLabel
               OpBranch %54
         %54 = OpLabel
               OpBranchConditional %false %51 %55
         %55 = OpLabel
               OpReturnValue %false
               OpFunctionEnd
%test_if_break_b = OpFunction %bool None %29
         %66 = OpLabel
               OpBranch %67
         %67 = OpLabel
               OpLoopMerge %71 %70 None
               OpBranch %68
         %68 = OpLabel
         %72 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %73 = OpLoad %v4float %72
         %74 = OpCompositeExtract %float %73 1
         %75 = OpFOrdGreaterThan %bool %74 %float_0
               OpSelectionMerge %78 None
               OpBranchConditional %75 %76 %77
         %76 = OpLabel
               OpBranch %71
         %77 = OpLabel
               OpBranch %70
         %78 = OpLabel
               OpBranch %69
         %69 = OpLabel
               OpBranch %70
         %70 = OpLabel
               OpBranchConditional %false %67 %71
         %71 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%test_else_b = OpFunction %bool None %29
         %79 = OpLabel
               OpBranch %80
         %80 = OpLabel
               OpLoopMerge %84 %83 None
               OpBranch %81
         %81 = OpLabel
         %85 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %86 = OpLoad %v4float %85
         %87 = OpCompositeExtract %float %86 1
         %88 = OpFOrdEqual %bool %87 %float_0
               OpSelectionMerge %91 None
               OpBranchConditional %88 %89 %90
         %89 = OpLabel
               OpReturnValue %false
         %90 = OpLabel
               OpReturnValue %true
         %91 = OpLabel
               OpBranch %82
         %82 = OpLabel
               OpBranch %83
         %83 = OpLabel
               OpBranchConditional %false %80 %84
         %84 = OpLabel
               OpUnreachable
               OpFunctionEnd
%test_loop_return_b = OpFunction %bool None %29
         %92 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
%test_loop_break_b = OpFunction %bool None %29
         %93 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
               OpStore %x %int_0
               OpBranch %96
         %96 = OpLabel
               OpLoopMerge %100 %99 None
               OpBranch %97
         %97 = OpLabel
        %101 = OpLoad %int %x
        %103 = OpSLessThanEqual %bool %101 %int_1
               OpBranchConditional %103 %98 %100
         %98 = OpLabel
               OpBranch %100
         %99 = OpLabel
        %104 = OpLoad %int %x
        %105 = OpIAdd %int %104 %int_1
               OpStore %x %105
               OpBranch %96
        %100 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
       %main = OpFunction %v4float None %106
        %107 = OpFunctionParameter %_ptr_Function_v2float
        %108 = OpLabel
        %138 = OpVariable %_ptr_Function_v4float Function
        %109 = OpFunctionCall %bool %test_return_b
               OpSelectionMerge %111 None
               OpBranchConditional %109 %110 %111
        %110 = OpLabel
        %112 = OpFunctionCall %bool %test_break_b
               OpBranch %111
        %111 = OpLabel
        %113 = OpPhi %bool %false %108 %112 %110
               OpSelectionMerge %115 None
               OpBranchConditional %113 %114 %115
        %114 = OpLabel
        %116 = OpFunctionCall %bool %test_continue_b
               OpBranch %115
        %115 = OpLabel
        %117 = OpPhi %bool %false %111 %116 %114
               OpSelectionMerge %119 None
               OpBranchConditional %117 %118 %119
        %118 = OpLabel
        %120 = OpFunctionCall %bool %test_if_return_b
               OpBranch %119
        %119 = OpLabel
        %121 = OpPhi %bool %false %115 %120 %118
               OpSelectionMerge %123 None
               OpBranchConditional %121 %122 %123
        %122 = OpLabel
        %124 = OpFunctionCall %bool %test_if_break_b
               OpBranch %123
        %123 = OpLabel
        %125 = OpPhi %bool %false %119 %124 %122
               OpSelectionMerge %127 None
               OpBranchConditional %125 %126 %127
        %126 = OpLabel
        %128 = OpFunctionCall %bool %test_else_b
               OpBranch %127
        %127 = OpLabel
        %129 = OpPhi %bool %false %123 %128 %126
               OpSelectionMerge %131 None
               OpBranchConditional %129 %130 %131
        %130 = OpLabel
        %132 = OpFunctionCall %bool %test_loop_return_b
               OpBranch %131
        %131 = OpLabel
        %133 = OpPhi %bool %false %127 %132 %130
               OpSelectionMerge %135 None
               OpBranchConditional %133 %134 %135
        %134 = OpLabel
        %136 = OpFunctionCall %bool %test_loop_break_b
               OpBranch %135
        %135 = OpLabel
        %137 = OpPhi %bool %false %131 %136 %134
               OpSelectionMerge %142 None
               OpBranchConditional %137 %140 %141
        %140 = OpLabel
        %143 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %144 = OpLoad %v4float %143
               OpStore %138 %144
               OpBranch %142
        %141 = OpLabel
        %145 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %146 = OpLoad %v4float %145
               OpStore %138 %146
               OpBranch %142
        %142 = OpLabel
        %147 = OpLoad %v4float %138
               OpReturnValue %147
               OpFunctionEnd
