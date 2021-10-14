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
#include "Platforms/Common/mrt_platform.h"

/* Exported macro ------------------------------------------------------------*/

#define ANV401_HEAD 0xf5 
#define ANV401_TAIL 0xf5 

#define ANV401_CMD_OFFSET 1   
#define ANV401_STATUS_OFFSET 4  //Only valid for 8 byte transactions

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
  ANV401_CMD_HEAD	 = 0xF5,
  ANV401_CMD_TAIL	 = 	0xF5,
  ANV401_CMD_ADD_1    = 0x01,
  ANV401_CMD_ADD_2    = 0x02,
  ANV401_CMD_ADD_3	 = 0x03,
  ANV401_CMD_MATCH	 = 0x0C,
  ANV401_CMD_DEL		 = 0x04,
  ANV401_CMD_DEL_ALL	 = 0x05,
  ANV401_CMD_USER_CNT = 0x09,
  ANV401_CMD_COM_LEV	 = 0x28,
  ANV401_CMD_LP_MODE  = 0x2C,
  ANV401_CMD_TIMEOUT  = 0x2E
} anv401_cmd_e;


typedef struct{
  mrt_uart_handle_t mUart;  //uart Handle
  mrt_gpio_t mIrq;          //IRQ/wake line
}anv401_t;

/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/**
 * @brief Initialize device
 * @param dev ptr to device struct
 * @param uart uart handle for device
 * @param irq gpio handle for interrupt pin
 */
void anv401_init(anv401_t* dev, mrt_uart_handle_t uart, mrt_gpio_t irq);

/**
 * @brief Deinitialize device and free any resources
 * @param dev ptr to device struct
 */
void anv401_deinit(anv401_t* dev);

/**
 * @brief Handles an inerrupt from the fingerprint sensor. This should be called on the Falling edge of the IRQ line
 * @param dev ptr to device struct
 * @return anv401_status_e 
 */
anv401_status_e anv401_handle_irq(anv401_t* dev);

/**
 * @brief Gets the number of existing fingerprints in db
 * @param dev ptr to device struct
 * @return number of fingerprints, or -1 if there is an error
 */
int anv401_get_user_count(anv401_t* dev);

/**
 * @brief Registers a fingerprint
 * 
 * @param dev ptr to device struct
 * @return anv401_status_e 
 */
anv401_status_e anv401_add_user(anv401_t* dev);

anv401_status_e anv401_get_compare_level(anv401_t* dev);
anv401_status_e anv401_set_compare_level( anv401_t* dev ,uint8_t temp);	// The default value is 5, can be set to 0-9, the bigger, the stricter
anv401_status_e anv401_clear_all_users(anv401_t* dev);
anv401_status_e anv401_verify_user(anv401_t* dev);
anv401_status_e anv401_get_timeout(anv401_t* dev);	


#ifdef __cplusplus
}
#endif



