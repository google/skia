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
               OpName %main "main"
               OpName %inputVal "inputVal"
               OpName %expected "expected"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
         %44 = OpConstantComposite %v4bool %true %false %true %false
     %v2bool = OpTypeVector %bool 2
     %v3bool = OpTypeVector %bool 3
         %68 = OpConstantComposite %v2bool %true %false
         %75 = OpConstantComposite %v3bool %true %false %true
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
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
   %inputVal = OpVariable %_ptr_Function_v4bool Function
   %expected = OpVariable %_ptr_Function_v4bool Function
         %83 = OpVariable %_ptr_Function_v4float Function
         %27 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %31 = OpLoad %v4float %27
         %32 = OpCompositeExtract %float %31 0
         %33 = OpFUnordNotEqual %bool %32 %float_0
         %34 = OpCompositeExtract %float %31 1
         %35 = OpFUnordNotEqual %bool %34 %float_0
         %36 = OpCompositeExtract %float %31 2
         %37 = OpFUnordNotEqual %bool %36 %float_0
         %38 = OpCompositeExtract %float %31 3
         %39 = OpFUnordNotEqual %bool %38 %float_0
         %40 = OpCompositeConstruct %v4bool %33 %35 %37 %39
               OpStore %inputVal %40
               OpStore %expected %44
         %46 = OpVectorShuffle %v2bool %40 %40 0 1
         %45 = OpLogicalNot %v2bool %46
         %48 = OpVectorShuffle %v2bool %44 %44 0 1
         %49 = OpLogicalEqual %v2bool %45 %48
         %50 = OpAll %bool %49
               OpSelectionMerge %52 None
               OpBranchConditional %50 %51 %52
         %51 = OpLabel
         %54 = OpVectorShuffle %v3bool %40 %40 0 1 2
         %53 = OpLogicalNot %v3bool %54
         %56 = OpVectorShuffle %v3bool %44 %44 0 1 2
         %57 = OpLogicalEqual %v3bool %53 %56
         %58 = OpAll %bool %57
               OpBranch %52
         %52 = OpLabel
         %59 = OpPhi %bool %false %22 %58 %51
               OpSelectionMerge %61 None
               OpBranchConditional %59 %60 %61
         %60 = OpLabel
         %62 = OpLogicalNot %v4bool %40
         %63 = OpLogicalEqual %v4bool %62 %44
         %64 = OpAll %bool %63
               OpBranch %61
         %61 = OpLabel
         %65 = OpPhi %bool %false %52 %64 %60
               OpSelectionMerge %67 None
               OpBranchConditional %65 %66 %67
         %66 = OpLabel
         %69 = OpVectorShuffle %v2bool %44 %44 0 1
         %70 = OpLogicalEqual %v2bool %68 %69
         %71 = OpAll %bool %70
               OpBranch %67
         %67 = OpLabel
         %72 = OpPhi %bool %false %61 %71 %66
               OpSelectionMerge %74 None
               OpBranchConditional %72 %73 %74
         %73 = OpLabel
         %76 = OpVectorShuffle %v3bool %44 %44 0 1 2
         %77 = OpLogicalEqual %v3bool %75 %76
         %78 = OpAll %bool %77
               OpBranch %74
         %74 = OpLabel
         %79 = OpPhi %bool %false %67 %78 %73
               OpSelectionMerge %81 None
               OpBranchConditional %79 %80 %81
         %80 = OpLabel
               OpBranch %81
         %81 = OpLabel
         %82 = OpPhi %bool %false %74 %true %80
               OpSelectionMerge %87 None
               OpBranchConditional %82 %85 %86
         %85 = OpLabel
         %88 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %89 = OpLoad %v4float %88
               OpStore %83 %89
               OpBranch %87
         %86 = OpLabel
         %90 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %92 = OpLoad %v4float %90
               OpStore %83 %92
               OpBranch %87
         %87 = OpLabel
         %93 = OpLoad %v4float %83
               OpReturnValue %93
               OpFunctionEnd
