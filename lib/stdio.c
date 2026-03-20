#include "stdio.h"
#include "../os/os.h"
#include <stdarg.h>
#include <stddef.h>

static size_t my_strlen(const char *s) {
  size_t n = 0;
  while (s && s[n]) n++;
  return n;
}

static int is_space(char c) {
  return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

static void write_c(char c) {
  os_write(&c, 1);
}

static void write_s(const char *s) {
  os_write(s, my_strlen(s));
}

static void itoa_dec(int v, char *out, size_t out_sz) {
  if (!out || out_sz == 0) return;

  unsigned int x;
  int neg = 0;
  if (v < 0) { neg = 1; x = (unsigned int)(-v); }
  else { x = (unsigned int)v; }

  char tmp[16];
  size_t i = 0;

  if (x == 0) tmp[i++] = '0';
  while (x && i < sizeof(tmp)) {
    tmp[i++] = (char)('0' + (x % 10u));
    x /= 10u;
  }

  size_t pos = 0;
  if (neg && pos + 1 < out_sz) out[pos++] = '-';

  while (i > 0 && pos + 1 < out_sz) {
    out[pos++] = tmp[--i];
  }
  out[pos] = '\0';
}

static int atoi_dec(const char *s) {
  if (!s) return 0;
  while (is_space(*s)) s++;

  int neg = 0;
  if (*s == '-') { neg = 1; s++; }
  else if (*s == '+') { s++; }

  int v = 0;
  while (*s >= '0' && *s <= '9') {
    v = v * 10 + (*s - '0');
    s++;
  }
  return neg ? -v : v;
}

static float atof_simple(const char *s) {
  if (!s) return 0.0f;
  while (is_space(*s)) s++;

  int neg = 0;
  if (*s == '-') { neg = 1; s++; }
  else if (*s == '+') { s++; }

  float val = 0.0f;
  while (*s >= '0' && *s <= '9') {
    val = val * 10.0f + (float)(*s - '0');
    s++;
  }

  if (*s == '.') {
    s++;
    float place = 0.1f;
    while (*s >= '0' && *s <= '9') {
      val = val + (float)(*s - '0') * place;
      place *= 0.1f;
      s++;
    }
  }

  return neg ? -val : val;
}

static void ftoa_simple(float f, char *out, size_t out_sz) {
  if (!out || out_sz == 0) return;

  // 2 decimales
  int neg = 0;
  if (f < 0.0f) { neg = 1; f = -f; }

  int ip = (int)f;
  float frac = f - (float)ip;
  int fp = (int)(frac * 100.0f + 0.5f);

  char ibuf[24];
  itoa_dec(ip, ibuf, sizeof(ibuf));

  char fbuf[8];
  // asegurar 2 dígitos
  fbuf[0] = (char)('0' + (fp / 10) % 10);
  fbuf[1] = (char)('0' + (fp % 10));
  fbuf[2] = '\0';

  size_t pos = 0;
  if (neg && pos + 1 < out_sz) out[pos++] = '-';

  for (size_t i = 0; ibuf[i] && pos + 1 < out_sz; i++) out[pos++] = ibuf[i];
  if (pos + 1 < out_sz) out[pos++] = '.';
  for (size_t i = 0; fbuf[i] && pos + 1 < out_sz; i++) out[pos++] = fbuf[i];
  out[pos] = '\0';
}

void PRINT(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  for (size_t i = 0; fmt && fmt[i]; i++) {
    if (fmt[i] != '%') {
      write_c(fmt[i]);
      continue;
    }

    i++;
    char spec = fmt[i];
    if (!spec) break;

    if (spec == '%') {
      write_c('%');
    } else if (spec == 's') {
      const char *s = va_arg(ap, const char *);
      if (!s) s = "(null)";
      write_s(s);
    } else if (spec == 'd') {
      int v = va_arg(ap, int);
      char buf[24];
      itoa_dec(v, buf, sizeof(buf));
      write_s(buf);
    } else if (spec == 'f') {
      // en varargs, float se promueve a double
      double dv = va_arg(ap, double);
      char buf[32];
      ftoa_simple((float)dv, buf, sizeof(buf));
      write_s(buf);
    } else {
      // spec desconocido: imprimir tal cual
      write_c('%');
      write_c(spec);
    }
  }

  va_end(ap);
}


int READ(const char *fmt, ...) {
  char line[128];
  os_read_line(line, sizeof(line));

  // saltar espacios iniciales
  const char *p = line;
  while (is_space(*p)) p++;

  va_list ap;
  va_start(ap, fmt);

  // buscar primer %
  char spec = 0;
  for (size_t i = 0; fmt && fmt[i]; i++) {
    if (fmt[i] == '%' && fmt[i + 1]) { spec = fmt[i + 1]; break; }
  }

  int assigned = 0;

  if (spec == 'd') {
    int *out = va_arg(ap, int *);
    if (out) { *out = atoi_dec(p); assigned = 1; }
  } else if (spec == 'f') {
    float *out = va_arg(ap, float *);
    if (out) { *out = atof_simple(p); assigned = 1; }
  } else if (spec == 's') {
    char *out = va_arg(ap, char *);
    if (out) {
      // copiar hasta espacio
      size_t j = 0;
      while (p[j] && !is_space(p[j]) && j < 127) { out[j] = p[j]; j++; }
      out[j] = '\0';
      assigned = 1;
    }
  }

  va_end(ap);
  return assigned;
}
