/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

/*!
 * @addtogroup pin_mux
 * @{
 */

/***********************************************************************************************************************
 * API
 **********************************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Calls initialization functions.
 *
 */
void BOARD_InitBootPins(void);

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins(void);

/*! @name PORTB18 (number 23), J4[2]/A1/SW2
  @{ */
#define BOARD_SW2_GPIO GPIOB /*!<@brief GPIO device name: GPIOB */
#define BOARD_SW2_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_SW2_PIN 18U    /*!<@brief PORTB pin index: 18 */
                             /* @} */

/*! @name PORTC2 (number 38), SW3
  @{ */
#define BOARD_SW3_GPIO GPIOC /*!<@brief GPIO device name: GPIOC */
#define BOARD_SW3_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_SW3_PIN 2U     /*!<@brief PORTC pin index: 2 */
                             /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitButtonsPins(void);

/*! @name PORTB2 (number 18), J4[3]/Q3[1]/A2/LEDRGB_BLUE
  @{ */
#define BOARD_LED_BLUE_GPIO GPIOB /*!<@brief GPIO device name: GPIOB */
#define BOARD_LED_BLUE_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_LED_BLUE_PIN 2U     /*!<@brief PORTB pin index: 2 */
                                  /* @} */

/*! @name PORTA16 (number 4), J2[1]/Q2[1]/D8/LEDRGB_GREEN
  @{ */
#define BOARD_LED_GREEN_GPIO GPIOA /*!<@brief GPIO device name: GPIOA */
#define BOARD_LED_GREEN_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_LED_GREEN_PIN 16U    /*!<@brief PORTA pin index: 16 */
                                   /* @} */

/*! @name PORTC1 (number 37), J2[2]/Q1[1]/D9/LEDRGB_RED
  @{ */
#define BOARD_LED_RED_GPIO GPIOC /*!<@brief GPIO device name: GPIOC */
#define BOARD_LED_RED_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_LED_RED_PIN 1U     /*!<@brief PORTC pin index: 1 */
                                 /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitLEDsPins(void);

#define SOPT5_LPUART0ODE_0b0 0x00u    /*!<@brief LPUART0 Open Drain Enable: Open drain is disabled on LPUART0. */
#define SOPT5_LPUART0RXSRC_0b0 0x00u  /*!<@brief LPUART0 Receive Data Source Select: LPUART_RX pin */
#define SOPT5_LPUART0TXSRC_0b00 0x00u /*!<@brief LPUART0 Transmit Data Source Select: LPUART0_TX pin */

/*! @name PORTC7 (number 43), J10[1]/U5[24]/U6[1]/UART0_TX_TGTMCU
  @{ */
#define BOARD_DEBUG_UART_TX_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_DEBUG_UART_TX_PIN 7U     /*!<@brief PORTC pin index: 7 */
                                       /* @} */

/*! @name PORTC6 (number 42), J8[1]/U4[4]/U5[25]/UART0_RX_TGTMCU
  @{ */
#define BOARD_DEBUG_UART_RX_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_DEBUG_UART_RX_PIN 6U     /*!<@brief PORTC pin index: 6 */
                                       /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitDEBUG_UARTPins(void);

/*! @name EXTAL (number 30), Y1[3]/EXTAL_32M
  @{ */
/* @} */

/*! @name XTAL (number 31), Y1[1]/XTAL_32M
  @{ */
/* @} */

/*! @name PORTB16 (number 21), Y2[1]/EXTAL32K
  @{ */
#define BOARD_EXTAL32K_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_EXTAL32K_PIN 16U    /*!<@brief PORTB pin index: 16 */
                                  /* @} */

/*! @name PORTB17 (number 22), Y2[2]/XTAL32K
  @{ */
#define BOARD_XTAL32K_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_XTAL32K_PIN 17U    /*!<@brief PORTB pin index: 17 */
                                 /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitOSCPins(void);

/*! @name PORTB1 (number 17), J2[9]/U11[6]/D14/I2C_SDA_FXOS8700CQ
  @{ */
#define BOARD_ACCEL_SDA_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_ACCEL_SDA_PIN 1U     /*!<@brief PORTB pin index: 1 */
                                   /* @} */

/*! @name PORTA19 (number 7), J2[3]/U11[11]/D10/INT1_COMBO
  @{ */
#define BOARD_ACCEL_INT1_GPIO GPIOA /*!<@brief GPIO device name: GPIOA */
#define BOARD_ACCEL_INT1_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_ACCEL_INT1_PIN 19U    /*!<@brief PORTA pin index: 19 */
                                    /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitACCELPins(void);

/*! @name PORTC17 (number 46), J9[1]/U3[1]/SI
  @{ */
#define BOARD_FLASH_SI_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_FLASH_SI_PIN 17U    /*!<@brief PORTC pin index: 17 */
                                  /* @} */

/*! @name PORTC16 (number 45), J2[6]/U3[2]/D13/SCK
  @{ */
#define BOARD_FLASH_SCK_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_FLASH_SCK_PIN 16U    /*!<@brief PORTC pin index: 16 */
                                   /* @} */

/*! @name PORTC19 (number 48), J1[3]/U3[4]/D2/CS
  @{ */
#define BOARD_FLASH_CS_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_FLASH_CS_PIN 19U    /*!<@brief PORTC pin index: 19 */
                                  /* @} */

/*! @name PORTC18 (number 47), J7[1]/U3[8]/SO
  @{ */
#define BOARD_FLASH_SO_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_FLASH_SO_PIN 18U    /*!<@brief PORTC pin index: 18 */
                                  /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitFLASHPins(void);

/*! @name ADC0_DM0 (number 25), J4[6]/A5/ADC0_DM0/CMP0_IN1/THER_B
  @{ */
/* @} */

/*! @name ADC0_DP0 (number 24), J4[1]/A0/ADC0_DP0/CMP0_IN0/THER_A
  @{ */
/* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitThermistorPins(void);

/*! @name PORTC4 (number 40), J1[6]/U19[4]/U17[4]/D5/CAN_RX
  @{ */
#define BOARD_CAN_RX_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_CAN_RX_PIN 4U     /*!<@brief PORTC pin index: 4 */
                                /* @} */

/*! @name PORTC3 (number 39), J1[4]/U19[1]/U20[4]/D3/CAN_TX
  @{ */
#define BOARD_CAN_TX_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_CAN_TX_PIN 3U     /*!<@brief PORTC pin index: 3 */
                                /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitCANPins(void);

#define SOPT5_LPUART1ODE_0b0 0x00u    /*!<@brief LPUART1 Open Drain Enable: Open drain is disabled on LPUART1. */
#define SOPT5_LPUART1RXSRC_0b0 0x00u  /*!<@brief LPUART1 Receive Data Source Select: LPUART1_RX pin */
#define SOPT5_LPUART1TXSRC_0b00 0x00u /*!<@brief LPUART1 Transmit Data Source Select: LPUART1_TX pin */

/*! @name PORTA17 (number 5), J1[5]/U10[1]/U12[4]/D4/LIN_RX_LS
  @{ */
#define BOARD_LIN_RX_LS_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_LIN_RX_LS_PIN 17U    /*!<@brief PORTA pin index: 17 */
                                   /* @} */

/*! @name PORTA18 (number 6), J1[7]/U7[2]/U10[4]/D6/LIN_TX_LS
  @{ */
#define BOARD_LIN_TX_LS_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_LIN_TX_LS_PIN 18U    /*!<@brief PORTA pin index: 18 */
                                   /* @} */

/*! @name PORTC5 (number 41), J1[8]/U7[1]/U10[2]/D7/LIN_SLP
  @{ */
#define BOARD_LIN_SLP_GPIO GPIOC /*!<@brief GPIO device name: GPIOC */
#define BOARD_LIN_SLP_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_LIN_SLP_PIN 5U     /*!<@brief PORTC pin index: 5 */
                                 /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitLINPins(void);

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _PIN_MUX_H_ */

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
