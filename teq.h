/*
 * Tiny Event Queue is a simple event driven job queue.
 * header file
 */
#ifndef _TEQ_H_
#define _TEQ_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define SYSTICK_FREQ 1000 // set hardware timer freq
#define TEQ_JOB_MAX 8     // numbers of job

    enum TEQ_PRI
    {
        TEQ_PRI_MUTE = 0,
        TEQ_PRI_LOW = 1,
        // user defined PRI
        TEQ_PRI_MID = 4,
        TEQ_PRI_HIGH = 8
    };
    enum TEQ_RET_E
    {
        TEQ_OK,
        TEQ_ERROR = -1,
        TEQ_FULL = -2,
        TEQ_DISABLE = -3
    };

    enum TEQ_SM_T
    {
        TEQ_SM_IDLE = 0,
        TEQ_SM_WAIT,  // wait to be scheduled
        TEQ_SM_SCHED, // ready to run (loop aways in this mode)
        TEQ_SM_RUN    // running now
    };

    typedef enum TEQ_MODE
    {
        TEQ_ONESHOT,
        TEQ_LOOP
    } TEQ_MODE_TYPE;
    typedef void (*job_cb)(void *);
    typedef uint32_t (*tick_func)(void);
    typedef enum TEQ_RET_E teq_ret;
    typedef struct teq_job
    {
        struct teq_job *next; // next job in list
        struct teq_job *pre;  // previous job in list
        uint8_t id;           // job id which is unique
        uint8_t status;       // job status
        int8_t pri;           // job priority bigger is higher
        char *des;            // job description string
        job_cb func;          // job invoke routine
        void *args;           // job invoke routine's args
        uint64_t timer;       // job sched time
        TEQ_MODE_TYPE mode;   // oneshot or loop
        uint32_t delay;       // delay to run, for loop mode delay should at least 1
    } teq_job_t;

    /*
     * teq_init invoked befor any teq function.
     * This function will initial queue buffer and enable teq called function.
     */
    teq_ret teq_init(void);

    /*
     * teq_run invoked after teq_init and some scheduled job added.
     * This function will block run forever.
     * Please Be Aware all the code after this function will not be executed.
     */
    teq_ret teq_run(void);

    /*
     * teq_add_job invoked to add new job for later run.
     * id: job id for sched
     * cb: job callback which is a function pointer.
     * data: job callback parameter.
     * pri: job's priority bigger is higher.
     * mode: indicate job run once(oneshot) or repeated(loop).
     * delay: in oneshot mode job will delay "delay" system tick and schedule to run,
     *        in loop mode job will delay at least 1 system tick to run and repeat.
     * des: job's name, for debug.
     */
    teq_ret teq_add_job(uint8_t *id, job_cb cb, void *data, uint8_t pri, TEQ_MODE_TYPE mode, uint32_t delay, char *des);

    /*
     * teq_create_loop_job invoked to create a loop running job.
     * job_handler: job id for sched
     * cb: job callback which is a function pointer.
     * pri: job's priority bigger is higher.
     * period: job will run after period ticks and repeatly.
     * des: job's name, for debug.
     */
    teq_ret teq_create_loop_job(uint8_t *job_handler, job_cb cb, uint8_t pri, uint32_t period, char *des);

    /*
     * teq_create_oneshot_job invoked to create a oneshot job. The job will not run until teq_sched is called.
     * job_handler: job id for sched
     * cb: job callback which is a function pointer.
     * pri: job's priority bigger is higher.
     * des: job's name, for debug.
     */
    teq_ret teq_create_oneshot_job(uint8_t *job_handler, job_cb cb, uint8_t pri, char *des);

    /*
     * teq_sched sched a job to runnable
     * id: job id which to run.
     * arg: job callback parameter.
     * delay: for delay ms to run.
     */
    teq_ret teq_sched(uint8_t id, void *arg, uint16_t delay);

    /*
     * teq_tick must be called by hardware timer every system tick
     */
    void teq_tick(void);

    /*
     * teq_get_idle_time to get idle time for analyzing CPU load
     */
    uint32_t teq_get_idle_time(void);

    /*
     * teq_tick_inc called by system tick irq
     */
    void teq_tick_inc(void);

#ifdef __cplusplus
}
#endif

#endif // _TEQ_H_