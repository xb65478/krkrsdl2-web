// Runtime log verbosity filter implementation. See LogFilter.h.
#include "LogFilter.h"
#include <string.h>
#include <stdio.h>

int g_krkr_log_level = 1;
int g_krkr_perf_enabled = 0;

void TVPLogFilterInitFromArgs(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i) {
        if (!argv[i]) continue;
        const char *a = argv[i];
        if (strncmp(a, "-krkr-loglevel=", 15) == 0) {
            int v = a[15] - '0';
            if (v >= 1 && v <= 3) {
                g_krkr_log_level = v;
                fprintf(stderr, "[LOG] verbosity level set to L%d\n", v);
            }
            continue;
        }
        if (strcmp(a, "-krkr-perf") == 0 || strcmp(a, "-krkr-perf=1") == 0 ||
            strcmp(a, "-krkr_perf") == 0 || strcmp(a, "-krkr_perf=1") == 0) {
            g_krkr_perf_enabled = 1;
            fprintf(stderr, "[PERF] enabled=1 source=argv\n");
            continue;
        }
    }
}
