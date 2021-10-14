ANV401 Fingerprint Sensor
=========================

This is the device driver for the ANV401 capacitive fingerprint sensor module.


Example Code 
------------

This example is based on an stm32 platform using huart1 for the device, and the irq and reset signals labeled as FINGER_EXTI and FINER_RST


.. code:: c 

    /* Includes ------------------------------------------------------------------*/
    #include "main.h"
    #include "Devices/Biometric/ANV401-FingerprintSensor/anv401.h"

    /* Private variables ---------------------------------------------------------*/
    anv401_t fpSensor;
    volatile bool fpPresent = false;
    volatile bool addNewUser = false;

    void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
    {

        if(GPIO_Pin == FINGER_EXTI_Pin)
        {
            fpPresent = true;
        }

        if(GPIO_Pin == NEW_USER_BUTTON_Pin)
        {
            addNewUser = true;
        }

    }

    int main(void)
    {
        /* Initialization of HAL, UART, GPIO etc.. */ 

        //Initialize driver
        anv401_init(&fpSensor, MRT_GPIO(FINGER_EXTI), MRT_GPIO(FINGER_RST)));

        while(1)
        {
            if(addNewUser)
            {
                //Add whoever is touching the sensor as a new user with permission level 3
                anv401_add_user(&fpSensor, 3);
                
                addNewUser = false;
            }

            if(fpPresent)
            {
                anv401_user_t user = anv401_compare_fingerprint(&fpSensor);

                if(user.mId == ANV401_USER_NONE)
                {
                    printf("No Matching User found, Access denied");
                }
                else 
                {
                    printf("User identified\nId: %04X\nPerm: %d", user.mId, user.mPerm);
                }

                fpPresent = false;
            }
        }

    }


When the NEW_USER_BUTTON is pressed, the finger currently touching the sensor would be added as a new user. Whenever a finger touches the sensor, it will toggle the EXTI/IRQ signal, and then we can look for a match
