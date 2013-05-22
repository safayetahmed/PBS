all:
	$(MAKE) -C pbs_module/
	$(MAKE) -C allocator/
	$(MAKE) -C sqrwavSRT/
	$(MAKE) -C poll_pbs_actv/

setup_expt: all
	sh system_scripts/moderate_enable.sh
	sh system_scripts/rt_limit_disable.sh
	insmod pbs_module/pbs_module.ko
	
clean:
	$(MAKE) -C poll_pbs_actv/ clean
	$(MAKE) -C sqrwavSRT/ clean
	$(MAKE) -C allocator/ clean
	$(MAKE) -C pbs_module/ clean
