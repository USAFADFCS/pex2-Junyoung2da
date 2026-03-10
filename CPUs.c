/** CPUs.c
 * ===========================================================
 * Name: Kim, Andrew
 * Section: M3
 * Project: PEX2 - CPU Scheduling Simulator
 * Purpose: Implements six CPU scheduling algorithms as POSIX threads.
 *          Each thread follows the same pattern every timestep:
 *            1. Wait on its per-CPU semaphore (posted by main).
 *            2. Optionally preempt the current process back to readyQ.
 *            3. Select a new process from readyQ if idle.
 *            4. Decrement burstRemaining by one (one unit of execution).
 *            5. Move the process to finishedQ if it is complete.
 *            6. Post to mainSem to signal the clock thread.
 *          All accesses to readyQ and finishedQ are protected by their
 *          respective mutex locks.
 * ===========================================================
 * Documentation Statement: <describe any help received>
 * =========================================================== */

#include <stdio.h>
#include <stdlib.h>
#include "CPUs.h"
#include "processQueue.h"

// ============================================================
// FIFO — First In First Out (non-preemptive)
// Runs each process to completion; always selects the head of
// the ready queue (the process that arrived earliest).
//
// Synchronization overview (same pattern used by all algorithms):
//   - main() is the clock driver. Each tick it posts cpuSems[i] to
//     wake this thread, then waits on mainSem for this thread to reply.
//   - This thread does one unit of work per tick, then posts mainSem
//     so main can advance the clock to the next timestep.
//   - readyQ and finishedQ are shared with main and other CPU threads,
//     so every access must be wrapped in the corresponding mutex lock.
// ============================================================
void* FIFOcpu(void* param) {
    int threadNum = ((CpuParams*) param)->threadNumber;
    SharedVars* svars = ((CpuParams*) param)->svars;

    // p is the process currently running on this CPU.
    // p == NULL means the CPU is idle and must pick a new process from readyQ.
    Process* p = NULL;

    // This thread runs forever — one loop iteration = one simulation timestep.
    while (1) {
        // ── Sync point 1: wait for main to start this timestep ──────────
        // main() posts cpuSems[threadNum] once per tick.  We block here
        // until that post arrives, keeping this CPU in lockstep with the clock.
        sem_wait(svars->cpuSems[threadNum]);

        // ── Selection (only when idle) ───────────────────────────────────
        // FIFO is non-preemptive: once a process is running (p != NULL) we
        // never replace it mid-burst.  We only enter this block when the CPU
        // has nothing to run.
        if (p == NULL) {
            // Lock readyQ before inspecting or modifying it — another CPU
            // thread (or main inserting a new arrival) could touch it right now.
            pthread_mutex_lock(&(svars->readyQLock));

            // Index 0 = head of the list = the process that has been waiting
            // the longest (qInsert always appends to the tail, so the head is
            // always the oldest arrival — that is the FIFO selection rule).
            p = qRemove(&(svars->readyQ), 0);

            if (p == NULL) {
                // readyQ was empty — CPU stays idle this tick.
                printf("No process to schedule\n");
            } else {
                printf("Scheduling PID %d\n", p->PID);
            }

            pthread_mutex_unlock(&(svars->readyQLock));
        }

        // ── Execution: one unit of work ──────────────────────────────────
        // If we have a process (carried over from a prior tick or just
        // selected above), burn one unit of its remaining CPU burst.
        if (p != NULL) {
            p->burstRemaining--;

            if (p->burstRemaining == 0) {
                // Process is done — move it to finishedQ so main can
                // compute and print wait-time statistics at simulation end.
                pthread_mutex_lock(&(svars->finishedQLock));
                qInsert(&(svars->finishedQ), p);
                pthread_mutex_unlock(&(svars->finishedQLock));

                // CPU is now idle; it will select a new process next tick.
                p = NULL;
            }
        }

        // ── Sync point 2: signal main that this CPU is done ─────────────
        // main() waits on mainSem once per CPU per tick.  Posting here
        // tells main this CPU has finished its work for the current timestep.
        sem_post(svars->mainSem);
    }
}

// ============================================================
// SJF — Shortest Job First (non-preemptive)
// Runs each process to completion; selects the process with the
// smallest burstRemaining (equals burstTotal for unscheduled processes).
// ============================================================
void* SJFcpu(void* param) {
    int threadNum = ((CpuParams*) param)->threadNumber;
    SharedVars* svars = ((CpuParams*) param)->svars;

    Process* p = NULL;  // TODO: uncomment when you implement this function


    // This thread runs forever — one loop iteration = one simulation timestep.
    while (1) {
        sem_wait(svars->cpuSems[threadNum]);
        // ── Selection (only when idle) ───────────────────────────────────
        // SJF is non preemptive
        if (p == NULL) {
            // ── Sync point 1: wait for main to start this timestep ──────────
            pthread_mutex_lock(&(svars->readyQLock));

            // Index 0 = head of the list = the process that has been waiting
            // remove the process with the shortest burstRemaining
            p = qRemove(&(svars->readyQ), qShortest(&(svars->readyQ)));

            if (p == NULL) {
                // readyQ was empty — CPU stays idle this tick.
                printf("No process to schedule\n");
            } else {
                printf("Scheduling PID %d\n", p->PID);
            }

            pthread_mutex_unlock(&(svars->readyQLock));
        }

        // ── Execution: one unit of work ──────────────────────────────────
        // If we have a process (carried over from a prior tick or just
        // selected above), burn one unit of its remaining CPU burst.
        if (p != NULL) {
            p->burstRemaining--;

            if (p->burstRemaining == 0) {
                // Process is done — move it to finishedQ so main can
                // compute and print wait-time statistics at simulation end.
                pthread_mutex_lock(&(svars->finishedQLock));
                qInsert(&(svars->finishedQ), p);
                pthread_mutex_unlock(&(svars->finishedQLock));

                // CPU is now idle; it will select a new process next tick.
                p = NULL;
            }
        }
        // ── Sync point 2: signal main that this CPU is done ─────────────
        sem_post(svars->mainSem);
    }
}

// ============================================================
// NPP — Non-Preemptive Priority
// Runs each process to completion; selects the process with the
// highest priority (lowest-numbered priority value) from the queue.
// Remember: lower priority number = higher priority.
// ============================================================
void* NPPcpu(void* param) {
    int threadNum = ((CpuParams*) param)->threadNumber;
    SharedVars* svars = ((CpuParams*) param)->svars;

    Process* p = NULL;  // TODO: uncomment when you implement this function


    // This thread runs forever — one loop iteration = one simulation timestep.
    while (1) {
        sem_wait(svars->cpuSems[threadNum]);
        // ── Selection (only when idle) ───────────────────────────────────
        // NPP is non preemptive
        if (p == NULL) {
            // ── Sync point 1: wait for main to start this timestep ──────────
            pthread_mutex_lock(&(svars->readyQLock));

            // Index 0 = head of the list = the process that has been waiting
            // remove the process with the highest priority
            p = qRemove(&(svars->readyQ), qPriority(&(svars->readyQ)));

            if (p == NULL) {
                // readyQ was empty — CPU stays idle this tick.
                printf("No process to schedule\n");
            } else {
                printf("Scheduling PID %d\n", p->PID);
            }

            pthread_mutex_unlock(&(svars->readyQLock));
        }

        // ── Execution: one unit of work ──────────────────────────────────
        // If we have a process (carried over from a prior tick or just
        // selected above), burn one unit of its remaining CPU burst.
        if (p != NULL) {
            p->burstRemaining--;

            if (p->burstRemaining == 0) {
                // Process is done — move it to finishedQ so main can
                // compute and print wait-time statistics at simulation end.
                pthread_mutex_lock(&(svars->finishedQLock));
                qInsert(&(svars->finishedQ), p);
                pthread_mutex_unlock(&(svars->finishedQLock));

                // CPU is now idle; it will select a new process next tick.
                p = NULL;
            }
        }
        // ── Sync point 2: signal main that this CPU is done ─────────────
        sem_post(svars->mainSem);
    }
}

// ============================================================
// RR — Round Robin (quantum-based preemption)
// Runs a process for at most 'quantum' timesteps before requeuing
// it; always selects from the head of the ready queue.
// ============================================================
void* RRcpu(void* param) {
    int threadNum = ((CpuParams*) param)->threadNumber;
    SharedVars* svars = ((CpuParams*) param)->svars;

    // int to track if a context switch is necessary due to quantum running out.
    int quant = 0;

    Process* p = NULL;  // TODO: uncomment when you implement this function


    // This thread runs forever — one loop iteration = one simulation timestep.
    while (1) {
        sem_wait(svars->cpuSems[threadNum]);
        // ── Selection (only when idle) ───────────────────────────────────
        // NPP is non preemptive
        if (p == NULL) {
            // ── Sync point 1: wait for main to start this timestep ──────────
            pthread_mutex_lock(&(svars->readyQLock));

            // Index 0 = head of the list = the process that has been waiting
            // remove the process at head of the queue
            p = qRemove(&(svars->readyQ), 0);

            if (p == NULL) {
                // readyQ was empty — CPU stays idle this tick.
                printf("No process to schedule\n");
            } else {
                printf("Scheduling PID %d\n", p->PID);
            }

            pthread_mutex_unlock(&(svars->readyQLock));
        }

        // if quantum has run out for the current process on the CPU
        if ((p != NULL) && (quant != 0) && ((quant % svars->quantum) == 0) && (p->burstRemaining != 0)){
            //set quant back to 0
            quant = 0;
            //set requeued to TRUE
            p->requeued = true;

            //lock the mutex so we can put it back in
            pthread_mutex_lock(&(svars->readyQLock));
            qInsert(&(svars->readyQ), p); // put the old process in back of queue
            p = qRemove(&(svars->readyQ), 0); // pull another processs from head of queue
            // print what is being scheduled
            if (p == NULL) {
                // readyQ was empty — CPU stays idle this tick.
                printf("No process to schedule\n");
            }
            else {
                printf("Scheduling PID %d\n", p->PID);
            }
            
            pthread_mutex_unlock(&(svars->readyQLock)); //unlock ready queue

        }

        // ── Execution: one unit of work ──────────────────────────────────
        // If we have a process (carried over from a prior tick or just
        // selected above), burn one unit of its remaining CPU burst.
        if (p != NULL) {
            p->burstRemaining--;
            // add one to quant to allow the thread to determine if if quantum is ending
            quant++;

            if (p->burstRemaining == 0) {
                // Process is done — move it to finishedQ so main can
                // compute and print wait-time statistics at simulation end.
                pthread_mutex_lock(&(svars->finishedQLock));
                qInsert(&(svars->finishedQ), p);
                pthread_mutex_unlock(&(svars->finishedQLock));

                //reset quantum to 0.
                quant = 0;

                // CPU is now idle; it will select a new process next tick.
                p = NULL;
            }
        }
        // ── Sync point 2: signal main that this CPU is done ─────────────
        sem_post(svars->mainSem);
    }
}

// ============================================================
// SRTF — Shortest Remaining Time First (preemptive)
// Preempts the running process whenever a shorter job is in the
// ready queue; selects the process with the smallest burstRemaining.
// ============================================================
void* SRTFcpu(void* param) {
    int threadNum = ((CpuParams*) param)->threadNumber;
    SharedVars* svars = ((CpuParams*) param)->svars;


    Process* p = NULL;  // TODO: uncomment when you implement this function


    // This thread runs forever — one loop iteration = one simulation timestep.
    while (1) {
        sem_wait(svars->cpuSems[threadNum]);
        // ── Selection (only when idle) ───────────────────────────────────
        // NPP is non preemptive
        if (p == NULL) {
            // ── Sync point 1: wait for main to start this timestep ──────────
            pthread_mutex_lock(&(svars->readyQLock));

            // Index 0 = head of the list = the process that has been waiting
            // remove the process at head of the queue
            p = qRemove(&(svars->readyQ), qShortest(&(svars->readyQ)));

            if (p == NULL) {
                // readyQ was empty — CPU stays idle this tick.
                printf("No process to schedule\n");
            } else {
                printf("Scheduling PID %d\n", p->PID);
            }

            pthread_mutex_unlock(&(svars->readyQLock));
        }

        // if there is another process that enters and is shorter, put that in CPU
        if ((p != NULL)){
            //lock
            pthread_mutex_lock(&(svars->readyQLock));
            // check to see if there is a shorter remaining burst time process
            if ((qShortestBR(&(svars->readyQ)) < p->burstRemaining)){
                //set requeued to TRUE
                p->requeued = true;

                qInsert(&(svars->readyQ), p); // put the current process in back of queue
                p = qRemove(&(svars->readyQ), qShortest(&(svars->readyQ))); // pull SRT process
                // print what is being scheduled
                if (p == NULL) {
                    // readyQ was empty — CPU stays idle this tick.
                    printf("No process to schedule\n");
                }
                else {
                    printf("Scheduling PID %d\n", p->PID);
                }
            
                pthread_mutex_unlock(&(svars->readyQLock)); //unlock ready queue
            }
            else{
                pthread_mutex_unlock(&(svars->readyQLock)); //unlock ready queue
            }


        }

        // ── Execution: one unit of work ──────────────────────────────────
        // If we have a process (carried over from a prior tick or just
        // selected above), burn one unit of its remaining CPU burst.
        if (p != NULL) {
            p->burstRemaining--;

            if (p->burstRemaining == 0) {
                // Process is done — move it to finishedQ so main can
                // compute and print wait-time statistics at simulation end.
                pthread_mutex_lock(&(svars->finishedQLock));
                qInsert(&(svars->finishedQ), p);
                pthread_mutex_unlock(&(svars->finishedQLock));

                // CPU is now idle; it will select a new process next tick.
                p = NULL;
            }
        }
        // ── Sync point 2: signal main that this CPU is done ─────────────
        sem_post(svars->mainSem);
    }
}

// ============================================================
// PP — Preemptive Priority
// Preempts the running process when a higher-priority (lower-
// numbered) process is in the ready queue.
// ============================================================
void* PPcpu(void* param) {
    int threadNum = ((CpuParams*) param)->threadNumber;
    SharedVars* svars = ((CpuParams*) param)->svars;


    Process* p = NULL;  // TODO: uncomment when you implement this function


    // This thread runs forever — one loop iteration = one simulation timestep.
    while (1) {
        sem_wait(svars->cpuSems[threadNum]);
        // ── Selection (only when idle) ───────────────────────────────────
        // NPP is non preemptive
        if (p == NULL) {
            // ── Sync point 1: wait for main to start this timestep ──────────
            pthread_mutex_lock(&(svars->readyQLock));

            // Index 0 = head of the list = the process that has been waiting
            // remove the process at head of the queue
            p = qRemove(&(svars->readyQ), qPriority(&(svars->readyQ)));

            if (p == NULL) {
                // readyQ was empty — CPU stays idle this tick.
                printf("No process to schedule\n");
            } else {
                printf("Scheduling PID %d\n", p->PID);
            }

            pthread_mutex_unlock(&(svars->readyQLock));
        }

        // if there is another process that enters and is higher priority, do context switch
        if ((p != NULL)){
            //lock
            pthread_mutex_lock(&(svars->readyQLock));
            if ((qGetPriority(&(svars->readyQ)) < p->priority)){
                //set requeued to TRUE
                p->requeued = true;

                qInsert(&(svars->readyQ), p); // put the current process in back of queue
                // get the highest priority process from the ready list
                p = qRemove(&(svars->readyQ), qPriority(&(svars->readyQ)));
                // print what is being scheduled
                if (p == NULL) {
                    // readyQ was empty — CPU stays idle this tick.
                    printf("No process to schedule\n");
                }
                else {
                    printf("Scheduling PID %d\n", p->PID);
                }
            
                pthread_mutex_unlock(&(svars->readyQLock)); //unlock ready queue
            }
            else{
                pthread_mutex_unlock(&(svars->readyQLock)); //unlock ready queue
            }


        }

        // ── Execution: one unit of work ──────────────────────────────────
        // If we have a process (carried over from a prior tick or just
        // selected above), burn one unit of its remaining CPU burst.
        if (p != NULL) {
            p->burstRemaining--;

            if (p->burstRemaining == 0) {
                // Process is done — move it to finishedQ so main can
                // compute and print wait-time statistics at simulation end.
                pthread_mutex_lock(&(svars->finishedQLock));
                qInsert(&(svars->finishedQ), p);
                pthread_mutex_unlock(&(svars->finishedQLock));

                // CPU is now idle; it will select a new process next tick.
                p = NULL;
            }
        }
        // ── Sync point 2: signal main that this CPU is done ─────────────
        sem_post(svars->mainSem);
    }
}
