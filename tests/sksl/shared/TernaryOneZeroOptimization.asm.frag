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
               OpName %ok "ok"
               OpName %TRUE "TRUE"
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
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
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
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %false = OpConstantFalse %bool
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
     %v2bool = OpTypeVector %bool 2
         %64 = OpConstantComposite %v2bool %true %true
      %v2int = OpTypeVector %int 2
         %72 = OpConstantComposite %v2int %int_1 %int_1
         %80 = OpConstantComposite %v2float %float_1 %float_1
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
         %ok = OpVariable %_ptr_Function_bool Function
       %TRUE = OpVariable %_ptr_Function_bool Function
         %86 = OpVariable %_ptr_Function_v4float Function
               OpStore %ok %true
         %28 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %32 = OpLoad %v4float %28
         %33 = OpCompositeExtract %float %32 1
         %34 = OpFUnordNotEqual %bool %33 %float_0
               OpStore %TRUE %34
               OpSelectionMerge %37 None
               OpBranchConditional %true %36 %37
         %36 = OpLabel
         %39 = OpSelect %int %34 %int_1 %int_0
         %40 = OpIEqual %bool %int_1 %39
               OpBranch %37
         %37 = OpLabel
         %41 = OpPhi %bool %false %22 %40 %36
               OpStore %ok %41
               OpSelectionMerge %43 None
               OpBranchConditional %41 %42 %43
         %42 = OpLabel
         %45 = OpSelect %float %34 %float_1 %float_0
         %46 = OpFOrdEqual %bool %float_1 %45
               OpBranch %43
         %43 = OpLabel
         %47 = OpPhi %bool %false %37 %46 %42
               OpStore %ok %47
               OpSelectionMerge %49 None
               OpBranchConditional %47 %48 %49
         %48 = OpLabel
               OpBranch %49
         %49 = OpLabel
         %50 = OpPhi %bool %false %43 %34 %48
               OpStore %ok %50
               OpSelectionMerge %52 None
               OpBranchConditional %50 %51 %52
         %51 = OpLabel
         %53 = OpSelect %int %34 %int_1 %int_0
         %54 = OpIEqual %bool %int_1 %53
               OpBranch %52
         %52 = OpLabel
         %55 = OpPhi %bool %false %49 %54 %51
               OpStore %ok %55
               OpSelectionMerge %57 None
               OpBranchConditional %55 %56 %57
         %56 = OpLabel
         %58 = OpSelect %float %34 %float_1 %float_0
         %59 = OpFOrdEqual %bool %float_1 %58
               OpBranch %57
         %57 = OpLabel
         %60 = OpPhi %bool %false %52 %59 %56
               OpStore %ok %60
               OpSelectionMerge %62 None
               OpBranchConditional %60 %61 %62
         %61 = OpLabel
         %65 = OpCompositeConstruct %v2bool %34 %34
         %66 = OpLogicalEqual %v2bool %64 %65
         %67 = OpAll %bool %66
               OpBranch %62
         %62 = OpLabel
         %68 = OpPhi %bool %false %57 %67 %61
               OpStore %ok %68
               OpSelectionMerge %70 None
               OpBranchConditional %68 %69 %70
         %69 = OpLabel
         %73 = OpSelect %int %34 %int_1 %int_0
         %74 = OpCompositeConstruct %v2int %73 %73
         %75 = OpIEqual %v2bool %72 %74
         %76 = OpAll %bool %75
               OpBranch %70
         %70 = OpLabel
         %77 = OpPhi %bool %false %62 %76 %69
               OpStore %ok %77
               OpSelectionMerge %79 None
               OpBranchConditional %77 %78 %79
         %78 = OpLabel
         %81 = OpSelect %float %34 %float_1 %float_0
         %82 = OpCompositeConstruct %v2float %81 %81
         %83 = OpFOrdEqual %v2bool %80 %82
         %84 = OpAll %bool %83
               OpBranch %79
         %79 = OpLabel
         %85 = OpPhi %bool %false %70 %84 %78
               OpStore %ok %85
               OpSelectionMerge %90 None
               OpBranchConditional %85 %88 %89
         %88 = OpLabel
         %91 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %92 = OpLoad %v4float %91
               OpStore %86 %92
               OpBranch %90
         %89 = OpLabel
         %93 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %94 = OpLoad %v4float %93
               OpStore %86 %94
               OpBranch %90
         %90 = OpLabel
         %95 = OpLoad %v4float %86
               OpReturnValue %95
               OpFunctionEnd
