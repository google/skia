               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testMatrix2x2"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %inputVal "inputVal"
               OpName %expected "expected"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %105 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
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
    %float_2 = OpConstant %float 2
   %float_n2 = OpConstant %float -2
    %float_1 = OpConstant %float 1
    %float_8 = OpConstant %float 8
         %37 = OpConstantComposite %v4float %float_2 %float_n2 %float_1 %float_8
    %float_3 = OpConstant %float 3
    %float_5 = OpConstant %float 5
   %float_13 = OpConstant %float 13
         %43 = OpConstantComposite %v4float %float_3 %float_3 %float_5 %float_13
      %false = OpConstantFalse %bool
%float_0_0500000007 = OpConstant %float 0.0500000007
    %v3float = OpTypeVector %float 3
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
   %inputVal = OpVariable %_ptr_Function_v4float Function
   %expected = OpVariable %_ptr_Function_v4float Function
         %99 = OpVariable %_ptr_Function_v4float Function
         %28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %32 = OpLoad %v4float %28
         %38 = OpFAdd %v4float %32 %37
               OpStore %inputVal %38
               OpStore %expected %43
         %47 = OpCompositeExtract %float %38 0
         %46 = OpExtInst %float %1 Length %47
         %48 = OpFSub %float %46 %float_3
         %45 = OpExtInst %float %1 FAbs %48
         %50 = OpFOrdLessThan %bool %45 %float_0_0500000007
               OpSelectionMerge %52 None
               OpBranchConditional %50 %51 %52
         %51 = OpLabel
         %55 = OpVectorShuffle %v2float %38 %38 0 1
         %54 = OpExtInst %float %1 Length %55
         %56 = OpFSub %float %54 %float_3
         %53 = OpExtInst %float %1 FAbs %56
         %57 = OpFOrdLessThan %bool %53 %float_0_0500000007
               OpBranch %52
         %52 = OpLabel
         %58 = OpPhi %bool %false %25 %57 %51
               OpSelectionMerge %60 None
               OpBranchConditional %58 %59 %60
         %59 = OpLabel
         %63 = OpVectorShuffle %v3float %38 %38 0 1 2
         %62 = OpExtInst %float %1 Length %63
         %65 = OpFSub %float %62 %float_5
         %61 = OpExtInst %float %1 FAbs %65
         %66 = OpFOrdLessThan %bool %61 %float_0_0500000007
               OpBranch %60
         %60 = OpLabel
         %67 = OpPhi %bool %false %52 %66 %59
               OpSelectionMerge %69 None
               OpBranchConditional %67 %68 %69
         %68 = OpLabel
         %71 = OpExtInst %float %1 Length %38
         %72 = OpFSub %float %71 %float_13
         %70 = OpExtInst %float %1 FAbs %72
         %73 = OpFOrdLessThan %bool %70 %float_0_0500000007
               OpBranch %69
         %69 = OpLabel
         %74 = OpPhi %bool %false %60 %73 %68
               OpSelectionMerge %76 None
               OpBranchConditional %74 %75 %76
         %75 = OpLabel
         %78 = OpFSub %float %float_3 %float_3
         %77 = OpExtInst %float %1 FAbs %78
         %79 = OpFOrdLessThan %bool %77 %float_0_0500000007
               OpBranch %76
         %76 = OpLabel
         %80 = OpPhi %bool %false %69 %79 %75
               OpSelectionMerge %82 None
               OpBranchConditional %80 %81 %82
         %81 = OpLabel
         %84 = OpFSub %float %float_3 %float_3
         %83 = OpExtInst %float %1 FAbs %84
         %85 = OpFOrdLessThan %bool %83 %float_0_0500000007
               OpBranch %82
         %82 = OpLabel
         %86 = OpPhi %bool %false %76 %85 %81
               OpSelectionMerge %88 None
               OpBranchConditional %86 %87 %88
         %87 = OpLabel
         %90 = OpFSub %float %float_5 %float_5
         %89 = OpExtInst %float %1 FAbs %90
         %91 = OpFOrdLessThan %bool %89 %float_0_0500000007
               OpBranch %88
         %88 = OpLabel
         %92 = OpPhi %bool %false %82 %91 %87
               OpSelectionMerge %94 None
               OpBranchConditional %92 %93 %94
         %93 = OpLabel
         %96 = OpFSub %float %float_13 %float_13
         %95 = OpExtInst %float %1 FAbs %96
         %97 = OpFOrdLessThan %bool %95 %float_0_0500000007
               OpBranch %94
         %94 = OpLabel
         %98 = OpPhi %bool %false %88 %97 %93
               OpSelectionMerge %102 None
               OpBranchConditional %98 %100 %101
        %100 = OpLabel
        %103 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %105 = OpLoad %v4float %103
               OpStore %99 %105
               OpBranch %102
        %101 = OpLabel
        %106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %108 = OpLoad %v4float %106
               OpStore %99 %108
               OpBranch %102
        %102 = OpLabel
        %109 = OpLoad %v4float %99
               OpReturnValue %109
               OpFunctionEnd
