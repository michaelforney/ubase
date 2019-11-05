/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "util.h"

struct vm {
	intmax_t cpu_user;
	intmax_t cpu_nice;
	intmax_t cpu_system;
	intmax_t cpu_idle;
	intmax_t cpu_iowait;
	intmax_t cpu_irq;
	intmax_t cpu_softirq;
	intmax_t cpu_steal;
	intmax_t cpu_guest;
	intmax_t cpu_guest_nice;
	intmax_t cpu_unknown;
	intmax_t page_in;
	intmax_t page_out;
	intmax_t swap_in;
	intmax_t swap_out;
	intmax_t intr;
	intmax_t ctxt_switches;
	intmax_t processes;
	intmax_t proc_running;
	intmax_t proc_blocked;
	intmax_t mem_free;
	intmax_t buffers;
	intmax_t cached;
	intmax_t active;
	intmax_t inactive;
	intmax_t swap_total;
	intmax_t swap_free;
	intmax_t sreclaimable;
};

static intmax_t kb_per_page;
static intmax_t hz;

static int
stpstarts(char *str, const char *head, char **end)
{
	size_t n = strlen(head);
	if (!strncmp(str, head, n)) {
		*end = &str[n];
		return 1;
	}
	return 0;
}

static intmax_t
read_ints(const char *s, intmax_t *arr, size_t n)
{
	const char *beginning = s;
	intmax_t rest = 0;
	size_t tmp;
	int negative;

	memset(arr, 0, n * sizeof(*arr));

	for (; n--; arr++) {
		while (*s && !isdigit(*s))
			s++;
		negative = (s != beginning && s[-1] == '-');
		while (isdigit(*s))
			*arr = *arr * 10 + (*s++ - '0');
		if (negative)
			*arr = -*arr;
	}

	for (; *s; rest += tmp) {
		tmp = 0;
		while (*s && !isdigit(*s))
			s++;
		negative = (s != beginning && s[-1] == '-');
		while (isdigit(*s))
			tmp = tmp * 10 + (*s++ - '0');
		if (negative)
			tmp = -tmp;
	}

	return rest;
}

static void
load_vm(struct vm *s)
{
	static intmax_t debt = 0;

	int have_page = 0, have_swap = 0;
	char *line = NULL, *p;
	size_t size = 0;
	ssize_t len;
	FILE *fp;

	memset(s, 0, sizeof(*s));

	fp = fopen("/proc/stat", "r");
	if (!fp)
		eprintf("fopen /proc/stat:");
	while ((len = getline(&line, &size, fp)) >= 0) {
		if (stpstarts(line, "cpu ", &p)) {
			s->cpu_unknown = read_ints(p, &s->cpu_user, 10);
		} else if (stpstarts(line, "page ", &p)) {
			read_ints(p, &s->page_in, 2);
			have_page = 1;
		} else if (stpstarts(line, "swap ", &p)) {
			read_ints(p, &s->swap_in, 2);
			have_swap = 1;
		} else if (stpstarts(line, "intr ", &p)) {
			read_ints(p, &s->intr, 1);
		} else if (stpstarts(line, "ctxt ", &p)) {
			read_ints(p, &s->ctxt_switches, 1);
		} else if (stpstarts(line, "processes ", &p)) {
			read_ints(p, &s->processes, 1);
		} else if (stpstarts(line, "proc_running ", &p)) {
			read_ints(p, &s->proc_running, 1);
		} else if (stpstarts(line, "proc_blocked ", &p)) {
			read_ints(p, &s->proc_blocked, 1);
		}
	}
	if (ferror(fp))
		eprintf("getline /proc/stat:");
	fclose(fp);

	if (!have_page || !have_swap) {
		fp = fopen("/proc/vmstat", "r");
		if (!fp)
			eprintf("fopen /proc/vmstat:");
		while ((len = getline(&line, &size, fp)) >= 0) {
			if (!have_page && stpstarts(line, "pgpgin ", &p))
				read_ints(p, &s->page_in, 1);
			else if (!have_page && stpstarts(line, "pgpgout ", &p))
				read_ints(p, &s->page_out, 1);
			else if (!have_swap && stpstarts(line, "pswpin ", &p))
				read_ints(p, &s->swap_in, 1);
			else if (!have_swap && stpstarts(line, "pswpout ", &p))
				read_ints(p, &s->swap_out, 1);
		}
		if (ferror(fp))
			eprintf("getline /proc/vmstat:");
		fclose(fp);
	}

	fp = fopen("/proc/meminfo", "r");
	if (!fp)
		eprintf("fopen /proc/meminfo:");
	while ((len = getline(&line, &size, fp)) >= 0) {
		if (stpstarts(line, "MemFree:", &p))
			read_ints(p, &s->mem_free, 1);
		else if (stpstarts(line, "Buffers:", &p))
			read_ints(p, &s->buffers, 1);
		else if (stpstarts(line, "Cached:", &p))
			read_ints(p, &s->cached, 1);
		else if (stpstarts(line, "Active:", &p))
			read_ints(p, &s->active, 1);
		else if (stpstarts(line, "Inactive:", &p))
			read_ints(p, &s->inactive, 1);
		else if (stpstarts(line, "SwapTotal:", &p))
			read_ints(p, &s->swap_total, 1);
		else if (stpstarts(line, "SwapFree:", &p))
			read_ints(p, &s->swap_free, 1);
		else if (stpstarts(line, "SReclaimable:", &p))
			read_ints(p, &s->sreclaimable, 1);
	}
	if (ferror(fp))
		eprintf("getline /proc/meminfo:");
	fclose(fp);

	/* yes, this is actually needed */
	s->cpu_idle += debt;
	debt = 0;
	if (s->cpu_idle < 0) {
		debt = s->cpu_idle;
		s->cpu_idle = 0;
	}

	free(line);
}

static void
print_vm(struct vm *s1, struct vm *s0, int active_mem, int timestamp, int print_header)
{
	struct vm s = *s1;
	intmax_t ticks;
	char timezone[21];
	char timestr[21];
	struct tm *tm;
	time_t now;
	size_t n;

	ticks  = s.cpu_user       -= s0->cpu_user;
	ticks += s.cpu_nice       -= s0->cpu_nice;
	ticks += s.cpu_system     -= s0->cpu_system;
	ticks += s.cpu_idle       -= s0->cpu_idle;
	ticks += s.cpu_iowait     -= s0->cpu_iowait;
	ticks += s.cpu_irq        -= s0->cpu_irq;
	ticks += s.cpu_softirq    -= s0->cpu_softirq;
	ticks += s.cpu_steal      -= s0->cpu_steal;
	ticks += s.cpu_guest      -= s0->cpu_guest;
	ticks += s.cpu_guest_nice -= s0->cpu_guest_nice;
	ticks += s.cpu_unknown    -= s0->cpu_unknown;
	s.processes         -= s0->processes;
	s.intr              -= s0->intr;
	s.ctxt_switches     -= s0->ctxt_switches;
	s.page_in           -= s0->page_in;
	s.page_out          -= s0->page_out;

	s.cpu_user  += s.cpu_nice;
	s.cpu_guest += s0->cpu_guest_nice;
	s.cpu_idle  += !ticks;
	ticks       += !ticks;

	if (timestamp) {
		now = time(NULL);
		tm = localtime(&now);
		strftime(timestr, sizeof(timestr), " %Y-%m-%d %H:%M:%S", tm);
		strftime(timezone, sizeof(timezone), "%Z", tm);
		n = strlen(timezone) + 1;
		memmove(&timezone[sizeof(timezone) - n], timezone, n);
		memset(timezone, ' ', sizeof(timezone) - n);
	}

#define PERCENT(X) ((X) * 100 + ticks / 2) / ticks
#define HERTZ(X)   ((X) * hz  + ticks / 2) / ticks

	if (!print_header)
		goto print;
	printf("----procs---- -------------------memory------------------ ---swap-- -----io---- --system- --------------cpu--------------%s\n",
	       timestamp ? " -----timestamp-----" : "");
	printf(" r  b      fk       swpd       free      " "%s     "  "%s   si   so    bi    bo   in   cs  us  sy  id  wa  in  si  st  gt%s\n",
	       active_mem ? "inact" :  " buff", active_mem ? "active" : " cache", timestamp ? timezone : "");
print:
	printf("%2ji %2ji %7ji %10ji "   "%10ji "    "%10ji "  "%10ji %4ji %4ji %5ji %5ji ""%4ji %4ji %3ji %3ji %3ji %3ji %3ji %3ji %3ji %3ji%s\n",
	       s.proc_running, s.proc_blocked, s.processes,
	       s.swap_total - s.swap_free, s.mem_free, active_mem ? s.inactive : s.buffers, active_mem ? s.active : s.cached  + s.sreclaimable,
	       HERTZ(s.swap_in * kb_per_page), HERTZ(s.swap_out * kb_per_page),
	       HERTZ(s.page_in), HERTZ(s.page_out),
	       HERTZ(s.intr), HERTZ(s.ctxt_switches),
	       PERCENT(s.cpu_user), PERCENT(s.cpu_system), PERCENT(s.cpu_idle),
	       PERCENT(s.cpu_iowait), PERCENT(s.cpu_irq), PERCENT(s.cpu_softirq),
	       PERCENT(s.cpu_steal), PERCENT(s.cpu_guest),
	       timestamp ? timestr : "");

#undef PERCENT
#undef HERTZ
}

static void
usage(void)
{
	eprintf("usage: %s [-ant] [delay [count]]\n", argv0);
}

int
main(int argc, char *argv[])
{
	static struct vm vm[2];
	static struct timespec delay;
	char *end;
	double tmp;
	unsigned long long int count = 0, i = 0;
	int one_header = 0;
	int active_mem = 0;
	int timestamp = 0;

	ARGBEGIN {
	case 'a':
		active_mem = 1;
		break;
	case 'n':
		one_header = 1;
		break;
	case 't':
		timestamp = 1;
		break;
	case 'w':
		/* Ignored for compatibility (allow output to be wider than 80 columns) */
		break;
	default:
		usage();
	} ARGEND;

	if (argc > 2)
		usage();

	kb_per_page = (intmax_t)(sysconf(_SC_PAGESIZE) / 1024);
	hz = (intmax_t)sysconf(_SC_CLK_TCK);

	if (argc) {
		errno = 0;
		tmp = strtod(argv[0], &end);
		if (errno || *end || tmp <= 0)
			eprintf("%s: not a valid positive number\n", argv[0]);
		delay.tv_sec = (time_t)tmp;
		tmp = (tmp - (double)delay.tv_sec) * 1000000000.;
		delay.tv_nsec = (long int)tmp;
		if (delay.tv_nsec > 999999999L)
			delay.tv_nsec = 999999999L;

		if (argc > 1)
			count = (unsigned long long int)atoll(argv[1]);
	}

	for (;;) {
		load_vm(&vm[i & 1]);
		print_vm(&vm[i & 1], &vm[~i & 1], active_mem, timestamp, one_header ? !i : (i % 50 == 0));
		i++;
		if (!argc || (argc == 2 && i == count))
			break;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
	}

	return 0;
}
