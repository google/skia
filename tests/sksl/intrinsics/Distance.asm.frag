               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "pos1"
               OpMemberName %_UniformBuffer 1 "pos2"
               OpMemberName %_UniformBuffer 2 "colorGreen"
               OpMemberName %_UniformBuffer 3 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %expected "expected"
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
               OpMemberDecorate %_UniformBuffer 3 Offset 48
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %expected RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
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
    %float_3 = OpConstant %float 3
    %float_5 = OpConstant %float 5
   %float_13 = OpConstant %float 13
         %31 = OpConstantComposite %v4float %float_3 %float_3 %float_5 %float_13
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
    %v3float = OpTypeVector %float 3
       %true = OpConstantTrue %bool
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
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
   %expected = OpVariable %_ptr_Function_v4float Function
         %90 = OpVariable %_ptr_Function_v4float Function
               OpStore %expected %31
         %34 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %38 = OpLoad %v4float %34
         %39 = OpCompositeExtract %float %38 0
         %40 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %42 = OpLoad %v4float %40
         %43 = OpCompositeExtract %float %42 0
         %33 = OpExtInst %float %1 Distance %39 %43
         %44 = OpFOrdEqual %bool %33 %float_3
               OpSelectionMerge %46 None
               OpBranchConditional %44 %45 %46
         %45 = OpLabel
         %48 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %49 = OpLoad %v4float %48
         %50 = OpVectorShuffle %v2float %49 %49 0 1
         %51 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %52 = OpLoad %v4float %51
         %53 = OpVectorShuffle %v2float %52 %52 0 1
         %47 = OpExtInst %float %1 Distance %50 %53
         %54 = OpFOrdEqual %bool %47 %float_3
               OpBranch %46
         %46 = OpLabel
         %55 = OpPhi %bool %false %25 %54 %45
               OpSelectionMerge %57 None
               OpBranchConditional %55 %56 %57
         %56 = OpLabel
         %59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %60 = OpLoad %v4float %59
         %61 = OpVectorShuffle %v3float %60 %60 0 1 2
         %63 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %64 = OpLoad %v4float %63
         %65 = OpVectorShuffle %v3float %64 %64 0 1 2
         %58 = OpExtInst %float %1 Distance %61 %65
         %66 = OpFOrdEqual %bool %58 %float_5
               OpBranch %57
         %57 = OpLabel
         %67 = OpPhi %bool %false %46 %66 %56
               OpSelectionMerge %69 None
               OpBranchConditional %67 %68 %69
         %68 = OpLabel
         %71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %72 = OpLoad %v4float %71
         %73 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %74 = OpLoad %v4float %73
         %70 = OpExtInst %float %1 Distance %72 %74
         %75 = OpFOrdEqual %bool %70 %float_13
               OpBranch %69
         %69 = OpLabel
         %76 = OpPhi %bool %false %57 %75 %68
               OpSelectionMerge %78 None
               OpBranchConditional %76 %77 %78
         %77 = OpLabel
               OpBranch %78
         %78 = OpLabel
         %80 = OpPhi %bool %false %69 %true %77
               OpSelectionMerge %82 None
               OpBranchConditional %80 %81 %82
         %81 = OpLabel
               OpBranch %82
         %82 = OpLabel
         %83 = OpPhi %bool %false %78 %true %81
               OpSelectionMerge %85 None
               OpBranchConditional %83 %84 %85
         %84 = OpLabel
               OpBranch %85
         %85 = OpLabel
         %86 = OpPhi %bool %false %82 %true %84
               OpSelectionMerge %88 None
               OpBranchConditional %86 %87 %88
         %87 = OpLabel
               OpBranch %88
         %88 = OpLabel
         %89 = OpPhi %bool %false %85 %true %87
               OpSelectionMerge %93 None
               OpBranchConditional %89 %91 %92
         %91 = OpLabel
         %94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %96 = OpLoad %v4float %94
               OpStore %90 %96
               OpBranch %93
         %92 = OpLabel
         %97 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
         %99 = OpLoad %v4float %97
               OpStore %90 %99
               OpBranch %93
         %93 = OpLabel
        %100 = OpLoad %v4float %90
               OpReturnValue %100
               OpFunctionEnd
