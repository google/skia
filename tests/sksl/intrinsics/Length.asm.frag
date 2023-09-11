               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testMatrix2x2"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %inputVal "inputVal"
               OpName %expected "expected"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %103 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
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
    %float_2 = OpConstant %float 2
   %float_n2 = OpConstant %float -2
    %float_1 = OpConstant %float 1
    %float_8 = OpConstant %float 8
         %34 = OpConstantComposite %v4float %float_2 %float_n2 %float_1 %float_8
    %float_3 = OpConstant %float 3
    %float_5 = OpConstant %float 5
   %float_13 = OpConstant %float 13
         %40 = OpConstantComposite %v4float %float_3 %float_3 %float_5 %float_13
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%float_0_0500000007 = OpConstant %float 0.0500000007
    %v3float = OpTypeVector %float 3
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
   %inputVal = OpVariable %_ptr_Function_v4float Function
   %expected = OpVariable %_ptr_Function_v4float Function
         %97 = OpVariable %_ptr_Function_v4float Function
         %25 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %29 = OpLoad %v4float %25
         %35 = OpFAdd %v4float %29 %34
               OpStore %inputVal %35
               OpStore %expected %40
         %45 = OpCompositeExtract %float %35 0
         %44 = OpExtInst %float %1 Length %45
         %46 = OpFSub %float %44 %float_3
         %43 = OpExtInst %float %1 FAbs %46
         %48 = OpFOrdLessThan %bool %43 %float_0_0500000007
               OpSelectionMerge %50 None
               OpBranchConditional %48 %49 %50
         %49 = OpLabel
         %53 = OpVectorShuffle %v2float %35 %35 0 1
         %52 = OpExtInst %float %1 Length %53
         %54 = OpFSub %float %52 %float_3
         %51 = OpExtInst %float %1 FAbs %54
         %55 = OpFOrdLessThan %bool %51 %float_0_0500000007
               OpBranch %50
         %50 = OpLabel
         %56 = OpPhi %bool %false %22 %55 %49
               OpSelectionMerge %58 None
               OpBranchConditional %56 %57 %58
         %57 = OpLabel
         %61 = OpVectorShuffle %v3float %35 %35 0 1 2
         %60 = OpExtInst %float %1 Length %61
         %63 = OpFSub %float %60 %float_5
         %59 = OpExtInst %float %1 FAbs %63
         %64 = OpFOrdLessThan %bool %59 %float_0_0500000007
               OpBranch %58
         %58 = OpLabel
         %65 = OpPhi %bool %false %50 %64 %57
               OpSelectionMerge %67 None
               OpBranchConditional %65 %66 %67
         %66 = OpLabel
         %69 = OpExtInst %float %1 Length %35
         %70 = OpFSub %float %69 %float_13
         %68 = OpExtInst %float %1 FAbs %70
         %71 = OpFOrdLessThan %bool %68 %float_0_0500000007
               OpBranch %67
         %67 = OpLabel
         %72 = OpPhi %bool %false %58 %71 %66
               OpSelectionMerge %74 None
               OpBranchConditional %72 %73 %74
         %73 = OpLabel
         %76 = OpFSub %float %float_3 %float_3
         %75 = OpExtInst %float %1 FAbs %76
         %77 = OpFOrdLessThan %bool %75 %float_0_0500000007
               OpBranch %74
         %74 = OpLabel
         %78 = OpPhi %bool %false %67 %77 %73
               OpSelectionMerge %80 None
               OpBranchConditional %78 %79 %80
         %79 = OpLabel
         %82 = OpFSub %float %float_3 %float_3
         %81 = OpExtInst %float %1 FAbs %82
         %83 = OpFOrdLessThan %bool %81 %float_0_0500000007
               OpBranch %80
         %80 = OpLabel
         %84 = OpPhi %bool %false %74 %83 %79
               OpSelectionMerge %86 None
               OpBranchConditional %84 %85 %86
         %85 = OpLabel
         %88 = OpFSub %float %float_5 %float_5
         %87 = OpExtInst %float %1 FAbs %88
         %89 = OpFOrdLessThan %bool %87 %float_0_0500000007
               OpBranch %86
         %86 = OpLabel
         %90 = OpPhi %bool %false %80 %89 %85
               OpSelectionMerge %92 None
               OpBranchConditional %90 %91 %92
         %91 = OpLabel
         %94 = OpFSub %float %float_13 %float_13
         %93 = OpExtInst %float %1 FAbs %94
         %95 = OpFOrdLessThan %bool %93 %float_0_0500000007
               OpBranch %92
         %92 = OpLabel
         %96 = OpPhi %bool %false %86 %95 %91
               OpSelectionMerge %100 None
               OpBranchConditional %96 %98 %99
         %98 = OpLabel
        %101 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %103 = OpLoad %v4float %101
               OpStore %97 %103
               OpBranch %100
         %99 = OpLabel
        %104 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %106 = OpLoad %v4float %104
               OpStore %97 %106
               OpBranch %100
        %100 = OpLabel
        %107 = OpLoad %v4float %97
               OpReturnValue %107
               OpFunctionEnd
