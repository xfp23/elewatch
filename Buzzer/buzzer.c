#include "buzzer.h"
#include "stdlib.h"
#include "string.h"

Buzzer_Status_t Buzzer_Init(Buzzer_Handle_t *handle, Buzzer_Conf_t *conf)
{
    if (handle == NULL || *handle != NULL)
        return BUZZER_ERROR;

    *handle = (Buzzer_Obj *)calloc(1, sizeof(Buzzer_Obj));

    if ((*handle) == NULL)
        return BUZZER_ERROR;

    (*handle)->type = conf->type;

    if ((*handle)->type == BUZZER_ACTIVE)
    {
        (*handle)->Hardware.beep.port = conf->beep.port;
        (*handle)->Hardware.beep.pin = conf->beep.pin;
        (*handle)->Hardware.tim = NULL;
        (*handle)->Hardware.channel = 0xFF;
    }
    else if ((*handle)->type == BUZZER_PASSIVE)
    {
        (*handle)->Hardware.tim = conf->tim;
        (*handle)->Hardware.channel = conf->channel;
    }
    else
    {
        free(*handle);
        *handle = NULL;
        return BUZZER_TYPE_ERROR;
    }

    return BUZZER_OK;
}

Buzzer_Status_t Buzzer_TickHandler(Buzzer_Handle_t *handle)
{
    if (handle == NULL || (*handle) == NULL)
        return BUZZER_ERROR;

    if ((*handle)->flag.CNT_INT == BUZZER_ON)
    {
        (*handle)->beep_1ms_count = (*handle)->dur;
        (*handle)->flag.CNT_INT = BUZZER_OFF;
    }

    switch ((*handle)->type)
    {
    case BUZZER_PASSIVE:
        // 无源蜂鸣器
        if ((*handle)->flag.EN == BUZZER_OFF)
        {
            HAL_TIM_PWM_Stop((*handle)->Hardware.tim, (*handle)->Hardware.channel);
            return BUZZER_OK;
        }

        if ((*handle)->rpt > 0) // 总次数
        {
            if ((*handle)->flag.BEEP == BUZZER_ON)
            {
                (*handle)->flag.BEEP = BUZZER_OFF;
                HAL_TIM_PWM_Stop((*handle)->Hardware.tim, (*handle)->Hardware.channel);
            }
            if ((*handle)->beep_1ms_count == 0) // 走时间
            {
                (*handle)->flag.CNT_INT = BUZZER_ON;
                HAL_TIM_PWM_Start((*handle)->Hardware.tim, (*handle)->Hardware.channel);
                (*handle)->flag.BEEP = BUZZER_ON;
                (*handle)->rpt--;
            }
            else
            {
                (*handle)->beep_1ms_count--;
            }
        }
        else
        {

            (*handle)->flag.EN = BUZZER_OFF;
        }
        break;
    case BUZZER_ACTIVE:
        // 有源蜂鸣器
        if ((*handle)->flag.EN == BUZZER_OFF)
        {
            HAL_GPIO_WritePin((*handle)->Hardware.beep.port, (*handle)->Hardware.beep.pin, GPIO_PIN_RESET);
            return BUZZER_OK;
        }

        if ((*handle)->rpt > 0) // 总次数
        {
            if ((*handle)->flag.BEEP == BUZZER_ON)
            {
                (*handle)->flag.BEEP = BUZZER_OFF;
                HAL_GPIO_WritePin((*handle)->Hardware.beep.port, (*handle)->Hardware.beep.pin, GPIO_PIN_SET);
            }
            if ((*handle)->beep_1ms_count == 0) // 走时间
            {
                (*handle)->flag.CNT_INT = BUZZER_ON;
                HAL_GPIO_WritePin((*handle)->Hardware.beep.port, (*handle)->Hardware.beep.pin, GPIO_PIN_RESET);
                (*handle)->flag.BEEP = BUZZER_ON;
                (*handle)->rpt--;
            }
            else
            {
                (*handle)->beep_1ms_count--;
            }
        }
        else
        {

            (*handle)->flag.EN = BUZZER_OFF;
        }
        break;
    default:
        return BUZZER_TYPE_ERROR;
    }
    return BUZZER_OK;
}

Buzzer_Status_t Buzzer_StartBeep(Buzzer_Handle_t *handle, uint32_t dur, uint32_t rpt, int value)
{
    if (handle == NULL || *handle == NULL)
        return BUZZER_ERROR;

    if ((*handle)->type != BUZZER_ACTIVE && (*handle)->type != BUZZER_PASSIVE)
        return BUZZER_TYPE_ERROR;

    (*handle)->flag.EN = BUZZER_ON;
    (*handle)->flag.CNT_INT = BUZZER_ON;
    (*handle)->flag.BEEP = BUZZER_ON;
    (*handle)->dur = dur;
    (*handle)->rpt = rpt;
    if ((*handle)->type == BUZZER_ACTIVE)
        return BUZZER_OK;

    if (value < 0)
        value = 50;
    if (value > 100)
        value = 100;

    uint16_t duty = (uint32_t)(((*handle)->Hardware.tim->Init.Period + 1) * value / 100);
    __HAL_TIM_SET_COMPARE((*handle)->Hardware.tim, (*handle)->Hardware.channel, duty);

    return BUZZER_OK;
}

Buzzer_Status_t Buzzer_Delete(Buzzer_Handle_t *handle)
{
    if (handle == NULL || *handle == NULL)
        return BUZZER_ERROR;

    free(*handle);
    *handle = NULL;

    return BUZZER_OK;
}
