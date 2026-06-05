# Fase 2 Requirements Status

| Seccion | Requisito | Estado | Evidencia |
|---|---|---|---|
| 3.4 | Initial boot transition a modo `USR` | PASA | `MODE_SWITCH KERNEL_TO_USER pid=1 reason=initial_launch` en `coolterm_final_log.txt`. |
| 3.5 | Timer IRQ con scheduler y retorno a `USR` | PASA | `reason=timer_irq` seguido de `reason=dispatch`. |
| 3.6 | Exception path para faults | PASA | `reason=fault type=data_abort` y `reason=fault_recovery`. |
| 3.7 | Syscall path con `svc #0` | PASA | `reason=syscall id=<id>` y `reason=syscall_return`. |
| 3.8 | Catalogo unificado `MODE_SWITCH` | PASA | Las 7 rutas aparecen en `coolterm_final_log.txt`. |
| 4.3 | ABI de registros `r0-r3` | PASA | Documentado en `phase2_syscall_abi.md`; implementado en `lib/user_syscalls.h` y `os/beagle/syscall.c`. |
| 4.4 | IDs `SYS_YIELD=0`, `SYS_EXIT=1`, `SYS_WRITE=2` | PASA | Definidos en `os/beagle/os.h` y `lib/user_syscalls.h`. |
| 4.5 | `SYS_YIELD` | PASA | Trazas `id=0 rc=0`. |
| 4.6 | `SYS_EXIT` | PASA | P1 llama `id=1` y el kernel entra a idle. |
| 4.7 | `SYS_WRITE` validado | PASA | `id=2 rc>0`, `rc=-2` y `rc=-3`. |
| 4.8 | Errores deterministas | PASA | `rc=-1`, `rc=-2`, `rc=-3`. |
| 5.1-5.3 | Fault containment | PASA | P2 causa `data_abort`; P1 continua. |
| 6 | PCB/TCB con estado unificado | PASA | Documentado en `phase2_pcb_context.md`. |
| 7 | Entregables | PASA | Este directorio `docs/` y codigo fuente actualizado. |
| 8 | Acceptance criteria | PASA | Ver `phase2_test_report.md`. |

