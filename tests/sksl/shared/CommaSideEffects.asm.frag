               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorRed"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorWhite"
               OpMemberName %_UniformBuffer 3 "colorBlack"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %setToColorBlack_vh4 "setToColorBlack_vh4"
               OpName %main "main"
               OpName %a "a"
               OpName %b "b"
               OpName %c "c"
               OpName %d "d"
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
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %32 RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %c RelaxedPrecision
               OpDecorate %d RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %25 = OpTypeFunction %void %_ptr_Function_v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
         %33 = OpTypeFunction %v4float %_ptr_Function_v2float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %false = OpConstantFalse %bool
     %v4bool = OpTypeVector %bool 4
%_entrypoint_v = OpFunction %void None %16
         %17 = OpLabel
         %21 = OpVariable %_ptr_Function_v2float Function
               OpStore %21 %20
         %23 = OpFunctionCall %v4float %main %21
               OpStore %sk_FragColor %23
               OpReturn
               OpFunctionEnd
%setToColorBlack_vh4 = OpFunction %void None %25
         %26 = OpFunctionParameter %_ptr_Function_v4float
         %27 = OpLabel
         %28 = OpAccessChain %_ptr_Uniform_v4float %11 %int_3
         %32 = OpLoad %v4float %28
               OpStore %26 %32
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %33
         %34 = OpFunctionParameter %_ptr_Function_v2float
         %35 = OpLabel
          %a = OpVariable %_ptr_Function_v4float Function
          %b = OpVariable %_ptr_Function_v4float Function
          %c = OpVariable %_ptr_Function_v4float Function
          %d = OpVariable %_ptr_Function_v4float Function
         %46 = OpVariable %_ptr_Function_v4float Function
         %83 = OpVariable %_ptr_Function_v4float Function
         %40 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %42 = OpLoad %v4float %40
               OpStore %b %42
         %43 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %45 = OpLoad %v4float %43
               OpStore %c %45
         %47 = OpFunctionCall %void %setToColorBlack_vh4 %46
         %48 = OpLoad %v4float %46
               OpStore %d %48
         %49 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %51 = OpLoad %v4float %49
               OpStore %a %51
         %52 = OpFMul %v4float %51 %51
               OpStore %a %52
         %53 = OpFMul %v4float %42 %42
               OpStore %b %53
         %54 = OpFMul %v4float %45 %45
               OpStore %c %54
         %55 = OpFMul %v4float %48 %48
               OpStore %d %55
         %57 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %58 = OpLoad %v4float %57
         %59 = OpFOrdEqual %v4bool %52 %58
         %61 = OpAll %bool %59
               OpSelectionMerge %63 None
               OpBranchConditional %61 %62 %63
         %62 = OpLabel
         %64 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %65 = OpLoad %v4float %64
         %66 = OpFOrdEqual %v4bool %53 %65
         %67 = OpAll %bool %66
               OpBranch %63
         %63 = OpLabel
         %68 = OpPhi %bool %false %35 %67 %62
               OpSelectionMerge %70 None
               OpBranchConditional %68 %69 %70
         %69 = OpLabel
         %71 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %72 = OpLoad %v4float %71
         %73 = OpFOrdEqual %v4bool %54 %72
         %74 = OpAll %bool %73
               OpBranch %70
         %70 = OpLabel
         %75 = OpPhi %bool %false %63 %74 %69
               OpSelectionMerge %77 None
               OpBranchConditional %75 %76 %77
         %76 = OpLabel
         %78 = OpAccessChain %_ptr_Uniform_v4float %11 %int_3
         %79 = OpLoad %v4float %78
         %80 = OpFOrdEqual %v4bool %55 %79
         %81 = OpAll %bool %80
               OpBranch %77
         %77 = OpLabel
         %82 = OpPhi %bool %false %70 %81 %76
               OpSelectionMerge %86 None
               OpBranchConditional %82 %84 %85
         %84 = OpLabel
         %87 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %88 = OpLoad %v4float %87
               OpStore %83 %88
               OpBranch %86
         %85 = OpLabel
         %89 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %90 = OpLoad %v4float %89
               OpStore %83 %90
               OpBranch %86
         %86 = OpLabel
         %91 = OpLoad %v4float %83
               OpReturnValue %91
               OpFunctionEnd
