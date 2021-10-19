/*
 * Copyright 2019,2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v8.0
processor: MKW38A512xxx4
package_id: MKW38A512VFT4
mcu_data: ksdk2_0
processor_version: 0.9.0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

#include "fsl_common.h"
#include "fsl_port.h"
#include "pin_mux.h"

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 *
 * END ****************************************************************************************************************/
void BOARD_InitBootPins(void)
{
    BOARD_InitPins();
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '6', peripheral: SPI1, signal: SCK, pin_signal: PTA18/LLWU_P6/SPI1_SCK/LPUART1_TX/CAN0_RX/TPM2_CH0, direction: INPUT}
  - {pin_num: '5', peripheral: SPI1, signal: SIN, pin_signal: PTA17/LLWU_P5/SPI1_SIN/LPUART1_RX/CAN0_TX/TPM_CLKIN1}
  - {pin_num: '4', peripheral: SPI1, signal: SOUT, pin_signal: PTA16/LLWU_P4/SPI1_SOUT/LPUART1_RTS_b/TPM0_CH0}
  - {pin_num: '42', peripheral: LPUART0, signal: RX, pin_signal: PTC6/LLWU_P14/RF_RFOSC_EN/I2C1_SCL/LPUART0_RX/TPM2_CH0}
  - {pin_num: '43', peripheral: LPUART0, signal: TX, pin_signal: PTC7/LLWU_P15/SPI0_PCS2/I2C1_SDA/LPUART0_TX/TPM2_CH1}
  - {pin_num: '40', peripheral: SPI1, signal: PCS0_SS, pin_signal: PTC4/LLWU_P12/RF_ACTIVE/ANT_A/EXTRG_IN/LPUART0_CTS_b/TPM1_CH0/I2C0_SCL/SPI1_PCS0/CAN0_RX, direction: INPUT,
    slew_rate: fast}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitPins(void)
{
    /* Port A Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortA);
    /* Port C Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortC);

    /* PORTA16 (pin 4) is configured as SPI1_SOUT */
    PORT_SetPinMux(PORTA, 16U, kPORT_MuxAlt2);

    /* PORTA17 (pin 5) is configured as SPI1_SIN */
    PORT_SetPinMux(PORTA, 17U, kPORT_MuxAlt2);

    /* PORTA18 (pin 6) is configured as SPI1_SCK */
    PORT_SetPinMux(PORTA, 18U, kPORT_MuxAlt2);

    /* PORTC4 (pin 40) is configured as SPI1_PCS0 */
    PORT_SetPinMux(PORTC, 4U, kPORT_MuxAlt8);

    PORTC->PCR[4] = ((PORTC->PCR[4] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_SRE_MASK | PORT_PCR_ISF_MASK)))

                     /* Slew Rate Enable: Fast slew rate is configured on the corresponding pin, if the pin is
                      * configured as a digital output. */
                     | PORT_PCR_SRE(kPORT_FastSlewRate));

    /* PORTC6 (pin 42) is configured as LPUART0_RX */
    PORT_SetPinMux(PORTC, 6U, kPORT_MuxAlt4);

    /* PORTC7 (pin 43) is configured as LPUART0_TX */
    PORT_SetPinMux(PORTC, 7U, kPORT_MuxAlt4);

    SIM->SOPT5 = ((SIM->SOPT5 &
                   /* Mask bits to zero which are setting */
                   (~(SIM_SOPT5_LPUART0TXSRC_MASK | SIM_SOPT5_LPUART0RXSRC_MASK)))

                  /* LPUART0 Transmit Data Source Select: LPUART0_TX pin. */
                  | SIM_SOPT5_LPUART0TXSRC(SOPT5_LPUART0TXSRC_0b00)

                  /* LPUART0 Receive Data Source Select: LPUART_RX pin. */
                  | SIM_SOPT5_LPUART0RXSRC(SOPT5_LPUART0RXSRC_0b0));
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
