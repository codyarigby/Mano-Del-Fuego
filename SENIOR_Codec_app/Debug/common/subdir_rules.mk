################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
common/AIC23.obj: ../common/AIC23.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_common/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/include" --include_path="C:/ti/DSP2833x_Audio_App/AIC23_driver_include" --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_headers/include" -g --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="common/AIC23.d" --obj_directory="common" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

common/AIC23_SPI_control.obj: ../common/AIC23_SPI_control.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_common/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/include" --include_path="C:/ti/DSP2833x_Audio_App/AIC23_driver_include" --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_headers/include" -g --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="common/AIC23_SPI_control.d" --obj_directory="common" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

common/DSP2833x_CSMPasswords.obj: ../common/DSP2833x_CSMPasswords.asm $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_common/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/include" --include_path="C:/ti/DSP2833x_Audio_App/AIC23_driver_include" --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_headers/include" -g --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="common/DSP2833x_CSMPasswords.d" --obj_directory="common" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

common/DSP2833x_CodeStartBranch.obj: ../common/DSP2833x_CodeStartBranch.asm $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_common/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/include" --include_path="C:/ti/DSP2833x_Audio_App/AIC23_driver_include" --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_headers/include" -g --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="common/DSP2833x_CodeStartBranch.d" --obj_directory="common" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

common/DSP2833x_CpuTimers.obj: ../common/DSP2833x_CpuTimers.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_common/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/include" --include_path="C:/ti/DSP2833x_Audio_App/AIC23_driver_include" --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_headers/include" -g --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="common/DSP2833x_CpuTimers.d" --obj_directory="common" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

common/DSP2833x_DefaultIsr.obj: ../common/DSP2833x_DefaultIsr.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_common/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/include" --include_path="C:/ti/DSP2833x_Audio_App/AIC23_driver_include" --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_headers/include" -g --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="common/DSP2833x_DefaultIsr.d" --obj_directory="common" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

common/DSP2833x_DisInt.obj: ../common/DSP2833x_DisInt.asm $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_common/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/include" --include_path="C:/ti/DSP2833x_Audio_App/AIC23_driver_include" --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_headers/include" -g --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="common/DSP2833x_DisInt.d" --obj_directory="common" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

common/DSP2833x_Mcbsp.obj: ../common/DSP2833x_Mcbsp.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_common/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/include" --include_path="C:/ti/DSP2833x_Audio_App/AIC23_driver_include" --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_headers/include" -g --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="common/DSP2833x_Mcbsp.d" --obj_directory="common" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

common/DSP2833x_PieCtrl.obj: ../common/DSP2833x_PieCtrl.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_common/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/include" --include_path="C:/ti/DSP2833x_Audio_App/AIC23_driver_include" --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_headers/include" -g --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="common/DSP2833x_PieCtrl.d" --obj_directory="common" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

common/DSP2833x_SysCtrl.obj: ../common/DSP2833x_SysCtrl.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_common/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/include" --include_path="C:/ti/DSP2833x_Audio_App/AIC23_driver_include" --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_headers/include" -g --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="common/DSP2833x_SysCtrl.d" --obj_directory="common" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

common/DSP2833x_usDelay.obj: ../common/DSP2833x_usDelay.asm $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_common/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/include" --include_path="C:/ti/DSP2833x_Audio_App/AIC23_driver_include" --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_headers/include" -g --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="common/DSP2833x_usDelay.d" --obj_directory="common" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


