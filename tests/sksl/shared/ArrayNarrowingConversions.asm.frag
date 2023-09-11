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
               OpName %i2 "i2"
               OpName %s2 "s2"
               OpName %f2 "f2"
               OpName %h2 "h2"
               OpName %cf2 "cf2"
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
               OpDecorate %_arr_int_int_2 ArrayStride 16
               OpDecorate %s2 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %h2 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
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
      %int_2 = OpConstant %int 2
%_arr_int_int_2 = OpTypeArray %int %int_2
%_ptr_Function__arr_int_int_2 = OpTypePointer Function %_arr_int_int_2
      %int_1 = OpConstant %int 1
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
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
         %i2 = OpVariable %_ptr_Function__arr_int_int_2 Function
         %s2 = OpVariable %_ptr_Function__arr_int_int_2 Function
         %f2 = OpVariable %_ptr_Function__arr_float_int_2 Function
         %h2 = OpVariable %_ptr_Function__arr_float_int_2 Function
        %cf2 = OpVariable %_ptr_Function__arr_float_int_2 Function
         %59 = OpVariable %_ptr_Function_v4float Function
         %32 = OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
               OpStore %i2 %32
         %34 = OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
               OpStore %s2 %34
         %40 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
               OpStore %f2 %40
         %42 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
               OpStore %h2 %42
               OpStore %i2 %34
               OpStore %s2 %34
               OpStore %f2 %42
               OpStore %h2 %42
               OpStore %cf2 %40
         %46 = OpLogicalAnd %bool %true %true
               OpSelectionMerge %48 None
               OpBranchConditional %46 %47 %48
         %47 = OpLabel
         %49 = OpLogicalAnd %bool %true %true
               OpBranch %48
         %48 = OpLabel
         %50 = OpPhi %bool %false %25 %49 %47
               OpSelectionMerge %52 None
               OpBranchConditional %50 %51 %52
         %51 = OpLabel
         %53 = OpLogicalAnd %bool %true %true
               OpBranch %52
         %52 = OpLabel
         %54 = OpPhi %bool %false %48 %53 %51
               OpSelectionMerge %56 None
               OpBranchConditional %54 %55 %56
         %55 = OpLabel
         %57 = OpLogicalAnd %bool %true %true
               OpBranch %56
         %56 = OpLabel
         %58 = OpPhi %bool %false %52 %57 %55
               OpSelectionMerge %63 None
               OpBranchConditional %58 %61 %62
         %61 = OpLabel
         %64 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %67 = OpLoad %v4float %64
               OpStore %59 %67
               OpBranch %63
         %62 = OpLabel
         %68 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %69 = OpLoad %v4float %68
               OpStore %59 %69
               OpBranch %63
         %63 = OpLabel
         %70 = OpLoad %v4float %59
               OpReturnValue %70
               OpFunctionEnd
