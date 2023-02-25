 /******************************************************************************
 *
 * Module: Port
 *
 * File Name: Port.c
 *
 * Description: Source file for TM4C123GH6PM Microcontroller - Port Driver.
 *
 * Author: Ahmed Essam
 ******************************************************************************/

#include "Port.h"
#include "Port_Regs.h"

#if (PORT_DEV_ERROR_DETECT == STD_ON)

#include "Det.h"
/* AUTOSAR Version checking between Det and Port Modules */
#if ((DET_AR_MAJOR_VERSION != PORT_AR_RELEASE_MAJOR_VERSION)\
  || (DET_AR_MINOR_VERSION != PORT_AR_RELEASE_MINOR_VERSION)\
  || (DET_AR_PATCH_VERSION != PORT_AR_RELEASE_PATCH_VERSION))
  #error "The AR version of Det.h does not match the expected version"
#endif

#endif
  
STATIC const Port_ConfigPin* Port_Pins = NULL_PTR;
STATIC uint8 Port_Status = PORT_NOT_INITIALIZED;

/************************************************************************************
* Service Name: Port_Init
* Sync/Async: Synchronous
* Reentrancy: Non reentrant
* Parameters (in): ConfigPtr - Pointer to post-build configuration data
* Parameters (inout): None
* Parameters (out): None
* Return value: None
* Description: Initializes the Port Driver module. 
************************************************************************************/
void Port_Init(const Port_ConfigType * ConfigPtr )
{
    volatile uint32 * PortGpio_Ptr = NULL_PTR; /* point to the required Port Registers base address */
    volatile uint32 delay = 0;
    
#if (PORT_DEV_ERROR_DETECT == STD_ON)
	/* check if the input configuration pointer is not a NULL_PTR */
	if (NULL_PTR == ConfigPtr)
	{
		Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_INIT_SID,
		     PORT_E_PARAM_CONFIG);
	}
	else
#endif
        {
          Port_Pins = ConfigPtr->Pins;          /* points to address of the first pins structure --> Pins[0] */
          uint8 Pin_Count = 0;  /* count to configure each port pin */
         
          for(Pin_Count = 0 ; Pin_Count < PORT_NUMBER_OF_PORT_PINS ; Pin_Count++)
          {
            /* Point to the correct PORT register according to the Port Id stored in the Port_Num member */
            switch(Port_Pins[Pin_Count].Port_Num)
            {
                case  PORT_A: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTA_BASE_ADDRESS; /* PORTA Base Address */
                        break;
                case  PORT_B: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTB_BASE_ADDRESS; /* PORTB Base Address */
                        break;
                case  PORT_C: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTC_BASE_ADDRESS; /* PORTC Base Address */
                        break;
                case  PORT_D: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTD_BASE_ADDRESS; /* PORTD Base Address */
                        break;
                case  PORT_E: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTE_BASE_ADDRESS; /* PORTE Base Address */
                        break;
                case  PORT_F: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTF_BASE_ADDRESS; /* PORTF Base Address */
                        break;
                        
                default: break;
            }
            
            /* Enable clock for PORT and allow time for clock to start*/
            SET_BIT(SYSCTL_REGCGC2_REG, Port_Pins[Pin_Count].Port_Num);
            delay = SYSCTL_REGCGC2_REG;
            
            /* Unlock port pins for use, enable commit to write on pins and Secure the JTAG pins */
            if( ((Port_Pins[Pin_Count].Port_Num == PORT_D) && (Port_Pins[Pin_Count].Pin_Num == PIN_7)) || 
               ((Port_Pins[Pin_Count].Port_Num == PORT_F) && (Port_Pins[Pin_Count].Pin_Num == PIN_0)) ) /* PD7 or PF0 */
            {
                *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_LOCK_REG_OFFSET) = 0x4C4F434B;                     /* Unlock the GPIOCR register */   
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_COMMIT_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);  /* Set the corresponding bit in GPIOCR register to allow changes on this pin */
            }
            else if( (Port_Pins[Pin_Count].Port_Num == PORT_C) && (Port_Pins[Pin_Count].Pin_Num <= PIN_3) ) /* PC0 to PC3 */
            {
                /* Do Nothing ...  this is the JTAG pins */
            }
            else
            {
                /* Do Nothing ... No need to unlock the commit register for this pin */
            }
            
            /* Check for port pin Direction, Internal Resistor and set Pin level Initial Value */
            if(Port_Pins[Pin_Count].Port_Pin_Direction == PORT_PIN_OUT)
            {
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);                /* Set the corresponding bit in the GPIODIR register to configure it as output pin */
                
                if(Port_Pins[Pin_Count].Port_Pin_Level_Value == STD_HIGH)
                {
                    SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DATA_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);          /* Set the corresponding bit in the GPIODATA register to provide initial value 1 */
                }
                else
                {
                    CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DATA_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);        /* Clear the corresponding bit in the GPIODATA register to provide initial value 0 */
                }
                
                #if (PORT_OPTIONAL_CONFIG == STD_ON)
                /* Check for port pin Open Drain Mode */
                if(Port_Pins[Pin_Count].Pin_Driven_Mode == STD_ON)
                {
                   SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_OPEN_DRAIN_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);       /* Set the corresponding bit in the GPIOODR register to enable the open-drain mode pin */
                }
                else
                {
                   CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_OPEN_DRAIN_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIOODR register to disable the open-drain mode pin */
                }
                
                /* Check for port pin Output Current */
                if(Port_Pins[Pin_Count].Pin_Current == R2R)
                { 
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DR2R_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);              /* Set the corresponding bit in the GPIODR2R register to have 2mA drive */ 
                }
                else if(Port_Pins[Pin_Count].Pin_Current == R4R)
                { 
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DR4R_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);              /* Set the corresponding bit in the GPIODR2R register to have 4mA drive */ 
                }
                else if(Port_Pins[Pin_Count].Pin_Current == R8R)
                { 
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DR8R_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);              /* Set the corresponding bit in the GPIODR2R register to have 8mA drive */ 
                  
                  /* Control the Pin slew rate when using the 8-mA drive strength option */
                  if(Port_Pins[Pin_Count].Pin_Slew_Rate == STD_ACTIVE)
                  {
                    SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_SLEW_RATE_CTL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);            /* Set the corresponding bit in the GPIOSLR register to enable slew rate control */ 
                  }
                  else
                  {
                    CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_SLEW_RATE_CTL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);            /* Set the corresponding bit in the GPIOSLR register to disable slew rate control */ 
                  }
                }
                else
                {
                    /* Do Nothing */
                }
                #endif
            }
            else if(Port_Pins[Pin_Count].Port_Pin_Direction == PORT_PIN_IN)
            {
                
                CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);             /* Clear the corresponding bit in the GPIODIR register to configure it as input pin */
                
                if(Port_Pins[Pin_Count].Port_Pin_Resistor == PULL_UP)
                {
                    SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_PULL_UP_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);       /* Set the corresponding bit in the GPIOPUR register to enable the internal pull up pin */
                }
                else if(Port_Pins[Pin_Count].Port_Pin_Resistor == PULL_DOWN)
                {
                    SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_PULL_DOWN_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Set the corresponding bit in the GPIOPDR register to enable the internal pull down pin */
                }
                else
                {
                    CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_PULL_UP_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIOPUR register to disable the internal pull up pin */
                    CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_PULL_DOWN_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);   /* Clear the corresponding bit in the GPIOPDR register to disable the internal pull down pin */
                }
                
            }
            else
            {
                /* Do Nothing */
            }
            
            /* Check for port pin Mode */
            switch(Port_Pins[Pin_Count].Port_Pin_Mode)
            {
              case PORT_PIN_MODE_DIO:
              {
                if( (Port_Pins[Pin_Count].Port_Num == PORT_C) && (Port_Pins[Pin_Count].Pin_Num <= PIN_3) ) /* PC0 to PC3 */
                {
                  /* Do Nothing ...  this is the JTAG pins */
                }
                else
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);      /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);             /* Disable Alternative function for this pin by clear the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) &= ~(0x0000000F << (Port_Pins[Pin_Count].Pin_Num * 4));     /* Clear the PMCx bits for this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);         /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */   
                }
                
                break;
              }
              case PORT_PIN_MODE_ADC:
              {
                if( Port_Pins[Pin_Count].Port_Pin_Direction == PORT_PIN_IN &&
                  ( Pin_Count == PORT_B_PIN_4 || Pin_Count == PORT_B_PIN_5 || (Pin_Count >= PORT_D_PIN_0 && Pin_Count <= PORT_D_PIN_3) 
                 || (Pin_Count >= PORT_E_PIN_0 && Pin_Count <= PORT_E_PIN_5) )  )
                {
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);             /* Enable Alternative function for this pin by setting the corresponding bit in GPIOAFSEL register */
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIODEN register to disable digital functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);      /* Set the corresponding bit in the GPIOAMSEL register to enable analog functionality on this pin */
                }
                else
                {}
                
                break;
              }
              case PORT_PIN_MODE_UART:          /* UART 1 RTS, CTS with PC4, PC5 is not handled. */
              {
                if( Pin_Count == PORT_A_PIN_0 || Pin_Count == PORT_A_PIN_1 || Pin_Count == PORT_B_PIN_0 || Pin_Count == PORT_B_PIN_1 
                 || Pin_Count == PORT_C_PIN_4 || Pin_Count == PORT_C_PIN_5 || Pin_Count == PORT_C_PIN_6 || Pin_Count == PORT_C_PIN_7 
                 || Pin_Count == PORT_D_PIN_4 || Pin_Count == PORT_D_PIN_5 || Pin_Count == PORT_D_PIN_6 || Pin_Count == PORT_D_PIN_7
                 || Pin_Count == PORT_E_PIN_0 || Pin_Count == PORT_E_PIN_1 || Pin_Count == PORT_E_PIN_4 || Pin_Count == PORT_E_PIN_5 
                 || Pin_Count == PORT_F_PIN_0 || Pin_Count == PORT_F_PIN_1 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000001 << (Port_Pins[Pin_Count].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                
                /* Check if PB0 and PB1 pins are not used as UART 1, so set PC4 and PC5 on UART 1 (Port Mode = 2) instead of UART 4 (Port Mode = 1) */
                if( ( Pin_Count == PORT_C_PIN_4 && !(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) & (0x0000000F<<(Port_Pins[PORT_B_PIN_0].Pin_Num * 4)) ) ) 
                 || ( Pin_Count == PORT_C_PIN_5 && !(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) & (0x0000000F<<(Port_Pins[PORT_B_PIN_1].Pin_Num * 4)) ) ) )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000002 << (Port_Pins[Pin_Count].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
              case PORT_PIN_MODE_SSI:           /* SSI 1 with PD0 -> PD03 is not handled. */
              {
                if( (Pin_Count >= PORT_A_PIN_2 && Pin_Count <= PORT_A_PIN_5)
                 || (Pin_Count >= PORT_B_PIN_4 && Pin_Count <= PORT_B_PIN_7) 
                 || (Pin_Count >= PORT_F_PIN_0 && Pin_Count <= PORT_F_PIN_3) )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000002 << (Port_Pins[Pin_Count].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else if(Pin_Count == PORT_D_PIN_0 || Pin_Count == PORT_D_PIN_1 || Pin_Count == PORT_D_PIN_2 || Pin_Count == PORT_D_PIN_3 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000001 << (Port_Pins[Pin_Count].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
              case PORT_PIN_MODE_I2C:
              {
                if( Pin_Count == PORT_A_PIN_6 || Pin_Count == PORT_A_PIN_7 || Pin_Count == PORT_B_PIN_2 || Pin_Count == PORT_B_PIN_3
                 || Pin_Count == PORT_D_PIN_0 || Pin_Count == PORT_D_PIN_1 || Pin_Count == PORT_E_PIN_4 || Pin_Count == PORT_E_PIN_5 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000003 << (Port_Pins[Pin_Count].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
              case PORT_PIN_MODE_CAN:
              {
                if( Pin_Count == PORT_A_PIN_0 || Pin_Count == PORT_A_PIN_1 || Pin_Count == PORT_B_PIN_4 || Pin_Count == PORT_B_PIN_5
                 || Pin_Count == PORT_E_PIN_4 || Pin_Count == PORT_E_PIN_5 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000008 << (Port_Pins[Pin_Count].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else if( Pin_Count == PORT_F_PIN_0 || Pin_Count == PORT_F_PIN_3 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000003 << (Port_Pins[Pin_Count].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
              case PORT_PIN_MODE_USB:
              {
                if( Pin_Count == PORT_B_PIN_0 || Pin_Count == PORT_B_PIN_1 || Pin_Count == PORT_D_PIN_4 || Pin_Count == PORT_D_PIN_5 )
                {
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);             /* Enable Alternative function for this pin by setting the corresponding bit in GPIOAFSEL register */
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIODEN register to disable digital functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);      /* Set the corresponding bit in the GPIOAMSEL register to enable analog functionality on this pin */
                }
                else if( Pin_Count == PORT_C_PIN_6 || Pin_Count == PORT_C_PIN_7 || Pin_Count == PORT_D_PIN_2 || Pin_Count == PORT_D_PIN_3
                      || Pin_Count == PORT_F_PIN_4 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000008 << (Port_Pins[Pin_Count].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                }
                else
                {}
                
                break;
              }
              case PORT_PIN_MODE_GPT:
              {
                if( !(Pin_Count <= PORT_A_PIN_7) || !(Pin_Count >= PORT_E_PIN_0 && Pin_Count <= PORT_E_PIN_5)  )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000007 << (Port_Pins[Pin_Count].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
               case PORT_PIN_MODE_PWM:          /* M1PWM with PD0, PD1, PE4, PE5, PF2 is not handled. */
              {
                if( (Pin_Count >= PORT_B_PIN_4 && Pin_Count <= PORT_B_PIN_7) || Pin_Count == PORT_C_PIN_4 || Pin_Count == PORT_C_PIN_5
                 || Pin_Count == PORT_D_PIN_0 || Pin_Count == PORT_D_PIN_1 || Pin_Count == PORT_D_PIN_2 || Pin_Count == PORT_D_PIN_6 
                 || Pin_Count == PORT_E_PIN_4 || Pin_Count == PORT_E_PIN_5 || Pin_Count == PORT_F_PIN_2 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000004 << (Port_Pins[Pin_Count].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else if( Pin_Count == PORT_A_PIN_6 || Pin_Count == PORT_A_PIN_7
                      || Pin_Count == PORT_F_PIN_0 || Pin_Count == PORT_F_PIN_1 || Pin_Count == PORT_F_PIN_3 || Pin_Count == PORT_F_PIN_4 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000005 << (Port_Pins[Pin_Count].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
              case PORT_PIN_MODE_QEI:
              {
                if( (Pin_Count >= PORT_C_PIN_4 && Pin_Count <= PORT_C_PIN_6) || Pin_Count == PORT_D_PIN_3 || Pin_Count == PORT_D_PIN_6
                 || Pin_Count == PORT_D_PIN_7 || Pin_Count == PORT_F_PIN_0 || Pin_Count == PORT_F_PIN_1 || Pin_Count == PORT_F_PIN_4 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000006 << (Port_Pins[Pin_Count].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
              case PORT_PIN_MODE_ANALOG_COMP:
              {
                if( Pin_Count >= PORT_C_PIN_4 && Pin_Count <= PORT_C_PIN_7 )
                {
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);             /* Enable Alternative function for this pin by setting the corresponding bit in GPIOAFSEL register */
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIODEN register to disable digital functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);      /* Set the corresponding bit in the GPIOAMSEL register to enable analog functionality on this pin */
                }
                else if( Pin_Count == PORT_F_PIN_0 || Pin_Count == PORT_F_PIN_1 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000009 << (Port_Pins[Pin_Count].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                }
                else
                {}
                
                break;
              }
              case PORT_PIN_MODE_NMI:
              {
                if( Pin_Count == PORT_D_PIN_7 || Pin_Count == PORT_F_PIN_0 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000008 << (Port_Pins[Pin_Count].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
              case PORT_PIN_MODE_TRACE:
              {
                if( Pin_Count >= PORT_F_PIN_1 && Pin_Count <= PORT_F_PIN_3 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x0000000E << (Port_Pins[Pin_Count].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
              
              default: break;
            }
          }     /* for(Pin_Count = 0 ; Pin_Count<=PORT_NUMBER_OF_PORT_PINS ; Pin_Count++) */
        
          Port_Status = PORT_INITIALIZED;
        }
}

/************************************************************************************
* Service Name: Port_SetPinDirection
* Service ID[hex]: 0x01
* Sync/Async: Synchronous
* Reentrancy: Reentrant
* Parameters (in): Pin - Port Pin ID number.
*                  Direction - Port Pin Direction .
* Parameters (inout): None
* Parameters (out): None
* Return value: None
* Description: Sets the port pin direction.
************************************************************************************/
#if (PORT_SET_PIN_DIRECTION_API == STD_ON)
void Port_SetPinDirection(Port_PinType Pin, Port_PinDirectionType Direction)
{
    volatile uint32 * PortGpio_Ptr = NULL_PTR; /* point to the required Port Registers base address */
    boolean error = FALSE;
    
#if (PORT_DEV_ERROR_DETECT == STD_ON)
        /* Check if the Driver is initialized before using this function */
	if (PORT_NOT_INITIALIZED == Port_Status)
	{
		Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_DIRECTION_SID,
		PORT_E_UNINIT);
                error = TRUE;
	}
	else
	{
          /* No Action Required */
	}
        /* Check if the used pin is within the valid range */
	if (PORT_NUMBER_OF_PORT_PINS <= Pin && FALSE == error)
	{
		Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_DIRECTION_SID,
		PORT_E_PARAM_PIN);
                error = TRUE;
	} 
        else
	{
          /* No Action Required */
	}
        /* Check if the used pin's direction is changeable during run time */
        if(STD_OFF == Port_Pins[Pin].Port_Pin_Direction_Changeable && FALSE == error)
        {
          	Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_DIRECTION_SID,
			PORT_E_DIRECTION_UNCHANGEABLE);
          	error = TRUE;
        }
        else 
        {
          /* No Action Required */
        }
#endif
        
        /* In-case there are no errors */
	if(FALSE == error)
	{
            /* Point to the correct PORT register according to the Port Id stored in the Port_Num member */
            switch(Port_Pins[Pin].Port_Num)
            {
                case  PORT_A: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTA_BASE_ADDRESS; /* PORTA Base Address */
                        break;
                case  PORT_B: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTB_BASE_ADDRESS; /* PORTB Base Address */
                        break;
                case  PORT_C: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTC_BASE_ADDRESS; /* PORTC Base Address */
                        break;
                case  PORT_D: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTD_BASE_ADDRESS; /* PORTD Base Address */
                        break;
                case  PORT_E: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTE_BASE_ADDRESS; /* PORTE Base Address */
                        break;
                case  PORT_F: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTF_BASE_ADDRESS; /* PORTF Base Address */
                        break;
                        
                default: break;
            }
            
            if(Direction == PORT_PIN_OUT)
            {
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET), Port_Pins[Pin].Pin_Num);                /* Set the corresponding bit in the GPIODIR register to configure it as output pin */

            }
            else if(Direction == PORT_PIN_IN)
            {
                CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET), Port_Pins[Pin].Pin_Num);             /* Clear the corresponding bit in the GPIODIR register to configure it as input pin */
                
            }
            else
            {
                /* Do Nothing */
            }
            
        }
        else
	{
		/* No Action Required */
	}

}
#endif

/************************************************************************************
* Service Name: Port_RefreshPortDirection
* Service ID[hex]: 0x02
* Sync/Async: Synchronous
* Reentrancy: Non reentrant
* Parameters (in): None
* Parameters (inout): None
* Parameters (out): None
* Return value: None
* Description: Refreshes port direction.
************************************************************************************/
void Port_RefreshPortDirection(void)
{
  volatile uint32 * PortGpio_Ptr = NULL_PTR; /* point to the required Port Registers base address */
  boolean error =  FALSE;
  
#if (PORT_DEV_ERROR_DETECT == STD_ON)
        /* Check if the Driver is initialized before using this function */
	/* Check if the Driver is initialized before using this function */
	if (PORT_NOT_INITIALIZED == Port_Status)
	{
		Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_REFRESH_PORT_DIRECTION_SID,
		PORT_E_UNINIT);
                error = TRUE;
	}
	else
	{
          /* No Action Required */
	}
#endif
        
        /* In-case there are no errors */
	if(FALSE == error)
	{
          for(uint8 Pin_Count = 0 ; Pin_Count < PORT_NUMBER_OF_PORT_PINS ; Pin_Count++)
          {
            /* Exclude the port pins from refreshing that are configured as pin direction changeable during runtime. */ 
            if(STD_OFF == Port_Pins[Pin_Count].Port_Pin_Direction_Changeable)
            {
              /* Point to the correct PORT register according to the Port Id stored in the Port_Num member */
              switch(Port_Pins[Pin_Count].Port_Num)
              {
                  case  PORT_A: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTA_BASE_ADDRESS; /* PORTA Base Address */
                          break;
                  case  PORT_B: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTB_BASE_ADDRESS; /* PORTB Base Address */
                          break;
                  case  PORT_C: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTC_BASE_ADDRESS; /* PORTC Base Address */
                          break;
                  case  PORT_D: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTD_BASE_ADDRESS; /* PORTD Base Address */
                          break;
                  case  PORT_E: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTE_BASE_ADDRESS; /* PORTE Base Address */
                          break;
                  case  PORT_F: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTF_BASE_ADDRESS; /* PORTF Base Address */
                          break;
                          
                  default: break;
              }
              
              /* Read port pin direction and write it on the same pin */
              if( BIT_IS_SET(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num) )
              {
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num); 
              }
              else if( BIT_IS_CLEAR(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num) )
              {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET) , Port_Pins[Pin_Count].Pin_Num); 
              }
              else 
              {
                /* Do Nothing */
              }
            
            }
            else
            {
              /* Do Not Refresh */
            }
    
          }
        }
        else
	{
		/* No Action Required */
	}
}

/************************************************************************************
* Service Name: Port_GetVersionInfo
* Service ID[hex]: 0x03
* Sync/Async: Synchronous
* Reentrancy: Non reentrant
* Parameters (in): None
* Parameters (inout): None
* Parameters (out): VersionInfo - Pointer to where to store the version information of this module.
* Return value: None
* Description: Function to get the version information of this module.
************************************************************************************/
#if (PORT_VERSION_INFO_API == STD_ON)
void Port_GetVersionInfo(Std_VersionInfoType *versioninfo)
{
#if (PORT_DEV_ERROR_DETECT == STD_ON)
	/* Check if input pointer is not Null pointer */
	if(NULL_PTR == versioninfo)
	{
		/* Report to DET  */
		Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID,
				PORT_GET_VERSION_INFO_SID, PORT_E_PARAM_POINTER);
	}
	else
#endif /* (PORT_DEV_ERROR_DETECT == STD_ON) */
	{
		/* Copy the vendor Id */
		versioninfo->vendorID = (uint16)PORT_VENDOR_ID;
		/* Copy the module Id */
		versioninfo->moduleID = (uint16)PORT_MODULE_ID;
		/* Copy Software Major Version */
		versioninfo->sw_major_version = (uint8)PORT_SW_MAJOR_VERSION;
		/* Copy Software Minor Version */
		versioninfo->sw_minor_version = (uint8)PORT_SW_MINOR_VERSION;
		/* Copy Software Patch Version */
		versioninfo->sw_patch_version = (uint8)PORT_SW_PATCH_VERSION;
	}
}
#endif

/************************************************************************************
* Service Name: Port_SetPinMode
* Service ID[hex]: 0x04
* Sync/Async: Synchronous
* Reentrancy: Reentrant
* Parameters (in): Pin - Port Pin ID number.
*                  Mode - New Port Pin mode to be set on port pin.
* Parameters (inout): None
* Parameters (out): None
* Return value: None
* Description: Sets the port pin mode.
************************************************************************************/
#if (PORT_SET_PIN_MOPDE_API == STD_ON)
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode)
{
    volatile uint32 * PortGpio_Ptr = NULL_PTR; /* point to the required Port Registers base address */
    boolean error = FALSE;
    
#if (PORT_DEV_ERROR_DETECT == STD_ON)
        /* Check if the Driver is initialized before using this function */
	if (PORT_NOT_INITIALIZED == Port_Status)
	{
		Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_MODE_SID,
		PORT_E_UNINIT);
                error = TRUE;
	}
	else
	{
          /* No Action Required */
	}
        /* Check if the used pin is within the valid range */
	if (PORT_NUMBER_OF_PORT_PINS <= Pin && FALSE == error)
	{
		Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_MODE_SID,
		PORT_E_PARAM_PIN);
                error = TRUE;
	} 
        else
	{
          /* No Action Required */
	}
        /* Check if the used mode is within the valid range */
	if (PORT_NUMBER_OF_PORT_PIN_MODES <= Mode && FALSE == error)
	{
		Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_MODE_SID,
		PORT_E_PARAM_INVALID_MODE);
                error = TRUE;
	} 
        else
	{
          /* No Action Required */
	}
        /* Check if the used pin's mode is changeable during run time */
        if(STD_OFF == Port_Pins[Pin].Port_Pin_Mode_Changeable && FALSE == error)
        {
          	Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_MODE_SID,
			PORT_E_MODE_UNCHANGEABLE);
          	error = TRUE;
        }
        else 
        {
          /* No Action Required */
        }
#endif
        uint8 Pin_Count = 0;  /* count to configure each port pin */
        
        /* In-case there are no errors */
	if(FALSE == error)
	{
            /* Point to the correct PORT register according to the Port Id stored in the Port_Num member */
            switch(Port_Pins[Pin].Port_Num)
            {
                case  PORT_A: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTA_BASE_ADDRESS; /* PORTA Base Address */
                        break;
                case  PORT_B: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTB_BASE_ADDRESS; /* PORTB Base Address */
                        break;
                case  PORT_C: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTC_BASE_ADDRESS; /* PORTC Base Address */
                        break;
                case  PORT_D: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTD_BASE_ADDRESS; /* PORTD Base Address */
                        break;
                case  PORT_E: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTE_BASE_ADDRESS; /* PORTE Base Address */
                        break;
                case  PORT_F: PortGpio_Ptr = (volatile uint32 *)GPIO_PORTF_BASE_ADDRESS; /* PORTF Base Address */
                        break;
                        
                default: break;
            }
            
            /* Check for port pin Mode */
            switch(Mode)
            {
              case PORT_PIN_MODE_DIO:
              {
                CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);      /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);             /* Disable Alternative function for this pin by clear the corresponding bit in GPIOAFSEL register */
                *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) &= ~(0x0000000F << (Port_Pins[Pin].Pin_Num * 4));     /* Clear the PMCx bits for this pin */
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin].Pin_Num);         /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
              case PORT_PIN_MODE_ADC:
              {
                if( Pin_Count == PORT_B_PIN_4 || Pin_Count == PORT_B_PIN_5 || (Pin_Count >= PORT_D_PIN_0 && Pin_Count <= PORT_D_PIN_3) 
                 || (Pin_Count >= PORT_E_PIN_0 && Pin_Count <= PORT_E_PIN_5) )
                {
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);             /* Enable Alternative function for this pin by setting the corresponding bit in GPIOAFSEL register */
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin].Pin_Num);     /* Clear the corresponding bit in the GPIODEN register to disable digital functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);      /* Set the corresponding bit in the GPIOAMSEL register to enable analog functionality on this pin */
                }
                else
                {}
                
                break;
              }
              case PORT_PIN_MODE_UART:          /* UART 1 RTS, CTS with PC4, PC5 is not handled. */
              {
                if( Pin_Count == PORT_A_PIN_0 || Pin_Count == PORT_A_PIN_1 || Pin_Count == PORT_B_PIN_0 || Pin_Count == PORT_B_PIN_1 
                 || Pin_Count == PORT_C_PIN_4 || Pin_Count == PORT_C_PIN_5 || Pin_Count == PORT_C_PIN_6 || Pin_Count == PORT_C_PIN_7 
                 || Pin_Count == PORT_D_PIN_4 || Pin_Count == PORT_D_PIN_5 || Pin_Count == PORT_D_PIN_6 || Pin_Count == PORT_D_PIN_7
                 || Pin_Count == PORT_E_PIN_0 || Pin_Count == PORT_E_PIN_1 || Pin_Count == PORT_E_PIN_4 || Pin_Count == PORT_E_PIN_5 
                 || Pin_Count == PORT_F_PIN_0 || Pin_Count == PORT_F_PIN_1 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000001 << (Port_Pins[Pin].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                
                /* Check if PB0 and PB1 pins are not used as UART 1, so set PC4 and PC5 on UART 1 (Port Mode = 2) instead of UART 4 (Port Mode = 1) */
                if( ( Pin_Count == PORT_C_PIN_4 && !(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) & (0x0000000F<<(Port_Pins[PORT_B_PIN_0].Pin_Num * 4))) ) 
                 || ( Pin_Count == PORT_C_PIN_5 && !(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) & (0x0000000F<<(Port_Pins[PORT_B_PIN_1].Pin_Num * 4))) ) )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000002 << (Port_Pins[Pin].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
              case PORT_PIN_MODE_SSI:           /* SSI 1 with PD0 -> PD03 is not handled. */
              {
                if( (Pin_Count >= PORT_A_PIN_2 && Pin_Count <= PORT_A_PIN_5)
                 || (Pin_Count >= PORT_B_PIN_4 && Pin_Count <= PORT_B_PIN_7) 
                 || (Pin_Count >= PORT_F_PIN_0 && Pin_Count <= PORT_F_PIN_3) )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000002 << (Port_Pins[Pin].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else if(Pin_Count == PORT_D_PIN_0 || Pin_Count == PORT_D_PIN_1 || Pin_Count == PORT_D_PIN_2 || Pin_Count == PORT_D_PIN_3 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000001 << (Port_Pins[Pin].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
              case PORT_PIN_MODE_I2C:
              {
                if( Pin_Count == PORT_A_PIN_6 || Pin_Count == PORT_A_PIN_7 || Pin_Count == PORT_B_PIN_2 || Pin_Count == PORT_B_PIN_3
                 || Pin_Count == PORT_D_PIN_0 || Pin_Count == PORT_D_PIN_1 || Pin_Count == PORT_E_PIN_4 || Pin_Count == PORT_E_PIN_5 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000003 << (Port_Pins[Pin].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
              case PORT_PIN_MODE_CAN:
              {
                if( Pin_Count == PORT_A_PIN_0 || Pin_Count == PORT_A_PIN_1 || Pin_Count == PORT_B_PIN_4 || Pin_Count == PORT_B_PIN_5
                 || Pin_Count == PORT_E_PIN_4 || Pin_Count == PORT_E_PIN_5 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000008 << (Port_Pins[Pin].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else if( Pin_Count == PORT_F_PIN_0 || Pin_Count == PORT_F_PIN_3 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000003 << (Port_Pins[Pin].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
              case PORT_PIN_MODE_USB:
              {
                if( Pin_Count == PORT_B_PIN_0 || Pin_Count == PORT_B_PIN_1 || Pin_Count == PORT_D_PIN_4 || Pin_Count == PORT_D_PIN_5 )
                {
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);             /* Enable Alternative function for this pin by setting the corresponding bit in GPIOAFSEL register */
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin].Pin_Num);     /* Clear the corresponding bit in the GPIODEN register to disable digital functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);      /* Set the corresponding bit in the GPIOAMSEL register to enable analog functionality on this pin */
                }
                else if( Pin_Count == PORT_C_PIN_6 || Pin_Count == PORT_C_PIN_7 || Pin_Count == PORT_D_PIN_2 || Pin_Count == PORT_D_PIN_3
                      || Pin_Count == PORT_F_PIN_4 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000008 << (Port_Pins[Pin].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                }
                else
                {}
                
                break;
              }
              case PORT_PIN_MODE_GPT:
              {
                if( !(Pin_Count <= PORT_A_PIN_7) || !(Pin_Count >= PORT_E_PIN_0 && Pin_Count <= PORT_E_PIN_5)  )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000007 << (Port_Pins[Pin].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
               case PORT_PIN_MODE_PWM:          /* M1PWM with PD0, PD1, PE4, PE5, PF2 is not handled. */
              {
                if( (Pin_Count >= PORT_B_PIN_4 && Pin_Count <= PORT_B_PIN_7) || Pin_Count == PORT_C_PIN_4 || Pin_Count == PORT_C_PIN_5
                 || Pin_Count == PORT_D_PIN_0 || Pin_Count == PORT_D_PIN_1 || Pin_Count == PORT_D_PIN_2 || Pin_Count == PORT_D_PIN_6 
                 || Pin_Count == PORT_E_PIN_4 || Pin_Count == PORT_E_PIN_5 || Pin_Count == PORT_F_PIN_2 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000004 << (Port_Pins[Pin].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else if( Pin_Count == PORT_A_PIN_6 || Pin_Count == PORT_A_PIN_7
                      || Pin_Count == PORT_F_PIN_0 || Pin_Count == PORT_F_PIN_1 || Pin_Count == PORT_F_PIN_3 || Pin_Count == PORT_F_PIN_4 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000005 << (Port_Pins[Pin].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
              case PORT_PIN_MODE_QEI:
              {
                if( (Pin_Count >= PORT_C_PIN_4 && Pin_Count <= PORT_C_PIN_6) || Pin_Count == PORT_D_PIN_3 || Pin_Count == PORT_D_PIN_6
                 || Pin_Count == PORT_D_PIN_7 || Pin_Count == PORT_F_PIN_0 || Pin_Count == PORT_F_PIN_1 || Pin_Count == PORT_F_PIN_4 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000006 << (Port_Pins[Pin].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
              case PORT_PIN_MODE_ANALOG_COMP:
              {
                if( Pin_Count >= PORT_C_PIN_4 && Pin_Count <= PORT_C_PIN_7 )
                {
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);             /* Enable Alternative function for this pin by setting the corresponding bit in GPIOAFSEL register */
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin].Pin_Num);     /* Clear the corresponding bit in the GPIODEN register to disable digital functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);      /* Set the corresponding bit in the GPIOAMSEL register to enable analog functionality on this pin */
                }
                else if( Pin_Count == PORT_F_PIN_0 || Pin_Count == PORT_F_PIN_1 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000009 << (Port_Pins[Pin].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                }
                else
                {}
                
                break;
              }
              case PORT_PIN_MODE_NMI:
              {
                if( Pin_Count == PORT_D_PIN_7 || Pin_Count == PORT_F_PIN_0 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x00000008 << (Port_Pins[Pin].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
              case PORT_PIN_MODE_TRACE:
              {
                if( Pin_Count >= PORT_F_PIN_1 && Pin_Count <= PORT_F_PIN_3 )
                {
                  CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_Pins[Pin].Pin_Num);     /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
                  SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_Pins[Pin].Pin_Num);              /* Enable Alternative function for this pin by settomg the corresponding bit in GPIOAFSEL register */
                  *(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (0x0000000E << (Port_Pins[Pin].Pin_Num * 4));     /* Set the PMCx bits for this pin */
                }
                else
                {}
                SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_Pins[Pin].Pin_Num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
                
                break;
              }
              
              default: break;
            }
            
        }
        else
	{
		/* No Action Required */
	}
  
  
}
#endif
