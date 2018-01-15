set_property PACKAGE_PIN AB11 [get_ports {ncs}];		# "JA7"
set_property PACKAGE_PIN AB10 [get_ports {sdata1}];		# "JA8"
set_property PACKAGE_PIN AA8  [get_ports {sclk}];		# "JA10"
set_property PACKAGE_PIN Y21  [get_ports {blue[0]}];  	# "VGA-B1"
set_property PACKAGE_PIN Y20  [get_ports {blue[1]}];  	# "VGA-B2"
set_property PACKAGE_PIN AB20 [get_ports {blue[2]}];  	# "VGA-B3"
set_property PACKAGE_PIN AB19 [get_ports {blue[3]}];  	# "VGA-B4"
set_property PACKAGE_PIN AB22 [get_ports {green[0]}];  	# "VGA-G1"
set_property PACKAGE_PIN AA22 [get_ports {green[1]}];  	# "VGA-G2"
set_property PACKAGE_PIN AB21 [get_ports {green[2]}];  	# "VGA-G3"
set_property PACKAGE_PIN AA21 [get_ports {green[3]}];  	# "VGA-G4"
set_property PACKAGE_PIN AA19 [get_ports {hsync}];  	# "VGA-HS"
set_property PACKAGE_PIN V20  [get_ports {red[0]}];  	# "VGA-R1"
set_property PACKAGE_PIN U20  [get_ports {red[1]}];  	# "VGA-R2"
set_property PACKAGE_PIN V19  [get_ports {red[2]}];  	# "VGA-R3"
set_property PACKAGE_PIN V18  [get_ports {red[3]}];  	# "VGA-R4"
set_property PACKAGE_PIN Y19  [get_ports {vsync}];  	# "VGA-VS"
set_property PACKAGE_PIN P16 [get_ports {trigger_n_p}]; # "BTNC"
set_property PACKAGE_PIN R16 [get_ports {trigger_down}];# "BTND"
set_property PACKAGE_PIN N15 [get_ports {temp_down}];  	# "BTNL"
set_property PACKAGE_PIN R18 [get_ports {temp_up}];  	# "BTNR"
set_property PACKAGE_PIN T18 [get_ports {trigger_up}];  # "BTNU"
set_property PACKAGE_PIN F22 [get_ports {sw0}];  # "SW0"

# ----------------------------------------------------------------------------
# IOSTANDARD Constraints
#
# Note that these IOSTANDARD constraints are applied to all IOs currently
# assigned within an I/O bank.  If these IOSTANDARD constraints are 
# evaluated prior to other PACKAGE_PIN constraints being applied, then 
# the IOSTANDARD specified will likely not be applied properly to those 
# pins.  Therefore, bank wide IOSTANDARD constraints should be placed 
# within the XDC file in a location that is evaluated AFTER all 
# PACKAGE_PIN constraints within the target bank have been evaluated.
#
# Un-comment one or more of the following IOSTANDARD constraints according to
# the bank pin assignments that are required within a design.
# ---------------------------------------------------------------------------- 

# Note that the bank voltage for IO Bank 33 is fixed to 3.3V on ZedBoard. 
set_property IOSTANDARD LVCMOS33 [get_ports -of_objects [get_iobanks 33]];

# Set the bank voltage for IO Bank 34 to 1.8V by default.
# set_property IOSTANDARD LVCMOS33 [get_ports -of_objects [get_iobanks 34]];
# set_property IOSTANDARD LVCMOS25 [get_ports -of_objects [get_iobanks 34]];
set_property IOSTANDARD LVCMOS18 [get_ports -of_objects [get_iobanks 34]];

# Set the bank voltage for IO Bank 35 to 1.8V by default.
# set_property IOSTANDARD LVCMOS33 [get_ports -of_objects [get_iobanks 35]];
# set_property IOSTANDARD LVCMOS25 [get_ports -of_objects [get_iobanks 35]];
set_property IOSTANDARD LVCMOS18 [get_ports -of_objects [get_iobanks 35]];

# Note that the bank voltage for IO Bank 13 is fixed to 3.3V on ZedBoard. 
set_property IOSTANDARD LVCMOS33 [get_ports -of_objects [get_iobanks 13]];