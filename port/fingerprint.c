#include "stm32f1xx_hal.h"
#include "usart.h"
#include "fingerprint.h"
#include <string.h>



			
uint8_t finger_TxBuf[9];			

uint8_t     Finger_SleepFlag;


/***************************************************************************
* @brief      Send a byte of data to the serial port
* @param      temp : Data to send
****************************************************************************/
HAL_StatusTypeDef  TxByte(uint8_t temp)
{	
	return   HAL_UART_Transmit(&huart1, &temp, 1 , 100);    
}

/***************************************************************************
* @brief      send a command, and wait for the response of module
* @param      Scnt: The number of bytes to send
	      Rcnt: expect the number of bytes response module
	      Delay_ms: wait timeout
* @return     ACK_SUCCESS: success
  	      other: see the macro definition
****************************************************************************/
uint8_t TxAndRxCmd(uint8_t Scnt, uint8_t Rcnt, uint16_t Delay_ms)
{
	uint8_t  i, j, CheckSum;
  uint32_t before_tick;        
  uint32_t after_tick;
  uint8_t   overflow_Flag = 0;
              
                       
	TxByte(CMD_HEAD);			 
	CheckSum = 0;
	for (i = 0; i < Scnt; i++)
	{
		TxByte(finger_TxBuf[i]);		 
		CheckSum ^= finger_TxBuf[i];
	}	
	TxByte(CheckSum);
	TxByte(CMD_TAIL);  
        
        
  Usart1_ReceiveStruct.RX_Size = 0;  // clear  RX_Size  before next receive
        
        // Receive time out: Delay_ms
  before_tick = HAL_GetTick();	 
  do
  {
    overflow_Flag = 0;
    after_tick = HAL_GetTick();	
    if(before_tick > after_tick)   //if overflow (go back to zero)
    {
      before_tick = HAL_GetTick();	  // get time_before again
      overflow_Flag = 1;
    }

  } while (((Usart1_ReceiveStruct.RX_Size < Rcnt) && (after_tick - before_tick < Delay_ms)) || (overflow_Flag == 1));
      
  if (Usart1_ReceiveStruct.RX_flag != 1)   return ACK_TIMEOUT;
	
  Usart1_ReceiveStruct.RX_flag = 0;	// clean flag
        
	if (Usart1_ReceiveStruct.RX_Size!= Rcnt)	return ACK_TIMEOUT;
	if (Usart1_ReceiveStruct.RX_pData[0] != CMD_HEAD) 	   return ACK_FAIL;
	if (Usart1_ReceiveStruct.RX_pData[Rcnt - 1] != CMD_TAIL)    return ACK_FAIL;
	if (Usart1_ReceiveStruct.RX_pData[1] != (finger_TxBuf[0]))  return ACK_FAIL;

	CheckSum = 0;
	
	for (j = 1; j < (Usart1_ReceiveStruct.RX_Size) - 1; j++) CheckSum ^= Usart1_ReceiveStruct.RX_pData[j];
	
	if (CheckSum != 0)   return ACK_FAIL; 	  

	return  ACK_SUCCESS;
}	 

/***************************************************************************
* @brief      Query the number of existing fingerprints
* @return     0xFF: error
  	      other: success, the value is the number of existing fingerprints
****************************************************************************/
uint8_t GetUserCount(void)
{
	uint8_t m;
	
	finger_TxBuf[0] = CMD_USER_CNT;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = 0;
	finger_TxBuf[3] = 0;
	finger_TxBuf[4] = 0;	
	
	m = TxAndRxCmd(5, 8, 100);
			
	if (m == ACK_SUCCESS && Usart1_ReceiveStruct.RX_pData[4] == ACK_SUCCESS)
	{
	    return Usart1_ReceiveStruct.RX_pData[3];
	}
	else
	{
	 	return 0xFF;
	}
}

/***************************************************************************
* @brief      Get Compare Level
* @return     0xFF: error
  	      other: success, the value is compare level
****************************************************************************/
uint8_t GetcompareLevel(void)
{
	uint8_t m;
	
	finger_TxBuf[0] = CMD_COM_LEV;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = 0;
	finger_TxBuf[3] = 1;
	finger_TxBuf[4] = 0;	
	
	m = TxAndRxCmd(5, 8, 100);
		
	if (m == ACK_SUCCESS && Usart1_ReceiveStruct.RX_pData[4] == ACK_SUCCESS)
	{
	    return Usart1_ReceiveStruct.RX_pData[3];
	}
	else
	{
	 	return 0xFF;
	}
}

/***************************************************************************
* @brief      Set Compare Level
* @param      temp: Compare Level,the default value is 5, can be set to 0-9, the bigger, the stricter
* @return     0xFF: error
  	      other: success, the value is compare level
****************************************************************************/
uint8_t SetcompareLevel(uint8_t temp)
{
	uint8_t m;
	
	finger_TxBuf[0] = CMD_COM_LEV;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = temp;
	finger_TxBuf[3] = 0;
	finger_TxBuf[4] = 0;	
	
	m = TxAndRxCmd(5, 8, 100);
		
	if (m == ACK_SUCCESS && Usart1_ReceiveStruct.RX_pData[4] == ACK_SUCCESS)
	{
	    return Usart1_ReceiveStruct.RX_pData[3];
	}
	else
	{
	 	return 0xFF;
	}
}

/***************************************************************************
* @brief      Get the time that fingerprint collection wait timeout 
* @return     0xFF: error
  	      other: success, the value is the time that fingerprint collection wait timeout 
****************************************************************************/
uint8_t GetTimeOut(void)
{
	uint8_t m;
	
	finger_TxBuf[0] = CMD_TIMEOUT;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = 0;
	finger_TxBuf[3] = 1;
	finger_TxBuf[4] = 0;	
	
	m = TxAndRxCmd(5, 8, 100);
		
	if (m == ACK_SUCCESS && Usart1_ReceiveStruct.RX_pData[4] == ACK_SUCCESS)
	{
	    return Usart1_ReceiveStruct.RX_pData[3];
	}
	else
	{
	 	return 0xFF;
	}
}

/***************************************************************************
* @brief      Register fingerprint
* @return     ACK_SUCCESS: success
  	      other: see the macro definition
****************************************************************************/
uint8_t AddUser(void)
{
	uint8_t m;
	
	m = GetUserCount();
	if (m >= USER_MAX_CNT)
		return ACK_FULL;

	finger_TxBuf[0] = CMD_ADD_1;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = m +1;
	finger_TxBuf[3] = 3;
	finger_TxBuf[4] = 0;		
	m = TxAndRxCmd(5, 8, 5000);	
	if (m == ACK_SUCCESS && Usart1_ReceiveStruct.RX_pData[4] == ACK_SUCCESS)
	{
		finger_TxBuf[0] = CMD_ADD_3;
		m = TxAndRxCmd(5, 8, 5000);
		if (m == ACK_SUCCESS && Usart1_ReceiveStruct.RX_pData[4] == ACK_SUCCESS)
		{
			return ACK_SUCCESS;
		}
		else
		  return ACK_FAIL;
	}
	else
		return ACK_GO_OUT;
}

/***************************************************************************
* @brief      Clear fingerprints
* @return     ACK_SUCCESS:  success
  	      ACK_FAIL:     error
****************************************************************************/
uint8_t  ClearAllUser(void)
{
 	uint8_t m;
	
	finger_TxBuf[0] = CMD_DEL_ALL;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = 0;
	finger_TxBuf[3] = 0;
	finger_TxBuf[4] = 0;
	
	m = TxAndRxCmd(5, 8, 500);
	
	if (m == ACK_SUCCESS && Usart1_ReceiveStruct.RX_pData[4] == ACK_SUCCESS)
	{	    
		return ACK_SUCCESS;
	}
	else
	{
		return ACK_FAIL;
	}
}

/***************************************************************************
* @brief      Check if user ID is between 1 and 3
* @return     TRUE
  	      FALSE
****************************************************************************/
uint8_t IsMasterUser(uint8_t UserID)
{
    if ((UserID == 1) || (UserID == 2) || (UserID == 3)) return TRUE;
		else  return FALSE;
}	 

/***************************************************************************
* @brief      Fingerprint matching
* @return     ACK_SUCCESS: success
  	      other: see the macro definition
****************************************************************************/
uint8_t VerifyUser(void)
{
	uint8_t m;
	
	finger_TxBuf[0] = CMD_MATCH;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = 0;
	finger_TxBuf[3] = 0;
	finger_TxBuf[4] = 0;
	
	m = TxAndRxCmd(5, 8, 5000);
	
	if ((m == ACK_SUCCESS) && (IsMasterUser(Usart1_ReceiveStruct.RX_pData[4]) == TRUE))
	{	
		 return ACK_SUCCESS;
	}
	else if(Usart1_ReceiveStruct.RX_pData[4] == ACK_NO_USER)
	{
		return ACK_NO_USER;
	}
	else if(Usart1_ReceiveStruct.RX_pData[4] == ACK_TIMEOUT)
	{
		return ACK_TIMEOUT;
	}
	else{
		return ACK_FAIL;
	}
}


/***************************************************************************
* @brief      Analysis of the serial port 2 command (computer serial assistant)
****************************************************************************/
void Analysis_PC_Command(void)
{		
	if(Usart2_ReceiveStruct.RX_Size != 4 )  
                return;		
	if((Usart2_ReceiveStruct.RX_pData[0] == 'C')&&(Usart2_ReceiveStruct.RX_pData[1] == 'M')&&(Usart2_ReceiveStruct.RX_pData[2] == 'D'))
	{
		switch(Usart2_ReceiveStruct.RX_pData[3])
		{						
			case '1':
				if(Finger_SleepFlag == 1)  break;
				printf("Number of fingerprints already available:  %d\r\n",GetUserCount());
				break;			
			case '2':
				if(Finger_SleepFlag == 1)  break;
        printf("Add fingerprint (Put your finger on sensor until successfully/failed information returned) \r\n");
				switch(AddUser())
				{
					case ACK_SUCCESS:
						printf("Fingerprint added successfully !\r\n");
						break;
					
					case ACK_FAIL: 			
						printf("Failed: Please try to place the center of the fingerprint flat to sensor, or this fingerprint already exists !\r\n");
						break;
					
					case ACK_FULL:			
						printf("Failed: The fingerprint library is full !\r\n");
						break;		
				}
				break;					
			case '3':
				if(Finger_SleepFlag == 1)  break;
				printf("Waiting Finger......Please try to place the center of the fingerprint flat to sensor !\r\n");
				switch(VerifyUser())
				{
					case ACK_SUCCESS:	
						printf("Matching successful !\r\n");
						break;
					case ACK_NO_USER:
						printf("Failed: This fingerprint was not found in the library !\r\n");
						break;
					case ACK_TIMEOUT:	
						printf("Failed: Time out !\r\n");
						break;	
					case ACK_GO_OUT:
						printf("Failed: Please try to place the center of the fingerprint flat to sensor !\r\n");
						break;
				}
				break;				
			case '4':
				if(Finger_SleepFlag == 1)  break;
				ClearAllUser();
				printf("All fingerprints have been cleared !\r\n");
				break;				
			case '5':
				if(Finger_SleepFlag == 1)  break;
				Finger_RST_Pin_LOW;
				Finger_SleepFlag = 1;
				printf("Module has entered sleep mode: you can use the finger Automatic wake-up function, in this mode, only CMD6 is valid, send CMD6 to pull up the RST pin of module, so that the module exits sleep !\r\n");	
				break;
			case '6':					
				Finger_RST_Pin_HIGH;
				HAL_Delay(300);  // Wait for module to start
				Finger_SleepFlag = 0;						
				printf("The module is awake. All commands are valid !\r\n");	
				break;
			
			default: break;
		}
	}             
}

/***************************************************************************
* @brief  
     If you enter the sleep mode, then open the Automatic wake-up function of the finger,
     begin to check if the finger is pressed, and then start the module and match
****************************************************************************/
void Auto_Verify_Finger(void)
{
	if(Read_Finger_WAKE_Pin == GPIO_PIN_SET)   // If you press your finger
	{		
	  while(Read_Finger_WAKE_Pin != GPIO_PIN_RESET){
			Finger_RST_Pin_HIGH;   // Pull up the RST to start the module and start matching the fingers
			LED1_Pin_HIGH;
			HAL_Delay(300);	   // Wait for module to start
						
			printf("Waiting Finger......Please try to place the center of the fingerprint flat to sensor !\r\n");
			switch(VerifyUser())
			{
				case ACK_SUCCESS:	
					printf("Matching successful !\r\n");
					break;
				case ACK_NO_USER:
					printf("Failed: This fingerprint was not found in the library !\r\n");
					break;
				case ACK_TIMEOUT:	
					printf("Failed: Time out !\r\n");
					break;	
				case ACK_GO_OUT:
					printf("Failed: Please try to place the center of the fingerprint flat to sensor !\r\n");
					break;
				default:
					break;
			}
			
			//After the matching action is completed, drag RST down to sleep
			//and continue to wait for your fingers to press
		}	
		Finger_RST_Pin_LOW;
		LED1_Pin_LOW;
		return;
	}
}

