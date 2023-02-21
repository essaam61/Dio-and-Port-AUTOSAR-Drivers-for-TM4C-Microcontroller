 /******************************************************************************
 *
 * Module: Port
 *
 * File Name: Port.h
 *
 * Description: Header file for TM4C123GH6PM Microcontroller - Port Driver.
 *
 * Author: Ahmed Essam
 ******************************************************************************/

#ifndef PORT_H
#define PORT_H

/* Id for the company in the AUTOSAR */
#define PORT_VENDOR_ID    (1000U)

/* Port Module Id */
#define PORT_MODULE_ID    (124U)

/* Port Instance Id */
#define PORT_INSTANCE_ID  (0U)

/*
 * Module Version 1.0.0
 */
#define PORT_SW_MAJOR_VERSION           (1U)
#define PORT_SW_MINOR_VERSION           (0U)
#define PORT_SW_PATCH_VERSION           (0U)

/*
 * AUTOSAR Version 4.0.3
 */
#define PORT_AR_RELEASE_MAJOR_VERSION   (4U)
#define PORT_AR_RELEASE_MINOR_VERSION   (0U)
#define PORT_AR_RELEASE_PATCH_VERSION   (3U)

/*
* Macros for Port Status
*/
#define PORT_INITIALIZED                (1U)
#define PORT_NOT_INITIALIZED            (0U)
   
/* Standard AUTOSAR types */
#include "Std_Types.h"
   
/* AUTOSAR checking between Std Types and Port Modules */
#if ((STD_TYPES_AR_RELEASE_MAJOR_VERSION != PORT_AR_RELEASE_MAJOR_VERSION)\
 ||  (STD_TYPES_AR_RELEASE_MINOR_VERSION != PORT_AR_RELEASE_MINOR_VERSION)\
 ||  (STD_TYPES_AR_RELEASE_PATCH_VERSION != PORT_AR_RELEASE_PATCH_VERSION))
  #error "The AR version of Std_Types.h does not match the expected version"
#endif
   
/* Port Pre-Compile Configuration Header file */
#include "Port_Cfg.h"

/* AUTOSAR Version checking between Dio_Cfg.h and Dio.h files */
#if ((PORT_CFG_AR_RELEASE_MAJOR_VERSION != PORT_AR_RELEASE_MAJOR_VERSION)\
 ||  (PORT_CFG_AR_RELEASE_MINOR_VERSION != PORT_AR_RELEASE_MINOR_VERSION)\
 ||  (PORT_CFG_AR_RELEASE_PATCH_VERSION != PORT_AR_RELEASE_PATCH_VERSION))
  #error "The AR version of Port_Cfg.h does not match the expected version"
#endif

/* Software Version checking between Dio_Cfg.h and Dio.h files */
#if ((DIO_CFG_SW_MAJOR_VERSION != DIO_SW_MAJOR_VERSION)\
 ||  (DIO_CFG_SW_MINOR_VERSION != DIO_SW_MINOR_VERSION)\
 ||  (DIO_CFG_SW_PATCH_VERSION != DIO_SW_PATCH_VERSION))
  #error "The SW version of Port_Cfg.h does not match the expected version"
#endif

/* Non AUTOSAR files */
#include "Common_Macros.h"

/******************************************************************************
 *                      API Service Id Macros                                 *
 ******************************************************************************/
/* Service ID for PORT Init Channel */
#define PORT_INIT_SID                           (uint8)0x00

/* Service ID for PORT set Pin Direction */
#define PORT_SET_PIN_DIRECTION_SID              (uint8)0x01

/* Service ID for PORT refresh Port Direction */
#define PORT_REFRESH_PORT_DIRECTION_SID         (uint8)0x02

/* Service ID for PORT GetVersionInfo */
#define PORT_GET_VERSION_INFO_SID               (uint8)0x03

/* Service ID for PORT set Pin Mode */
#define PORT_SET_PIN_MODE_SID                   (uint8)0x04
   
 /*******************************************************************************
 *                      DET Error Codes                                        *
 *******************************************************************************/
/* DET code to report Invalid Port Pin ID */
#define PORT_E_PARAM_PIN                 (uint8)0x0A

/* DET code to report unchangeable Port Pin Direction in configuration */
#define PORT_E_DIRECTION_UNCHANGEABLE   (uint8)0x0B
   
/* Port_Init API service called with NULL pointer parameter */
#define PORT_E_PARAM_CONFIG             (uint8)0x0C

/* Port_SetPinMode API service called with Invalid Mode parameter */
#define PORT_E_PARAM_INVALID_MODE       (uint8)0x0D

/* Port_SetPinMode API service called with unchangeable Port Pin Mode in config */
#define PORT_E_MODE_UNCHANGEABLE        (uint8)0x0E

/*
 * API service used without module initialization is reported using following
 * error code.
 */
#define PORT_E_UNINIT                   (uint8)0x0F

/*
 * The API service shall return immediately without any further action,
 * beside reporting this development error.
 */
#define PORT_E_PARAM_POINTER             (uint8)0x10
   
   
/*******************************************************************************
 *                              Module Data Types                              *
 *******************************************************************************/

/* Description: Unsigned char to hold mode type for PIN */
typedef uint8 Port_PinModeType;

/* Description: Unsigned char to hold the port pin number */
typedef uint8 Port_PinType;
   
/* Description: Enum to hold PIN direction */
typedef enum
{
    PORT_PIN_IN, PORT_PIN_OUT
}Port_PinDirectionType;

/* Description: Enum to hold internal resistor type for PIN */
typedef enum
{
    OFF, PULL_UP, PULL_DOWN
}Port_PinInternalResistorType;

/* Description: Enum to hold output current drive type for PIN */
typedef enum
{
    R2R,
    R4R,
    R8R
}Port_PinOutputCurrentType;

/* Description: Structure to configure each individual PIN:
 *	1. the mode of the port pin
 *	2. the PORT Which the pin belongs to: 0, 1, 2, 3, 4 or 5
 *	3. the number of the pin in the PORT
 *      4. the direction of pin --> INPUT or OUTPUT
 *      5. the internal resistor --> Disable, Pull up, Pull down or Open drain
 *      6. the initial value of pin level
 *      7. the pin direction changeable during run time --> ON or OFF
 *      8. the pin mode changeable during run time --> ON or OFF
 */
typedef struct 
{ 
    Port_PinModeType mode;
    uint8 port_num;
    Port_PinType pin_num;
    Port_PinDirectionType direction;
    Port_PinInternalResistorType resistor;
    uint8 initial_value;
    uint8 changeable_pindirection;
    uint8 changeable_pinmode;

    #if (PORT_OPTIONAL_CONFIG == STD_ON)
    uint8 opendrain_mode;
    Port_PinOutputCurrentType current;
    uint8 slewrate;
    #endif
    
}Port_ConfigPin;

/* Data Structure required for initializing the Port Driver */
typedef struct
{
  Port_ConfigPin Pins[PORT_TOTAL_NO_OF_PINS];
}Port_ConfigType;

/*******************************************************************************
 *                      Function Prototypes                                    *
 *******************************************************************************/

/* Function for PORT Initialization API */
void Port_Init(const Port_ConfigType *ConfigPtr );

/* Function for PORT set Pin Direction API */
void Port_SetPinDirection(Port_PinType Pin, Port_PinDirectionType Direction);

/* Function for PORT refresh Port Direction API */
void Port_RefreshPortDirection(void);

#if (PORT_VERSION_INFO_API == STD_ON)
/* Function for PORT Get Version Info API */
void Port_GetVersionInfo(Std_VersionInfoType *versioninfo);
#endif

/* Function for PORT set Pin Mode API */
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Direction);
  
/*******************************************************************************
 *                       External Variables                                    *
 *******************************************************************************/

/* Extern PB structures to be used by Port and other modules */
extern const Port_ConfigType Port_Configuration;

#endif /* PORT_H */
