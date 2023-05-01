all:
	$(MAKE) -C "/home/chenzh/solvers/Light/solvers/kissat-inc/build"
kissat:
	$(MAKE) -C "/home/chenzh/solvers/Light/solvers/kissat-inc/build" kissat
tissat:
	$(MAKE) -C "/home/chenzh/solvers/Light/solvers/kissat-inc/build" tissat
clean:
	rm -f "/home/chenzh/solvers/Light/solvers/kissat-inc"/makefile
	-$(MAKE) -C "/home/chenzh/solvers/Light/solvers/kissat-inc/build" clean
	rm -rf "/home/chenzh/solvers/Light/solvers/kissat-inc/build"
coverage:
	$(MAKE) -C "/home/chenzh/solvers/Light/solvers/kissat-inc/build" coverage
indent:
	$(MAKE) -C "/home/chenzh/solvers/Light/solvers/kissat-inc/build" indent
test:
	$(MAKE) -C "/home/chenzh/solvers/Light/solvers/kissat-inc/build" test
.PHONY: all clean coverage indent kissat test tissat
