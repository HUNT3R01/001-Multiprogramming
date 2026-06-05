# Fase 2 Syscall ABI

## Objetivo

La ABI de syscalls define como un programa de usuario entra al kernel de forma controlada. En esta fase, los programas de usuario no deben invocar directamente funciones del kernel; deben usar `svc #0`.

## Convencion de registros ARM

| Registro | Entrada a syscall | Salida de syscall |
|---|---|---|
| `r0` | ID de syscall | Valor de retorno `int32_t` |
| `r1` | Argumento 1 | No definido |
| `r2` | Argumento 2 | No definido |
| `r3` | Argumento 3 | No definido |

La instruccion usada por usuario es:

```asm
svc #0
```

El wrapper de usuario esta en:

```text
lib/user_syscalls.h
```

La entrada kernel esta en:

```text
os/beagle/root.s      -> svc_handler
os/beagle/syscall.c   -> svc_handler_c
```

## Tabla de syscalls

| Simbolo | ID | Argumentos | Retorno | Descripcion |
|---|---:|---|---|---|
| `SYS_YIELD` | `0` | ninguno | `0` si se procesa | Cede voluntariamente el CPU y ejecuta scheduler. |
| `SYS_EXIT` | `1` | `r1 = exit_code` | No debe retornar al proceso que sale | Marca el proceso como `TERMINATED`. |
| `SYS_WRITE` | `2` | `r1 = fd`, `r2 = buf`, `r3 = len` | bytes escritos o error negativo | Escribe en UART/stdout desde un buffer de usuario validado. |

## Retornos y errores

| Valor | Significado |
|---:|---|
| `>= 0` | Exito. En `SYS_WRITE`, cantidad de bytes escritos. |
| `-1` | ID de syscall invalido o desconocido. |
| `-2` | Descriptor o argumento invalido. Ejemplo: `fd != 1` o `len > SYS_WRITE_MAX`. |
| `-3` | Puntero de usuario invalido o fuera del rango permitido del proceso. |

## Validacion de `SYS_WRITE`

`SYS_WRITE` se implementa en `os/beagle/syscall.c` mediante `sys_write_impl()`.

Validaciones:

1. `fd` debe ser `1`.
2. `len` no debe superar `SYS_WRITE_MAX`.
3. El rango `[buf, buf + len)` debe estar dentro de `pcb[current_process].user_start` y `pcb[current_process].user_end`.
4. Se detecta overflow en `buf + len`.
5. Si el puntero no es valido, el kernel devuelve `-3` sin dereferenciar el buffer.

## Syscall desconocida

Si `r0` contiene un ID no reconocido, el dispatcher no ejecuta ninguna accion peligrosa y devuelve:

```text
rc=-1
```

Evidencia en la demo:

```text
MODE_SWITCH USER_TO_KERNEL pid=2 reason=syscall id=99
MODE_SWITCH KERNEL_TO_USER pid=2 reason=syscall_return id=99 rc=-1
```

## Trazas obligatorias

Cada syscall debe producir trazas de entrada y salida:

```text
MODE_SWITCH USER_TO_KERNEL pid=<n> reason=syscall id=<id>
MODE_SWITCH KERNEL_TO_USER pid=<m> reason=syscall_return id=<id> rc=<rc>
```

