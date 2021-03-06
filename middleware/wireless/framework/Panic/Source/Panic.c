/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2017 NXP
* All rights reserved.
*
* \file
*
* This is the source file for the Panic module.
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */

#include "Panic.h"
#include "fsl_os_abstraction.h"

/*! *********************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
********************************************************************************** */
#if gUsePanic_c
static panicData_t panic_data;
#endif

/*! *********************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
********************************************************************************** */
#define NOP __asm volatile ("NOP")

/*! *********************************************************************************
* \brief  This function will halt the system
*
* \param[in]  id Description of the param2 in parameter
* \param[in]  location address where the Panic occurred
* \param[in]  extra1 parameter to be stored in Panic structure
* \param[in]  extra2 parameter to be stored in Panic structure
*
********************************************************************************** */

void panic( panicId_t id, uint32_t location, uint32_t extra1, uint32_t extra2 )
{
#if gUsePanic_c
    /* Save the Link Register */
    panic_data.id = id;
    panic_data.location = location;
    panic_data.extra1 = extra1;
    panic_data.extra2 = extra2;
    panic_data.linkRegister = (uint32_t)__get_LR();
    panic_data.cpsr_contents   = 0;
    (void)panic_data;

    OSA_InterruptDisable(); /* disable interrupts */

    /* infinite loop just to ensure this routine never returns */
    for(;;)
    {
        NOP;
    }
#endif
}

#if defined(__GNUC__)
void __attribute__((weak)) __assertion_failed(char* s) {}

#endif
