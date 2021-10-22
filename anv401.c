/**
  *@file anv401.c
  *@brief header for 
  *@author jason
  *@date 10/13/21
  */

/* Includes ------------------------------------------------------------------*/
#include "anv401.h"
#include <string.h>

/* Private Variables ---------------------------------------------------------*/

/* Private Functions ---------------------------------------------------------*/

/**
 * @brief builds out trx data with command byte 
 * @param trx 
 * @param cmd 
 */
void anv401_build_trx(anv401_trx_t* trx, uint8_t cmd)
{
  trx->mCmd = cmd;
  memset(trx->mData,0,3);
}

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
 *  Transacton structure for standard 8 byte packet
 * 
 * +--------+--------+--------+--------+--------+--------+--------+--------+--------+
 * | Byte   |   0    |   1    |   2    |    3   |    4   |    5   |    6   |    7   |
 * +========+========+========+========+========+========+========+========+========+
 * |  CMD   | 0xF5   |  cmd   |   P1   |   P2   |   P3   |    0   |  chk   |  0xf5  |
 * +--------+--------+--------+--------+--------+--------+--------+--------+--------+
 * |  ACK   | 0xF5   |  cmd   |   Q1   |   Q2   |   Q3   |    0   |  chck  |  0xf5  |
 * +--------+--------+--------+--------+--------+--------+--------+--------+--------+
 * 
 *
 *   chk = XOR[2]:[5]
 * 
 */

  //Set head/tail bytes
  buf[0] = ANV401_HEAD;
  buf[tx_len-1] = ANV401_TAIL;

  //Calculate and insert checksum 
  for(int i=1 ; i< tx_len -2; i++ )
  {
    chk ^= buf[i];
  }
  buf[tx_len-2] = chk;
  
  //Send tail byte ,command, then tail byte
  MRT_UART_TX(dev->mUart, buf, tx_len, 1000);

  //Give device time to process transaction
  MRT_DELAY_MS(10);

  //Read in bytes from UART buffer
  len = MRT_UART_RX(dev->mUart, buf, rx_len, 1000);

  //If we do not get the right number of bytes, report and return error
  if(len < rx_len)
  {
    MRT_PRINTF("[ANV401] expected %d bytes, but received %d", rx_len, len);
    return ANV401_STATUS_FAIL;
  }
  
  //TODO verify packet checksum
  chk = 0;
  for(int i=1 ; i< rx_len -2; i++ )
  {
    chk ^= buf[i];
  }

  if(chk != buf[rx_len -2])
  {
    MRT_PRINTF("[ANV401] Bad Checksum. Expected %02X , but received %02X", chk, buf[rx_len -2]);
    return ANV401_STATUS_FAIL;
  }


  return ANV401_STATUS_SUCCESS;

}

/**
 * @brief Processes a standard 8 byte transaction
 * 
 * @param dev ptr to device struct
 * @param trx ptr to transaction struct
 * @return anv401_status_e 
 * 
 * 
 * +--------+--------+--------+--------+--------+--------+--------+--------+--------+
 * | Byte   |   0    |   1    |   2    |    3   |    4   |    5   |    6   |    7   |
 * +========+========+========+========+========+========+========+========+========+
 * |  CMD   | 0xF5   |  cmd   |   P1   |   P2   |   P3   |    0   |  chk   |  0xf5  |
 * +--------+--------+--------+--------+--------+--------+--------+--------+--------+
 * |  ACK   | 0xF5   |  cmd   |   Q1   |   Q2   |   Q3   |    0   |  chck  |  0xf5  |
 * +--------+--------+--------+--------+--------+--------+--------+--------+--------+
 */
 
anv401_status_e anv401_std_transaction(anv401_t* dev, anv401_trx_t* trx )
{
  uint8_t buf[8] = {0};
  trx->mStatus = ANV401_STATUS_FAIL;

  //Copy command
  buf[ANV401_CMD_OFFSET] = trx->mCmd;

  //Copy parameters
  memcpy(&buf[2], trx->mData, 3);

  //Add Null
  buf[5] = 0;

  //Process transaction
  trx->mStatus = anv401_transaction(dev, buf, 8,8);

  if(trx->mStatus == ANV401_STATUS_FAIL)
  {
    return trx->mStatus;
  }

  //This is not the status byte for all 8-byte transactions. but it is for most
  trx->mStatus = buf[4];

  //Copy in info from ack
  trx->mCmd = buf[ANV401_CMD_OFFSET];
  memcpy( trx->mData,&buf[2], 3);


  return trx->mStatus ;
}

/* Public Functions ---------------------------------------------------------*/

void anv401_init(anv401_t* dev, mrt_uart_handle_t uart, mrt_gpio_t irq,  mrt_gpio_t rst)
{
  dev->mUart = uart;
  dev->mIrq = irq;
  dev->mRst = rst;
}

anv401_status_e anv401_sleep_mode(anv401_t* dev)
{
	anv401_trx_t trx;

	anv401_build_trx(&trx, ANV401_CMD_SLP);

	return anv401_std_transaction(dev, &trx);
}

uint16_t anv401_get_user_count(anv401_t* dev)
{
  uint16_t count =0;
  anv401_trx_t trx; 

  anv401_build_trx(&trx, ANV401_CMD_USER_CNT);

  anv401_std_transaction(dev, &trx);

  if(trx.mStatus == ANV401_STATUS_SUCCESS)
  { 
    //Get Count from return data
    count = ((trx.mData[0] << 8) | trx.mData[1]);
  }


  return count;
}

/**
 * @brief Registers a fingerprint
 * 
 * @param dev ptr to device struct
 * @return anv401_status_e 
 */
anv401_status_e anv401_add_user(anv401_t* dev, uint8_t perm)
{
  uint16_t user_count = anv401_get_user_count(dev); 
  uint16_t new_user_id = user_count+1;
  anv401_trx_t trx;

  if(user_count >=ANV401_USER_MAX_CNT )
  {
    return ANV401_STATUS_FULL;
  }

  //Must be called 3 times, with a different command each time (see page 6 in datasheet)
  
  //First
  anv401_build_trx(&trx, ANV401_CMD_ADD_1);
  trx.mData[0] = new_user_id >> 8;
  trx.mData[1] = new_user_id && 0xFF;
  trx.mData[3] = perm;

  if(anv401_std_transaction(dev, &trx) != ANV401_STATUS_SUCCESS)
  {
    return trx.mStatus;
  }

  //Second
  anv401_build_trx(&trx, ANV401_CMD_ADD_2);
  trx.mData[0] = new_user_id >> 8;
  trx.mData[1] = new_user_id && 0xFF;
  trx.mData[3] = perm;

  if(anv401_std_transaction(dev, &trx) != ANV401_STATUS_SUCCESS)
  {
    return trx.mStatus;
  }

  //Third
  anv401_build_trx(&trx, ANV401_CMD_ADD_3);
  trx.mData[0] = new_user_id >> 8;
  trx.mData[1] = new_user_id && 0xFF;
  trx.mData[3] = perm;

  if(anv401_std_transaction(dev, &trx) != ANV401_STATUS_SUCCESS)
  {
    return trx.mStatus;
  }

  return trx.mStatus;
}

anv401_status_e anv401_clear_all_users(anv401_t* dev)
{
  anv401_trx_t trx;
  anv401_build_trx(&trx, ANV401_CMD_DEL_ALL);
  return anv401_std_transaction(dev,&trx);
}

uint8_t anv401_get_comp_level(anv401_t* dev)
{
  anv401_trx_t trx;
  anv401_build_trx(&trx, ANV401_CMD_COM_LEV);

  //This byte detemermines if we are setting, or getting
  trx.mData[2] = ANV401_QRY;

  if(anv401_std_transaction(dev,&trx) != ANV401_STATUS_SUCCESS)
  {
    return 0xFF;
  }

  //Current level is in data[1]
  return trx.mData[1];

}

anv401_status_e anv401_set_comp_level( anv401_t* dev ,uint8_t val)
{
  anv401_trx_t trx;
  anv401_build_trx(&trx, ANV401_CMD_COM_LEV);

  //This byte detemermines if we are setting, or getting
  trx.mData[2] = ANV401_SET;
  trx.mData[1] = val;

  return anv401_std_transaction(dev,&trx);
}

anv401_user_t anv401_compare_fingerprint(anv401_t* dev)
{
  anv401_trx_t trx;
  anv401_user_t user;

  user.mPerm = 0;
  user.mId = ANV401_USER_NONE;

  anv401_build_trx(&trx, ANV401_CMD_COMPARE);

  anv401_std_transaction(dev,&trx);

  //Compare does not return a normal status
  //Status for this transactions will be a permission level (1-3), ACK_NOUSER, or ACK_TIMEOUT
  if( trx.mStatus < 4)
  {
    user.mPerm = trx.mStatus;
    user.mId = (trx.mData[0] << 8 ) | trx.mData[1];
  }

  return user;
}


uint8_t anv401_get_timeout(anv401_t* dev)
{
  anv401_trx_t trx;
  anv401_build_trx(&trx, ANV401_CMD_TIMEOUT);
  uint8_t timeout = 0;

  //This byte detemermines if we are setting, or getting
  trx.mData[2] = ANV401_QRY;

  if( anv401_std_transaction(dev,&trx) == ANV401_STATUS_SUCCESS)
  {
    timeout = trx.mData[1];
  }

  return timeout;
}	

anv401_status_e anv401_set_timeout(anv401_t* dev, uint8_t val)	
{
  anv401_trx_t trx;
  anv401_build_trx(&trx, ANV401_CMD_TIMEOUT);
  uint8_t timeout = 0;

  //This byte detemermines if we are setting, or getting
  trx.mData[2] = ANV401_QRY;
  trx.mData[1] =val;

  return anv401_std_transaction(dev,&trx);
}	
