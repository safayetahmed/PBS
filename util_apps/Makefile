#define the build commands
CC=gcc
CFLAGS=-Wall
LIBFLAGS=

BINDIR=../bin
SRCDIR=./

all: $(BINDIR)/poll_pbs_actv $(BINDIR)/cpufreq_step $(BINDIR)/cpufreq_oscillate MOCsample_apps

$(BINDIR)/poll_pbs_actv: $(SRCDIR)/poll_pbs_actv.c $(SRCDIR)/pbspoll_helper.h $(SRCDIR)/pbspoll_helper.c
	$(CC) $(CFLAGS) -o $(BINDIR)/poll_pbs_actv $(SRCDIR)/poll_pbs_actv.c $(SRCDIR)/pbspoll_helper.c -lrt

$(BINDIR)/cpufreq_step: $(SRCDIR)/cpufreq_step.c $(SRCDIR)/cpufreq_helper.h $(SRCDIR)/cpufreq_helper.c
	$(CC) $(CFLAGS) -o $(BINDIR)/cpufreq_step $(SRCDIR)/cpufreq_step.c $(SRCDIR)/cpufreq_helper.c -lm -lrt

$(BINDIR)/cpufreq_oscillate: $(SRCDIR)/cpufreq_oscillate.c $(SRCDIR)/cpufreq_helper.h $(SRCDIR)/cpufreq_helper.c
	$(CC) $(CFLAGS) -o $(BINDIR)/cpufreq_oscillate $(SRCDIR)/cpufreq_oscillate.c $(SRCDIR)/cpufreq_helper.c -lm -lrt

timing_apps:
	$(MAKE) -C timing_src/

MOCsample_apps:
	$(MAKE) -C MOCsample_src/

clean:
	$(MAKE) -C timing_src/ clean
	$(MAKE) -C MOCsample_src/ clean
	rm -rf $(BINDIR)/poll_pbs_actv $(BINDIR)/cpufreq_step $(BINDIR)/cpufreq_oscillate

