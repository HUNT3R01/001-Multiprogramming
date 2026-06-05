# Fase 2 Demonstration Run

## Objetivo de la demo

La demo final muestra en una sola corrida:

1. Arranque inicial de kernel a usuario.
2. Preemption ordinaria por timer IRQ.
3. Syscalls exitosas desde tareas de usuario.
4. Errores controlados de syscall.
5. Una tarea de usuario que falla por `data_abort`.
6. Aislamiento de la tarea fallida sin colapso del kernel.
7. Continuacion de una tarea sana.
8. `SYS_EXIT` e idle final.

## Programas usados

- `P1`: proceso sano. Abre una ventana larga para que el timer lo interrumpa, luego ejecuta `sys_write`, `sys_yield` y finalmente `sys_exit`.
- `P2`: proceso de validacion. Ejecuta `sys_write`, `sys_yield`, pruebas negativas y finalmente fuerza un `data_abort`.

## Resultado esperado resumido

```text
initial_launch
syscall id=2 de P1
[P1] timer preemption window
timer_irq desde P1
dispatch hacia P2
syscalls validas de P2
errores rc=-2, rc=-3, rc=-1
[P2] forcing data abort
fault type=data_abort
fault_recovery hacia P1
P1 continua
P1 exit
Kernel idle
```

## Log completo

Ver:

```text
=== OS Multiprogramming - Phase 2 Beagle ===
Target: BeagleBone Black
PCBs y stacks inicializados
Timer listo
Listo - iniciando P1 en modo USR
MODE_SWITCH KERNEL_TO_USER pid=1 reason=initial_launch
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=2
[P1] timer preemption window
MODE_SWITCH KERNEL_TO_USER pid=1 reason=syscall_return id=2 rc=30
MODE_SWITCH USER_TO_KERNEL pid=1 reason=timer_irq
MODE_SWITCH KERNEL_TO_USER pid=2 reason=dispatch
MODE_SWITCH USER_TO_KERNEL pid=2 reason=syscall id=2
[P2] syscall validation
MODE_SWITCH KERNEL_TO_USER pid=2 reason=syscall_return id=2 rc=25
MODE_SWITCH USER_TO_KERNEL pid=2 reason=syscall id=2
[P2] write + yield
MODE_SWITCH KERNEL_TO_USER pid=2 reason=syscall_return id=2 rc=20
MODE_SWITCH USER_TO_KERNEL pid=2 reason=syscall id=0
MODE_SWITCH KERNEL_TO_USER pid=1 reason=syscall_return id=0 rc=0
MODE_SWITCH USER_TO_KERNEL pid=1 reason=timer_irq
MODE_SWITCH KERNEL_TO_USER pid=2 reason=dispatch
MODE_SWITCH USER_TO_KERNEL pid=2 reason=syscall id=2
[P2] write + yield
MODE_SWITCH KERNEL_TO_USER pid=2 reason=syscall_return id=2 rc=20
MODE_SWITCH USER_TO_KERNEL pid=2 reason=syscall id=0
MODE_SWITCH KERNEL_TO_USER pid=1 reason=syscall_return id=0 rc=0
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=2
[P1] write + yield
MODE_SWITCH KERNEL_TO_USER pid=1 reason=syscall_return id=2 rc=20
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=0
MODE_SWITCH KERNEL_TO_USER pid=2 reason=syscall_return id=0 rc=0
MODE_SWITCH USER_TO_KERNEL pid=2 reason=syscall id=2
MODE_SWITCH KERNEL_TO_USER pid=2 reason=syscall_return id=2 rc=-2
MODE_SWITCH USER_TO_KERNEL pid=2 reason=syscall id=2
MODE_SWITCH KERNEL_TO_USER pid=2 reason=syscall_return id=2 rc=-3
MODE_SWITCH USER_TO_KERNEL pid=2 reason=syscall id=99
MODE_SWITCH KERNEL_TO_USER pid=2 reason=syscall_return id=99 rc=-1
MODE_SWITCH USER_TO_KERNEL pid=2 reason=syscall id=2
[P2] negative syscall tests OK
MODE_SWITCH KERNEL_TO_USER pid=2 reason=syscall_return id=2 rc=32
MODE_SWITCH USER_TO_KERNEL pid=2 reason=timer_irq
MODE_SWITCH KERNEL_TO_USER pid=1 reason=dispatch
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=2
[P1] write + yield
MODE_SWITCH KERNEL_TO_USER pid=1 reason=syscall_return id=2 rc=20
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=0
MODE_SWITCH KERNEL_TO_USER pid=2 reason=syscall_return id=0 rc=0
MODE_SWITCH USER_TO_KERNEL pid=2 reason=syscall id=0
MODE_SWITCH KERNEL_TO_USER pid=1 reason=syscall_return id=0 rc=0
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=2
[P1] write + yield
MODE_SWITCH KERNEL_TO_USER pid=1 reason=syscall_return id=2 rc=20
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=0
MODE_SWITCH KERNEL_TO_USER pid=2 reason=syscall_return id=0 rc=0
MODE_SWITCH USER_TO_KERNEL pid=2 reason=syscall id=2
[P2] forcing data abort
MODE_SWITCH KERNEL_TO_USER pid=2 reason=syscall_return id=2 rc=25
MODE_SWITCH USER_TO_KERNEL pid=2 reason=timer_irq
MODE_SWITCH KERNEL_TO_USER pid=1 reason=dispatch
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=2
[P1] write + yield
MODE_SWITCH KERNEL_TO_USER pid=1 reason=syscall_return id=2 rc=20
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=0
MODE_SWITCH KERNEL_TO_USER pid=2 reason=syscall_return id=0 rc=0
MODE_SWITCH USER_TO_KERNEL pid=2 reason=fault type=data_abort
FAULT_INFO pid=2 pc=2183135784 addr=4294967295 status=1
MODE_SWITCH KERNEL_TO_USER pid=1 reason=fault_recovery
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=2
[P1] write + yield
MODE_SWITCH KERNEL_TO_USER pid=1 reason=syscall_return id=2 rc=20
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=0
MODE_SWITCH KERNEL_TO_USER pid=1 reason=syscall_return id=0 rc=0
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=2
[P1] write + yield
MODE_SWITCH KERNEL_TO_USER pid=1 reason=syscall_return id=2 rc=20
MODE_SWITCH USER_TO_KERNEL pid=1 reason=timer_irq
MODE_SWITCH KERNEL_TO_USER pid=1 reason=dispatch
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=0
MODE_SWITCH KERNEL_TO_USER pid=1 reason=syscall_return id=0 rc=0
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=2
[P1] write + yield
MODE_SWITCH KERNEL_TO_USER pid=1 reason=syscall_return id=2 rc=20
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=0
MODE_SWITCH KERNEL_TO_USER pid=1 reason=syscall_return id=0 rc=0
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=2
[P1] write + yield
MODE_SWITCH KERNEL_TO_USER pid=1 reason=syscall_return id=2 rc=20
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=0
MODE_SWITCH KERNEL_TO_USER pid=1 reason=syscall_return id=0 rc=0
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=2
[P1] exit
MODE_SWITCH KERNEL_TO_USER pid=1 reason=syscall_return id=2 rc=11
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=1
No hay procesos runnable. Kernel idle.
```

