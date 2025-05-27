               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %valueIsNaN "valueIsNaN"
               OpName %valueIsNumber "valueIsNumber"
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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %valueIsNaN RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %valueIsNumber RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %31 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
    %float_1 = OpConstant %float 1
         %37 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
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
 %valueIsNaN = OpVariable %_ptr_Function_v4float Function
%valueIsNumber = OpVariable %_ptr_Function_v4float Function
         %90 = OpVariable %_ptr_Function_v4float Function
         %25 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %29 = OpLoad %v4float %25
         %30 = OpVectorShuffle %v4float %29 %29 1 1 1 1
         %32 = OpFDiv %v4float %31 %30
               OpStore %valueIsNaN %32
         %35 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %36 = OpLoad %v4float %35
         %38 = OpFDiv %v4float %37 %36
               OpStore %valueIsNumber %38
         %42 = OpCompositeExtract %float %32 0
         %41 = OpIsNan %bool %42
               OpSelectionMerge %44 None
               OpBranchConditional %41 %43 %44
         %43 = OpLabel
         %47 = OpVectorShuffle %v2float %32 %32 0 1
         %46 = OpIsNan %v2bool %47
         %45 = OpAll %bool %46
               OpBranch %44
         %44 = OpLabel
         %49 = OpPhi %bool %false %22 %45 %43
               OpSelectionMerge %51 None
               OpBranchConditional %49 %50 %51
         %50 = OpLabel
         %54 = OpVectorShuffle %v3float %32 %32 0 1 2
         %53 = OpIsNan %v3bool %54
         %52 = OpAll %bool %53
               OpBranch %51
         %51 = OpLabel
         %57 = OpPhi %bool %false %44 %52 %50
               OpSelectionMerge %59 None
               OpBranchConditional %57 %58 %59
         %58 = OpLabel
         %61 = OpIsNan %v4bool %32
         %60 = OpAll %bool %61
               OpBranch %59
         %59 = OpLabel
         %63 = OpPhi %bool %false %51 %60 %58
               OpSelectionMerge %65 None
               OpBranchConditional %63 %64 %65
         %64 = OpLabel
         %68 = OpCompositeExtract %float %38 0
         %67 = OpIsNan %bool %68
         %66 = OpLogicalNot %bool %67
               OpBranch %65
         %65 = OpLabel
         %69 = OpPhi %bool %false %59 %66 %64
               OpSelectionMerge %71 None
               OpBranchConditional %69 %70 %71
         %70 = OpLabel
         %75 = OpVectorShuffle %v2float %38 %38 0 1
         %74 = OpIsNan %v2bool %75
         %73 = OpAny %bool %74
         %72 = OpLogicalNot %bool %73
               OpBranch %71
         %71 = OpLabel
         %76 = OpPhi %bool %false %65 %72 %70
               OpSelectionMerge %78 None
               OpBranchConditional %76 %77 %78
         %77 = OpLabel
         %82 = OpVectorShuffle %v3float %38 %38 0 1 2
         %81 = OpIsNan %v3bool %82
         %80 = OpAny %bool %81
         %79 = OpLogicalNot %bool %80
               OpBranch %78
         %78 = OpLabel
         %83 = OpPhi %bool %false %71 %79 %77
               OpSelectionMerge %85 None
               OpBranchConditional %83 %84 %85
         %84 = OpLabel
         %88 = OpIsNan %v4bool %38
         %87 = OpAny %bool %88
         %86 = OpLogicalNot %bool %87
               OpBranch %85
         %85 = OpLabel
         %89 = OpPhi %bool %false %78 %86 %84
               OpSelectionMerge %93 None
               OpBranchConditional %89 %91 %92
         %91 = OpLabel
         %94 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %96 = OpLoad %v4float %94
               OpStore %90 %96
               OpBranch %93
         %92 = OpLabel
         %97 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %99 = OpLoad %v4float %97
               OpStore %90 %99
               OpBranch %93
         %93 = OpLabel
        %100 = OpLoad %v4float %90
               OpReturnValue %100
               OpFunctionEnd
