 /******************************************************************************
 *
 * Module: Port
 *
 * File Name: Port_Lcfg.c
 *
 * Description: Post Build Configuration Source file for TM4C123GH6PM Microcontroller - Port Driver
 *
 * Author: Ahmed Essam
 ******************************************************************************/

/*
 * Module Version 1.0.0
 */
#define PORT_LCFG_SW_MAJOR_VERSION              (1U)
#define PORT_LCFG_SW_MINOR_VERSION              (0U)
#define PORT_LCFG_SW_PATCH_VERSION              (0U)

/*
 * AUTOSAR Version 4.0.3
 */
#define PORT_LCFG_AR_RELEASE_MAJOR_VERSION     (4U)
#define PORT_LCFG_AR_RELEASE_MINOR_VERSION     (0U)
#define PORT_LCFG_AR_RELEASE_PATCH_VERSION     (3U)

#include "Port.h"

/* AUTOSAR Version checking between Port_PBcfg.c and Port.h files */
#if ((PORT_LCFG_AR_RELEASE_MAJOR_VERSION != PORT_AR_RELEASE_MAJOR_VERSION)\
 ||  (PORT_LCFG_AR_RELEASE_MINOR_VERSION != PORT_AR_RELEASE_MINOR_VERSION)\
 ||  (PORT_LCFG_AR_RELEASE_PATCH_VERSION != PORT_AR_RELEASE_PATCH_VERSION))
  #error "The AR version of Port_Lcfg.c does not match the expected version"
#endif

/* Software Version checking between Port_PBcfg.c and Dio.h files */
#if ((PORT_LCFG_SW_MAJOR_VERSION != PORT_SW_MAJOR_VERSION)\
 ||  (PORT_LCFG_SW_MINOR_VERSION != PORT_SW_MINOR_VERSION)\
 ||  (PORT_LCFG_SW_PATCH_VERSION != PORT_SW_PATCH_VERSION))
  #error "The SW version of Port_Lcfg.c does not match the expected version"
#endif
   
