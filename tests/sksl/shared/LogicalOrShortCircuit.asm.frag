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
               OpName %TrueFalse_b "TrueFalse_b"
               OpName %x "x"
               OpName %y "y"
               OpName %FalseTrue_b "FalseTrue_b"
               OpName %x_0 "x"
               OpName %y_0 "y"
               OpName %FalseFalse_b "FalseFalse_b"
               OpName %x_1 "x"
               OpName %y_1 "y"
               OpName %main "main"
               OpName %_0_TrueTrue "_0_TrueTrue"
               OpName %_2_y "_2_y"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %108 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %24 = OpTypeFunction %bool
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_1 = OpConstant %int 1
       %true = OpConstantTrue %bool
      %int_3 = OpConstant %int 3
      %false = OpConstantFalse %bool
      %int_2 = OpConstant %int 2
         %82 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
%TrueFalse_b = OpFunction %bool None %24
         %25 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
          %y = OpVariable %_ptr_Function_int Function
               OpStore %x %int_1
               OpStore %y %int_1
               OpSelectionMerge %33 None
               OpBranchConditional %true %33 %32
         %32 = OpLabel
         %34 = OpIAdd %int %int_1 %int_1
               OpStore %y %34
         %36 = OpIEqual %bool %34 %int_3
               OpBranch %33
         %33 = OpLabel
         %37 = OpPhi %bool %true %25 %36 %32
               OpSelectionMerge %40 None
               OpBranchConditional %37 %38 %39
         %38 = OpLabel
               OpSelectionMerge %43 None
               OpBranchConditional %true %42 %43
         %42 = OpLabel
         %44 = OpLoad %int %y
         %45 = OpIEqual %bool %44 %int_1
               OpBranch %43
         %43 = OpLabel
         %46 = OpPhi %bool %false %38 %45 %42
               OpReturnValue %46
         %39 = OpLabel
               OpReturnValue %false
         %40 = OpLabel
               OpUnreachable
               OpFunctionEnd
%FalseTrue_b = OpFunction %bool None %24
         %47 = OpLabel
        %x_0 = OpVariable %_ptr_Function_int Function
        %y_0 = OpVariable %_ptr_Function_int Function
               OpStore %x_0 %int_1
               OpStore %y_0 %int_1
         %51 = OpIEqual %bool %int_1 %int_2
               OpSelectionMerge %53 None
               OpBranchConditional %51 %53 %52
         %52 = OpLabel
         %54 = OpIAdd %int %int_1 %int_1
               OpStore %y_0 %54
         %55 = OpIEqual %bool %54 %int_2
               OpBranch %53
         %53 = OpLabel
         %56 = OpPhi %bool %true %47 %55 %52
               OpSelectionMerge %59 None
               OpBranchConditional %56 %57 %58
         %57 = OpLabel
               OpSelectionMerge %61 None
               OpBranchConditional %true %60 %61
         %60 = OpLabel
         %62 = OpLoad %int %y_0
         %63 = OpIEqual %bool %62 %int_2
               OpBranch %61
         %61 = OpLabel
         %64 = OpPhi %bool %false %57 %63 %60
               OpReturnValue %64
         %58 = OpLabel
               OpReturnValue %false
         %59 = OpLabel
               OpUnreachable
               OpFunctionEnd
%FalseFalse_b = OpFunction %bool None %24
         %65 = OpLabel
        %x_1 = OpVariable %_ptr_Function_int Function
        %y_1 = OpVariable %_ptr_Function_int Function
               OpStore %x_1 %int_1
               OpStore %y_1 %int_1
         %68 = OpIEqual %bool %int_1 %int_2
               OpSelectionMerge %70 None
               OpBranchConditional %68 %70 %69
         %69 = OpLabel
         %71 = OpIAdd %int %int_1 %int_1
               OpStore %y_1 %71
         %72 = OpIEqual %bool %71 %int_3
               OpBranch %70
         %70 = OpLabel
         %73 = OpPhi %bool %true %65 %72 %69
               OpSelectionMerge %76 None
               OpBranchConditional %73 %74 %75
         %74 = OpLabel
               OpReturnValue %false
         %75 = OpLabel
               OpSelectionMerge %78 None
               OpBranchConditional %true %77 %78
         %77 = OpLabel
         %79 = OpLoad %int %y_1
         %80 = OpIEqual %bool %79 %int_2
               OpBranch %78
         %78 = OpLabel
         %81 = OpPhi %bool %false %75 %80 %77
               OpReturnValue %81
         %76 = OpLabel
               OpUnreachable
               OpFunctionEnd
       %main = OpFunction %v4float None %82
         %83 = OpFunctionParameter %_ptr_Function_v2float
         %84 = OpLabel
%_0_TrueTrue = OpVariable %_ptr_Function_bool Function
       %_2_y = OpVariable %_ptr_Function_int Function
        %100 = OpVariable %_ptr_Function_v4float Function
               OpStore %_2_y %int_1
               OpStore %_0_TrueTrue %true
               OpSelectionMerge %89 None
               OpBranchConditional %true %88 %89
         %88 = OpLabel
         %90 = OpFunctionCall %bool %TrueFalse_b
               OpBranch %89
         %89 = OpLabel
         %91 = OpPhi %bool %false %84 %90 %88
               OpSelectionMerge %93 None
               OpBranchConditional %91 %92 %93
         %92 = OpLabel
         %94 = OpFunctionCall %bool %FalseTrue_b
               OpBranch %93
         %93 = OpLabel
         %95 = OpPhi %bool %false %89 %94 %92
               OpSelectionMerge %97 None
               OpBranchConditional %95 %96 %97
         %96 = OpLabel
         %98 = OpFunctionCall %bool %FalseFalse_b
               OpBranch %97
         %97 = OpLabel
         %99 = OpPhi %bool %false %93 %98 %96
               OpSelectionMerge %104 None
               OpBranchConditional %99 %102 %103
        %102 = OpLabel
        %105 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %108 = OpLoad %v4float %105
               OpStore %100 %108
               OpBranch %104
        %103 = OpLabel
        %109 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %110 = OpLoad %v4float %109
               OpStore %100 %110
               OpBranch %104
        %104 = OpLabel
        %111 = OpLoad %v4float %100
               OpReturnValue %111
               OpFunctionEnd
