#include "flag.h"

void parseJsondata(char *data)
{
        cJSON *json_doc = cJSON_Parse((const char *)data);
    if (json_doc == NULL)
    {
        return;
    }

        if (cJSON_HasObjectItem(json_doc, "Year")) // 年
    {
        GetData.Year = atoi(cJSON_GetObjectItem(json_doc, "Year")->valuestring);
    }

    if (cJSON_HasObjectItem(json_doc, "Month")) // 月
    {
        GetData.Month = atoi(cJSON_GetObjectItem(json_doc, "Month")->valuestring);
    }

    if (cJSON_HasObjectItem(json_doc, "day")) // 日
    {
         GetData.Date = atoi(cJSON_GetObjectItem(json_doc, "day")->valuestring);
    }

    if (cJSON_HasObjectItem(json_doc, "Week")) // 周
    {
        GetData.WeekDay = atoi(cJSON_GetObjectItem(json_doc, "Week")->valuestring);
    }

    if (cJSON_HasObjectItem(json_doc, "Hour")) // 时
    {
		 UserCommon.flag.isSetRTC = ON;
        GetTime.Hours = atoi(cJSON_GetObjectItem(json_doc, "Hour")->valuestring); // 小时
    }

    if (cJSON_HasObjectItem(json_doc, "Minute")) // 分
    {
        GetTime.Minutes = atoi(cJSON_GetObjectItem(json_doc, "Minute")->valuestring);
        UserCommon.flag.isSetRTC = ON;
    }

    if (cJSON_HasObjectItem(json_doc, "Sec")) // 秒
    {
		 UserCommon.flag.isSetRTC = ON;
        GetTime.Seconds = atoi(cJSON_GetObjectItem(json_doc, "Sec")->valuestring);
    }

    cJSON_Delete(json_doc);
}

void Dealwith_Uartdata()
{
    if(UserCommon.flag.isUartReceive == ON)
    {
        UserCommon.flag.isUartReceive = OFF;

        parseJsondata(UserCommon.uartReceive);
        memset((void *)&UserCommon.uartReceive,0,UART_BUFFSIZE);
//		__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
//		HAL_UART_Receive_IT(&huart1, (uint8_t *)UserCommon.uartReceive, UART_BUFFSIZE);
    }

    if(UserCommon.flag.isSetRTC == ON)
    {
        UserCommon.flag.isSetRTC = OFF;
        HAL_RTC_SetTime(&hrtc,&GetTime,FORMAT_BIN);
        HAL_RTC_SetDate(&hrtc,&GetData,FORMAT_BIN);

    }
}
