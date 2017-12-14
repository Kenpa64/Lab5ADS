library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity vga_control is
port(
	mode:	in std_logic;
	clk:	in std_logic;
	reset:	in std_logic;
	vsync:	out std_logic;
	hsync:	out std_logic;
	red:	out std_logic_vector(3 downto 0);
	green:	out std_logic_vector(3 downto 0);
	blue:	out std_logic_vector(3 downto 0)
	);
end vga_control;


architecture arch of vga_control is
	constant PPL: integer:= 1280;	-- pixels per line
	constant HFP: integer:= 48;		-- hsync front porch
	constant HBP: integer:= 248;	-- hsync back porch
	constant HRE: integer:= 112;	-- hsync retrace
	constant LIN: integer:= 1024;	-- vertical lines
	constant VFP: integer:= 1;		-- vsync front porch
	constant VBP: integer:= 38;		-- vsync vakc porch
	constant VRE: integer:= 3;		-- vsync retrace

	-- counter variables
	signal count_1688, count_1688_next: unsigned(10 downto 0);
	signal count_1066, count_1066_next: unsigned(10 downto 0);
    
    -- pixel counter
	signal count_col, count_col_next: std_logic_vector(11 downto 0);

	-- control variables
	signal h_end, v_end, h_screen, v_screen: std_logic;
    
    -- output register sync signals
	signal vsync_reg, vsync_reg2: std_logic;
	signal hsync_reg, hsync_reg2: std_logic;
    
	signal output_colour: std_logic_vector(11 downto 0);

	begin
    
    -- register to output sync
	process (clk, reset)
		begin
        if(clk'event and clk = '1') then
            if (reset = '1') then	
                -- All the signals are reseted in the processes below
            else
                -- Sync output control signals
                vsync_reg2 <= vsync_reg;
                vsync <= vsync_reg2;
                hsync_reg2 <= hsync_reg;
                hsync <= hsync_reg2;

                --Sync colours
                red <= output_colour(11 downto 8);
                green <= output_colour(7 downto 4);
                blue <= output_colour(3 downto 0);
            end if;
        end if;
		
	end process;
	
	-- colour counter process, clock time
	countercol: process (clk, reset)
		begin
        if (clk'event and clk = '1') then
            if (reset = '1' or h_end =	 '1') then
                count_col_next <= (others => '0');
            else
                if(h_screen = '1' and v_screen = '1') then
                    count_col_next <= count_col + 1;
                end if;
            end if;
        end if;
		
	end process;
    
	-- 1688 counter, clock times for horizontal pixels
	counter1688: process (clk, reset)
		begin
        if (clk'event and clk = '1') then
            if (reset = '1') then
                count_1688_next <= (others => '0');
            else
                -- it resets when the line ends, if not it increases in 1
                if (h_end = '1') then
                    count_1688_next <= (others => '0');
                else
                    count_1688_next <= count_1688 + 1;
                end if;
            end if;
        end if;
	end process;

    -- 1066 counter, 1688 counter for lines
	counter1066: process (clk, reset)
		begin
        if (clk'event and clk = '1') then
            if (reset = '1') then
                count_1066_next <= (others => '0');
            else
                -- it resets when the frame ends, if not it increases in 1
                if (v_end = '1') then
                    count_1066_next <= (others => '0');
                elsif (h_end = '1') then
                    count_1066_next <= count_1066 + 1;
                end if;
            end if;
        end if;
	end process;
    
    -- VGA pattern and mode selector process
	signalgen: process (clk, reset)
		begin
        if(clk'event and clk = '1') then
			if (reset = '1' or v_end = '1')  then 
                -- force the output colour to all '0' when the frame ends or the system resets
				output_colour <= (others => '0');
			else
                -- "vertical" mode
				if (mode = '0') then
                    -- when the counters are inside the active area, the output must be the colour counter, if not is forced to zero
					if (h_screen = '1' and v_screen = '1') then
						output_colour <= '0' & count_col(10 downto 0);
					else
						output_colour <= (others => '0');
					end if;
                -- "horizontal" mode
				elsif (mode = '1') then
                    -- when the counters are outside the active area, the output must be forced to zero
					if (h_screen = '0' or v_screen = '0') then
						output_colour <= (others => '0');
                    -- if the line counter is inside the first 1/3 of the active area, the output will be forced to blue
					elsif (VBP+342 > count_1066) then
						output_colour <= "000000001111";
                    -- if the line counter is between the second 1/3 of the active area, the output will be forced to green
					elsif (VBP+682 > count_1066) then
						output_colour <= "000011110000";
                    -- if the line counter is between the last 1/3 of active area, the output will be forced to red
					elsif (VBP+1024 > count_1066) then
						output_colour <= "111100000000";
					else
						output_colour <= (others => '0');
					end if;
				end if;
			end if;		
        end if;
	end process;

    -- set hsync and vsync signals to '1' when any counter is on retrace
	hsync_reg <= '1' when count_1688 < (PPL+HFP+HBP) else '0';
	vsync_reg <= '1' when count_1066 < (LIN+VFP+VBP) else '0';
	
    -- set the counter values to the actual ones
	count_1688 <= count_1688_next;
	count_1066 <= count_1066_next;
	count_col <= count_col_next;
	
    -- set these internal signals to '1' when a line or a frame is ended
	h_end <= '1' when count_1688 = (PPL + HFP + HBP + HRE - 1) else '0';
	v_end <= '1' when count_1066 = (LIN + VFP + VBP + VRE - 1) else '0';
	
    -- set internal flags to '1' when the counters are inside the active area
	h_screen <= '1' when (count_1688 > HBP and count_1688 <= HBP+1280) else '0';
	v_screen <= '1' when (count_1066 > VBP and count_1066 <= VBP+1024) else '0';

end arch;