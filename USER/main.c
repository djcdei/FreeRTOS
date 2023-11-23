#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "dht11.h"
#include "sr04.h"
#include "FreeRTOS.h"
#include "task.h"

static TaskHandle_t app_task_led_handle = NULL;
static TaskHandle_t app_task_beep_handle = NULL;
static TaskHandle_t app_task_dht_handle = NULL;
static TaskHandle_t app_task_sr04_handle = NULL;

/* 	led任务 */ 
static void app_task_led(void* pvParameters);  

/* 	beep任务 */  
static void app_task_beep(void* pvParameters); 

/* 	dth11任务 */  
static void app_task_dht(void* pvParameters);

/*	sr04任务*/
static void app_task_sr04(void* pvParameters);

int main(void)
{
	/* 设置系统中断优先级分组只能选择第4组 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	/* 系统定时器中断频率为configTICK_RATE_HZ 168000000/1000 */
	SysTick_Config(SystemCoreClock/configTICK_RATE_HZ);									
	
	/* 初始化串口1 */
	uart_init(9600);     							
	LED_Init();
	/* 创建led任务 */
	xTaskCreate((TaskFunction_t )app_task_led,  		/* 任务入口函数 */
			  (const char*    )"app_task_led",			/* 任务名字 */
			  (uint16_t       )512,  				/* 任务栈大小 */
			  (void*          )NULL,				/* 任务入口函数参数 */
			  (UBaseType_t    )4, 					/* 任务的优先级 */
			  (TaskHandle_t*  )&app_task_led_handle);	/* 任务控制块指针 */ 
	
	/* 创建beep任务 */		  
	xTaskCreate((TaskFunction_t )app_task_beep,  		/* 任务入口函数 */
			  (const char*    )"app_task_beep",			/* 任务名字 */
			  (uint16_t       )512,  				/* 任务栈大小 */
			  (void*          )NULL,				/* 任务入口函数参数 */
			  (UBaseType_t    )4, 					/* 任务的优先级 */
			  (TaskHandle_t*  )&app_task_beep_handle);	/* 任务控制块指针 */ 
			  
	/* 创建dht任务*/
	xTaskCreate(app_task_dht,"app_task_dht",256,NULL,4,&app_task_dht_handle);		  

	/* 创建sr04任务*/
	xTaskCreate(app_task_sr04,"app_task_sr04",256,NULL,4,&app_task_sr04_handle);
	/* 开启任务调度 */
	vTaskStartScheduler(); 
			  
	while(1);

}
static void app_task_led(void* pvParameters)
{
	for(;;)//while(1)
	{
		printf("app_task_led is running ...\r\n");
		vTaskDelay(100); //睡眠延时1000个节拍，每个节拍就是1ms
	}
}   

static void app_task_beep(void* pvParameters)
{
	for(;;)//while(1)
	{
		printf("app_task_beep is running ...\r\n");
		vTaskDelay(100);
	}
} 
static void app_task_dht(void* pvParameters)
{
	for(;;)//while(1)
	{

		printf("app_task_dht is running ...\r\n");
		vTaskDelay(100);
	}
} 

static void app_task_sr04(void* pvParameters)
{
	for(;;)//while(1)
	{
		printf("app_task_sr04 is running ...\r\n");
		vTaskDelay(100);
	}
}

/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}


void vApplicationTickHook( void )
{

}
