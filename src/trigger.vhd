library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity trigger is
port(
	-- vsync in? the system will read only when retrace, vsync = '0'
	vsync: in std_logic;
	sample_ready:	in std_logic; -- I think is ncs, thus, the system will read the ADC when sample_ready = '1'
	clk:	in std_logic;
	reset:	in std_logic; -- active low
	trigger_up:	in std_logic;
	trigger_down:	in std_logic;
	trigger_n_p:	in std_logic;
	data1: in std_logic_vector(11 downto 0);
	we:	out std_logic;
	addr_in:	out std_logic_vector(10 downto 0);
	data_in:	out std_logic_vector(11 downto 0);
	trigger_level:	out std_logic_vector(8 downto 0)

	);
end trigger;

architecture arch of trigger is
	--constant default_value: integer:= 256; --100000000, I don't know if we could not use this signal
	signal actual_trigger: std_logic_vector(8 downto 0):= "100000000";
	signal last_data1: std_logic_vector(3 downto 0);
	constant trigger_unit: integer:= 16;
	signal trigger_slope, we: std_logic;
	--registers init
	signal trigger_up_sync, trigger_up_sync2, trigger_up_sync3: std_logic;
	signal trigger_down_sync, trigger_down_sync2, trigger_down_sync3: std_logic;
	signal trigger_n_p_sync, trigger_n_p_sync2, trigger_n_p_sync3: std_logic;
	signal trigger_level_reg: std_logic_vector(8 downto 0);

	signal data1_value : std_logic_vector(11 downto 0);
	signal count_1280, count_1280_next: unsigned(10 downto 0);
	signal ongoing: std_logic;
	
	comparison: process(clk, reset)
	begin
		
	end process;
	
	sync: process(clk, reset)
	begin
		if(clk'event and clk = '1') then
			if(reset = '0') then
				-- reset registers, supose active low
				trigger_up_sync <= '1';
				trigger_down_sync <= '1';
				trigger_n_p_sync <= '1';
				trigger_level_reg <= (others => '0');
			else
				trigger_up_sync <= trigger_up;
				trigger_up_sync2 <= trigger_up_sync;
				trigger_up_sync3 <= trigger_up_sync2;
				trigger_down_sync <= trigger_down;
				trigger_down_sync2 <= trigger_down_sync;
				trigger_down_sync3 <= trigger_down_sync2;
				trigger_n_p_sync <= trigger_n_p;
				trigger_n_p_sync2 <= trigger_n_p_sync;
				trigger_n_p_sync3 <= trigger_n_p_sync2;
				
				trigger_level_reg <= actual_trigger;
				trigger_level <= trigger_level_reg;
			end if;
		end if;
	end process;
	
	
	trigger_movement: process(clk, reset)
	begin
		if(clk'event and clk='1') then
			if(reset = '0') then
				actual_trigger = "100000000";
				-- 0 negative, 1 positive
				trigger_slope <= '1';
			else
				-- falling edge detection
				if(trigger_up_sync3 = '1' and trigger_up_sync2 = '0') then
					actual_trigger = actual_trigger + trigger_unit;
				elsif (trigger_down_sync3 = '1' and trigger_down_sync2 = '0') then
					actual_trigger = actual_trigger - trigger_unit;
				end if;
				
				if(trigger_n_p_sync3 = '1' and trigger_n_p_sync2 = '0') then
					trigger_slope <= not(trigger_slope);
				end if;
			end if;
		end if;
	end process;

	-- Conversion var status
	ongoing_process: process(clk, reset)
		begin
		if (clk'event and clk = '1') then
			if (reset = '0') then
                ongoing <= '0';
            else
            	if (vsync = '0' and vsync_reg = '1') then
            		ongoing <= '1'
            	elsif (counter1280 = 1280) then
            		ongoing <= '0'
            	end if;
            end if;
        end if;
	end process;

	-- 1280 counter, conversion time for memory locations¡
	counter1280: process (clk, reset)
		begin
        if (clk'event and clk = '1') then
            if (reset = '0' or count_1280 = 1280) then
                count_1280_next <= (others => '0');
            else
                -- it resets when the line ends, if not it increases in 1
                if (sample_ready = '1' and process_read = '1') then
                    count_1280_next <= (others => '0');
                else
                    count_1280_next <= count_1280 + 1;
                end if;
            end if;
        end if;
	end process;
	
	-- Comparison process trigger decision
	comparison: process(clk, reset)
	begin
		if(clk'event and clk = '1') then
			if(reset = '0') then
				last_data1 <= (others => '0');
				process_read <= '0';
			else
				if(sample_ready = '1' and process_read = '1' and ongoing = '1') then
					if(trigger_slope = '0') then
						if((data1(11 downto 3) = actual_trigger) and (last_data1 > data1(11 downto 8))) then
							data1_value <= data1;
							process_read <= '1';
						end if;
					elsif(trigger_slope = '1') then
						if((data1(11 downto 3) = actual_trigger) and (last_data1 < data1(11 downto 8))) then
							data1_value <= data1;
							process_read <= '1';
						end if;
					elsif(process_read = '1') then
						data1_value <= data1;
						if (count_1280 = 1280) then
							process_read <= '0';
						end if;
					end if;
					last_data1 <= data1(11 downto 8);
				end if;
			end if;
		end if;
	end process;

	-- Write to memory process
	writeToMemory: process(clk, reset)
	begin
		if(clk'event and clk = '1') then
			if(reset = '0') then
<<<<<<< HEAD
				we <= '0';
				addr_in <= (others => '0');
				data_in <= (others => '0');
			else
				if(sample_ready = '1' and process_read = '1') then
					we <= '1';
					addr_in <= counter1280-1;
					data_in <= data1_value;
				end if;
			end if;
=======

			end if;
			else
>>>>>>> master
				
		end if;
	end process;

	count_1280 <= count_1280_next;
	
end arch;