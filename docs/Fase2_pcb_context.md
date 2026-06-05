# Fase 2 PCB / TCB and Unified Context

## Objetivo

El PCB debe guardar suficiente informacion para que IRQ, SVC y faults puedan suspender y restaurar procesos de usuario de forma coherente.

La estructura esta definida en:

```text
os/beagle/os.h
```

## Campos principales del PCB

| Campo | Proposito |
|---|---|
| `pid` | Identificador del proceso. |
| `regs[13]` | Espacio logico para registros generales. El trap-frame real queda en el stack. |
| `sp` | Stack pointer al trap-frame guardado. Es el punto que se restaura al volver al proceso. |
| `lr` | Link Register esperado o de diagnostico. |
| `pc` | Program Counter inicial o de diagnostico. |
| `cpsr` | CPSR esperado para volver a modo `USR`. |
| `state` | Estado del proceso: `READY`, `RUNNING`, `TERMINATED`. |
| `exit_code` | Codigo recibido por `SYS_EXIT`. |
| `syscall_id` | Ultimo ID de syscall observado. |
| `syscall_ret` | Ultimo retorno escrito en `r0`. |
| `fault_type` | Tipo de fault observado: `data_abort`, `prefetch_abort` o ninguno. |
| `termination_reason` | Razon de terminacion: exit o fault. |
| `user_start` / `user_end` | Rango valido de memoria de usuario usado para validar punteros. |

## Estados de proceso

| Estado | Significado |
|---|---|
| `READY` | Proceso listo para ser elegido por el scheduler. |
| `RUNNING` | Proceso actualmente en ejecucion. |
| `TERMINATED` | Proceso terminado por `SYS_EXIT` o por fault; no debe volver a ejecutarse. |

## Trap-frame compartido

`root.s` guarda el contexto con un orden compatible para IRQ, SVC y Abort:

```text
frame[0]  = r0
frame[1]  = r1
frame[2]  = r2
frame[3]  = r3
frame[4]  = r4
...
frame[12] = r12
frame[13] = lr_usr
frame[14] = cpsr_usr
frame[15] = pc_usr
```

`syscall.c` usa el frame para leer `r0-r3` y colocar el retorno en `r0`.

`fault.c` usa el frame para leer `CPSR` y `PC` del proceso que fallo.

`scheduler.c` guarda y restaura `pcb[current_process].sp` para cambiar de proceso.

## Construccion del primer contexto

`pcb.c` construye un contexto sintetico para cada proceso con:

```text
PC   = P1_ENTRY o P2_ENTRY
CPSR = USER_CPSR
LR   = 0
R0-R12 = 0
```

Luego `start_first_process()` en `root.s` usa ese frame para entrar por primera vez a `USR` mediante retorno estilo excepcion.

