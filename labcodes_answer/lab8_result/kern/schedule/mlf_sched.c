#include <defs.h>
#include <list.h>
#include <proc.h>
#include <assert.h>
#include "mlf_sched.h"

static void
mlf_init(struct run_queue *rq) {
    int factor = 1;
    for (int i = 0; i < MLF_LEVEL_NUM; i++) {
        list_init(&(rq->mlf_run_list[i]));
        rq->level_max_time_slice[i] = factor * rq->max_time_slice;
        factor *= 2;
    }
    rq->proc_num = 0;
}

static void
mlf_enqueue(struct run_queue *rq, struct proc_struct *proc) {
    if (proc->time_slice == 0 && proc->mlf_level < MLF_LEVEL_NUM - 1) {
        proc->mlf_level += 1;
    }
    int max_time_slice =  rq->level_max_time_slice[proc->mlf_level];
    if (proc->time_slice == 0 || proc->time_slice > max_time_slice) {
        proc->time_slice = max_time_slice;
    }

    list_add_before(&(rq->mlf_run_list[proc->mlf_level]), &(proc->run_link));

    proc->rq = rq;
    rq->proc_num += 1;
}

static void
mlf_dequeue(struct run_queue *rq, struct proc_struct *proc) {
    list_del_init(&(proc->run_link));
    rq->proc_num -= 1;
}

static struct proc_struct *
mlf_pick_next(struct run_queue *rq) {
    for (int i = 0; i < MLF_LEVEL_NUM; i++) {
        if (list_empty(&(rq->mlf_run_list[i]))) {
            continue;
        }
        else {
            return list_next(&(rq->mlf_run_list[i]));
        }
    }
    return NULL;
}

static void
mlf_proc_tick(struct run_queue *rq, struct proc_struct *proc) {
    if (proc->time_slice > 0) {
        proc->time_slice -= 1;
    }
    if (proc->time_slice == 0) {
        proc->need_resched = 1;
    }
}

struct sched_class mlf_sched_class = {
    .name = "mlf_scheduler",
    .init = mlf_init,
    .enqueue = mlf_enqueue,
    .dequeue = mlf_dequeue,
    .pick_next = mlf_pick_next,
    .proc_tick = mlf_proc_tick,
};