set(sources
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_adc.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_adc_ex.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_can.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_cec.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_cortex.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_crc.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_cryp.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_cryp_ex.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_dac.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_dac_ex.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_dcmi.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_dcmi_ex.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_dfsdm.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_dma.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_dma2d.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_dma_ex.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_dsi.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_eth.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_exti.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_flash.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_flash_ex.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_flash_ramfunc.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_fmpi2c.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_fmpi2c_ex.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_fmpsmbus.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_fmpsmbus_ex.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_gpio.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_hash.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_hash_ex.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_hcd.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_i2c.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_i2c_ex.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_i2s.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_i2s_ex.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_irda.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_iwdg.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_lptim.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_ltdc.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_ltdc_ex.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_mmc.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_msp_template.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_nand.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_nor.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_pccard.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_pcd.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_pcd_ex.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_pwr.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_pwr_ex.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_qspi.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_rcc.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_rcc_ex.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_rng.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_rtc.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_rtc_ex.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_sai.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_sai_ex.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_sd.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_sdram.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_smartcard.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_smbus.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_spdifrx.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_spi.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_sram.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_tim.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_tim_ex.c
#    stm32f4xx_hal_driver/Src/stm32f4xx_hal_timebase_rtc_alarm_template.c
#    stm32f4xx_hal_driver/Src/stm32f4xx_hal_timebase_rtc_wakeup_template.c
#    stm32f4xx_hal_driver/Src/stm32f4xx_hal_timebase_tim_template.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_uart.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_usart.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal_wwdg.c
    stm32f4xx_hal_driver/Src/stm32f4xx_hal.c
    misc/pure_virtual.c            # this is needed for CPP size optimization
)

add_library(stm32f4xx ${sources})

# add include directories for StdPeriph library and CMSIS
target_include_directories(stm32f4xx PUBLIC stm32f4xx_hal_driver/Inc)

target_link_libraries(stm32f4xx PUBLIC cmsis)

# set up some defines for the StdPeriph library
target_compile_definitions(stm32f4xx PUBLIC USE_STDPERIPH_DRIVER)
target_compile_definitions(stm32f4xx PUBLIC ${DEVICE_FAMILY})
target_compile_definitions(stm32f4xx PUBLIC HSE_VALUE=${HSE_VALUE})

# additional compiler options: use size-optimized version of library in release build, use -O0 in debug build
if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(additional_flags -O0)
else()
  set(additional_flags -Os)
endif()

target_compile_options(stm32f4xx PRIVATE ${additional_flags})
#target_link_libraries(stm32f4xx PRIVATE ${additional_flags})