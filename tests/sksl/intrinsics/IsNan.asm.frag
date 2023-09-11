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
               OpName %valueIsNaN "valueIsNaN"
               OpName %valueIsNumber "valueIsNumber"
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
               OpDecorate %valueIsNaN RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %valueIsNumber RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %34 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
    %float_1 = OpConstant %float 1
         %40 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
      %false = OpConstantFalse %bool
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
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
 %valueIsNaN = OpVariable %_ptr_Function_v4float Function
%valueIsNumber = OpVariable %_ptr_Function_v4float Function
         %92 = OpVariable %_ptr_Function_v4float Function
         %28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %32 = OpLoad %v4float %28
         %33 = OpVectorShuffle %v4float %32 %32 1 1 1 1
         %35 = OpFDiv %v4float %34 %33
               OpStore %valueIsNaN %35
         %38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %39 = OpLoad %v4float %38
         %41 = OpFDiv %v4float %40 %39
               OpStore %valueIsNumber %41
         %44 = OpCompositeExtract %float %35 0
         %43 = OpIsNan %bool %44
               OpSelectionMerge %46 None
               OpBranchConditional %43 %45 %46
         %45 = OpLabel
         %49 = OpVectorShuffle %v2float %35 %35 0 1
         %48 = OpIsNan %v2bool %49
         %47 = OpAll %bool %48
               OpBranch %46
         %46 = OpLabel
         %51 = OpPhi %bool %false %25 %47 %45
               OpSelectionMerge %53 None
               OpBranchConditional %51 %52 %53
         %52 = OpLabel
         %56 = OpVectorShuffle %v3float %35 %35 0 1 2
         %55 = OpIsNan %v3bool %56
         %54 = OpAll %bool %55
               OpBranch %53
         %53 = OpLabel
         %59 = OpPhi %bool %false %46 %54 %52
               OpSelectionMerge %61 None
               OpBranchConditional %59 %60 %61
         %60 = OpLabel
         %63 = OpIsNan %v4bool %35
         %62 = OpAll %bool %63
               OpBranch %61
         %61 = OpLabel
         %65 = OpPhi %bool %false %53 %62 %60
               OpSelectionMerge %67 None
               OpBranchConditional %65 %66 %67
         %66 = OpLabel
         %70 = OpCompositeExtract %float %41 0
         %69 = OpIsNan %bool %70
         %68 = OpLogicalNot %bool %69
               OpBranch %67
         %67 = OpLabel
         %71 = OpPhi %bool %false %61 %68 %66
               OpSelectionMerge %73 None
               OpBranchConditional %71 %72 %73
         %72 = OpLabel
         %77 = OpVectorShuffle %v2float %41 %41 0 1
         %76 = OpIsNan %v2bool %77
         %75 = OpAny %bool %76
         %74 = OpLogicalNot %bool %75
               OpBranch %73
         %73 = OpLabel
         %78 = OpPhi %bool %false %67 %74 %72
               OpSelectionMerge %80 None
               OpBranchConditional %78 %79 %80
         %79 = OpLabel
         %84 = OpVectorShuffle %v3float %41 %41 0 1 2
         %83 = OpIsNan %v3bool %84
         %82 = OpAny %bool %83
         %81 = OpLogicalNot %bool %82
               OpBranch %80
         %80 = OpLabel
         %85 = OpPhi %bool %false %73 %81 %79
               OpSelectionMerge %87 None
               OpBranchConditional %85 %86 %87
         %86 = OpLabel
         %90 = OpIsNan %v4bool %41
         %89 = OpAny %bool %90
         %88 = OpLogicalNot %bool %89
               OpBranch %87
         %87 = OpLabel
         %91 = OpPhi %bool %false %80 %88 %86
               OpSelectionMerge %95 None
               OpBranchConditional %91 %93 %94
         %93 = OpLabel
         %96 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %98 = OpLoad %v4float %96
               OpStore %92 %98
               OpBranch %95
         %94 = OpLabel
         %99 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %101 = OpLoad %v4float %99
               OpStore %92 %101
               OpBranch %95
         %95 = OpLabel
        %102 = OpLoad %v4float %92
               OpReturnValue %102
               OpFunctionEnd
