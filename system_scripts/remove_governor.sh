#!/bin/bash
echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
rmmod freq_change_test
rmmod LAMbS_governor 

