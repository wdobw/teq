/*
 * Tiny Event Queue is a simple event driven job queue.
 * implementation
 */

#include <stdlib.h>
#include "teq.h"

static teq_job_t teq_queue[TEQ_JOB_MAX];

static uint8_t teq_start;
static uint64_t sys_tick;
static uint32_t idle_time, idle_cnt;
static uint8_t job_cnt;


static void teq_sched(void)
{
    teq_job_t *job = &teq_queue[1];

    do
    {
        if (job == NULL)
        {
            break;
        }

        if (job->timer == 0)
        {
            job->timer = job->delay;
            (job->func)(job->args);
        }

        job = job->next;
    } while (1);

    teq_queue[0].func(NULL);
}

static void teq_idle(void *arg)
{
    job_cb func;

    if (arg == NULL)
    {
        teq_sched();
        __asm("wfi");
    }
    else
    {
        func = arg;
        (func)(NULL);
    }
}

teq_ret teq_add_job(uint8_t *id, job_cb cb, void *data, int32_t delay, char *des)
{
    teq_job_t *ptr;

    job_cnt++;
    if (job_cnt > TEQ_JOB_MAX)
    {
        return TEQ_ERROR;
    }

    ptr = (teq_job_t *)&teq_queue[job_cnt];

    if (ptr->status != TEQ_SM_IDLE)
    {
        return TEQ_ERROR;
    }
    *id = job_cnt;
    ptr->id = job_cnt;
    ptr->func = cb;
    ptr->args = data;
    ptr->delay = delay;
    ptr->timer = delay;
    ptr->des = des;

    return TEQ_OK;
}

teq_ret teq_job_wakeup(uint8_t id, void *arg, uint16_t delay)
{
    if (teq_start)
    {
        if (id <= job_cnt)
        {
            teq_queue[id].args = arg;
            teq_queue[id].timer = 0;
            return TEQ_OK;
        }
    }
    return TEQ_ERROR;
}

teq_ret teq_init(job_cb cb)
{
    if (TEQ_JOB_MAX < 2)
    {
        return TEQ_ERROR;
    }

    for (uint32_t i = 1; i < TEQ_JOB_MAX - 1; i++)
    {
        teq_queue[i].next = &teq_queue[i + 1];
        teq_queue[i].pre = &teq_queue[i - 1];
    }
    teq_queue[TEQ_JOB_MAX - 1].next = &teq_queue[0];
    teq_queue[TEQ_JOB_MAX - 1].pre = &teq_queue[TEQ_JOB_MAX - 2];
    teq_queue[0].next = &teq_queue[1];
    teq_queue[0].pre = &teq_queue[TEQ_JOB_MAX - 1];
    
    teq_add_job(NULL, teq_idle, cb, 1, "idle");

    return TEQ_OK;
}

teq_ret teq_run(void)
{
    teq_start = 1;
    teq_idle(NULL);
    return TEQ_OK;
}

teq_ret teq_create_job(uint8_t *job_handler, job_cb cb, uint32_t period, char *des)
{
    return teq_add_job(job_handler, cb, NULL, period, des);
}

void teq_tick(void)
{
    teq_job_t *job = &teq_queue[0];

    do
    {
        if (job == NULL)
        {
            return;
        }

        if (job->timer > 0)
        {
            job->timer--;
        }

        job = job->next;
    } while (1);  
}