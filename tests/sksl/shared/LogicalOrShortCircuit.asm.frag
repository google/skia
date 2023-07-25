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
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %13 Binding 0
               OpDecorate %13 DescriptorSet 0
               OpDecorate %110 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %26 = OpTypeFunction %bool
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_1 = OpConstant %int 1
       %true = OpConstantTrue %bool
      %int_3 = OpConstant %int 3
      %false = OpConstantFalse %bool
      %int_2 = OpConstant %int 2
         %84 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %18
         %19 = OpLabel
         %23 = OpVariable %_ptr_Function_v2float Function
               OpStore %23 %22
         %25 = OpFunctionCall %v4float %main %23
               OpStore %sk_FragColor %25
               OpReturn
               OpFunctionEnd
%TrueFalse_b = OpFunction %bool None %26
         %27 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
          %y = OpVariable %_ptr_Function_int Function
               OpStore %x %int_1
               OpStore %y %int_1
               OpSelectionMerge %35 None
               OpBranchConditional %true %35 %34
         %34 = OpLabel
         %36 = OpIAdd %int %int_1 %int_1
               OpStore %y %36
         %38 = OpIEqual %bool %36 %int_3
               OpBranch %35
         %35 = OpLabel
         %39 = OpPhi %bool %true %27 %38 %34
               OpSelectionMerge %42 None
               OpBranchConditional %39 %40 %41
         %40 = OpLabel
               OpSelectionMerge %45 None
               OpBranchConditional %true %44 %45
         %44 = OpLabel
         %46 = OpLoad %int %y
         %47 = OpIEqual %bool %46 %int_1
               OpBranch %45
         %45 = OpLabel
         %48 = OpPhi %bool %false %40 %47 %44
               OpReturnValue %48
         %41 = OpLabel
               OpReturnValue %false
         %42 = OpLabel
               OpUnreachable
               OpFunctionEnd
%FalseTrue_b = OpFunction %bool None %26
         %49 = OpLabel
        %x_0 = OpVariable %_ptr_Function_int Function
        %y_0 = OpVariable %_ptr_Function_int Function
               OpStore %x_0 %int_1
               OpStore %y_0 %int_1
         %53 = OpIEqual %bool %int_1 %int_2
               OpSelectionMerge %55 None
               OpBranchConditional %53 %55 %54
         %54 = OpLabel
         %56 = OpIAdd %int %int_1 %int_1
               OpStore %y_0 %56
         %57 = OpIEqual %bool %56 %int_2
               OpBranch %55
         %55 = OpLabel
         %58 = OpPhi %bool %true %49 %57 %54
               OpSelectionMerge %61 None
               OpBranchConditional %58 %59 %60
         %59 = OpLabel
               OpSelectionMerge %63 None
               OpBranchConditional %true %62 %63
         %62 = OpLabel
         %64 = OpLoad %int %y_0
         %65 = OpIEqual %bool %64 %int_2
               OpBranch %63
         %63 = OpLabel
         %66 = OpPhi %bool %false %59 %65 %62
               OpReturnValue %66
         %60 = OpLabel
               OpReturnValue %false
         %61 = OpLabel
               OpUnreachable
               OpFunctionEnd
%FalseFalse_b = OpFunction %bool None %26
         %67 = OpLabel
        %x_1 = OpVariable %_ptr_Function_int Function
        %y_1 = OpVariable %_ptr_Function_int Function
               OpStore %x_1 %int_1
               OpStore %y_1 %int_1
         %70 = OpIEqual %bool %int_1 %int_2
               OpSelectionMerge %72 None
               OpBranchConditional %70 %72 %71
         %71 = OpLabel
         %73 = OpIAdd %int %int_1 %int_1
               OpStore %y_1 %73
         %74 = OpIEqual %bool %73 %int_3
               OpBranch %72
         %72 = OpLabel
         %75 = OpPhi %bool %true %67 %74 %71
               OpSelectionMerge %78 None
               OpBranchConditional %75 %76 %77
         %76 = OpLabel
               OpReturnValue %false
         %77 = OpLabel
               OpSelectionMerge %80 None
               OpBranchConditional %true %79 %80
         %79 = OpLabel
         %81 = OpLoad %int %y_1
         %82 = OpIEqual %bool %81 %int_2
               OpBranch %80
         %80 = OpLabel
         %83 = OpPhi %bool %false %77 %82 %79
               OpReturnValue %83
         %78 = OpLabel
               OpUnreachable
               OpFunctionEnd
       %main = OpFunction %v4float None %84
         %85 = OpFunctionParameter %_ptr_Function_v2float
         %86 = OpLabel
%_0_TrueTrue = OpVariable %_ptr_Function_bool Function
       %_2_y = OpVariable %_ptr_Function_int Function
        %102 = OpVariable %_ptr_Function_v4float Function
               OpStore %_2_y %int_1
               OpStore %_0_TrueTrue %true
               OpSelectionMerge %91 None
               OpBranchConditional %true %90 %91
         %90 = OpLabel
         %92 = OpFunctionCall %bool %TrueFalse_b
               OpBranch %91
         %91 = OpLabel
         %93 = OpPhi %bool %false %86 %92 %90
               OpSelectionMerge %95 None
               OpBranchConditional %93 %94 %95
         %94 = OpLabel
         %96 = OpFunctionCall %bool %FalseTrue_b
               OpBranch %95
         %95 = OpLabel
         %97 = OpPhi %bool %false %91 %96 %94
               OpSelectionMerge %99 None
               OpBranchConditional %97 %98 %99
         %98 = OpLabel
        %100 = OpFunctionCall %bool %FalseFalse_b
               OpBranch %99
         %99 = OpLabel
        %101 = OpPhi %bool %false %95 %100 %98
               OpSelectionMerge %106 None
               OpBranchConditional %101 %104 %105
        %104 = OpLabel
        %107 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
        %110 = OpLoad %v4float %107
               OpStore %102 %110
               OpBranch %106
        %105 = OpLabel
        %111 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
        %112 = OpLoad %v4float %111
               OpStore %102 %112
               OpBranch %106
        %106 = OpLabel
        %113 = OpLoad %v4float %102
               OpReturnValue %113
               OpFunctionEnd
