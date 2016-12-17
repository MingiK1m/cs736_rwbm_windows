# cs736_rwbm_windows
windows r/w operation file system microbenchmark

rw_benchmark.exe [write|read] [block_size] [repeat_count] [log_filename] [on|off(fsync)]

<b>Steps</b>

1) rw_benchmark.exe write 4096 10000 log_wrt off

2) [REBOOT]

3) rw_benchmark.exe read 4096 10000 log_rd on

4) rw_benchmark.exe remove 4096 10000 na on

5) [REBOOT]

6) rw_benchmark.exe write 4096 10000 log_wrt_fsync on

7) rw_benchmark.exe remove 4096 10000 na on
