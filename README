The simplest job queue schedule implementation for microcontroller.

The state machine of TEQ:
                                    ┌────────────┐
┌────────────────────┐              │            │
│create oneshot job  │------------► │ TEQ_WAIT   │◄────────┐
└────────────────────┘              │            │         │
                                    └──────┬─────┘         │
                                           │               │
                                       teq_sched()         │
                                           │               │
    ┌────────────────┐              ┌──────▼─────┐         │
    │create loop job │------------► │            │         │
    └────────────────┘              │ TEQ_SCHED  │      ONESHOT
                          ┌────────►│            │         │
                          │         └──────┬─────┘         │
                          │                │               │
                          │                │               │
                         LOOP          ticks match         │
                          │                │               │
                          │         ┌──────▼─────┐         │
                          │         │            │         │
                          └─────────┤  TEQ_RUN   ├─────────┘
                                    │            │
                                    └────────────┘