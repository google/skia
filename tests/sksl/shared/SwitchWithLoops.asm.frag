               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %switch_with_continue_in_loop_bi "switch_with_continue_in_loop_bi"
               OpName %val "val"
               OpName %i "i"
               OpName %loop_with_break_in_switch_bi "loop_with_break_in_switch_bi"
               OpName %val_0 "val"
               OpName %i_0 "i"
               OpName %main "main"
               OpName %x "x"
               OpName %_0_val "_0_val"
               OpName %_1_i "_1_i"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
         %27 = OpTypeFunction %bool %_ptr_Function_int
      %int_0 = OpConstant %int 0
     %int_10 = OpConstant %int 10
      %int_1 = OpConstant %int 1
     %int_11 = OpConstant %int 11
      %false = OpConstantFalse %bool
     %int_20 = OpConstant %int 20
         %80 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_2 = OpConstant %int 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %17
         %18 = OpLabel
         %22 = OpVariable %_ptr_Function_v2float Function
               OpStore %22 %21
         %24 = OpFunctionCall %v4float %main %22
               OpStore %sk_FragColor %24
               OpReturn
               OpFunctionEnd
%switch_with_continue_in_loop_bi = OpFunction %bool None %27
         %28 = OpFunctionParameter %_ptr_Function_int
         %29 = OpLabel
        %val = OpVariable %_ptr_Function_int Function
          %i = OpVariable %_ptr_Function_int Function
               OpStore %val %int_0
         %32 = OpLoad %int %28
               OpSelectionMerge %33 None
               OpSwitch %32 %35 1 %34
         %34 = OpLabel
               OpStore %i %int_0
               OpBranch %37
         %37 = OpLabel
               OpLoopMerge %41 %40 None
               OpBranch %38
         %38 = OpLabel
         %42 = OpLoad %int %i
         %44 = OpSLessThan %bool %42 %int_10
               OpBranchConditional %44 %39 %41
         %39 = OpLabel
         %46 = OpLoad %int %val
         %47 = OpIAdd %int %46 %int_1
               OpStore %val %47
               OpBranch %40
         %40 = OpLabel
         %48 = OpLoad %int %i
         %49 = OpIAdd %int %48 %int_1
               OpStore %i %49
               OpBranch %37
         %41 = OpLabel
               OpBranch %35
         %35 = OpLabel
         %50 = OpLoad %int %val
         %51 = OpIAdd %int %50 %int_1
               OpStore %val %51
               OpBranch %33
         %33 = OpLabel
         %52 = OpLoad %int %val
         %54 = OpIEqual %bool %52 %int_11
               OpReturnValue %54
               OpFunctionEnd
%loop_with_break_in_switch_bi = OpFunction %bool None %27
         %55 = OpFunctionParameter %_ptr_Function_int
         %56 = OpLabel
      %val_0 = OpVariable %_ptr_Function_int Function
        %i_0 = OpVariable %_ptr_Function_int Function
               OpStore %val_0 %int_0
               OpStore %i_0 %int_0
               OpBranch %59
         %59 = OpLabel
               OpLoopMerge %63 %62 None
               OpBranch %60
         %60 = OpLabel
         %64 = OpLoad %int %i_0
         %65 = OpSLessThan %bool %64 %int_10
               OpBranchConditional %65 %61 %63
         %61 = OpLabel
         %66 = OpLoad %int %55
               OpSelectionMerge %67 None
               OpSwitch %66 %69 1 %68
         %68 = OpLabel
         %70 = OpLoad %int %val_0
         %71 = OpIAdd %int %70 %int_1
               OpStore %val_0 %71
               OpBranch %67
         %69 = OpLabel
               OpReturnValue %false
         %67 = OpLabel
         %73 = OpLoad %int %val_0
         %74 = OpIAdd %int %73 %int_1
               OpStore %val_0 %74
               OpBranch %62
         %62 = OpLabel
         %75 = OpLoad %int %i_0
         %76 = OpIAdd %int %75 %int_1
               OpStore %i_0 %76
               OpBranch %59
         %63 = OpLabel
         %77 = OpLoad %int %val_0
         %79 = OpIEqual %bool %77 %int_20
               OpReturnValue %79
               OpFunctionEnd
       %main = OpFunction %v4float None %80
         %81 = OpFunctionParameter %_ptr_Function_v2float
         %82 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
     %_0_val = OpVariable %_ptr_Function_int Function
       %_1_i = OpVariable %_ptr_Function_int Function
        %113 = OpVariable %_ptr_Function_int Function
        %119 = OpVariable %_ptr_Function_int Function
        %122 = OpVariable %_ptr_Function_v4float Function
         %84 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %86 = OpLoad %v4float %84
         %87 = OpCompositeExtract %float %86 1
         %88 = OpConvertFToS %int %87
               OpStore %x %88
               OpStore %_0_val %int_0
               OpSelectionMerge %90 None
               OpSwitch %88 %92 1 %91
         %91 = OpLabel
               OpStore %_1_i %int_0
               OpBranch %94
         %94 = OpLabel
               OpLoopMerge %98 %97 None
               OpBranch %95
         %95 = OpLabel
         %99 = OpLoad %int %_1_i
        %100 = OpSLessThan %bool %99 %int_10
               OpBranchConditional %100 %96 %98
         %96 = OpLabel
        %101 = OpLoad %int %_0_val
        %102 = OpIAdd %int %101 %int_1
               OpStore %_0_val %102
               OpBranch %98
         %97 = OpLabel
        %103 = OpLoad %int %_1_i
        %104 = OpIAdd %int %103 %int_1
               OpStore %_1_i %104
               OpBranch %94
         %98 = OpLabel
               OpBranch %92
         %92 = OpLabel
        %105 = OpLoad %int %_0_val
        %106 = OpIAdd %int %105 %int_1
               OpStore %_0_val %106
               OpBranch %90
         %90 = OpLabel
        %107 = OpLoad %int %_0_val
        %109 = OpIEqual %bool %107 %int_2
               OpSelectionMerge %111 None
               OpBranchConditional %109 %110 %111
        %110 = OpLabel
        %112 = OpLoad %int %x
               OpStore %113 %112
        %114 = OpFunctionCall %bool %switch_with_continue_in_loop_bi %113
               OpBranch %111
        %111 = OpLabel
        %115 = OpPhi %bool %false %90 %114 %110
               OpSelectionMerge %117 None
               OpBranchConditional %115 %116 %117
        %116 = OpLabel
        %118 = OpLoad %int %x
               OpStore %119 %118
        %120 = OpFunctionCall %bool %loop_with_break_in_switch_bi %119
               OpBranch %117
        %117 = OpLabel
        %121 = OpPhi %bool %false %111 %120 %116
               OpSelectionMerge %126 None
               OpBranchConditional %121 %124 %125
        %124 = OpLabel
        %127 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %128 = OpLoad %v4float %127
               OpStore %122 %128
               OpBranch %126
        %125 = OpLabel
        %129 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %130 = OpLoad %v4float %129
               OpStore %122 %130
               OpBranch %126
        %126 = OpLabel
        %131 = OpLoad %v4float %122
               OpReturnValue %131
               OpFunctionEnd
