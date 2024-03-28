#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);
#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <string.h>
#include <stdio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/random/rand32.h>
#include <zephyr/sys/mutex.h>

//#include "tasks/global.h"
//#include "tasks/display.h"
#include "tasks/monkeylist.h"
#include "tasks/display_controller.h"
#include "tasks/button_manager.h"


K_HEAP_DEFINE(monkeyListHeap,32768);

#define STACK_SIZE 1024
#define THREAD_PRIORITY 5

// Function prototypes for thread functions
void thread1(void *, void *, void *);
void thread2(void *, void *, void *);

static struct k_thread th1;
static struct k_thread th2;


// Thread stacks
K_THREAD_STACK_DEFINE(STACK1, STACK_SIZE);
K_THREAD_STACK_DEFINE(STACK2, STACK_SIZE);



void main(void)
{
        k_thread_name_set(k_current_get(), "main");

        initMonkeyList();

        button_manager_init();

        Task_Display_Controller_Init();

        // Create thread 1
        k_tid_t tid1 = k_thread_create(&th1, STACK1, STACK_SIZE, thread1, NULL, NULL, NULL,
                                        THREAD_PRIORITY, 0, K_NO_WAIT);
        k_thread_name_set(tid1, "Thread1");

        // Create thread 2
        k_tid_t tid2 = k_thread_create(&th2, STACK2, STACK_SIZE, thread2, NULL, NULL, NULL,
                                        THREAD_PRIORITY, 0, K_NO_WAIT);
        k_thread_name_set(tid2, "Thread2");
}
 
// Thread function for thread 1
void thread1(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1)
    {
        // Generate random Monkey attributes
        int num = sys_rand32_get() % 5 + 1;
        int rssi = sys_rand32_get() % 100 - 50;
        int record_time = sys_rand32_get() % 10 + 1;
        bt_addr_le_t addr;
        enum main_state state =  ST_INIT + (sys_rand32_get() % 3);

        // Append or modify Monkey
        appendOrModifyMonkey(num, rssi, record_time, state,addr);

        // Print the Monkey list
        //printk("Thread1 Monkey List:\n");
        //printMonkeys();

        k_sleep(K_MSEC(2000));
    }
}

// Thread function for thread 2
void thread2(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1)
    {

        // Generate random Monkey number to remove
        int num = sys_rand32_get() % 5 + 1;

        // Remove Monkey
        removeMonkey(num);

        // Print the Monkey list
        //printk("Thread2 Monkey List:\n");
        //printMonkeys();

        //get all monkeys example
        int tot = getNumMonkeys();
        struct Monkey array[tot];
        getAllMonkeys(array);

        for(int i = 0; i<= tot;i++)
        {
            //printk("%d, ",array[i].num);
        }

        k_sleep(K_MSEC(3000));
    }
}