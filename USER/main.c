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

/* 	led���� */ 
static void app_task_led(void* pvParameters);  

/* 	beep���� */  
static void app_task_beep(void* pvParameters); 

/* 	dth11���� */  
static void app_task_dht(void* pvParameters);

/*	sr04����*/
static void app_task_sr04(void* pvParameters);

int main(void)
{
	/* ����ϵͳ�ж����ȼ�����ֻ��ѡ���4�� */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	/* ϵͳ��ʱ���ж�Ƶ��ΪconfigTICK_RATE_HZ 168000000/1000 */
	SysTick_Config(SystemCoreClock/configTICK_RATE_HZ);									
	
	/* ��ʼ������1 */
	uart_init(9600);     							
	LED_Init();
	/* ����led���� */
	xTaskCreate((TaskFunction_t )app_task_led,  		/* ������ں��� */
			  (const char*    )"app_task_led",			/* �������� */
			  (uint16_t       )512,  				/* ����ջ��С */
			  (void*          )NULL,				/* ������ں������� */
			  (UBaseType_t    )4, 					/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_led_handle);	/* ������ƿ�ָ�� */ 
	
	/* ����beep���� */		  
	xTaskCreate((TaskFunction_t )app_task_beep,  		/* ������ں��� */
			  (const char*    )"app_task_beep",			/* �������� */
			  (uint16_t       )512,  				/* ����ջ��С */
			  (void*          )NULL,				/* ������ں������� */
			  (UBaseType_t    )4, 					/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_beep_handle);	/* ������ƿ�ָ�� */ 
			  
	/* ����dht����*/
	xTaskCreate(app_task_dht,"app_task_dht",256,NULL,4,&app_task_dht_handle);		  

	/* ����sr04����*/
	xTaskCreate(app_task_sr04,"app_task_sr04",256,NULL,4,&app_task_sr04_handle);
	/* ����������� */
	vTaskStartScheduler(); 
			  
	while(1);

}
static void app_task_led(void* pvParameters)
{
	for(;;)//while(1)
	{
		printf("app_task_led is running ...\r\n");
		vTaskDelay(100); //˯����ʱ1000�����ģ�ÿ�����ľ���1ms
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
