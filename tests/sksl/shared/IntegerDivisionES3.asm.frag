               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %zero "zero"
               OpName %one "one"
               OpName %x "x"
               OpName %y "y"
               OpName %_0_x "_0_x"
               OpName %_1_result "_1_result"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
    %int_100 = OpConstant %int 100
       %bool = OpTypeBool
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
%float_0_00392156886 = OpConstant %float 0.00392156886
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
       %zero = OpVariable %_ptr_Function_int Function
        %one = OpVariable %_ptr_Function_int Function
          %x = OpVariable %_ptr_Function_int Function
          %y = OpVariable %_ptr_Function_int Function
       %_0_x = OpVariable %_ptr_Function_int Function
  %_1_result = OpVariable %_ptr_Function_int Function
         %26 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %29 = OpLoad %v4float %26
         %30 = OpCompositeExtract %float %29 0
         %31 = OpConvertFToS %int %30
               OpStore %zero %31
         %33 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %34 = OpLoad %v4float %33
         %35 = OpCompositeExtract %float %34 1
         %36 = OpConvertFToS %int %35
               OpStore %one %36
               OpStore %x %31
               OpBranch %38
         %38 = OpLabel
               OpLoopMerge %42 %41 None
               OpBranch %39
         %39 = OpLabel
         %43 = OpLoad %int %x
         %45 = OpSLessThan %bool %43 %int_100
               OpBranchConditional %45 %40 %42
         %40 = OpLabel
         %48 = OpLoad %int %one
               OpStore %y %48
               OpBranch %49
         %49 = OpLabel
               OpLoopMerge %53 %52 None
               OpBranch %50
         %50 = OpLabel
         %54 = OpLoad %int %y
         %55 = OpSLessThan %bool %54 %int_100
               OpBranchConditional %55 %51 %53
         %51 = OpLabel
         %57 = OpLoad %int %x
               OpStore %_0_x %57
               OpStore %_1_result %int_0
               OpBranch %59
         %59 = OpLabel
               OpLoopMerge %63 %62 None
               OpBranch %60
         %60 = OpLabel
         %64 = OpLoad %int %_0_x
         %65 = OpLoad %int %y
         %66 = OpSGreaterThanEqual %bool %64 %65
               OpBranchConditional %66 %61 %63
         %61 = OpLabel
         %68 = OpLoad %int %_1_result
         %69 = OpIAdd %int %68 %int_1
               OpStore %_1_result %69
         %70 = OpLoad %int %_0_x
         %71 = OpLoad %int %y
         %72 = OpISub %int %70 %71
               OpStore %_0_x %72
               OpBranch %62
         %62 = OpLabel
               OpBranch %59
         %63 = OpLabel
         %73 = OpLoad %int %x
         %74 = OpLoad %int %y
         %75 = OpSDiv %int %73 %74
         %76 = OpLoad %int %_1_result
         %77 = OpINotEqual %bool %75 %76
               OpSelectionMerge %79 None
               OpBranchConditional %77 %78 %79
         %78 = OpLabel
         %81 = OpLoad %int %x
         %82 = OpConvertSToF %float %81
         %84 = OpFMul %float %82 %float_0_00392156886
         %85 = OpLoad %int %y
         %86 = OpConvertSToF %float %85
         %87 = OpFMul %float %86 %float_0_00392156886
         %88 = OpCompositeConstruct %v4float %float_1 %84 %87 %float_1
               OpReturnValue %88
         %79 = OpLabel
               OpBranch %52
         %52 = OpLabel
         %89 = OpLoad %int %y
         %90 = OpIAdd %int %89 %int_1
               OpStore %y %90
               OpBranch %49
         %53 = OpLabel
               OpBranch %41
         %41 = OpLabel
         %91 = OpLoad %int %x
         %92 = OpIAdd %int %91 %int_1
               OpStore %x %92
               OpBranch %38
         %42 = OpLabel
         %93 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %94 = OpLoad %v4float %93
               OpReturnValue %94
               OpFunctionEnd
