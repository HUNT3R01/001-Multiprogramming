# Fase 2 Test Report

## Plataforma

- Target: BeagleBone Black
- Carga por U-Boot/CoolTerm
- Kernel: `build/os_beagle.bin` en `0x82000000`
- P1: `build/p1.bin` en `0x82100000`
- P2: `build/p2.bin` en `0x82200000`

## Comando de compilacion recomendado

```bash
make clean
make beagle TEST=final
```

## Comandos de carga

```text
loady 0x82100000
# enviar build/p1.bin

loady 0x82200000
# enviar build/p2.bin

loady 0x82000000
# enviar build/os_beagle.bin

go 0x82000000
```

## Checklist pass/fail

| Prueba | Resultado esperado | Evidencia | Estado |
|---|---|---|---|
| Arranque inicial a usuario | `MODE_SWITCH KERNEL_TO_USER pid=1 reason=initial_launch` | Presente en `coolterm_final_log.txt`. | PASS |
| Timer IRQ | `MODE_SWITCH USER_TO_KERNEL pid=1 reason=timer_irq` | Presente antes de despachar P2. | PASS |
| Dispatch por scheduler | `MODE_SWITCH KERNEL_TO_USER pid=2 reason=dispatch` | Presente despues de timer IRQ. | PASS |
| Syscall `SYS_WRITE` | `id=2 rc>0` | P1/P2 imprimen mensajes. | PASS |
| Syscall `SYS_YIELD` | `id=0 rc=0` | P1/P2 alternan por yield. | PASS |
| Syscall `SYS_EXIT` | `id=1` y proceso no vuelve | P1 llama exit y el kernel entra a idle. | PASS |
| Descriptor invalido | `rc=-2` | P2 prueba `sys_write(9, ...)`. | PASS |
| Puntero invalido | `rc=-3` | P2 prueba `sys_write(1, 0xFFFFFFFF, ...)`. | PASS |
| Syscall desconocida | `rc=-1` | P2 llama syscall `99`. | PASS |
| Data abort | `reason=fault type=data_abort` | P2 fuerza lectura invalida. | PASS |
| Fault recovery | `reason=fault_recovery` | Kernel regresa a P1. | PASS |
| Kernel sigue vivo tras fault | P1 imprime despues del fault | `P1 write + yield` aparece luego de `fault_recovery`. | PASS |
| Idle final | `No hay procesos runnable. Kernel idle.` | Presente al final. | PASS |

## Keyed traces

### Initial boot

```text
MODE_SWITCH KERNEL_TO_USER pid=1 reason=initial_launch
```

### Timer IRQ / dispatch

```text
MODE_SWITCH USER_TO_KERNEL pid=1 reason=timer_irq
MODE_SWITCH KERNEL_TO_USER pid=2 reason=dispatch
```

### Syscall write

```text
MODE_SWITCH USER_TO_KERNEL pid=2 reason=syscall id=2
[P2] write + yield
MODE_SWITCH KERNEL_TO_USER pid=2 reason=syscall_return id=2 rc=20
```

### Syscall yield

```text
MODE_SWITCH USER_TO_KERNEL pid=2 reason=syscall id=0
MODE_SWITCH KERNEL_TO_USER pid=1 reason=syscall_return id=0 rc=0
```

### Errores de syscall

```text
MODE_SWITCH KERNEL_TO_USER pid=2 reason=syscall_return id=2 rc=-2
MODE_SWITCH KERNEL_TO_USER pid=2 reason=syscall_return id=2 rc=-3
MODE_SWITCH KERNEL_TO_USER pid=2 reason=syscall_return id=99 rc=-1
```

### Fault containment

```text
MODE_SWITCH USER_TO_KERNEL pid=2 reason=fault type=data_abort
FAULT_INFO pid=2 pc=2183135784 addr=4294967295 status=1
MODE_SWITCH KERNEL_TO_USER pid=1 reason=fault_recovery
```

### Exit / idle

```text
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=1
No hay procesos runnable. Kernel idle.
```

## Log completo

El log completo de CoolTerm esta guardado en:

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

