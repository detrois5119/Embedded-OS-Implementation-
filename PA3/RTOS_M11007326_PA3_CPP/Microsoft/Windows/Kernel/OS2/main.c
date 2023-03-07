/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can find our product's user manual, API reference, release notes and
*               more information at https://doc.micrium.com.
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                              uC/OS-II
*                                            EXAMPLE CODE
*
* Filename : main.c
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <cpu.h>
#include  <lib_mem.h>
#include  <os.h>

#include  "app_cfg.h"


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

#define TASK_STACKSIZE          2048
static  OS_STK  StartupTaskStk[APP_CFG_STARTUP_TASK_STK_SIZE];


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  StartupTask (void  *p_arg);
static  void  task1(void* p_arg);

OS_EVENT* R1;
OS_EVENT* R2;


/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

int  main (void)
{
#if OS_TASK_NAME_EN > 0u
    CPU_INT08U  os_err;
#endif


    CPU_IntInit();

    Mem_Init();                                                 /* Initialize Memory Managment Module                   */
    CPU_IntDis();                                               /* Disable all Interrupts                               */
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */

    OSInit();                                                   /* Initialize uC/OS-II                                  */
    /*Initialize Output File*/
    OutFileInit();

    /*Input File*/
    InputFile();

    /*Dynamic Create the Stack size*/
    Task_STK = malloc(TASK_NUMBER * sizeof(int*));

    /* for each pointer, allocate storage for an array of ints */
    int n;
    for (n = 0; n < TASK_NUMBER; n++) {
        Task_STK[n] = malloc(TASK_STACKSIZE * sizeof(int));
    }
/*
    OSTaskCreateExt(StartupTask,                               /* Create the startup task                             
                     0,
                    &StartupTaskStk[APP_CFG_STARTUP_TASK_STK_SIZE - 1u],
                     APP_CFG_STARTUP_TASK_PRIO,
                     APP_CFG_STARTUP_TASK_PRIO,
                    &StartupTaskStk[0u],
                     APP_CFG_STARTUP_TASK_STK_SIZE,
                     0u,
                    (OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
*/
    INT8U err;

    R1 = OSMutexCreate(R1MAXPrio, &err,1);
    R2 = OSMutexCreate(R2MAXPrio, &err,2);

    for (n = 0; n < TASK_NUMBER; n++) {
        OSTaskCreateExt(task1,
            &TaskParameter[n],
            &Task_STK[n][TASK_STACKSIZE - 1],
            TaskParameter[n].TaskPriority,
            TaskParameter[n].TaskID,
            &Task_STK[n][0],
            TASK_STACKSIZE,
            &TaskParameter[n],
            (OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR),
            TaskParameter[n].TaskExecutionTime,
            TaskParameter[n].TaskArriveTime,
            TaskParameter[n].R1lock,
            TaskParameter[n].R1unlock,
            TaskParameter[n].R2lock,
            TaskParameter[n].R2unlock,
            TaskParameter[n].TaskPeriodic);
    }

    OSTimeSet(0);
    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II)   */

    while (DEF_ON) {                                            /* Should Never Get Here.                               */
        ;
    }
 
}



/*
*********************************************************************************************************
*                                            STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'StartupTask()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/
void task1(void* p_arg) {
    task_para_set* task_data;
    task_data = p_arg;
    int n = OSTCBCur->OSTCBId;
    int r1flag = 0;
    int r2flag = 0;
    int temp[200];
    int tempcount = 0;
    INT8U err;
    
    while (1)
    {
        while (1) {
            if (OSTCBCur->OSTCBStartime == 0 && OSTCBCur->OSTCBWorkTime == 0) {
                OSTCBCur->OSTCBStartime = OSTimeGet();
            }
            if (OSTCBCur->OSTCBR1UnLock != 0 || OSTCBCur->OSTCBR2UnLock != 0) {

                if (OSTCBCur->OSTCBWorkTime == OSTCBCur->OSTCBR1Lock && r1flag == 0 && OSTCBCur->OSTCBR1UnLock!=0) {
                    OSMutexPend(R1, 0, &err);


                    r1flag = 1;
                }
                if (OSTCBCur->OSTCBWorkTime == OSTCBCur->OSTCBR1UnLock && r1flag == 1) {

                    r1flag = 0;

                    OSMutexPost(R1);
                    OS_Sched();
                    
                }
                if (OSTCBCur->OSTCBWorkTime == OSTCBCur->OSTCBR2Lock && r2flag == 0 && OSTCBCur->OSTCBR2UnLock != 0) {
                    OSMutexPend(R2, 0, &err);

                    r2flag = 1;
                }
                if (OSTCBCur->OSTCBWorkTime == OSTCBCur->OSTCBR2UnLock && r2flag == 1) {

                    r2flag = 0;

                    OSMutexPost(R2);
                    OS_Sched();
                    
                }
            
            }
            
            if (OSTimeGet() >= OSTCBCur->OSTCBPeriodic * (OSTCBCur->OSTCBJobNum + 1) + OSTCBCur->OSTCBArriveTime) {
                OS_Sched();
            }
            if (OSTCBCur->OSTCBWorkTime == OSTCBCur->OSTCBExecutionTime) {
                
                OSTimeDly(OSTCBCur->OSTCBPeriodic * (OSTCBCur->OSTCBJobNum + 1) + OSTCBCur->OSTCBArriveTime - OSTimeGet());
                break;
            }
        }
    }
}


static  void  StartupTask (void *p_arg)
{
   (void)p_arg;

    OS_TRACE_INIT();                                            /* Initialize the uC/OS-II Trace recorder               */

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running            */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif
    
    APP_TRACE_DBG(("uCOS-III is Running...\n\r"));

    while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.       */
        OSTimeDlyHMSM(0u, 0u, 1u, 0u);
		APP_TRACE_DBG(("Time: %d\n\r", OSTimeGet()));
    }
}

void mywait(int tick)
{
#if OS_CRITICAL_METHOD==3
    OS_CPU_SR cpu_sr = 0;
#endif
    int now, exit;
    OS_ENTER_CRITICAL();
    now = OSTimeGet();
    exit = now + tick;
    OS_EXIT_CRITICAL();
    while (1) {
        if (exit <= OSTimeGet())
            break;
    }
}