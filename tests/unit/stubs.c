#include <stdarg.h>
#include <stdbool.h>

bool k_is_user_context(void) { return false; }

void z_log_minimal_printk(const char *fmt, ...) {
    (void)fmt;
}
