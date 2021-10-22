/**
  *@file anv401.h
  *@brief header for 
  *@author jason
  *@date 10/13/21
  */

#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "Platforms/Common/mrt_platform.h"

/* Exported macro ------------------------------------------------------------*/

#define ANV401_HEAD 0xf5 
#define ANV401_TAIL 0xf5 

#define ANV401_CMD_OFFSET 1   
#define ANV401_STATUS_OFFSET 4  //Only valid for 8 byte transactions

#define ANV401_USER_MAX_CNT	   	  500	// Maximum fingerprint number

#define ANV401_SET 0 //sets value
#define ANV401_QRY 1 //querys value

#define ANV401_USER_NONE 0xFFFF

/* Exported types ------------------------------------------------------------*/

typedef enum{
  ANV401_STATUS_SUCCESS = 0x00,
  ANV401_STATUS_FAIL = 0x01,
  ANV401_STATUS_FULL = 0x04,
  ANV401_STATUS_NO_USER = 0x05,
  ANV401_STATUS_TIMEOUT = 0x08,
  ANV401_STATUS_GO_OUT = 0x0F,
} anv401_status_e;

typedef enum{
  ANV401_CMD_MOD_SN    = 0x08,
  ANV401_CMD_QRY_SN   = 0x2a,
  ANV401_CMD_SLP      = 0x2c,
  ANV401_CMD_ADD_1    = 0x01,
  ANV401_CMD_ADD_2    = 0x02,
  ANV401_CMD_ADD_3	 = 0x03,
  ANV401_CMD_COMPARE	 = 0x0C,
  ANV401_CMD_DEL		 = 0x04,
  ANV401_CMD_DEL_ALL	 = 0x05,
  ANV401_CMD_USER_CNT = 0x09,
  ANV401_CMD_COM_LEV	 = 0x28,
  ANV401_CMD_TIMEOUT  = 0x2E
} anv401_cmd_e;

/**
 * @brief transaction struct for a standard 8 byte transaction
 */
typedef struct{
  anv401_cmd_e mCmd;        //Command type
  uint8_t mData[3];         //Data (parameters for command, and return values in ack)
  anv401_status_e mStatus;  //Status of transaction
}anv401_trx_t;

/**
 * @brief user struct for anv401 device
 */
typedef struct{ 
  uint16_t mId;   //User Id
  uint8_t mPerm;  //User permission level (1-3)
}anv401_user_t;

/**
 * @brief Device struct for anv401 device
 */
typedef struct{
  mrt_uart_handle_t mUart;  //uart Handle
  mrt_gpio_t mIrq;          //IRQ/wake line
  mrt_gpio_t mRst;          //Reset pin of sensor
}anv401_t;


/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


/**
 * @brief Initialize device
 * @param dev ptr to device struct
 * @param uart uart handle for device
 * @param irq gpio handle for interrupt pin
 */
void anv401_init(anv401_t* dev, mrt_uart_handle_t uart, mrt_gpio_t irq,  mrt_gpio_t rst);


/**
 * @brief Deinitialize device and free any resources
 * @param dev ptr to device struct
 */
void anv401_deinit(anv401_t* dev);

/**
 * @brief Puts device in sleep mode
 *
 * @param dev ptr to device struct
 * @return anv401_status_e
 */
anv401_status_e anv401_sleep_mode(anv401_t* dev);

/**
 * @brief Gets the number of existing fingerprints in db
 * @param dev ptr to device struct
 * @return number of fingerprints
 */
uint16_t anv401_get_user_count(anv401_t* dev);

/**
 * @brief Registers a fingerprint
 * @param dev ptr to device struct
 * @param perm permission level for new user 1-3
 * @return anv401_status_e 
 */
anv401_status_e anv401_add_user(anv401_t* dev, uint8_t perm);

/**
 * @brief Clears all users from device
 * @param dev ptr to device
 * @return anv401_status_e 
 */
anv401_status_e anv401_clear_all_users(anv401_t* dev);

/**
 * @brief Gets the comparison level 
 * @param dev ptr to device
 * @return comparison level, 0-9 with 9 being the strictest
 */
uint8_t anv401_get_comp_level(anv401_t* dev);

/**
 * @brief sets comparison level. Default is 5
 * @param dev ptr to device
 * @param val comparison level, 0-9 with 9 being the strictest
 * @return status 
 */
anv401_status_e anv401_set_comp_level( anv401_t* dev ,uint8_t val);	// The default value is 5, can be set to 0-9, the bigger, the stricter

/**
 * @brief Checks for a match of the current fingerprint in the database
 * 
 * @param dev ptr to device
 * @return matching user. if none are found, user.mId = ANV401_USER_NONE
 */
anv401_user_t anv401_compare_fingerprint(anv401_t* dev);

/**
 * @brief Gets the fingerprint timeout value
 * @param dev ptr to device
 * @return timeout value. 
 */
uint8_t anv401_get_timeout(anv401_t* dev);	

/**
 * @brief Sets the fingerprint timeout value
 * @param dev ptr to device
 * @param val new timeout value
 * @return status
 */
anv401_status_e anv401_set_timeout(anv401_t* dev, uint8_t val);	


#ifdef __cplusplus
}
#endif



