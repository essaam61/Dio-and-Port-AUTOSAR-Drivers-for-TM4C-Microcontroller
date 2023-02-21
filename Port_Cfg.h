 /******************************************************************************
 *
 * Module: Port
 *
 * File Name: Port_Cfg.h
 *
 * Description: Pre-Compile Configuration Header file for TM4C123GH6PM Microcontroller - Port Driver
 *
 * Author: Ahmed Essam
 ******************************************************************************/

#ifndef PORT_CFG_H
#define PORT_CFG_H

/*
 * Module Version 1.0.0
 */
#define PORT_CFG_SW_MAJOR_VERSION              (1U)
#define PORT_CFG_SW_MINOR_VERSION              (0U)
#define PORT_CFG_SW_PATCH_VERSION              (0U)

/*
 * AUTOSAR Version 4.0.3
 */
#define PORT_CFG_AR_RELEASE_MAJOR_VERSION     (4U)
#define PORT_CFG_AR_RELEASE_MINOR_VERSION     (0U)
#define PORT_CFG_AR_RELEASE_PATCH_VERSION     (3U)

/* Pre-compile option for Development Error Detect */
#define PORT_DEV_ERROR_DETECT                (STD_ON)
   
/* Pre-compile option for Set Pin Direction API */
#define PORT_SET_PIN_DIRECTION_API           (STD_ON)

/* Pre-compile option for Version Info API */
#define PORT_VERSION_INFO_API                (STD_ON)

/* Pre-compile option for optional MCU Ports and Port Pins Configurations */
#define PORT_OPTIONAL_CONFIG                 (STD_OFF)
   
/* Microcontroller Port Pin Mode ID's */
#define PORT_PIN_MODE_DIO              (uint8)0
#define PORT_PIN_MODE_ADC              (uint8)1   
#define PORT_PIN_MODE_UART             (uint8)2
#define PORT_PIN_MODE_SSI              (uint8)3      
#define PORT_PIN_MODE_I2C              (uint8)4
#define PORT_PIN_MODE_CAN              (uint8)5   
#define PORT_PIN_MODE_USB              (uint8)6
#define PORT_PIN_MODE_GPT              (uint8)7
#define PORT_PIN_MODE_PWM              (uint8)8   
#define PORT_PIN_MODE_QEI              (uint8)9
#define PORT_PIN_MODE_ANALOG_COMP      (uint8)10
#define PORT_PIN_MODE_NMI              (uint8)11   
#define PORT_PIN_MODE_TRACE            (uint8)12

/* Port Configured Port ID's */
#define PORT_A          (uint8)0   
#define PORT_B          (uint8)1  
#define PORT_C          (uint8)2   
#define PORT_D          (uint8)3  
#define PORT_E          (uint8)4   
#define PORT_F          (uint8)5  
   
/* Port Configured Port Pin ID's */
#define PIN_0          (uint8)0   
#define PIN_1          (uint8)1  
#define PIN_2          (uint8)2   
#define PIN_3          (uint8)3  
#define PIN_4          (uint8)4   
#define PIN_5          (uint8)5  
#define PIN_6          (uint8)6  
#define PIN_7          (uint8)7  
   
/* Port Pin ID numbers */
#define PORT_A_PIN_0    (Port_PinType)0
#define PORT_A_PIN_1    (Port_PinType)1
#define PORT_A_PIN_2    (Port_PinType)2
#define PORT_A_PIN_3    (Port_PinType)3
#define PORT_A_PIN_4    (Port_PinType)4
#define PORT_A_PIN_5    (Port_PinType)5
#define PORT_A_PIN_6    (Port_PinType)6
#define PORT_A_PIN_7    (Port_PinType)7
   
#define PORT_B_PIN_0    (Port_PinType)8
#define PORT_B_PIN_1    (Port_PinType)9
#define PORT_B_PIN_2    (Port_PinType)10 
#define PORT_B_PIN_3    (Port_PinType)11 
#define PORT_B_PIN_4    (Port_PinType)12 
#define PORT_B_PIN_5    (Port_PinType)13 
#define PORT_B_PIN_6    (Port_PinType)14
#define PORT_B_PIN_7    (Port_PinType)15
   
#define PORT_C_PIN_0    (Port_PinType)16
#define PORT_C_PIN_1    (Port_PinType)17 
#define PORT_C_PIN_2    (Port_PinType)18
#define PORT_C_PIN_3    (Port_PinType)19
#define PORT_C_PIN_4    (Port_PinType)20
#define PORT_C_PIN_5    (Port_PinType)21
#define PORT_C_PIN_6    (Port_PinType)22 
#define PORT_C_PIN_7    (Port_PinType)23
   
#define PORT_D_PIN_0    (Port_PinType)24
#define PORT_D_PIN_1    (Port_PinType)25   
#define PORT_D_PIN_2    (Port_PinType)26   
#define PORT_D_PIN_3    (Port_PinType)27   
#define PORT_D_PIN_4    (Port_PinType)28   
#define PORT_D_PIN_5    (Port_PinType)29   
#define PORT_D_PIN_6    (Port_PinType)30   
#define PORT_D_PIN_7    (Port_PinType)31
   
#define PORT_E_PIN_0    (Port_PinType)32
#define PORT_E_PIN_1    (Port_PinType)33
#define PORT_E_PIN_2    (Port_PinType)34
#define PORT_E_PIN_3    (Port_PinType)35
#define PORT_E_PIN_4    (Port_PinType)36
#define PORT_E_PIN_5    (Port_PinType)37
   
#define PORT_F_PIN_0    (Port_PinType)38
#define PORT_F_PIN_1    (Port_PinType)39
#define PORT_F_PIN_2    (Port_PinType)40
#define PORT_F_PIN_3    (Port_PinType)41
#define PORT_F_PIN_4    (Port_PinType)42   

/* Number of the configured Port Pins */
#define PORT_TOTAL_NO_OF_PINS 43
   
/* Number of the configured Port Pin Modes */
#define PORT_TOTAL_NO_OF_MODES 13

#endif /* PORT_CFG_H */
