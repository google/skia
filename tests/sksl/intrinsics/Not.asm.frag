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
               OpName %main "main"
               OpName %inputVal "inputVal"
               OpName %expected "expected"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
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
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
         %46 = OpConstantComposite %v4bool %true %false %true %false
     %v2bool = OpTypeVector %bool 2
     %v3bool = OpTypeVector %bool 3
         %70 = OpConstantComposite %v2bool %true %false
         %77 = OpConstantComposite %v3bool %true %false %true
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
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
   %inputVal = OpVariable %_ptr_Function_v4bool Function
   %expected = OpVariable %_ptr_Function_v4bool Function
         %85 = OpVariable %_ptr_Function_v4float Function
         %29 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %33 = OpLoad %v4float %29
         %34 = OpCompositeExtract %float %33 0
         %35 = OpFUnordNotEqual %bool %34 %float_0
         %36 = OpCompositeExtract %float %33 1
         %37 = OpFUnordNotEqual %bool %36 %float_0
         %38 = OpCompositeExtract %float %33 2
         %39 = OpFUnordNotEqual %bool %38 %float_0
         %40 = OpCompositeExtract %float %33 3
         %41 = OpFUnordNotEqual %bool %40 %float_0
         %42 = OpCompositeConstruct %v4bool %35 %37 %39 %41
               OpStore %inputVal %42
               OpStore %expected %46
         %48 = OpVectorShuffle %v2bool %42 %42 0 1
         %47 = OpLogicalNot %v2bool %48
         %50 = OpVectorShuffle %v2bool %46 %46 0 1
         %51 = OpLogicalEqual %v2bool %47 %50
         %52 = OpAll %bool %51
               OpSelectionMerge %54 None
               OpBranchConditional %52 %53 %54
         %53 = OpLabel
         %56 = OpVectorShuffle %v3bool %42 %42 0 1 2
         %55 = OpLogicalNot %v3bool %56
         %58 = OpVectorShuffle %v3bool %46 %46 0 1 2
         %59 = OpLogicalEqual %v3bool %55 %58
         %60 = OpAll %bool %59
               OpBranch %54
         %54 = OpLabel
         %61 = OpPhi %bool %false %25 %60 %53
               OpSelectionMerge %63 None
               OpBranchConditional %61 %62 %63
         %62 = OpLabel
         %64 = OpLogicalNot %v4bool %42
         %65 = OpLogicalEqual %v4bool %64 %46
         %66 = OpAll %bool %65
               OpBranch %63
         %63 = OpLabel
         %67 = OpPhi %bool %false %54 %66 %62
               OpSelectionMerge %69 None
               OpBranchConditional %67 %68 %69
         %68 = OpLabel
         %71 = OpVectorShuffle %v2bool %46 %46 0 1
         %72 = OpLogicalEqual %v2bool %70 %71
         %73 = OpAll %bool %72
               OpBranch %69
         %69 = OpLabel
         %74 = OpPhi %bool %false %63 %73 %68
               OpSelectionMerge %76 None
               OpBranchConditional %74 %75 %76
         %75 = OpLabel
         %78 = OpVectorShuffle %v3bool %46 %46 0 1 2
         %79 = OpLogicalEqual %v3bool %77 %78
         %80 = OpAll %bool %79
               OpBranch %76
         %76 = OpLabel
         %81 = OpPhi %bool %false %69 %80 %75
               OpSelectionMerge %83 None
               OpBranchConditional %81 %82 %83
         %82 = OpLabel
               OpBranch %83
         %83 = OpLabel
         %84 = OpPhi %bool %false %76 %true %82
               OpSelectionMerge %89 None
               OpBranchConditional %84 %87 %88
         %87 = OpLabel
         %90 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %91 = OpLoad %v4float %90
               OpStore %85 %91
               OpBranch %89
         %88 = OpLabel
         %92 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %94 = OpLoad %v4float %92
               OpStore %85 %94
               OpBranch %89
         %89 = OpLabel
         %95 = OpLoad %v4float %85
               OpReturnValue %95
               OpFunctionEnd
