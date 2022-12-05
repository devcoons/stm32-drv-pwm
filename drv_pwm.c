/*!
	@file   drv_pwm.c
	@brief  <brief description here>
	@t.odo	-
	---------------------------------------------------------------------------

	MIT License
	Copyright (c) 2020 Ioannis Deligiannis, Federico Carnevale

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/
/******************************************************************************
* Preprocessor Definitions & Macros
******************************************************************************/


/******************************************************************************
* Includes
******************************************************************************/
#include "drv_pwm.h"

#ifdef DRV_PWM_ENABLED

/******************************************************************************
* Enumerations, structures & Variables
******************************************************************************/

static pwm_t* pwm_interfaces[16] = {NULL};
static uint32_t pwm_interfaces_cnt = 0;

/******************************************************************************
* Declaration | Static Functions
******************************************************************************/

/******************************************************************************
* Definition  | Static Functions
******************************************************************************/

/******************************************************************************
* Definition  | Public Functions
******************************************************************************/

i_status pwm_initialize(pwm_t *instance)
{
	if(instance == NULL)
		return I_ERROR;
	instance->init();
	instance->_frequency = 0;
	instance->_duty_cycle = 0;
	instance->state = PWM_INACTIVE;

	for(register uint32_t i=0;i<pwm_interfaces_cnt;i++)
		if(pwm_interfaces[i] == instance)
			return I_OK;

	pwm_interfaces[pwm_interfaces_cnt] = instance;
	pwm_interfaces_cnt++;

	return I_OK;
}

i_status pwm_deinitialize(pwm_t *instance)
{
	if(instance == NULL)
		return I_ERROR;

	instance->state = PWM_INACTIVE;
	HAL_TIM_PWM_DeInit(instance->handler);
	instance->_frequency = 0;
	instance->_duty_cycle = 0;
	return I_OK;
}

i_status pwm_start(pwm_t *instance)
{
	if(instance == NULL)
		return I_ERROR;

	if(instance->state == PWM_ACTIVE)
		return I_SKIP;

	instance->_frequency = 0;
	instance->_duty_cycle = 0;
	switch(instance->channel)
	{
		case PWM_CH_1:
			instance->handler->Instance->CCR1 = 0;
			break;
		case PWM_CH_2:
			instance->handler->Instance->CCR2 = 0;
			break;
		case PWM_CH_3:
			instance->handler->Instance->CCR3 = 0;
			break;
		case PWM_CH_4:
			instance->handler->Instance->CCR4 = 0;
			break;
		case PWM_CH_5:
			instance->handler->Instance->CCR5 = 0;
			break;
	}
	instance->handler->Instance->ARR = 2+(100*instance->clock_freq_mhz) ;
	HAL_TIM_PWM_Start(instance->handler,instance->channel);
	instance->state=PWM_ACTIVE;
	return I_OK;
}

i_status pwm_stop(pwm_t *instance)
{
	if(instance == NULL)
		return I_ERROR;

	HAL_TIM_PWM_Stop(instance->handler,instance->channel);
	instance->state=PWM_INACTIVE;
	return I_OK;
}

i_status pwm_set_period(pwm_t *instance, uint32_t value)
{
	if(instance == NULL)
		return I_ERROR;

	__disable_irq();
	instance->handler->Instance->ARR = 2+(value * instance->clock_freq_mhz) ;
	for(register uint32_t i=0;i<pwm_interfaces_cnt;i++)
	{
		if(pwm_interfaces[i]->handler == instance->handler && pwm_interfaces[i]->state == PWM_ACTIVE)
		{
			uint32_t avalue = pwm_interfaces[i]->_duty_cycle == 1000 ? pwm_interfaces[i]->handler->Instance->ARR+100 : 1+((pwm_interfaces[i]->_duty_cycle* pwm_interfaces[i]->handler->Instance->ARR)/1000);
			avalue = pwm_interfaces[i]->_duty_cycle == 0 ? 0 : avalue;
			switch(pwm_interfaces[i]->channel)
			{
			case PWM_CH_1:
				instance->handler->Instance->CCR1 = avalue;
				break;
			case PWM_CH_2:
				instance->handler->Instance->CCR2 = avalue;
				break;
			case PWM_CH_3:
				instance->handler->Instance->CCR3 = avalue;
				break;
			case PWM_CH_4:
				instance->handler->Instance->CCR4 = avalue;
				break;
			case PWM_CH_5:
				instance->handler->Instance->CCR5 = avalue;
				break;
			}
		}
	}
	__enable_irq();
	__DSB();
	return I_OK;
}

i_status pwm_set_duty_cycle(pwm_t *instance, float value)
{
	if(instance == NULL)
		return I_ERROR;

	uint32_t _d = value*10;

	if(instance->_duty_cycle == _d)
		return I_SKIP;

	instance->_duty_cycle = _d;

	if(instance->_duty_cycle > 1000)
		return I_ERROR;

	uint32_t avalue = instance->_duty_cycle == 1000 ? instance->handler->Instance->ARR+100 : 1+((instance->_duty_cycle*instance->handler->Instance->ARR)/1000);
	avalue = instance->_duty_cycle == 0 ? 0 : avalue;
	switch(instance->channel)
	{
		case PWM_CH_1:
			instance->handler->Instance->CCR1 = avalue;
			break;
		case PWM_CH_2:
			instance->handler->Instance->CCR2 = avalue;
			break;
		case PWM_CH_3:
			instance->handler->Instance->CCR3 = avalue;
			break;
		case PWM_CH_4:
			instance->handler->Instance->CCR4 = avalue;
			break;
		case PWM_CH_5:
			instance->handler->Instance->CCR5 = avalue;
			break;
	}
	__DSB();

	return I_OK;
}

i_status pwm_set_frequency(pwm_t *instance, uint32_t value)
{
	return pwm_set_period(instance,100000/(value/10));
}


/******************************************************************************
* EOF - NO CODE AFTER THIS LINE
******************************************************************************/
#endif
