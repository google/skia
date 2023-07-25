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
               OpName %check "check"
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
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
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
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
      %check = OpVariable %_ptr_Function_int Function
         %67 = OpVariable %_ptr_Function_v4float Function
               OpStore %check %int_0
         %30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %32 = OpLoad %v4float %30
         %33 = OpCompositeExtract %float %32 1
         %35 = OpFOrdEqual %bool %33 %float_1
         %36 = OpSelect %int %35 %int_0 %int_1
         %38 = OpIAdd %int %int_0 %36
               OpStore %check %38
         %39 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %40 = OpLoad %v4float %39
         %41 = OpCompositeExtract %float %40 0
         %42 = OpFOrdEqual %bool %41 %float_1
         %43 = OpSelect %int %42 %int_1 %int_0
         %44 = OpIAdd %int %38 %43
               OpStore %check %44
         %45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %46 = OpLoad %v4float %45
         %47 = OpVectorShuffle %v2float %46 %46 1 0
         %48 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %49 = OpLoad %v4float %48
         %50 = OpVectorShuffle %v2float %49 %49 0 1
         %51 = OpFOrdEqual %v2bool %47 %50
         %53 = OpAll %bool %51
         %54 = OpSelect %int %53 %int_0 %int_1
         %55 = OpIAdd %int %44 %54
               OpStore %check %55
         %56 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %57 = OpLoad %v4float %56
         %58 = OpVectorShuffle %v2float %57 %57 1 0
         %59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %60 = OpLoad %v4float %59
         %61 = OpVectorShuffle %v2float %60 %60 0 1
         %62 = OpFUnordNotEqual %v2bool %58 %61
         %63 = OpAny %bool %62
         %64 = OpSelect %int %63 %int_1 %int_0
         %65 = OpIAdd %int %55 %64
               OpStore %check %65
         %66 = OpIEqual %bool %65 %int_0
               OpSelectionMerge %71 None
               OpBranchConditional %66 %69 %70
         %69 = OpLabel
         %72 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %73 = OpLoad %v4float %72
               OpStore %67 %73
               OpBranch %71
         %70 = OpLabel
         %74 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %75 = OpLoad %v4float %74
               OpStore %67 %75
               OpBranch %71
         %71 = OpLabel
         %76 = OpLoad %v4float %67
               OpReturnValue %76
               OpFunctionEnd
