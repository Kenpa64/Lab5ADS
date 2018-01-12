library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity vga_control is
port(
	data_out: in std_logic_vector(11 downto 0);
	trigger_level: in std_logic_vector(8 downto 0);
	clk:	in std_logic;
	reset:	in std_logic;
	gpio_in: in std_logic_vector(22 downto 0);
	sw0: in std_logic;
	vsync:	out std_logic;
	hsync:	out std_logic;
	red:	out std_logic_vector(3 downto 0);
	green:	out std_logic_vector(3 downto 0);
	blue:	out std_logic_vector(3 downto 0);
	addr_out: out std_logic_vector(11 downto 0)
	);
end vga_control;


architecture arch of vga_control is
	constant PPL: integer:= 1280;	-- pixels per line
	constant HFP: integer:= 48;		-- hsync front porch
	constant HBP: integer:= 248;	-- hsync back porch
	constant HRE: integer:= 112;	-- hsync retrace
	constant LIN: integer:= 1024;	-- vertical lines
	constant VFP: integer:= 1;		-- vsync front porch
	constant VBP: integer:= 38;		-- vsync back porch
	constant VRE: integer:= 3;		-- vsync retrace
	constant offset: integer:= 256; -- center the display

	-- counter variables
	signal count_1688, count_1688_next: std_logic_vector(11 downto 0);
	signal count_1066, count_1066_next: std_logic_vector(10 downto 0);
    signal h_count: std_logic_vector(11 downto 0);
    signal v_count: std_logic_vector(10 downto 0);

	-- control variables
	signal h_end, v_end, h_screen, v_screen: std_logic;
    
    -- output register sync signals
	signal vsync_reg: std_logic;
	signal hsync_reg: std_logic;

	signal sw0_reg, sw0_reg1: std_logic;
	signal h_add: std_logic_vector(11 downto 0);
    
	signal output_colour: std_logic_vector(11 downto 0);

	signal data_to_vga: std_logic_vector(11 downto 0);
	
	signal t_temperature: std_logic_vector(10 downto 0);
	signal temperature: std_logic_vector(10 downto 0);
	signal alarm: std_logic;

	begin
    
    -- register to output sync
	process (clk, reset)
		begin
        if(clk'event and clk = '1') then
            if (reset = '1') then	
                -- All the signals are reseted in the processes below
            else
    			vsync <= vsync_reg;
    			hsync <= hsync_reg;

    			sw0_reg <= sw0;
    			sw0_reg1 <= sw0_reg;

                --Sync colours
                red <= output_colour(11 downto 8);
                green <= output_colour(7 downto 4);
                blue <= output_colour(3 downto 0);

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
			--if (reset = '1' or v_end = '1')  then 
			if (reset = '1' or v_end = '1')  then 
                -- force the output colour to all '0' when the frame ends or the system resets
				output_colour <= (others => '0');
				addr_out <= (others => '0');
				data_to_vga <= (others => '0');
			else
				if(sw0_reg1 = '0') then
					h_add <= 0;
				else
					h_add <= h_count + 2;
				end if;
				--if(h_count < 1279 and h_count >= -1 and v_count < 512) then
				if(h_count < 1279 and h_count >= -1 and v_count < 512 and v_count >=0) then
						addr_out <= h_count + 1 + h_add;
						data_to_vga <= data_out;
				end if;
				if(v_screen = '1' and h_screen = '1') then
					-- set output to '0' below the temperature display
					-- if(v_count(8 downto 0) = trigger_level and h_count < 20 and v_count < 512) then
					if(v_count(8 downto 0) = trigger_level and h_count < 20 and v_count < 512 and v_count >=0) then
						output_colour <= "000000001111";
					else
						--if(data_out(11 downto 3) = v_count(8 downto 0) and alarm = '0' and v_count < 512) then
						if(data_out(11 downto 3) = v_count(8 downto 0) and alarm = '0' and v_count < 512 and v_count >= 0) then
							output_colour <= "111111110000";
						else
							if(count_1066 >= (538 + VBP + offset) and count_1066 <= (573 + VBP + offset)) then
								if(h_count = t_temperature) then
									output_colour <= "000000001111" ;
								elsif((count_1066 >= (541 + VBP + offset)) and (count_1066 <= (571 + VBP + offset)) ) then 
									if(h_count <= temperature) then
										if(alarm = '0') then
											output_colour <= "000011110000";
										else
											output_colour <= "111100000000";
										end if;
									else
										output_colour <= (others => '0');
									end if;
								else
									output_colour <= (others => '0');
								end if;
							else
								output_colour <= (others => '0');
							end if;
					    end if;
					end if;
				else
					output_colour <= (others => '0');
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
	
    -- set these internal signals to '1' when a line or a frame is ended
	h_end <= '1' when count_1688 = (PPL + HFP + HBP + HRE - 1) else '0';
	v_end <= '1' when count_1066 = (LIN + VFP + VBP + VRE - 1) else '0';
	
    -- set internal flags to '1' when the counters are inside the active area
	h_screen <= '1' when (count_1688 > HBP and count_1688 <= HBP+1280) else '0';
	v_screen <= '1' when (count_1066 > VBP and count_1066 <= VBP+1024) else '0';

	--Substraction count to get a value refered to start of display, can be negative
	--v_count <= count_1066 - VBP;
	v_count <= count_1066 - VBP - offset;
	h_count <= count_1688 - HBP;
	
	-- Separate the GPIO signal in alarm, temperature and t_temperature
	alarm <= gpio_in(22);
	temperature <= gpio_in(21 downto 11);
	t_temperature <= gpio_in(10 downto 0);
end arch;