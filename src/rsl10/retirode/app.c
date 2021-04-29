/* ----------------------------------------------------------------------------
 * Copyright (c) 2019 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * This code is the property of ON Semiconductor and may not be redistributed
 * in any form without prior written permission from ON Semiconductor.
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 *
 * ----------------------------------------------------------------------------
 * app.c
 *  - Simple example on how to use RSL10 UART CMSIS-Driver
 * ------------------------------------------------------------------------- */

#include "app.h"
#include <printf.h>




static APP_Environemnt_t app_env = { 0 };

/** Cache for storing of image data before it gets transmitted over BLE. */
static uint8_t app_data_cache_storage[APP_DATA_CACHE_SIZE];

/* ----------------------------------------------------------------------------
 * Function      : void Button_EventCallback(void)
 * ----------------------------------------------------------------------------
 * Description   : This function is a callback registered by the function
 *                 Initialize. Based on event argument different actions are
 *                 executed.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Usart_EventCallBack(uint32_t event)
{
	RETIRODE_RMP_UARTEventHandler(event);
}




static void APP_ReadNextDataChunk(void)
{
    if ((!app_env.isp_read_in_progress)
        && (CIRCBUF_GetFree(&app_env.data_cache)
            >= 4*64))
    {


        app_env.isp_read_in_progress = true;
    }
}

static void APP_PushData(void)
{
    do
    {
        uint32_t max_data_to_push = RMTS_GetMax_TOFD_PushSize();
        uint32_t data_available = CIRCBUF_GetUsed(&app_env.data_cache);

        if ((max_data_to_push > 0) && (data_available > 0))
        {
            uint8_t buf[4*64];
            uint32_t status;
            uint32_t buf_to_write =
                    (max_data_to_push > data_available) ? data_available :
                                                          max_data_to_push;

            if (buf_to_write > 4*64)
            {
                buf_to_write = 4*64;
            }

            status = CIRCBUF_PopFront(buf, buf_to_write, &app_env.data_cache);
            //ENSURE(status == 0);

            status = RMTS_TOFD_Push(buf, buf_to_write);
            //ENSURE(status == PTSS_OK);

            /* for unused variable warnings. */
            (void)status;
        }
        else
        {
            break;
        }
    } while (1);
}

/**
 * Event handler for the Range Finder BLE service.*/

void APP_RMTS_EventHandler(RMTS_ControlPointOpCode_t opcode,
        const void *p_param)
{
    switch (opcode)
    {
       /* Connected peer device requested continuous image capture.*/
        case RMTS_OP_START_REQ:
        {


            break;
        }

        /* Connected peer device requested to abort any ongoing capture
         * operation.*/
        case RMTS_OP_CANCEL_REQ:
        {

            break;
        }

        /* Connected peer device indicates that it received image info and is
         * ready to accept image data.*/

        case RMTS_OP_DATA_TRANSFER_REQ:
        {

        	CIRCBUF_Initialize(app_data_cache_storage, APP_DATA_CACHE_SIZE,
        	        	                    &app_env.data_cache);

            app_env.isp_read_in_progress = 0;

            APP_ReadNextDataChunk();


            break;
        }


        /** RMTS is able to accept more data. */
        case RMTS_OP_DATA_SPACE_AVAIL_IND:
        {
            /* Push any cached data.*/
            APP_PushData();

            /* Start receiving more image data if cache has enough free space. */

            APP_ReadNextDataChunk();

            break;
        }

        default:
        {

            break;
        }
    }
}
/**
 * Event handler for the External Sensors Trigger BLE service.
**/

void APP_ESTS_Push_Query_Data(const RETIRODE_RMP_Query_response_t *p_param)
{

	uint16_t response[3];
	switch (p_param->reg)
	{
		case RETIRODE_RMP_ACTUAL_LASER_VOLTAGE_REGISTER:
		{
			response[0] = ESTS_OP_LASER_VOLTAGE;
			response[1] = ESTS_OP_LASER_VOLTAGE_ACTUAL;
			response[2] = p_param->value;
			break;
		}
		case RETIRODE_RMP_ACTUAL_BIAS_VOLTAGE_REGISTER:
		{
			response[0] = ESTS_OP_S_BIAS_POWER_VOLTAGE;
			response[1] = ESTS_OP_S_BIAS_POWER_VOLTAGE_ACTUAL;
			response[2] = p_param->value;
			break;
		}
		case RETIRODE_RMP_PULSE_COUNT_REGISTER:
		{
			response[0] = ESTS_OP_PULSE_COUNT;
			response[1] = ESTS_OP_PULSE_COUNT_VALUE;
			response[2] = p_param->value;
			break;
		}
	}

	ESTS_NOTIFY_QUERY_RESPONSE(response);
}
/**
 * Event handler for the External Sensors Trigger BLE service.
**/


void APP_ESTS_EventHandler(ESTS_RF_SETTING_ID_t sidx,
        const void *p_param)
{
    switch (sidx)
    {
       /** Connected peer device requested server reset. **/
        case ESTS_OP_SW_RESET:
        {
        	ESTS_OP_SW_RESET_params_t *params = p_param;

        	RETIRODE_RMP_SoftwareResetCommand();
            break;
        }
		case ESTS_OP_LASER_VOLTAGE:
		{
			ETSS_LASER_VOLTAGE_params_t *params = p_param;
			if(params->is_query == true)
			{
				if(params->type & 0x02)
				{
					RETIRODE_RMP_QueryCommand(RETIRODE_RMP_ACTUAL_LASER_VOLTAGE_REGISTER);
				}
			}
			else
			{
				RETIRODE_RMP_SetLaserPowerTargetVoltateCommand(params->value);
			}
			break;
		}
		case ESTS_OP_S_BIAS_POWER_VOLTAGE:
		{
			ETSS_S_BIAS_POWER_VOLTAGE_params_t *params = p_param;
			if(params->is_query == true)
			{
				if(params->type & 0x02)
				{
					RETIRODE_RMP_QueryCommand(RETIRODE_RMP_ACTUAL_BIAS_VOLTAGE_REGISTER);
				}
			}
			else
			{
				RETIRODE_RMP_SetPowerBiasTargetVoltateCommand(params->value);
			}
			break;
		}
		case ESTS_OP_CALIBRATE:
		{
			ETSS_CALIBRATE_params_t *params = p_param;

			if(params->is_query == true)
			{
				//SEND UART QUERY REQUEST
			}else{
				//SEND UART COMMAND
			}
			break;
		}
		case ESTS_OP_PULSE_COUNT:
		{
			ETSS_PULSE_COUNT_params_t *params = p_param;
			if(params->is_query == true)
			{
				RETIRODE_RMP_QueryCommand(RETIRODE_RMP_PULSE_COUNT_REGISTER);
			}
			else
			{
				RETIRODE_RMP_SetPulseCountCommand(params->value);
			}
			break;
		}
		default:
		{
			break;
		}
	}
}

void RETIRODE_RMP_Handler(RETIRODE_RMP_Event_t event,
        						const void *p_param)
{
 	switch(event)
	{
		case RETIRODE_RMP_EVENT_MEASUREMENT_DATA_READY:
		{
			int a = 5;
			break;
		}
		case RETIRODE_RMP_EVENT_QUERY_RESPONSE_READY:
		{
			RETIRODE_RMP_Query_response_t *params = p_param;
			APP_ESTS_Push_Query_Data(params);
			break;
		}
		case RETIRODE_RMP_EVENT_ERROR:
		{
			int a = 5;
			break;
		}
		case RETIRODE_RMP_EVENT_READY:
		{
			int a = 5;
			break;
		}
	}

	return;
}




int main(void)
{
    /* Initialize the system */
    Device_Initialize();

	int i = 0;
	/* Spin loop */
	while (true)
	{
		i++;

		RETIRODE_RMP_MainLoop();

		if(i == 20)
		{
			RETIRODE_RMP_PowerUpCommand();
		}

		/* Refresh the watchdog timer */
		Sys_Watchdog_Refresh();
		Kernel_Schedule();
	}


}
