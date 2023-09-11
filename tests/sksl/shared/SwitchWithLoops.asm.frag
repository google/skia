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
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %9 Binding 0
               OpDecorate %9 DescriptorSet 0
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %9 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
         %25 = OpTypeFunction %bool %_ptr_Function_int
      %int_0 = OpConstant %int 0
     %int_10 = OpConstant %int 10
      %int_1 = OpConstant %int 1
     %int_11 = OpConstant %int 11
      %false = OpConstantFalse %bool
     %int_20 = OpConstant %int 20
         %78 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_2 = OpConstant %int 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %14
         %15 = OpLabel
         %19 = OpVariable %_ptr_Function_v2float Function
               OpStore %19 %18
         %21 = OpFunctionCall %v4float %main %19
               OpStore %sk_FragColor %21
               OpReturn
               OpFunctionEnd
%switch_with_continue_in_loop_bi = OpFunction %bool None %25
         %26 = OpFunctionParameter %_ptr_Function_int
         %27 = OpLabel
        %val = OpVariable %_ptr_Function_int Function
          %i = OpVariable %_ptr_Function_int Function
               OpStore %val %int_0
         %30 = OpLoad %int %26
               OpSelectionMerge %31 None
               OpSwitch %30 %33 1 %32
         %32 = OpLabel
               OpStore %i %int_0
               OpBranch %35
         %35 = OpLabel
               OpLoopMerge %39 %38 None
               OpBranch %36
         %36 = OpLabel
         %40 = OpLoad %int %i
         %42 = OpSLessThan %bool %40 %int_10
               OpBranchConditional %42 %37 %39
         %37 = OpLabel
         %44 = OpLoad %int %val
         %45 = OpIAdd %int %44 %int_1
               OpStore %val %45
               OpBranch %38
         %38 = OpLabel
         %46 = OpLoad %int %i
         %47 = OpIAdd %int %46 %int_1
               OpStore %i %47
               OpBranch %35
         %39 = OpLabel
               OpBranch %33
         %33 = OpLabel
         %48 = OpLoad %int %val
         %49 = OpIAdd %int %48 %int_1
               OpStore %val %49
               OpBranch %31
         %31 = OpLabel
         %50 = OpLoad %int %val
         %52 = OpIEqual %bool %50 %int_11
               OpReturnValue %52
               OpFunctionEnd
%loop_with_break_in_switch_bi = OpFunction %bool None %25
         %53 = OpFunctionParameter %_ptr_Function_int
         %54 = OpLabel
      %val_0 = OpVariable %_ptr_Function_int Function
        %i_0 = OpVariable %_ptr_Function_int Function
               OpStore %val_0 %int_0
               OpStore %i_0 %int_0
               OpBranch %57
         %57 = OpLabel
               OpLoopMerge %61 %60 None
               OpBranch %58
         %58 = OpLabel
         %62 = OpLoad %int %i_0
         %63 = OpSLessThan %bool %62 %int_10
               OpBranchConditional %63 %59 %61
         %59 = OpLabel
         %64 = OpLoad %int %53
               OpSelectionMerge %65 None
               OpSwitch %64 %67 1 %66
         %66 = OpLabel
         %68 = OpLoad %int %val_0
         %69 = OpIAdd %int %68 %int_1
               OpStore %val_0 %69
               OpBranch %65
         %67 = OpLabel
               OpReturnValue %false
         %65 = OpLabel
         %71 = OpLoad %int %val_0
         %72 = OpIAdd %int %71 %int_1
               OpStore %val_0 %72
               OpBranch %60
         %60 = OpLabel
         %73 = OpLoad %int %i_0
         %74 = OpIAdd %int %73 %int_1
               OpStore %i_0 %74
               OpBranch %57
         %61 = OpLabel
         %75 = OpLoad %int %val_0
         %77 = OpIEqual %bool %75 %int_20
               OpReturnValue %77
               OpFunctionEnd
       %main = OpFunction %v4float None %78
         %79 = OpFunctionParameter %_ptr_Function_v2float
         %80 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
     %_0_val = OpVariable %_ptr_Function_int Function
       %_1_i = OpVariable %_ptr_Function_int Function
        %111 = OpVariable %_ptr_Function_int Function
        %117 = OpVariable %_ptr_Function_int Function
        %120 = OpVariable %_ptr_Function_v4float Function
         %82 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
         %84 = OpLoad %v4float %82
         %85 = OpCompositeExtract %float %84 1
         %86 = OpConvertFToS %int %85
               OpStore %x %86
               OpStore %_0_val %int_0
               OpSelectionMerge %88 None
               OpSwitch %86 %90 1 %89
         %89 = OpLabel
               OpStore %_1_i %int_0
               OpBranch %92
         %92 = OpLabel
               OpLoopMerge %96 %95 None
               OpBranch %93
         %93 = OpLabel
         %97 = OpLoad %int %_1_i
         %98 = OpSLessThan %bool %97 %int_10
               OpBranchConditional %98 %94 %96
         %94 = OpLabel
         %99 = OpLoad %int %_0_val
        %100 = OpIAdd %int %99 %int_1
               OpStore %_0_val %100
               OpBranch %96
         %95 = OpLabel
        %101 = OpLoad %int %_1_i
        %102 = OpIAdd %int %101 %int_1
               OpStore %_1_i %102
               OpBranch %92
         %96 = OpLabel
               OpBranch %90
         %90 = OpLabel
        %103 = OpLoad %int %_0_val
        %104 = OpIAdd %int %103 %int_1
               OpStore %_0_val %104
               OpBranch %88
         %88 = OpLabel
        %105 = OpLoad %int %_0_val
        %107 = OpIEqual %bool %105 %int_2
               OpSelectionMerge %109 None
               OpBranchConditional %107 %108 %109
        %108 = OpLabel
        %110 = OpLoad %int %x
               OpStore %111 %110
        %112 = OpFunctionCall %bool %switch_with_continue_in_loop_bi %111
               OpBranch %109
        %109 = OpLabel
        %113 = OpPhi %bool %false %88 %112 %108
               OpSelectionMerge %115 None
               OpBranchConditional %113 %114 %115
        %114 = OpLabel
        %116 = OpLoad %int %x
               OpStore %117 %116
        %118 = OpFunctionCall %bool %loop_with_break_in_switch_bi %117
               OpBranch %115
        %115 = OpLabel
        %119 = OpPhi %bool %false %109 %118 %114
               OpSelectionMerge %124 None
               OpBranchConditional %119 %122 %123
        %122 = OpLabel
        %125 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %126 = OpLoad %v4float %125
               OpStore %120 %126
               OpBranch %124
        %123 = OpLabel
        %127 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
        %128 = OpLoad %v4float %127
               OpStore %120 %128
               OpBranch %124
        %124 = OpLabel
        %129 = OpLoad %v4float %120
               OpReturnValue %129
               OpFunctionEnd
