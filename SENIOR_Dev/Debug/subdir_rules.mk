################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
AsciiLib.obj: ../AsciiLib.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_common/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/include" --include_path="C:/ti/DSP2833x_Audio_App/AIC23_driver_include" --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_headers/include" -g --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="AsciiLib.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

LCD.obj: ../LCD.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_common/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/include" --include_path="C:/ti/DSP2833x_Audio_App/AIC23_driver_include" --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_headers/include" -g --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="LCD.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

MDF_init.obj: ../MDF_init.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_common/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/include" --include_path="C:/ti/DSP2833x_Audio_App/AIC23_driver_include" --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_headers/include" -g --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="MDF_init.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

UI.obj: ../UI.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_common/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/include" --include_path="C:/ti/DSP2833x_Audio_App/AIC23_driver_include" --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_headers/include" -g --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="UI.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_common/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-c2000_15.12.1.LTS/include" --include_path="C:/ti/DSP2833x_Audio_App/AIC23_driver_include" --include_path="C:/ti/DSP2833x_Audio_App/DSP2833x_headers/include" -g --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="main.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


