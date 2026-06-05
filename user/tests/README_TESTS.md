# Tests de usuario para Fase 2

Cada prueba cambia únicamente los binarios de usuario `p1.bin` y `p2.bin`. El kernel sigue cargando P1 en `0x82100000`, P2 en `0x82200000` y el OS en `0x82000000`.

Comandos:

```bash
make clean
make beagle TEST=final
make beagle TEST=final_demo
make beagle TEST=write
make beagle TEST=yield
make beagle TEST=exit
make beagle TEST=errors
make beagle TEST=timer
make beagle TEST=fault
```

Después de compilar se cargan los binarios igual que siempre:

```text
loady 0x82100000   build/p1.bin
loady 0x82200000   build/p2.bin
loady 0x82000000   build/os_beagle.bin
go 0x82000000
```

Pruebas disponibles:

| TEST | Qué valida |
|---|---|
| `final` | Demo completa de Fase 2 usando `user/P1/main.c` y `user/P2/main.c`. |
| `final_demo` | Copia guardada de la demo completa dentro de `user/tests/final_demo`. |
| `write` | `SYS_WRITE` correcto en P1 y P2. |
| `yield` | `SYS_YIELD` cambia de proceso. |
| `exit` | `SYS_EXIT` termina P2 y no lo vuelve a ejecutar. |
| `errors` | `rc=-2`, `rc=-3` y `rc=-1`. |
| `timer` | Preemption por timer sin `sys_yield`. |
| `fault` | `data_abort` en P2 y recuperación hacia P1. |
