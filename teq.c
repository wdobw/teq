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
static tick_func tick_get;
static uint8_t job_cnt;

static teq_job_t *pop_high()
{
    teq_job_t *job = &teq_queue[0];
    teq_job_t *ptr = NULL;

    do
    {
        if (job->status == TEQ_SM_SCHED && job->timer <= sys_tick)
        {
            if (ptr == NULL)
            {
                ptr = job;
            }
            else if (job->pri > ptr->pri)
            {
                ptr = job;
            }
        }
        job = job->next;
    } while (job != &teq_queue[0]);

    return ptr;
}

static void teq_check(void)
{
    teq_job_t *job;

    do
    {
        job = pop_high();
        if (job == NULL)
        {
            return;
        }

        (job->func)(job->args);
        if (job->mode == TEQ_LOOP)
        {
            job->timer = sys_tick + job->delay;
        }
        else
        {
            job->status = TEQ_SM_WAIT;
        }

    } while (1);
}

static void teq_idle(void *arg)
{
    uint32_t last_tick = 0;
    while (teq_start)
    {

        last_tick = sys_tick;
        teq_check();

        if (sys_tick > last_tick)
        {
            idle_cnt += (sys_tick - last_tick);
        }
        __asm("wfi");
        if (tick_get)
        {
            sys_tick = tick_get();
        }
    }
}

teq_ret teq_add_job(uint8_t *id, job_cb cb, void *data, uint8_t pri, TEQ_MODE_TYPE mode, uint32_t delay, char *des)
{
    teq_job_t *ptr;

    job_cnt++;
    if (job_cnt > TEQ_JOB_MAX)
    {
        return TEQ_ERROR;
    }

    pri = pri < TEQ_PRI_LOW ? TEQ_PRI_LOW : pri;
    pri = pri > TEQ_PRI_HIGH ? TEQ_PRI_HIGH : pri;

    ptr = (teq_job_t *)&teq_queue[job_cnt];

    if (ptr->status != TEQ_SM_IDLE)
    {
        return TEQ_ERROR;
    }
    *id = job_cnt;
    ptr->id = job_cnt;
    ptr->func = cb;
    ptr->mode = mode;
    if (mode == TEQ_LOOP && delay == 0)
        delay = 1;
    ptr->delay = delay;
    ptr->timer = sys_tick + delay;
    ptr->args = data;
    ptr->des = des;
    ptr->pri = pri;
    if (ptr->mode == TEQ_LOOP)
    {
        ptr->status = TEQ_SM_SCHED;
    }
    else
    {
        ptr->status = TEQ_SM_WAIT;
    }

    return TEQ_OK;
}

teq_ret teq_sched(uint8_t id, void *arg, uint16_t delay)
{
    if (teq_start)
    {
        if (id < TEQ_JOB_MAX && teq_queue[id].status == TEQ_SM_WAIT)
        {
            teq_queue[id].status = TEQ_SM_SCHED;
            teq_queue[id].args = arg;
            teq_queue[id].timer = sys_tick + delay;
            return TEQ_OK;
        }
    }
    return TEQ_ERROR;
}

teq_ret teq_init(tick_func f)
{
    if (TEQ_JOB_MAX <= 0)
    {
        return TEQ_ERROR;
    }

    if (TEQ_JOB_MAX == 1)
    {
        teq_queue[0].next = &teq_queue[0];
        teq_queue[0].pre = &teq_queue[0];
    }
    else
    {
        for (uint32_t i = 1; i < TEQ_JOB_MAX - 1; i++)
        {
            teq_queue[i].next = &teq_queue[i + 1];
            teq_queue[i].pre = &teq_queue[i - 1];
        }
        teq_queue[TEQ_JOB_MAX - 1].next = &teq_queue[0];
        teq_queue[TEQ_JOB_MAX - 1].pre = &teq_queue[TEQ_JOB_MAX - 2];
        teq_queue[0].next = &teq_queue[1];
        teq_queue[0].pre = &teq_queue[TEQ_JOB_MAX - 1];
    }

    tick_get = f;
    return TEQ_OK;
}

teq_ret teq_run(void)
{
    teq_start = 1;
    teq_idle(NULL);
    return TEQ_OK;
}

uint32_t teq_get_idle_time(void)
{
    return idle_time;
}

teq_ret teq_create_loop_job(uint8_t *job_handler, job_cb cb, uint8_t pri, uint32_t period, char *des)
{
    return teq_add_job(job_handler, cb, NULL, pri, TEQ_LOOP, period, des);
}

teq_ret teq_create_oneshot_job(uint8_t *job_handler, job_cb cb, void *arg, uint8_t pri, char *des)
{
    return teq_add_job(job_handler, cb, arg, pri, TEQ_ONESHOT, 0, des);
}