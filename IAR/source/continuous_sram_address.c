/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_edma.h"
#include "fsl_lptmr.h"
#include "fsl_cmc.h"
#include "fsl_wuu.h"
#include "fsl_lpuart.h"
#include "fsl_port.h"
#include "fsl_spc.h"

#include "fsl_clock.h"
#include "fsl_reset.h"
#include <stdbool.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef enum _app_power_mode
{
    kAPP_PowerModeMin = 'A' - 1,
    kAPP_PowerModeActive,        /* Normal RUN mode. */
    kAPP_PowerModeSleep,         /* Sleep. */
    kAPP_PowerModeDeepSleep,     /* DeepSleep */
    kAPP_PowerModePowerDown,     /* PowerDown */
    kAPP_PowerModeDeepPowerDown, /* DeepPowerDown. */
    kAPP_PowerModeMax
} app_power_mode_t;

#define BUFFER_LENGTH 4U

#define EXAMPLE_DMA_BASEADDR DMA0
#define DEMO_DMA_CHANNEL_0   0U
#define APP_DMA_IRQ          DMA_CH0_IRQn
#define APP_DMA_IRQ_HANDLER  DMA_CH0_IRQHandler

#define APP_LPTMR             LPTMR0
#define APP_LPTMR_CLK_SOURCE  (16000UL)

#define APP_CMC               CMC

#define APP_WUU                                WUU0
#define APP_WUU_WAKEUP_LPTMR_DMA_REQUEST_IDX   4U
#define APP_WUU_WAKEUP_WKUP_BUTTON_IDX         9U

/* LPUART RX */
#define APP_DEBUG_CONSOLE_RX_PORT   PORT0
#define APP_DEBUG_CONSOLE_RX_GPIO   GPIO0
#define APP_DEBUG_CONSOLE_RX_PIN    2U
#define APP_DEBUG_CONSOLE_RX_PINMUX kPORT_MuxAlt2
/* LPUART TX */
#define APP_DEBUG_CONSOLE_TX_PORT   PORT0
#define APP_DEBUG_CONSOLE_TX_GPIO   GPIO0
#define APP_DEBUG_CONSOLE_TX_PIN    3U
#define APP_DEBUG_CONSOLE_TX_PINMUX kPORT_MuxAlt2

#define APP_SPC                   SPC0
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void APP_InitLptimer(void);
static void APP_SetCMCConfiguration(void);
static void APP_GetWakeupConfig(app_power_mode_t targetMode);
static void APP_PowerModeSwitch(app_power_mode_t targetPowerMode);
static void APP_EnterPowerDownMode(void);
static void APP_EnterDeepPowerDownMode(void);
static void APP_PowerPreSwitchHook(void);
static void APP_EnterDeepSleepMode(void);
static void APP_SetSPCConfiguration(void);
static void APP_PowerPostSwitchHook(void);
void APP_InitDebugConsole(void);
void APP_DeinitDebugConsole(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
uint32_t srcBuf1[BUFFER_LENGTH]               			= {0x01, 0x02, 0x03, 0x04};
uint32_t srcBuf2[BUFFER_LENGTH]               			= {0x00, 0x00, 0x00, 0x00};
uint32_t srcBuf3[BUFFER_LENGTH]               			= {0x00, 0x00, 0x00, 0x00};
uint32_t *p_destBuf 									= (uint32_t *)0x2001DFF8;
volatile bool g_Transfer_Done                 			= false;
/*******************************************************************************
 * Code
 ******************************************************************************/
/* EDMA transfer channel 0 IRQ handler */
void APP_DMA_IRQ_HANDLER(void)
{
    if ((EDMA_GetChannelStatusFlags(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0) & kEDMA_InterruptFlag) != 0U)
    {
        EDMA_ClearChannelStatusFlags(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0, kEDMA_InterruptFlag);
        g_Transfer_Done = true;
    }
}

void APP_DeinitDebugConsole(void)
{
    DbgConsole_Deinit();
    PORT_SetPinMux(APP_DEBUG_CONSOLE_RX_PORT, APP_DEBUG_CONSOLE_RX_PIN, kPORT_MuxAsGpio);
    PORT_SetPinMux(APP_DEBUG_CONSOLE_TX_PORT, APP_DEBUG_CONSOLE_TX_PIN, kPORT_MuxAsGpio);
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i;
    edma_transfer_config_t transferConfig;
    edma_config_t userConfig;
    app_power_mode_t targetPowerMode;

    CLOCK_SetupFRO16KClocking(kCLKE_16K_SYSTEM | kCLKE_16K_COREMAIN);

    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    if ((CMC_GetSystemResetStatus(APP_CMC) & kCMC_WakeUpReset) != 0UL)
    {
        SPC_ClearPeriphIOIsolationFlag(APP_SPC);
    }

    APP_SetSPCConfiguration();

    PRINTF("********Read and write test for contiguous SRAM address boundary unaligned address********\r\n");
    uint32_t *p_test = (uint32_t *)0x2001DFFF;
    *p_test = 0x00000000;
    PRINTF("32bit *p_test point address : 0x%p\r\n", p_test);
    PRINTF("Initial *p_test = 0x%x\r\n", *p_test);
    PRINTF("Write 0xFFFFFFFF to *p_test\r\n");
    *p_test = 0xFFFFFFFF;
    PRINTF("Updated *p_test = 0x%x\r\n", *p_test);

    PRINTF("\r\n***********************DMA access to contiguous SRAM address test***********************\r\n");
    PRINTF("\r\n*******************Active Mode*******************\r\n");
    PRINTF("\r\nSource Buffer 1:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", srcBuf1[i]);
    }

    PRINTF("\r\nSource Buffer 2:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", srcBuf2[i]);
    }

    PRINTF("\r\nSource Buffer 3:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", srcBuf3[i]);
    }

    PRINTF("\r\nDestination Buffer:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
    	*(p_destBuf+i) = 0;
        PRINTF("%d\t", *(p_destBuf+i));
    }

    PRINTF("\r\nDestination Buffer Address:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
    	PRINTF("0x%x\t", (p_destBuf+i));
    }

    PRINTF("\r\n\r\nEDMA Source Buffer 1 to Destination Buffer.\r\n");
    EDMA_GetDefaultConfig(&userConfig);
    EDMA_Init(EXAMPLE_DMA_BASEADDR, &userConfig);
    EDMA_PrepareTransfer(&transferConfig, srcBuf1, sizeof(srcBuf1[0]), p_destBuf, sizeof(p_destBuf), sizeof(srcBuf1),
                         sizeof(srcBuf1), kEDMA_MemoryToMemory);
    EDMA_SetTransferConfig(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0, &transferConfig, NULL);
    EnableIRQ(APP_DMA_IRQ);
    EDMA_TriggerChannelStart(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0);
    /* Wait for EDMA transfer finish */
    while (g_Transfer_Done != true)
    {
    }
    g_Transfer_Done = false;
    PRINTF("Destination Buffer:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
    	PRINTF("%d\t", *(p_destBuf+i));
    }

    PRINTF("\r\n\r\nEDMA Destination Buffer to Source Buffer 2.\r\n");
    EDMA_GetDefaultConfig(&userConfig);
    EDMA_Init(EXAMPLE_DMA_BASEADDR, &userConfig);
    EDMA_PrepareTransfer(&transferConfig, p_destBuf, sizeof(p_destBuf), srcBuf2, sizeof(srcBuf2[0]), sizeof(srcBuf2),
                         sizeof(srcBuf2), kEDMA_MemoryToMemory);
    EDMA_SetTransferConfig(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0, &transferConfig, NULL);
    EDMA_TriggerChannelStart(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0);
    /* Wait for EDMA transfer finish */
    while (g_Transfer_Done != true)
    {
    }
    g_Transfer_Done = false;
    PRINTF("Source Buffer 2:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", srcBuf2[i]);
    }

    PRINTF("\r\n\r\n*******************Power Down Mode*******************\r\n");
    PRINTF("EDMA will transport Destination Buffer to Source Buffer 3.\r\n");
    PRINTF("Input any key to enter Power Down.\r\n");
    GETCHAR();
    PRINTF("Entering Power Down mode...\r\n");
    PRINTF("Please Press SW2(WAKE UP) button to wake MCU and check Source Buffer 3.\r\n");

    APP_PowerPreSwitchHook();

    CLOCK_EnableClock(kCLOCK_GateDMA);
    RESET_PeripheralReset(kDMA_RST_SHIFT_RSTn);
    EDMA_GetDefaultConfig(&userConfig);
    EDMA_Init(EXAMPLE_DMA_BASEADDR, &userConfig);
    EDMA_PrepareTransfer(&transferConfig, p_destBuf, sizeof(p_destBuf), srcBuf3, sizeof(srcBuf3[0]), sizeof(srcBuf3),
                         sizeof(srcBuf3), kEDMA_MemoryToMemory);
    EDMA_SetTransferConfig(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0, &transferConfig, NULL);
    DMA0->CH[DEMO_DMA_CHANNEL_0].TCD_CSR &= ~(uint16_t)DMA_CSR_INTMAJOR_MASK;
    DMA0->CH[DEMO_DMA_CHANNEL_0].TCD_CSR &= ~(uint16_t)DMA_CSR_DREQ_MASK;
    EDMA_SetChannelMux(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0, 49);
    EDMA_EnableChannelRequest(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0);
    EXAMPLE_DMA_BASEADDR->CH[DEMO_DMA_CHANNEL_0].CH_CSR |= DMA_CH_CSR_EARQ_MASK;

    APP_InitLptimer();
    LPTMR_StartTimer(APP_LPTMR);
    APP_SetCMCConfiguration();
    targetPowerMode = kAPP_PowerModePowerDown;
    APP_GetWakeupConfig(targetPowerMode);
    APP_PowerModeSwitch(targetPowerMode);
    APP_PowerPostSwitchHook();
    LPTMR_StopTimer(APP_LPTMR);
    PRINTF("\r\nMCU woken up\r\n");
    PRINTF("Source Buffer 3:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", srcBuf3[i]);
    }

    while (1)
    {
    }
}

static void APP_InitLptimer(void)
{
    lptmr_config_t lptmr_config;
    LPTMR_GetDefaultConfig(&lptmr_config);
    /* Select clock source as FRO16K which is operatable in deep-sleep or deeper power modes. */
    lptmr_config.prescalerClockSource = kLPTMR_PrescalerClock_1;
    LPTMR_Init(APP_LPTMR, &lptmr_config);
    LPTMR_EnableTimerDMA(APP_LPTMR, true);
    LPTMR_SetTimerPeriod(APP_LPTMR,  APP_LPTMR_CLK_SOURCE - 1U);
    LPTMR_DisableInterrupts(APP_LPTMR, kLPTMR_TimerInterruptEnable);
}


static void APP_SetCMCConfiguration(void)
{
    /* Disable low power debug. */
    CMC_EnableDebugOperation(APP_CMC, false);
    /* Allow all power mode */
    CMC_SetPowerModeProtection(APP_CMC, kCMC_AllowAllLowPowerModes);

    /* Disable flash memory accesses and place flash memory in low-power state whenever the core clock
       is gated. And an attempt to access the flash memory will cause the flash memory to exit low-power
       state for the duration of the flash memory access. */
    CMC_ConfigFlashMode(APP_CMC, true, true, false);
}

static void APP_GetWakeupConfig(app_power_mode_t targetMode)
{
    wuu_external_wakeup_pin_config_t wakeupButtonConfig;

    wakeupButtonConfig.edge  = kWUU_ExternalPinFallingEdge;
    wakeupButtonConfig.event = kWUU_ExternalPinInterrupt;
    wakeupButtonConfig.mode  = kWUU_ExternalPinActiveAlways;
    WUU_SetExternalWakeUpPinsConfig(APP_WUU, APP_WUU_WAKEUP_WKUP_BUTTON_IDX, &wakeupButtonConfig);

    WUU_SetInternalWakeUpModulesConfig(APP_WUU, APP_WUU_WAKEUP_LPTMR_DMA_REQUEST_IDX, kWUU_InternalModuleDMATrigger);

    PRINTF("Enter Power Down and enable LPTMR request.\r\n");

    SYSCON->CLKUNLOCK &= ~SYSCON_CLKUNLOCK_UNLOCK_MASK;
    MRCC0->MRCC_GLB_ACC0 |= MRCC_MRCC_GLB_ACC0_DMA_MASK;

    /* set FRO_16K as bus clock, for DMA to interface the peripheral's registers */
    CLOCK_AttachClk(kCLK_16K_to_MAIN_CLK);

    LPTMR_StartTimer(APP_LPTMR);
}

static void APP_PowerModeSwitch(app_power_mode_t targetPowerMode)
{
    if (targetPowerMode != kAPP_PowerModeActive)
    {
        switch (targetPowerMode)
        {
            case kAPP_PowerModeDeepSleep:
                APP_EnterDeepSleepMode();
                break;
            case kAPP_PowerModePowerDown:
                APP_EnterPowerDownMode();
                break;
            case kAPP_PowerModeDeepPowerDown:
                APP_EnterDeepPowerDownMode();
                break;
            default:
                assert(false);
                break;
        }
    }
}

static void APP_EnterPowerDownMode(void)
{
    cmc_power_domain_config_t config;

    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_PowerDownMode;

    CMC_EnterLowPowerMode(APP_CMC, &config);
}

static void APP_EnterDeepPowerDownMode(void)
{
    cmc_power_domain_config_t config;

    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_DeepPowerDown;

    CMC_EnterLowPowerMode(APP_CMC, &config);
}

static void APP_PowerPreSwitchHook(void)
{
      /* Wait for debug console output finished. */
    while (!(kLPUART_TransmissionCompleteFlag & LPUART_GetStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR)))
    {
    }
    APP_DeinitDebugConsole();
}

static void APP_EnterDeepSleepMode(void)
{
    cmc_power_domain_config_t config;

    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_DeepSleepMode;

    CMC_EnterLowPowerMode(APP_CMC, &config);
}

static void APP_SetSPCConfiguration(void)
{
    status_t status;

    spc_active_mode_regulators_config_t activeModeRegulatorOption;

    SPC_EnableSRAMLdo(APP_SPC, true);

    /* Disable all modules that controlled by SPC in active mode.. */
    SPC_DisableActiveModeAnalogModules(APP_SPC, kSPC_controlAllModules);

    /* Disable LVDs and HVDs */
    SPC_EnableActiveModeCoreLowVoltageDetect(APP_SPC, false);
    SPC_EnableActiveModeSystemHighVoltageDetect(APP_SPC, false);
    SPC_EnableActiveModeSystemLowVoltageDetect(APP_SPC, false);

    activeModeRegulatorOption.bandgapMode = kSPC_BandgapEnabledBufferDisabled;
    /* DCDC output voltage is 1.1V in active mode. */
    activeModeRegulatorOption.CoreLDOOption.CoreLDOVoltage       = kSPC_CoreLDO_MidDriveVoltage;
    activeModeRegulatorOption.CoreLDOOption.CoreLDODriveStrength = kSPC_CoreLDO_NormalDriveStrength;

    status = SPC_SetActiveModeRegulatorsConfig(APP_SPC, &activeModeRegulatorOption);
#if !(defined(FSL_FEATURE_MCX_SPC_HAS_NO_GLITCH_DETECT) && FSL_FEATURE_MCX_SPC_HAS_NO_GLITCH_DETECT)
    /* Disable Vdd Core Glitch detector in active mode. */
    SPC_DisableActiveModeVddCoreGlitchDetect(APP_SPC, true);
#endif
    if (status != kStatus_Success)
    {
        PRINTF("Fail to set regulators in Active mode.");
        return;
    }
    while (SPC_GetBusyStatusFlag(APP_SPC))
        ;

    SPC_DisableLowPowerModeAnalogModules(APP_SPC, kSPC_controlAllModules);
    SPC_SetLowPowerWakeUpDelay(APP_SPC, 0xFF);
    spc_lowpower_mode_regulators_config_t lowPowerRegulatorOption;

    lowPowerRegulatorOption.lpIREF                             = false;
    lowPowerRegulatorOption.bandgapMode                        = kSPC_BandgapDisabled;
    lowPowerRegulatorOption.CoreLDOOption.CoreLDOVoltage       = kSPC_Core_LDO_RetentionVoltage;
    lowPowerRegulatorOption.CoreLDOOption.CoreLDODriveStrength = kSPC_CoreLDO_LowDriveStrength;

    status = SPC_SetLowPowerModeRegulatorsConfig(APP_SPC, &lowPowerRegulatorOption);
#if !(defined(FSL_FEATURE_MCX_SPC_HAS_NO_GLITCH_DETECT) && FSL_FEATURE_MCX_SPC_HAS_NO_GLITCH_DETECT)
    /* Disable Vdd Core Glitch detector in low power mode. */
    SPC_DisableLowPowerModeVddCoreGlitchDetect(APP_SPC, true);
#endif
    if (status != kStatus_Success)
    {
        PRINTF("Fail to set regulators in Low Power Mode.");
        return;
    }
    while (SPC_GetBusyStatusFlag(APP_SPC))
        ;

    /* Enable Low power request output to observe the entry/exit of
     * low power modes(including: deep sleep mode, power down mode, and deep power down mode).
     */
    spc_lowpower_request_config_t lpReqConfig = {
        .enable   = true,
        .override = kSPC_LowPowerRequestNotForced,
        .polarity = kSPC_LowTruePolarity,
    };

    SPC_SetLowPowerRequestConfig(APP_SPC, &lpReqConfig);
}

static void APP_PowerPostSwitchHook(void)
{
    BOARD_BootClockFRO48M();
    APP_InitDebugConsole();
}

void APP_InitDebugConsole(void)
{
    /*
     * Debug console RX pin is set to disable for current leakage, need to re-configure pinmux.
     * Debug console TX pin: Don't need to change.
     */
    BOARD_InitPins();
    BOARD_BootClockFRO48M();
    BOARD_InitDebugConsole();
}
