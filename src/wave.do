add wave sim:/portmap/*
force -freeze sim:/portmap/clk 1 0, 0 {5 ns} -r 10
force -freeze sim:/portmap/resetn 0 0
force -freeze sim:/portmap/sdata1 1 0
force -freeze sim:/portmap/sw0 0 0
force -freeze sim:/portmap/temp_up 0 0
force -freeze sim:/portmap/temp_down 0 0
force -freeze sim:/portmap/trigger_up 0 0
force -freeze sim:/portmap/trigger_down 0 0
force -freeze sim:/portmap/trigger_n_p 0 0
run
force -freeze sim:/portmap/resetn 1 0
run
