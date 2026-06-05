# Fase 2 Deliverables - BeagleBone Black

Este directorio contiene los entregables solicitados para la Fase 2: `Execution modes and Syscalls`.

## Mapa de entregables

| Entregable solicitado | Archivo / evidencia incluida |
|---|---|
| Updated kernel source: user/kernel boundary, IRQ return to USR, SVC pipeline, fault handling per policy | Codigo fuente en `os/beagle/`, especialmente `root.s`, `main.c`, `pcb.c`, `scheduler.c`, `syscall.c`, `fault.c`, `timer.c` y `os.h`. |
| PCB/TCB as in Section 6 | `docs/phase2_pcb_context.md` y estructura `PCB` en `os/beagle/os.h`. |
| Syscall ABI document | `docs/phase2_syscall_abi.md`. |
| Fault-handling policy document | `docs/phase2_fault_policy.md`. |
| Demonstration run | `docs/coolterm_final_log.txt`. |
| Test report: pass/fail checklist and keyed traces | `docs/phase2_test_report.md`. |
| Estado de requisitos por seccion | `docs/phase2_requirements_status.md`. |
| Pruebas seleccionables por programa | `user/tests/README_TESTS.md`. |

## Como compilar la demo final

```bash
make clean
make beagle TEST=final
```

Tambien puede usarse para los test:

```bash
make beagle TEST=write
make beagle TEST=yield
make beagle TEST=exit
make beagle TEST=errors
make beagle TEST=timer
make beagle TEST=fault
make beagle TEST=final
```

## Como cargar en BeagleBone Black

En U-Boot/CoolTerm:

```text
loady 0x82100000
# enviar build/p1.bin

loady 0x82200000
# enviar build/p2.bin

loady 0x82000000
# enviar build/os_beagle.bin

go 0x82000000
```

## Evidencia esperada principal

La corrida final debe contener, como minimo:

```text
MODE_SWITCH KERNEL_TO_USER pid=1 reason=initial_launch
MODE_SWITCH USER_TO_KERNEL pid=1 reason=timer_irq
MODE_SWITCH KERNEL_TO_USER pid=2 reason=dispatch
MODE_SWITCH USER_TO_KERNEL pid=<n> reason=syscall id=<id>
MODE_SWITCH KERNEL_TO_USER pid=<m> reason=syscall_return id=<id> rc=<rc>
MODE_SWITCH USER_TO_KERNEL pid=2 reason=fault type=data_abort
MODE_SWITCH KERNEL_TO_USER pid=1 reason=fault_recovery
No hay procesos runnable. Kernel idle.
```

