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

    enum TEQ_RET_E
    {
        TEQ_OK,
        TEQ_ERROR = -1,
        TEQ_FULL = -2,
        TEQ_DISABLE = -3
    };

    typedef void (*job_cb)(void *);
    typedef enum TEQ_RET_E teq_ret;
    typedef struct teq_job
    {
        job_cb func;          // job invoke routine
        void *args;           // job invoke routine's args
    } teq_job_t;

    /*
     * teq_run invoked after teq_init and some scheduled job added.
     * This function will block run forever.
     * Please Be Aware all the code after this function will not be executed.
     */
    teq_ret teq_run(void);

    /*
     * cb: job callback which is a function pointer.
     * data: job callback parameter.
     */
    teq_ret teq_add(job_cb cb, void *data);

#ifdef __cplusplus
}
#endif

#endif // _TEQ_H_