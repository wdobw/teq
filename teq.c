/*
 * Tiny Event Queue is a simple event driven job queue.
 * implementation
 */

#include <stdlib.h>
#include "teq.h"

#define TEQ_JOB_MAX             8     // numbers of job
static teq_job_t teq_queue[TEQ_JOB_MAX];
static volatile uint16_t teq_queue_mapping = 0;

teq_ret teq_add(job_cb cb, void *data)
{
    uint16_t i;

    for (i = 0; i < TEQ_JOB_MAX; i++)
    {
        if (teq_queue_mapping & (1 << i) == 0)
        {
            teq_queue_mapping |= (1 << i);
            teq_queue[i].func = cb;
            teq_queue[i].args = data;
            return TEQ_OK;
        }
    }

    return TEQ_FULL;
}

teq_ret teq_run(void)
{
    uint16_t i;

    while (1)
    {
        for (i = 0; i < TEQ_JOB_MAX; i++)
        {
            if (teq_queue_mapping & (1 << i) == 1)
            {
                (teq_queue[i].func)(teq_queue[i].args);
                teq_queue_mapping &= ~(1 << i);
            }
        }

        // idle 
        __asm("wfi");
    }

    return TEQ_OK;
}

