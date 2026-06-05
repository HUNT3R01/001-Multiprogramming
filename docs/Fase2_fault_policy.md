# Fase 2 Fault-Handling Policy

## Objetivo

La politica de faults busca contener fallos de procesos de usuario sin colapsar el kernel completo. Si un proceso en modo `USR` genera un `data_abort` o `prefetch_abort`, el kernel debe registrar el fallo, aislar al proceso culpable y continuar con otro proceso sano si existe.

## Rutas implementadas

| Fault | Handler ASM | Handler C | Estado |
|---|---|---|---|
| `data_abort` | `data_abort_handler` en `os/beagle/root.s` | `fault_handler_c()` en `os/beagle/fault.c` | Probado en demo final. |
| `prefetch_abort` | `prefetch_abort_handler` en `os/beagle/root.s` | `fault_handler_c()` en `os/beagle/fault.c` | Ruta implementada; no es la prueba principal. |

## Matriz de clasificacion y resultado

| Clasificacion | Fuente esperada | Datos leidos | Resultado |
|---|---|---|---|
| `FAULT_DATA_ABORT` | Acceso invalido a datos desde `USR` | `DFSR` como `status`, `DFAR` como `fault_addr` | Marcar proceso `TERMINATED`, guardar `TERM_FAULT_DATA_ABORT`, agendar otro proceso. |
| `FAULT_PREFETCH_ABORT` | Ejecucion invalida desde `USR` | `IFSR` como `status`, `IFAR` como `fault_addr` | Marcar proceso `TERMINATED`, guardar `TERM_FAULT_PREFETCH_ABORT`, agendar otro proceso. |
| Fault desde modo privilegiado | Bug del kernel | CPSR no indica `USR` | Halt del kernel por seguridad. |
| Fault desconocido | Clasificacion no reconocida | Estado recibido por handler | Marcar proceso con `TERM_FAULT_UNKNOWN` y agendar si es fault de usuario. |

## Flujo de `data_abort` 

1. P2 ejecuta una lectura invalida desde `0xFFFFFFFF`.
2. ARM entra a `data_abort_handler`.
3. `root.s` ajusta `lr`, guarda el trap-frame y lee registros de diagnostico.
4. `fault_handler_c()` imprime trazas de fault.
5. El PCB de P2 queda con `state = TERMINATED` y `termination_reason = TERM_FAULT_DATA_ABORT`.
6. El scheduler selecciona P1.
7. El kernel restaura P1 y vuelve a `USR`.

## Trazas esperadas

```text
MODE_SWITCH USER_TO_KERNEL pid=2 reason=fault type=data_abort
FAULT_INFO pid=2 pc=<pc> addr=4294967295 status=<status>
MODE_SWITCH KERNEL_TO_USER pid=1 reason=fault_recovery
```

## Evidencia de aislamiento

Esto demuestra que P2 falla, pero P1 continua ejecutandose:

```text
MODE_SWITCH USER_TO_KERNEL pid=2 reason=fault type=data_abort
MODE_SWITCH KERNEL_TO_USER pid=1 reason=fault_recovery
MODE_SWITCH USER_TO_KERNEL pid=1 reason=syscall id=2
[P1] write + yield
```

