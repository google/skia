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
               OpName %unpremul_h4h4 "unpremul_h4h4"
               OpName %live_fn_h4h4h4 "live_fn_h4h4h4"
               OpName %main "main"
               OpName %a "a"
               OpName %b "b"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %26 = OpTypeFunction %v4float %_ptr_Function_v4float
    %v3float = OpTypeVector %float 3
%float_9_99999975en05 = OpConstant %float 9.99999975e-05
    %float_1 = OpConstant %float 1
         %45 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
         %52 = OpTypeFunction %v4float %_ptr_Function_v2float
    %float_3 = OpConstant %float 3
         %58 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
   %float_n5 = OpConstant %float -5
         %61 = OpConstantComposite %v4float %float_n5 %float_n5 %float_n5 %float_n5
         %64 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
      %false = OpConstantFalse %bool
         %68 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
     %v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
         %18 = OpLabel
         %22 = OpVariable %_ptr_Function_v2float Function
               OpStore %22 %21
         %24 = OpFunctionCall %v4float %main %22
               OpStore %sk_FragColor %24
               OpReturn
               OpFunctionEnd
%unpremul_h4h4 = OpFunction %v4float None %26
         %27 = OpFunctionParameter %_ptr_Function_v4float
         %28 = OpLabel
         %29 = OpLoad %v4float %27
         %30 = OpVectorShuffle %v3float %29 %29 0 1 2
         %33 = OpLoad %v4float %27
         %34 = OpCompositeExtract %float %33 3
         %32 = OpExtInst %float %1 FMax %34 %float_9_99999975en05
         %37 = OpFDiv %float %float_1 %32
         %38 = OpVectorTimesScalar %v3float %30 %37
         %39 = OpCompositeExtract %float %38 0
         %40 = OpCompositeExtract %float %38 1
         %41 = OpCompositeExtract %float %38 2
         %42 = OpLoad %v4float %27
         %43 = OpCompositeExtract %float %42 3
         %44 = OpCompositeConstruct %v4float %39 %40 %41 %43
               OpReturnValue %44
               OpFunctionEnd
%live_fn_h4h4h4 = OpFunction %v4float None %45
         %46 = OpFunctionParameter %_ptr_Function_v4float
         %47 = OpFunctionParameter %_ptr_Function_v4float
         %48 = OpLabel
         %49 = OpLoad %v4float %46
         %50 = OpLoad %v4float %47
         %51 = OpFAdd %v4float %49 %50
               OpReturnValue %51
               OpFunctionEnd
       %main = OpFunction %v4float None %52
         %53 = OpFunctionParameter %_ptr_Function_v2float
         %54 = OpLabel
          %a = OpVariable %_ptr_Function_v4float Function
          %b = OpVariable %_ptr_Function_v4float Function
         %59 = OpVariable %_ptr_Function_v4float Function
         %62 = OpVariable %_ptr_Function_v4float Function
         %65 = OpVariable %_ptr_Function_v4float Function
         %77 = OpVariable %_ptr_Function_v4float Function
               OpStore %59 %58
               OpStore %62 %61
         %63 = OpFunctionCall %v4float %live_fn_h4h4h4 %59 %62
               OpStore %a %63
               OpStore %65 %64
         %66 = OpFunctionCall %v4float %unpremul_h4h4 %65
               OpStore %b %66
         %69 = OpFUnordNotEqual %v4bool %63 %68
         %71 = OpAny %bool %69
               OpSelectionMerge %73 None
               OpBranchConditional %71 %72 %73
         %72 = OpLabel
         %74 = OpFUnordNotEqual %v4bool %66 %68
         %75 = OpAny %bool %74
               OpBranch %73
         %73 = OpLabel
         %76 = OpPhi %bool %false %54 %75 %72
               OpSelectionMerge %80 None
               OpBranchConditional %76 %78 %79
         %78 = OpLabel
         %81 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %85 = OpLoad %v4float %81
               OpStore %77 %85
               OpBranch %80
         %79 = OpLabel
         %86 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %88 = OpLoad %v4float %86
               OpStore %77 %88
               OpBranch %80
         %80 = OpLabel
         %89 = OpLoad %v4float %77
               OpReturnValue %89
               OpFunctionEnd
