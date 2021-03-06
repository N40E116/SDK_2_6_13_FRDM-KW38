/*
 * Copyright (c) 2013-2015 Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "utilities/fsl_assert.h"
#include "bootloader/bl_context.h"
#include "bootloader_common.h"
#include "packet/command_packet.h"
#include "fsl_flexcan.h"
#include "fsl_device_registers.h"
#include "packet/serial_packet.h"
#include "utilities/fsl_rtos_abstraction.h"
#include "bootloader/bl_peripheral_interface.h"

#if defined(BL_CONFIG_CAN) && BL_CONFIG_CAN

//! @addtogroup flexcan_peripheral
//! @{

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

/*!
 * @brief Internal driver state information.
 *
 * @note The contents of this structure are internal to the driver and should not be
 *      modified by users. Also, contents of the structure are subject to change in
 *      future releases.
 */
typedef struct FlexCANState
{
    volatile uint32_t rx_mb_idx; /*!< Index of the message buffer for receiving*/
    volatile uint32_t tx_mb_idx; /*!< Index of the message buffer for transmitting*/
    semaphore_t txIrqSync;       /*!< Used to wait for ISR to complete its TX business.*/
    semaphore_t rxIrqSync;       /*!< Used to wait for ISR to complete its RX business.*/
} flexcan_state_t;

/*! @brief FlexCAN data info from user*/
typedef struct FlexCANDataInfo
{
    flexcan_frame_format_t msg_id_type; /*!< Type of message ID (standard or extended)*/
    uint32_t data_length;               /*!< Length of Data in Bytes*/
} flexcan_data_info_t;

/*! @brief FlexCAN operation modes*/
typedef enum _flexcan_operation_modes
{
    kFlexCanNormalMode,     /*!< Normal mode or user mode*/
    kFlexCanListenOnlyMode, /*!< Listen-only mode*/
    kFlexCanLoopBackMode,   /*!< Loop-back mode*/
    kFlexCanFreezeMode,     /*!< Freeze mode*/
    kFlexCanDisableMode,    /*!< Module disable mode*/
} flexcan_operation_modes_t;

//! @brief flexCAN global information structure.
typedef struct _flexcan_transfer_info
{
    flexcan_state_t state;       //!< state
    flexcan_data_info_t rx_info; //!< tx info
    flexcan_data_info_t tx_info; //!< tx info
    int8_t baudrate;             //!< baudrate (index)
    flexcan_operation_modes_t mode;
    bool baudrateDetect; //!< auto baudrate detection
    uint16_t txId;
    uint16_t rxId;
    void (*data_source)(uint8_t *source_byte);               // !< Callback used to get byte to transmit.
    void (*data_sink)(uint8_t sink_byte, uint32_t instance); // !< Callback used to put received byte.
} flexcan_transfer_info_t;

//! @brief flexCAN enums.
#define FLEXCAN_RX_MB 8u
#define FLEXCAN_TX_MB 9u
#define FLEXCAN_DATA_LENGTH 8u
#define FLEXCAN_MAX_MB 16u
#define FLEXCAN_MAX_SPEED 5u

#define FLEXCAN_DEFAULT_RX_ID 0x321u // default rxid 801
#define FLEXCAN_DEFAULT_TX_ID 0x123u // default txid 291
#define FLEXCAN_DEFAULT_SPEED 4u

// config1 8 bit
#define FLEXCAN_SPEED_MASK 0x0Fu       // bit[3:0]
#define FLEXCAN_SPEED_SPEC_MASK 0x08u  // bit[3:3]
#define FLEXCAN_SPEED_INDEX_MASK 0x07u // bit[2:0]
#define FLEXCAN_CLKSEL_MASK 0x80u      // bit[7:7]

#define FLEXCAN_PROPSEG_MASK 0x70u // bit[6:4]
#define FLEXCAN_PROPSEG_SHIFT 4u

// config2 16 bit
#define FLEXCAN_PRESCALER_MASK 0xFF00u // bit[15:8]
#define FLEXCAN_PRESCALER_SHIFT 8u
#define FLEXCAN_PSEG1_MASK 0x00E0u // bit[7:5]
#define FLEXCAN_PSEG1_SHIFT 5u
#define FLEXCAN_PSEG2_MASK 0x001Cu // bit[4:2]
#define FLEXCAN_PSEG2_SHIFT 2u
#define FLEXCAN_RJW_MASK 0x0003u // bit[1:0]
#define FLEXCAN_RJW_SHIFT 0u

#define FLEXCAN_ERROR_UPDATE_WAIT_CNT 100u

////////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////////

//! @brief flexCAN poll for activity function
static bool flexcan_poll_for_activity(const peripheral_descriptor_t *self);
//! @brief flexCAN init function
static status_t flexcan_full_init(const peripheral_descriptor_t *self, serial_byte_receive_func_t function);
//! @brief flexCAN shutdown function
static void flexcan_full_shutdown(const peripheral_descriptor_t *self);
//! @brief flexCAN receiving first byte data sink function
static void flexcan_initial_data_sink(uint8_t sink_byte, uint32_t instance);
//! @brief flexCAN receiving data sink function
static void flexcan_data_sink(uint8_t sink_byte, uint32_t instance);
//! @brief flexCAN internal init function
static void flexcan_peripheral_init(uint32_t instance);
//! @brief flexCAN sending data function
static status_t flexcan_write(const peripheral_descriptor_t *self, const uint8_t *buffer, uint32_t byteCount);
static status_t FLEXCAN_Send(uint8_t instance,
                             uint32_t mb_idx,
                             flexcan_data_info_t *tx_info,
                             uint32_t msg_id,
                             uint8_t *mb_data,
                             uint32_t timeout_ms);
void FLEXCAN_IRQBusoffHandler(uint8_t instance);
////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

/*!
 * @brief FlexCAN bit timing parameters calculated by using the method outlined in AN1798, section 4.1.
 */

/*!
 * @brief The table contains propseg, pseg1, pseg2, pre_divider, and rjw.
 */
#ifdef KV46F16_SERIES
/* 8Mhz osc clock based */
const flexcan_timing_config_t bit_rate_table_8Mhz[FLEXCAN_MAX_SPEED] = {
    { 7, 2, 2, 2, 0 }, /* 125 kHz */
    { 3, 2, 2, 2, 0 }, /* 250 kHz */
    { 1, 1, 1, 1, 2 }, /* 500 kHz */
    { 0, 2, 2, 2, 2 }, /* 750 kHz */
    { 0, 1, 1, 1, 2 }  /* 1   MHz */
};

/* 20.9715Mhz internal clock based */
const flexcan_timing_config_t bit_rate_table[] = {
    { 7, 3, 7, 7, 3 }, /* 125 kHz */
    { 3, 3, 7, 7, 3 }, /* 250 kHz */
    { 1, 3, 7, 7, 3 }, /* 500 kHz */
    { 1, 3, 3, 3, 5 }, /* 750 kHz */
    { 0, 3, 5, 5, 7 }  /* 1   MHz */
};
#endif

#ifdef KV58F24_SERIES
/* 20.9715Mhz internal clock based */
const flexcan_timing_config_t bit_rate_table[] = {
    { 7, 3, 7, 7, 3 }, /* 125 kHz */
    { 3, 3, 7, 7, 3 }, /* 250 kHz */
    { 1, 3, 7, 7, 3 }, /* 500 kHz */
    { 1, 3, 3, 3, 5 }, /* 750 kHz */
    { 0, 3, 5, 5, 7 }  /* 1   MHz */
};
#endif

#if defined(CPU_PKE18F512VLH15) || defined(CPU_MKE18Z256VLH7)
/* 24Mhz clock based (ROM version) */
const flexcan_timing_config_t bit_rate_table[FLEXCAN_MAX_SPEED] = {
    { 15, 3, 4, 4, 0 }, /* 125 kHz */
    { 7, 3, 4, 4, 0 },  /* 250 kHz */
    { 3, 3, 3, 3, 2 },  /* 500 kHz */
    { 3, 1, 1, 1, 2 },  /* 750 kHz */
    { 1, 2, 2, 2, 4 }   /* 1   MHz */
};

/* 12Mhz clock based (FPGA version only) */
const flexcan_timing_config_t bit_rate_table_12Mhz[FLEXCAN_MAX_SPEED] = {
    { 7, 3, 4, 4, 0 }, /* 125 kHz */
    { 3, 3, 4, 4, 0 }, /* 250 kHz */
    { 1, 3, 3, 3, 2 }, /* 500 kHz */
    { 1, 1, 1, 1, 2 }, /* 750 kHz */
    { 0, 2, 2, 2, 4 }  /* 1   MHz */
};

/* 6Mhz clock based (FPGA version only) */
const flexcan_timing_config_t bit_rate_table_6Mhz[FLEXCAN_MAX_SPEED] = {
    { 3, 3, 4, 4, 0 }, /* 125 kHz */
    { 1, 3, 4, 4, 0 }, /* 250 kHz */
    { 0, 3, 3, 3, 2 }, /* 500 kHz */
    { 0, 1, 1, 1, 2 }, /* 750 kHz */
    { 0, 0, 0, 1, 1 }  /* 1   MHz */
};

/* 8Mhz clock based (real hardware version) */
const flexcan_timing_config_t bit_rate_table_8Mhz[FLEXCAN_MAX_SPEED] = {
    { 7, 2, 2, 2, 0 }, /* 125 kHz */
    { 3, 2, 2, 2, 0 }, /* 250 kHz */
    { 1, 1, 1, 1, 2 }, /* 500 kHz */
    { 0, 2, 2, 2, 2 }, /* 750 kHz */
    { 0, 1, 1, 1, 2 }  /* 1   MHz */
};
#endif // CPU_PKE18F512VLH15

#ifdef KV11Z7_SERIES 
/* 10Mhz osc clock based */
const flexcan_timing_config_t bit_rate_table_10Mhz[FLEXCAN_MAX_SPEED] = {
    { 7, 3, 3, 3, 0 }, /* 125 kHz */
    { 3, 3, 3, 3, 0 }, /* 250 kHz */
    { 1, 2, 2, 2, 2 }, /* 500 kHz */
    { 0, 3, 3, 3, 3 }, /* 750 kHz */
    { 0, 1, 1, 1, 4 }  /* 1   MHz */
};

/* 20.9715Mhz internal clock based */
const flexcan_timing_config_t bit_rate_table[] = {
    { 7, 3, 7, 7, 3 }, /* 125 kHz */
    { 3, 3, 7, 7, 3 }, /* 250 kHz */
    { 1, 3, 7, 7, 3 }, /* 500 kHz */
    { 1, 3, 3, 3, 5 }, /* 750 kHz */
    { 0, 3, 5, 5, 7 }  /* 1   MHz */
};
#endif

#ifdef KS22F12_SERIES
#if defined(BL_TARGET_FLASH)
/* IRC48Mhz clock based */
const flexcan_timing_config_t bit_rate_table[] = {
    { 23, 3, 4, 4, 4 }, /* 125 kHz */
    { 11, 3, 4, 4, 4 }, /* 250 kHz */
    { 5, 3, 4, 4, 4 },  /* 500 kHz */
    { 3, 3, 4, 4, 4 },  /* 750 kHz */
    { 2, 3, 4, 4, 4 }   /* 1   MHz */
};
#elif defined(BL_TARGET_RAM)
/* 20.9715Mhz internal clock based */
const flexcan_timing_config_t bit_rate_table[] = {
    { 7, 3, 7, 7, 3 }, /* 125 kHz */
    { 3, 3, 7, 7, 3 }, /* 250 kHz */
    { 1, 3, 7, 7, 3 }, /* 500 kHz */
    { 1, 3, 3, 3, 5 }, /* 750 kHz */
    { 0, 3, 5, 5, 7 }  /* 1   MHz */
};
#else
#error "No specified bit rate table for current build!"
#endif // defined(BL_TARGET_FLASH)
#endif

#ifdef BL_FEATURE_CORE_CLOCK_DEFAULT
/* 20.9715Mhz internal clock based */
extern const flexcan_timing_config_t bit_rate_table[];
const flexcan_timing_config_t bit_rate_table[] = {
    { 7, 3, 7, 7, 3 }, /* 125 kHz */
    { 3, 3, 7, 7, 3 }, /* 250 kHz */
    { 1, 3, 7, 7, 3 }, /* 500 kHz */
    { 1, 3, 3, 3, 5 }, /* 750 kHz */
    { 0, 3, 5, 5, 7 }  /* 1   MHz */
};
#endif


/*!
 * @brief flexCAN control interface information
 */
const peripheral_control_interface_t g_flexcanControlInterface = {.pollForActivity = flexcan_poll_for_activity,
                                                                  .init = flexcan_full_init,
                                                                  .shutdown = flexcan_full_shutdown,
                                                                  .pump = 0u };

/*!
 * @brief flexCAN byte interface information
 */
const peripheral_byte_inteface_t g_flexcanByteInterface = {
    .init = NULL, .write = flexcan_write,
};

//! @brief Global state for the FLEXCAN peripheral interface.
extern flexcan_transfer_info_t s_flexcanInfo;
flexcan_transfer_info_t s_flexcanInfo = {0u};

//! @brief Flag for flexcan intialization state
static bool s_flexcanIntialized[FSL_FEATURE_SOC_FLEXCAN_COUNT] = { (_Bool)false };
static bool s_flexcanActivity[FSL_FEATURE_SOC_FLEXCAN_COUNT] = { (_Bool)false };

/*!
 * @brief flexCAN receiving data call back function
 */
static serial_byte_receive_func_t s_flexcan_app_data_sink_callback;
const static uint32_t g_flexcanBaseAddr[] = CAN_BASE_ADDRS;
/* Tables to save CAN IRQ enum numbers defined in CMSIS header file. */
const IRQn_Type g_flexcanRxWarningIrqId[] = CAN_Rx_Warning_IRQS;
const IRQn_Type g_flexcanTxWarningIrqId[] = CAN_Tx_Warning_IRQS;
const IRQn_Type g_flexcanWakeUpIrqId[] = CAN_Wake_Up_IRQS;
const IRQn_Type g_flexcanErrorIrqId[] = CAN_Error_IRQS;
const IRQn_Type g_flexcanBusOffIrqId[] = CAN_Bus_Off_IRQS;
const IRQn_Type g_flexcanOredMessageBufferIrqId[] = CAN_ORed_Message_buffer_IRQS;

void FLEXCAN_SoftReset(CAN_Type *baseAddr);
void FLEXCAN_ExitFreezeMode(CAN_Type *baseAddr);
void FLEXCAN_EnterFreezeMode(CAN_Type *baseAddr);
void FLEXCAN_EnableOperationMode(CAN_Type *baseAddr, flexcan_operation_modes_t mode);
void FLEXCAN_DisableOperationMode(CAN_Type *baseAddr, flexcan_operation_modes_t mode);
void FLEXCAN_IRQErrorHandler(uint8_t instance);
void CAN0_Error_IRQHandler(void);
void CAN0_Bus_Off_IRQHandler(void);
void FLEXCAN_CANIRQHandler(uint8_t instance);
void FLEXCAN_IRQHandler(uint8_t instance);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_SoftReset
 * Description   : Perform a soft reset to FexCAN.
 * This function will perfomr a soft reset to FlexCAN.
 *
 *END**************************************************************************/
void FLEXCAN_SoftReset(CAN_Type *baseAddr)
{
    baseAddr->MCR |= (uint32_t)CAN_MCR_SOFTRST_MASK;

    /* Wait till exit of reset*/
    while ((baseAddr->MCR & (uint32_t)CAN_MCR_SOFTRST_MASK) != 0u)
    {
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_ExitFreezeMode
 * Description   : Exit of freeze mode.
 *
 *END**************************************************************************/
void FLEXCAN_ExitFreezeMode(CAN_Type *baseAddr)
{
    baseAddr->MCR &= ~CAN_MCR_HALT_MASK;
    baseAddr->MCR &= ~CAN_MCR_FRZ_MASK;

    /* Wait till exit freeze mode*/
    while ((baseAddr->MCR & CAN_MCR_FRZACK_MASK) != 0u)
    {
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_EnterFreezeMode
 * Description   : Enter the freeze mode.
 *
 *END**************************************************************************/
void FLEXCAN_EnterFreezeMode(CAN_Type *baseAddr)
{
    baseAddr->MCR |= CAN_MCR_FRZ_MASK;
    baseAddr->MCR |= CAN_MCR_HALT_MASK;

    /* Wait for entering the freeze mode*/
    while ((baseAddr->MCR & CAN_MCR_FRZACK_MASK) == 0u)
    {
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_EnableOperationMode
 * Description   : Enable a FlexCAN operation mode.
 * This function will enable one of the modes listed in flexcan_operation_modes_t.
 *
 *END**************************************************************************/
void FLEXCAN_EnableOperationMode(CAN_Type *baseAddr, flexcan_operation_modes_t mode)
{
    if (mode == kFlexCanFreezeMode)
    {
        /* Debug mode, Halt and Freeze*/
        FLEXCAN_EnterFreezeMode(baseAddr);
    }
    else if (mode == kFlexCanDisableMode)
    {
        /* Debug mode, Halt and Freeze*/
        baseAddr->MCR |= CAN_MCR_MDIS_MASK;
    }
    else
    {
       /* nothing to handle */
    }

    /* Set Freeze mode*/
    FLEXCAN_EnterFreezeMode(baseAddr);

    if (mode == kFlexCanNormalMode)
    {
        baseAddr->MCR &= ~CAN_MCR_SUPV_MASK;
    }
    else if (mode == kFlexCanListenOnlyMode)
    {
        baseAddr->CTRL1 |= CAN_CTRL1_LOM_MASK;
    }
    else if (mode == kFlexCanLoopBackMode)
    {
        baseAddr->CTRL1 |= CAN_CTRL1_LPB_MASK;
    }
    else
    {
        /* nothing to handle */
    }

    /* De-assert Freeze Mode*/
    FLEXCAN_ExitFreezeMode(baseAddr);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DisableOperationMode
 * Description   : Disable a FlexCAN operation mode.
 * This function will disable one of the modes listed in flexcan_operation_modes_t.
 *
 *END**************************************************************************/
void FLEXCAN_DisableOperationMode(CAN_Type *baseAddr, flexcan_operation_modes_t mode)
{
    if (mode == kFlexCanFreezeMode)
    {
        /* De-assert Freeze Mode*/
        FLEXCAN_ExitFreezeMode(baseAddr);
    }
    else if (mode == kFlexCanDisableMode)
    {
        /* Disable module mode*/
        baseAddr->MCR &= ~CAN_MCR_MDIS_MASK;
    }
    else
    {
        /* nothing to hanlde */
    }

    /* Set Freeze mode*/
    FLEXCAN_EnterFreezeMode(baseAddr);

    if (mode == kFlexCanNormalMode)
    {
        baseAddr->MCR |= CAN_MCR_SUPV_MASK;
    }
    else if (mode == kFlexCanListenOnlyMode)
    {
        baseAddr->CTRL1 &= ~CAN_CTRL1_LOM_MASK;
    }
    else if (mode == kFlexCanLoopBackMode)
    {
        baseAddr->CTRL1 &= ~CAN_CTRL1_LPB_MASK;
    }
    else
    {
       /* nothing to handle */
    }

    /* De-assert Freeze Mode*/
    FLEXCAN_ExitFreezeMode(baseAddr);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : flexcan_poll_for_activity
 * Description   : Polling for FlexCAN activities
 *
 *END**************************************************************************/
static bool flexcan_poll_for_activity(const peripheral_descriptor_t *self)
{
    return s_flexcanActivity[self->instance];
}

/*FUNCTION**********************************************************************
 *
 * Function Name : flexcan_initial_data_sink
 * Description   : Receiving first byte data sink function
 *
 *END**************************************************************************/
static void flexcan_initial_data_sink(uint8_t sink_byte, uint32_t instance)
{
    if (sink_byte == (uint8_t)kFramingPacketStartByte)
    {
        s_flexcanActivity[instance] = true;
        s_flexcanInfo.data_sink = flexcan_data_sink;
        s_flexcan_app_data_sink_callback(sink_byte);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : flexcan_data_sink
 * Description   : Receiving data sink function
 *
 *END**************************************************************************/
static void flexcan_data_sink(uint8_t sink_byte, uint32_t instance)
{
    s_flexcan_app_data_sink_callback(sink_byte);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : flexcan_peripheral_init
 * Description   : Internal init function
 *
 *END**************************************************************************/
static void flexcan_peripheral_init(uint32_t instance)
{
    flexcan_config_t config;
    uint8_t canConfig1;
    uint16_t canConfig2;
    uint16_t canId;
    uint32_t baseAddr = g_flexcanBaseAddr[instance];

    FLEXCAN_GetDefaultConfig(&config);
    config.clkSrc = kFLEXCAN_ClkSrcPeri;

    flexcan_timing_config_t bit_rate_table1;
    config.maxMbNum = FLEXCAN_MAX_MB;

    // Read the can txId. If it is not set, i.e. 0xff,
    canId = g_bootloaderContext.propertyInterface->store->configurationData.canTxId;
    if (canId != 0xffffu)
    {
        s_flexcanInfo.txId = canId & 0x7ffu; // support 11bit std id format
    }
    else
    {
        s_flexcanInfo.txId = FLEXCAN_DEFAULT_TX_ID;
    }

    // Read the can rxId. If it is not set, i.e. 0xff,
    canId = g_bootloaderContext.propertyInterface->store->configurationData.canRxId;
    if (canId != 0xffffu)
    {
        s_flexcanInfo.rxId = canId & 0x7ffu; // support 11bit std id format
    }
    else
    {
        s_flexcanInfo.rxId = FLEXCAN_DEFAULT_RX_ID;
    }

    // Read the can config1. If it is not set, i.e. 0xff,
    canConfig1 = g_bootloaderContext.propertyInterface->store->configurationData.canConfig1;

    // check baud rate config
    if ((canConfig1 & (uint8_t)FLEXCAN_SPEED_MASK) == 0x0fu)
    {
        // not specified, go with auto detection feature enabled
        s_flexcanInfo.baudrateDetect = true;

        // select default baud rate 1M
        s_flexcanInfo.baudrate = (int8_t)FLEXCAN_MAX_SPEED - 1;
    }
    else
    {
        // baud rate config specified
        s_flexcanInfo.baudrateDetect = false;

        if ((canConfig1 & (uint8_t)FLEXCAN_SPEED_SPEC_MASK) != 0u)
        {
            // specified baud rate setting directly
            s_flexcanInfo.baudrate = (int8_t)FLEXCAN_MAX_SPEED;

            // get config data from BCA area
            canConfig2 = g_bootloaderContext.propertyInterface->store->configurationData.canConfig2;

            // prepare specified config data
            bit_rate_table1.preDivider = (canConfig2 & FLEXCAN_PRESCALER_MASK) >> FLEXCAN_PRESCALER_SHIFT;
            bit_rate_table1.phaseSeg1 = (uint8_t)((canConfig2 & FLEXCAN_PSEG1_MASK) >> FLEXCAN_PSEG1_SHIFT);
            bit_rate_table1.phaseSeg2 = (uint8_t)((canConfig2 & FLEXCAN_PSEG2_MASK) >> FLEXCAN_PSEG2_SHIFT);
            bit_rate_table1.rJumpwidth = (uint8_t)(canConfig2 & FLEXCAN_RJW_MASK);
            bit_rate_table1.propSeg = (uint8_t)((canConfig1 & FLEXCAN_PROPSEG_MASK) >> FLEXCAN_PROPSEG_SHIFT);
        }
        else
        {
            // specified speed index, baud rate settings from bootloader
            if (canConfig1 >= FLEXCAN_MAX_SPEED)
            {
                s_flexcanInfo.baudrate = (int8_t)FLEXCAN_MAX_SPEED - 1;;
            }
            else
            {
                s_flexcanInfo.baudrate = (int8_t)(canConfig1);
            }
        }
    }

    s_flexcanInfo.data_sink = flexcan_initial_data_sink;

    switch (s_flexcanInfo.baudrate)
    {
        case 0:
            config.baudRate = 125000u;
            break;
        case 1:
            config.baudRate = 256000u;
            break;
        case 2:
            config.baudRate = 500000u;
            break;
        case 3:
        case 4:
        default:
            config.baudRate = 1000000u;
            break;
    }

    /* Init the interrupt sync object.*/
    (void)OSA_SemaCreate(&s_flexcanInfo.state.txIrqSync, 0u);
    (void)OSA_SemaCreate(&s_flexcanInfo.state.rxIrqSync, 0u);
    s_flexcanInfo.state.rx_mb_idx = FLEXCAN_RX_MB;
    s_flexcanInfo.state.tx_mb_idx = FLEXCAN_TX_MB;

    // also need to get clock selection config data
    FLEXCAN_Init((CAN_Type *)baseAddr, &config, 1000000u * 8u * 10u);

    FLEXCAN_Enable((CAN_Type *)baseAddr, true);

    FLEXCAN_EnableInterrupts((CAN_Type *)baseAddr, (uint32_t)kFLEXCAN_ErrorInterruptEnable);

    if (s_flexcanInfo.baudrate == (int8_t)FLEXCAN_MAX_SPEED)
    {
        // specified baud rate settings directly, need to get other config data
        s_flexcanInfo.baudrate = (int8_t)FLEXCAN_MAX_SPEED - 1;

        // using provided settings
        FLEXCAN_SetTimingConfig((CAN_Type *)baseAddr, &bit_rate_table1);
    }
    else
    {
        // using setting table
        FLEXCAN_SetTimingConfig((CAN_Type *)baseAddr, &bit_rate_table[s_flexcanInfo.baudrate]);
    }

    // if auto detection enabled, then go to listen mode first
    if (s_flexcanInfo.baudrateDetect)
    {
        s_flexcanInfo.mode = kFlexCanListenOnlyMode;
        FLEXCAN_EnableOperationMode((CAN_Type *)baseAddr, kFlexCanListenOnlyMode);
    }

    // FlexCAN reveive config
    s_flexcanInfo.rx_info.msg_id_type = kFLEXCAN_FrameFormatStandard;
    s_flexcanInfo.rx_info.data_length = FLEXCAN_DATA_LENGTH;

    // Configure RX MB fields
    flexcan_rx_mb_config_t mbConfig;
    mbConfig.format = kFLEXCAN_FrameFormatStandard;
    mbConfig.id = CAN_ID_STD(s_flexcanInfo.rxId);
    mbConfig.type = kFLEXCAN_FrameTypeData;
    FLEXCAN_SetRxMbConfig((CAN_Type *)baseAddr, (uint8_t)FLEXCAN_RX_MB, &mbConfig, true);

    FLEXCAN_EnableMbInterrupts((CAN_Type *)baseAddr, ((uint32_t)1u << (uint32_t)FLEXCAN_RX_MB));

    // FlexCAN transfer config
    s_flexcanInfo.tx_info.msg_id_type = kFLEXCAN_FrameFormatStandard;
    s_flexcanInfo.tx_info.data_length = FLEXCAN_DATA_LENGTH;

    FLEXCAN_SetTxMbConfig((CAN_Type *)baseAddr, FLEXCAN_TX_MB, false);

    NVIC_EnableIRQ(g_flexcanErrorIrqId[instance]);
    NVIC_EnableIRQ(g_flexcanBusOffIrqId[instance]);
    NVIC_EnableIRQ(g_flexcanOredMessageBufferIrqId[instance]);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : flexcan_full_init
 * Description   : full init function
 *
 *END**************************************************************************/
static status_t flexcan_full_init(const peripheral_descriptor_t *self, serial_byte_receive_func_t function)
{
    s_flexcan_app_data_sink_callback = function;

    // Configure selected pin as flexcan peripheral interface
    self->pinmuxConfig(self->instance, kPinmuxType_Peripheral);

    flexcan_peripheral_init(self->instance);

    s_flexcanIntialized[self->instance] = true;
    return (int32_t)kStatus_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : flexcan_full_shutdown
 * Description   : full shutdown function
 *
 *END**************************************************************************/
static void flexcan_full_shutdown(const peripheral_descriptor_t *self)
{
    if (s_flexcanIntialized[self->instance])
    {
        uint32_t baseAddr = g_flexcanBaseAddr[self->instance];
        FLEXCAN_Deinit((CAN_Type *)baseAddr);

        // Restore selected pin to default state to reduce IDD.
        self->pinmuxConfig(self->instance, kPinmuxType_Default);

        s_flexcanIntialized[self->instance] = false;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : flexcan_write
 * Description   : Send data function
 *
 *END**************************************************************************/
static status_t flexcan_write(const peripheral_descriptor_t *self, const uint8_t *buffer, uint32_t byteCount)
{
    uint32_t sentCnt = 0u;
    const uint8_t *sendPtr = buffer;

    while (sentCnt < byteCount)
    {
        if ((byteCount - sentCnt) <= 8u)
        {
            s_flexcanInfo.tx_info.data_length = byteCount - sentCnt; // number of bytes to be sent
            sentCnt += byteCount - sentCnt;
        }
        else
        {
            s_flexcanInfo.tx_info.data_length = 8u; // number of bytes to be sent
            sentCnt += 8u;
        }

        
        union
        {
            const uint8_t *address;
            uint8_t *ptr;
        } send_ptr;

        send_ptr.address = sendPtr;
        (void)FLEXCAN_Send((uint8_t)self->instance, FLEXCAN_TX_MB, &s_flexcanInfo.tx_info, s_flexcanInfo.txId, send_ptr.ptr,
                     1000u);
        sendPtr += s_flexcanInfo.tx_info.data_length;
    }

    return (int32_t)kStatus_Success;
}

static status_t FLEXCAN_Send(uint8_t instance,
                      uint32_t mb_idx,
                      flexcan_data_info_t *tx_info,
                      uint32_t msg_id,
                      uint8_t *mb_data,
                      uint32_t timeout_ms)
{
    uint32_t baseAddr = g_flexcanBaseAddr[instance];
    osa_status_t syncStatus;
    uint8_t i;
    flexcan_frame_t frame;
    flexcan_mb_transfer_t xfer;
//    xfer.mbIdx = mb_idx;
    frame.format = (uint8_t)tx_info->msg_id_type;
    frame.length = (uint8_t)tx_info->data_length;
    frame.id = CAN_ID_STD(msg_id);
    frame.type = (uint8_t)kFLEXCAN_FrameTypeData;
    xfer.frame = &frame;
    status_t retStatus = (int32_t)kStatus_Fail;

    /* Copy user's buffer into the message buffer data area*/
    if (mb_data != (void *)0u)
    {
        xfer.frame->dataWord0 = 0x0u;
        xfer.frame->dataWord1 = 0x0u;

        for (i = 0u; i < tx_info->data_length; i++)
        {
            uint32_t temp, temp1;
            temp1 = (*(mb_data + i));
            if (i < 4u)
            {
                temp = temp1 << ((3u - i) * 8u);
                xfer.frame->dataWord0 |= temp;
            }
            else
            {
                temp = temp1 << ((7u - i) * 8u);
                xfer.frame->dataWord1 |= temp;
            }
        }
    }

    if ((int32_t)kStatus_Success == FLEXCAN_WriteTxMb((CAN_Type *)baseAddr, (uint8_t)mb_idx, xfer.frame))
    {
        /* Enable Message Buffer Interrupt. */
        FLEXCAN_EnableMbInterrupts((CAN_Type *)baseAddr, ((uint32_t)1u << mb_idx));

        do
        {
            syncStatus = OSA_SemaWait(&s_flexcanInfo.state.txIrqSync, timeout_ms);
        } while (syncStatus == kStatus_OSA_Idle);

        /* Disable message buffer interrupt*/
        FLEXCAN_DisableMbInterrupts((CAN_Type *)baseAddr, ((uint32_t)1u << mb_idx));

        /* Wait for the interrupt*/
        if (syncStatus != kStatus_OSA_Success)
        {
            retStatus = (int32_t)kStatus_Timeout;
        }
        else
        {
            retStatus = (int32_t)kStatus_Success;
        }
    }
    else
    {
        retStatus = (int32_t)kStatus_Fail;
    }

    return retStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_IRQBusoffHandler
 * Description   : Busoff interrupt handler
 *
 *END**************************************************************************/
void FLEXCAN_IRQBusoffHandler(uint8_t instance)
{
    uint32_t baseAddr = g_flexcanBaseAddr[instance];
    (void)FLEXCAN_GetStatusFlags((CAN_Type *)baseAddr);
    FLEXCAN_ClearStatusFlags((CAN_Type *)baseAddr, (uint32_t)kFLEXCAN_ErrorFlag | CAN_ESR1_ERRINT_MASK);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_IRQErrorHandler
 * Description   : General error interrupt handler including auto baud rate
 *                 detection processing
 *
 *END**************************************************************************/
void FLEXCAN_IRQErrorHandler(uint8_t instance)
{
    CAN_Type *baseAddr = (CAN_Type *)g_flexcanBaseAddr[instance];

    if (s_flexcanInfo.baudrateDetect)
    {
        uint32_t status = FLEXCAN_GetStatusFlags(baseAddr);

        if (s_flexcanInfo.mode == kFlexCanListenOnlyMode)
        {
            if (((status & (CAN_ESR1_RX_MASK | CAN_ESR1_BIT0ERR_MASK)) != 0u) &&
                ((status & (CAN_ESR1_STFERR_MASK | CAN_ESR1_FRMERR_MASK | CAN_ESR1_BIT1ERR_MASK)) == 0u))
            {
                // we found the right baud rate
                FLEXCAN_DisableOperationMode(baseAddr, kFlexCanListenOnlyMode);
                s_flexcanInfo.mode = kFlexCanNormalMode;
            }
            else
            {
                s_flexcanInfo.baudrate--;

                if (s_flexcanInfo.baudrate < 0)
                {
                    s_flexcanInfo.baudrate = (int8_t)FLEXCAN_MAX_SPEED - 1;
                }

                FLEXCAN_SetTimingConfig(baseAddr, &bit_rate_table[s_flexcanInfo.baudrate]);
            }
        }
        else if ((status & (CAN_ESR1_BIT0ERR_MASK | CAN_ESR1_STFERR_MASK | CAN_ESR1_FRMERR_MASK | CAN_ESR1_BIT1ERR_MASK)) != 0u)
        {
            /* Assert Soft Reset Signal. */
            FLEXCAN_SoftReset(baseAddr);
            flexcan_peripheral_init(instance);
        }
        else
        {
           // nothing to handle
        }
    }

    (void)FLEXCAN_GetStatusFlags(baseAddr);
    FLEXCAN_ClearStatusFlags(baseAddr, (uint32_t)kFLEXCAN_ErrorFlag | CAN_ESR1_ERRINT_MASK);
}

void FLEXCAN_IRQHandler(uint8_t instance)
{
    volatile uint32_t flag_reg;
    uint32_t temp;
    CAN_Type *baseAddr = (CAN_Type *)g_flexcanBaseAddr[instance];

    /* Get the interrupts that are enabled and ready */
    flag_reg = (baseAddr->IFLAG1 & baseAddr->IMASK1);

    /* Check Tx/Rx interrupt flag and clear the interrupt */
    if (flag_reg != 0u)
    {
        temp = ((uint32_t)1u << s_flexcanInfo.state.rx_mb_idx);
        if ((flag_reg & temp) != 0u)
        {
            (void)OSA_SemaPost(&s_flexcanInfo.state.rxIrqSync);

            flexcan_frame_t rxFrame;
            /* Get RX MB field values*/
            if (FLEXCAN_ReadRxMb(baseAddr, (uint8_t)s_flexcanInfo.state.rx_mb_idx, &rxFrame) == 0)
            {
                uint8_t i;
                uint8_t sink_byte = 0u;
                for (i = 0u; i < rxFrame.length; i++)
                {
                    switch (i)
                    {
                        case 0u:
                            sink_byte = rxFrame.dataByte0;
                            break;
                        case 1u:
                            sink_byte = rxFrame.dataByte1;
                            break;
                        case 2u:
                            sink_byte = rxFrame.dataByte2;
                            break;
                        case 3u:
                            sink_byte = rxFrame.dataByte3;
                            break;
                        case 4u:
                            sink_byte = rxFrame.dataByte4;
                            break;
                        case 5u:
                            sink_byte = rxFrame.dataByte5;
                            break;
                        case 6u:
                            sink_byte = rxFrame.dataByte6;
                            break;
                        case 7u:
                            sink_byte = rxFrame.dataByte7;
                            break;
                        default:
                            sink_byte = 0u;
                            break;
                    }
                    s_flexcanInfo.data_sink(sink_byte, instance);
                }
            }
        }

        temp = (uint32_t)((uint32_t)1u << s_flexcanInfo.state.tx_mb_idx);
        if ((flag_reg & temp) != 0u)
        {
            (void)OSA_SemaPost(&s_flexcanInfo.state.txIrqSync);
        }

        baseAddr->IFLAG1 = flag_reg;
    }

    /* Clear all other interrupts in ERRSTAT register (Error, Busoff, Wakeup) */
    (void)FLEXCAN_GetStatusFlags(baseAddr);
    FLEXCAN_ClearStatusFlags(baseAddr, (uint32_t)kFLEXCAN_ErrorFlag | CAN_ESR1_ERRINT_MASK);
}

#if defined (KV11Z7_SERIES) || defined (KW36Z4_SERIES) || defined (KW38A4_SERIES)
/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_CAN_IRQ_Handler
 * Description   : General error interrupt handler including auto baud rate
 *                 detection processing for KV11
 *
 *END**************************************************************************/
void FLEXCAN_CANIRQHandler(uint8_t instance)
{
    CAN_Type *baseAddr = (CAN_Type *)g_flexcanBaseAddr[instance];

    uint32_t status = baseAddr->ESR1;
    uint32_t ctrl1 = baseAddr->CTRL1;

    // Bus off interrupt
    if (((ctrl1 & CAN_CTRL1_BOFFMSK_MASK) != 0u) && ((status & CAN_ESR1_BOFFINT_MASK) != 0u))
    {
        FLEXCAN_IRQBusoffHandler(instance);
    }
    // Error interrupt
    else if (((ctrl1 & CAN_CTRL1_ERRMSK_MASK) != 0u) && ((status & CAN_ESR1_ERRINT_MASK) != 0u))
    {
        if (s_flexcanInfo.baudrateDetect)
        {
            if (s_flexcanInfo.mode == kFlexCanListenOnlyMode)
            {
                if (((status & (CAN_ESR1_RX_MASK | CAN_ESR1_BIT0ERR_MASK)) != 0u) &&
                    ((status & (CAN_ESR1_STFERR_MASK | CAN_ESR1_FRMERR_MASK | CAN_ESR1_BIT1ERR_MASK)) == 0u))
                {
                    // we found the right baud rate
                    FLEXCAN_DisableOperationMode(baseAddr, kFlexCanListenOnlyMode);
                    s_flexcanInfo.mode = kFlexCanNormalMode;
                }
                else
                {
                    s_flexcanInfo.baudrate--;

                    if (s_flexcanInfo.baudrate < 0)
                    {
                        s_flexcanInfo.baudrate = (int8_t)FLEXCAN_MAX_SPEED - 1;
                    }

                    FLEXCAN_SetTimingConfig(baseAddr, &bit_rate_table[s_flexcanInfo.baudrate]);
                }
            }
            else
            {
                /* Assert Soft Reset Signal. */
                FLEXCAN_SoftReset(baseAddr);
                flexcan_peripheral_init(instance);
            }
        }

        
        (void)FLEXCAN_GetStatusFlags(baseAddr);
        FLEXCAN_ClearStatusFlags(baseAddr, (uint32_t)kFLEXCAN_ErrorFlag | CAN_ESR1_ERRINT_MASK);
    }
    // wake up interrupt
    else if (((baseAddr->MCR & CAN_MCR_WAKMSK_MASK) != 0u) && ((status & CAN_ESR1_WAKINT_MASK) != 0u))
    {
        FLEXCAN_IRQHandler(instance);
    }
    else
    {
        FLEXCAN_IRQHandler(instance);
    }
}
#endif // KV11Z7_SERIES

#if (FSL_FEATURE_SOC_FLEXCAN_COUNT > 0U)
/* Implementation of CAN0 handler named in startup code. */
#ifdef CPU_PKE18F512VLH15
void CAN0_Oredbuf_IRQHandler(void)
#else
void CAN0_ORed_Message_buffer_IRQHandler(void);
void CAN0_ORed_Message_buffer_IRQHandler(void)
#endif
{
    FLEXCAN_IRQHandler(0u);
}

/* Implementation of CAN0 handler named in startup code. */
void CAN0_Bus_Off_IRQHandler(void)
{
    FLEXCAN_IRQBusoffHandler(0u);
}

/* Implementation of CAN0 handler named in startup code. */
void CAN0_Error_IRQHandler(void)
{
    FLEXCAN_IRQErrorHandler(0u);
}

/* Implementation of CAN0 handler named in startup code. */
#ifdef CPU_PKE18F512VLH15
void CAN0_Wakeup_IRQHandler(void)
#else
void CAN0_Wake_Up_IRQHandler(void);
void CAN0_Wake_Up_IRQHandler(void)
#endif
{
    FLEXCAN_IRQHandler(0u);
}

#if defined (KV11Z7_SERIES) || defined (KW36Z4_SERIES) || defined (KW38A4_SERIES)
void CAN0_IRQHandler(void);
void CAN0_IRQHandler(void)
{
//    FLEXCAN_IRQHandler(0);
    FLEXCAN_CANIRQHandler(0);
}
#endif // KV11Z7_SERIES

#if  defined(KW36Z4_SERIES) || defined (KW38A4_SERIES)
void CAN0_MB_IRQHandler(void);
void CAN0_MB_IRQHandler(void)
{ 
    FLEXCAN_CANIRQHandler(0);
}
#endif

#endif // (FSL_FEATURE_SOC_FLEXCAN_COUNT > 0U)

#if (FSL_FEATURE_SOC_FLEXCAN_COUNT > 1U)
/* Implementation of CAN1 handler named in startup code. */
void CAN1_ORed_Message_buffer_IRQHandler(void);
void CAN1_ORed_Message_buffer_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1u);
}

/* Implementation of CAN1 handler named in startup code. */
void CAN1_Bus_Off_IRQHandler(void);
void CAN1_Bus_Off_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1u);
}

/* Implementation of CAN1 handler named in startup code. */
void CAN1_Error_IRQHandler(void);
void CAN1_Error_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1u);
}

/* Implementation of CAN1 handler named in startup code. */
void CAN1_Wake_Up_IRQHandler(void);
void CAN1_Wake_Up_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1u);
}
#endif

//! @}

#endif // BL_CONFIG_CAN

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
