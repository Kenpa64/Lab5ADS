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
	vsync:	out std_logic;
	hsync:	out std_logic;
	red:	out std_logic_vector(3 downto 0);
	green:	out std_logic_vector(3 downto 0);
	blue:	out std_logic_vector(3 downto 0);
	addr_out: out std_logic_vector(10 downto 0)
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
	signal count_1688, count_1688_next: std_logic_vector(10 downto 0);
	signal count_1066, count_1066_next: std_logic_vector(10 downto 0);
    

	-- control variables
	signal h_end, v_end, h_screen, v_screen: std_logic;
    
    -- output register sync signals
	signal vsync_reg: std_logic;
	signal hsync_reg: std_logic;
    
	signal output_colour: std_logic_vector(11 downto 0);

	signal trigger_level_reg, trigger_level_reg2 : std_logic_vector(8 downto 0);
	signal data_to_vga: std_logic_vector(11 downto 0);

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

                --Sync colours
                red <= output_colour(11 downto 8);
                green <= output_colour(7 downto 4);
                blue <= output_colour(3 downto 0);

                --data_out_reg <= data_out;
                --data_out_reg2 <= data_out_reg;

                trigger_level_reg <= trigger_level;
                trigger_level_reg2 <=trigger_level_reg;
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
				addr_out <= (others => '0');
			else
				if(v_screen = '1') then
					if(('0'&counter1066) - VBP = trigger_level_reg2 and counter1688 - HBP <= 20) then
						if(counter1688 >= HBP) then
							output_colour(3 downto 0) <= (others => '1');
						else 
							output_colour <= (others => '0');
						end if;
						-- 3 because 1 clock in advance + 2 reg
					--elsif(count_1688 > HBP - 3 and count_1688 < HBP - 1) then
					--	addr_out <= count_1688-HBP-3; 
					--	output_colour <= (others => '0');
					elsif(count_1688 >= HBP) then
						if(count_1688 < 1280) then
							addr_out <= counter1688 - HBP + 1;
						end if;
						if(count_1688 > HBP + 1280) then 
							data_to_vga <= (others <= '0')
						elsif(count_1688 > HBP + 2) then
							data_to_vga <= data_out;
						end if;
						if(data_to_vga(11 downto 3) = counter1066(9 downto 0) - VBP) then
							output_colour <= "111100001111";
						else
							output_colour <= (others => '0');
					    end if;
					--elsif(count_1688 = HBP - 1) then
					--	data_to_vga <= data_out;
					--	addr_out <= count_1688-HBP-3; 
					--elsif(count_1688 > HBP-3) then
					--	data_to_vga <= data_out;
					--	if(data_to_vga = (('0'&counter1066) - VBP) then
					--		output_colour <= "111100001111";
					--	else 
					--		output_colour <= (others => '0');
					--  end if;

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
	
    -- set these internal signals to '1' when a line or a frame is ended
	h_end <= '1' when count_1688 = (PPL + HFP + HBP + HRE - 1) else '0';
	v_end <= '1' when count_1066 = (LIN + VFP + VBP + VRE - 1) else '0';
	
    -- set internal flags to '1' when the counters are inside the active area
	h_screen <= '1' when (count_1688 > HBP and count_1688 <= HBP+1280) else '0';
	v_screen <= '1' when (count_1066 > VBP and count_1066 <= VBP+1024) else '0';
end arch;