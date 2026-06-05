# Fase 2 Requirements Status

| Seccion | Requisito | Estado | Evidencia |
|---|---|---|---|
| 3.4 | Initial boot transition a modo `USR` | PASS | `MODE_SWITCH KERNEL_TO_USER pid=1 reason=initial_launch` en `coolterm_final_log.txt`. |
| 3.5 | Timer IRQ con scheduler y retorno a `USR` | PASS | `reason=timer_irq` seguido de `reason=dispatch`. |
| 3.6 | Exception path para faults | PASS | `reason=fault type=data_abort` y `reason=fault_recovery`. |
| 3.7 | Syscall path con `svc #0` | PASS | `reason=syscall id=<id>` y `reason=syscall_return`. |
| 3.8 | Catalogo unificado `MODE_SWITCH` | PASS | Las 7 rutas aparecen en `coolterm_final_log.txt`. |
| 4.3 | ABI de registros `r0-r3` | PASS | Documentado en `phase2_syscall_abi.md`; implementado en `lib/user_syscalls.h` y `os/beagle/syscall.c`. |
| 4.4 | IDs `SYS_YIELD=0`, `SYS_EXIT=1`, `SYS_WRITE=2` | PASS | Definidos en `os/beagle/os.h` y `lib/user_syscalls.h`. |
| 4.5 | `SYS_YIELD` | PASS | Trazas `id=0 rc=0`. |
| 4.6 | `SYS_EXIT` | PASS | P1 llama `id=1` y el kernel entra a idle. |
| 4.7 | `SYS_WRITE` validado | PASS | `id=2 rc>0`, `rc=-2` y `rc=-3`. |
| 4.8 | Errores deterministas | PASS | `rc=-1`, `rc=-2`, `rc=-3`. |
| 5.1-5.3 | Fault containment | PASS | P2 causa `data_abort`; P1 continua. |
| 6 | PCB/TCB con estado unificado | PASS | Documentado en `phase2_pcb_context.md`. |
| 7 | Entregables | PASS | Este directorio `docs/` y codigo fuente actualizado. |
| 8 | Acceptance criteria | PASS | Ver `phase2_test_report.md`. |

