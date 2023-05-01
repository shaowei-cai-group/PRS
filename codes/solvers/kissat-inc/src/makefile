all:
	$(MAKE) -C "/home/chenzh/benchmark/worker/sota/kissat-MAB/build"
kissat:
	$(MAKE) -C "/home/chenzh/benchmark/worker/sota/kissat-MAB/build" kissat
tissat:
	$(MAKE) -C "/home/chenzh/benchmark/worker/sota/kissat-MAB/build" tissat
clean:
	rm -f "/home/chenzh/benchmark/worker/sota/kissat-MAB"/makefile
	-$(MAKE) -C "/home/chenzh/benchmark/worker/sota/kissat-MAB/build" clean
	rm -rf "/home/chenzh/benchmark/worker/sota/kissat-MAB/build"
coverage:
	$(MAKE) -C "/home/chenzh/benchmark/worker/sota/kissat-MAB/build" coverage
indent:
	$(MAKE) -C "/home/chenzh/benchmark/worker/sota/kissat-MAB/build" indent
test:
	$(MAKE) -C "/home/chenzh/benchmark/worker/sota/kissat-MAB/build" test
.PHONY: all clean coverage indent kissat test tissat
