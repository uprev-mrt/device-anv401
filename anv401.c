/**
  *@file anv401.c
  *@brief header for 
  *@author jason
  *@date 10/13/21
  */

/* Includes ------------------------------------------------------------------*/
#include "anv401.h"

/* Private Variables ---------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**
 * @brief process transaction with device
 * @param dev ptr to device struct
 * @param buf data buffer
 * @param tx_len number of bytes to send 
 * @param rx_len number of bytes expected in response
 * @return anv401_status_e 
 */
anv401_status_e anv401_transaction(anv401_t* dev, uint8_t* buf, int tx_len, int rx_len )
{
  int len;  //Actual bytes receieved
  uint8_t chk = 0;

/*
 *  Transacton structure
 * 
 *   Byte      0       1       2       3       4     4:(n-3)      n-2    n-1    n
 *   ----  | -----   -----   -----   -----   -----   -------     ----- -----  -----
 *   cmd   |  0xf5     cmd     p1      p2      p3    ...           0     chk    0xf5
 *   ack   |  0xf5     cmd     q1      q2      q3    ...           0     chk    0xf5
 *
 *   chk = XOR[2]:[n-3]
 */

  //Set head/tail bytes
  buf[0] = ANV401_CMD_HEAD;
  buf[tx_len-1] = ANV401_CMD_TAIL;  

  //Calculate and insert checksum 
  for(int i=1 ; i< tx_len -2; i++ )
  {
    chk ^= buf[i];
  }
  buf[tx_len-1] = chk;
  
  //Send tail byte ,command, then tail byte
  MRT_UART_TX(dev->mUart, buf, tx_len, 1000);

  //Give device time to process transaction
  MRT_DELAY_MS(10);

  //Read in bytes from UART buffer
  len = MRT_UART_RX(dev->mUart, buf, rx_len, 1000);

  //TODO verify packet checksum


  //If we do not get the right number of bytes, report and return error
  if(len < rx_len)
  {
    MRT_PRINTF("[ANV401] expected %d bytes, but received %d", rx_len, len);
    return ANV401_STATUS_FAIL;
  }

  return ANV401_STATUS_SUCCESS;

}

int anv401_get_user_count(anv401_t* dev)
{
  uint8_t buf[8] = {0};
  int count; 
  anv401_status_e status;

  buf[ANV401_CMD_OFFSET] = ANV401_CMD_USER_CNT;

  status = anv401_transaction(dev, buf, 8,8); 

  if(status == ANV401_STATUS_SUCCESS)
  {
    status = buf[ANV401_STATUS_OFFSET]; 
  }


  count = (buf[2] << 8) | buf[3];

  return count;
}

/**
 * @brief Registers a fingerprint
 * 
 * @param dev ptr to device struct
 * @return anv401_status_e 
 */
anv401_status_e anv401_add_user(anv401_t* dev)
{

}