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
static  void  CUS(void* p_arg);

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
    AInputFile();

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
            TaskParameter[n].TaskPeriodic,
            TaskParameter[n].CusUti);
    }

    OSTaskCreateExt(CUS,
        &TaskParameter[TASK_NUMBER-1],
        &Task_STK[TASK_NUMBER-1][TASK_STACKSIZE - 1],
        TASK_NUMBER-1,
        TaskParameter[TASK_NUMBER-1].TaskID,
        &Task_STK[TASK_NUMBER-1][0],
        TASK_STACKSIZE,
        &TaskParameter[TASK_NUMBER-1],
        (OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR),
        TaskParameter[TASK_NUMBER-1].TaskExecutionTime,
        TaskParameter[TASK_NUMBER-1].TaskArriveTime,
        TaskParameter[TASK_NUMBER-1].TaskPeriodic,
        TaskParameter[TASK_NUMBER-1].CusUti);

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

    while (1)
    {
        while (1) {
            
            if (OSTimeGet() == OSTCBCur->OSTCBDeadLine) {
                OS_Sched();
            }
            if (OSTCBCur->OSTCBWorkTime == OSTCBCur->OSTCBExecutionTime) {
                OSTimeDly(OSTCBCur->OSTCBDeadLine - OSTimeGet());
                break;
            }
        }
    }
}

void CUS(void* p_arg) {

    while (1)
    {
        while (1) {
            if (OSTCBCur->OSTCBStartTime == 0 && OSTCBCur->OSTCBWorkTime == 0) {
                OSTCBCur->OSTCBStartTime = OSTimeGet();
            }
            if (OSTimeGet() == OSTCBCur->OSTCBDeadLine) {
                OS_Sched();
            }
            if (OSTCBCur->OSTCBWorkTime == OSTCBCur->OSTCBExecutionTime) {
                
                OSTCBCur->OSTCBStat = OS_STAT_SUSPEND;
                printf("%2d    Aperiodic job(%d) is finished.\n",OSTimeGet(), completeNumber);
                if ((Output_err = fopen_s(&Output_fp, "./Output.txt", "a")) == 0)
                {
                    fprintf(Output_fp, "%2d    Aperiodic job(%d) is finished.\n", OSTimeGet(), completeNumber);
                    fclose(Output_fp);
                }
                completeNumber++;
                OS_Sched();
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
