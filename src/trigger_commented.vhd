library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity trigger is
port(
	vsync: in std_logic;
	sample_ready:	in std_logic; 
	clk:	in std_logic;
	reset:	in std_logic; 
	trigger_up:	in std_logic;
	trigger_down:	in std_logic;
	trigger_n_p:	in std_logic;
	data1: in std_logic_vector(11 downto 0);
	we:	out std_logic;
	addr_in:	out std_logic_vector(11 downto 0);
	data_in:	out std_logic_vector(11 downto 0);
	trigger_level:	out std_logic_vector(8 downto 0)
	);
end trigger;

architecture arch of trigger is

	-- trigger signals including their registers
	-- trigger value with default
	signal actual_trigger: std_logic_vector(8 downto 0):= "100000000";
	-- last data vector for slope comparison
	signal last_data1: std_logic_vector(3 downto 0);
	signal trigger_unit: std_logic_vector(4 downto 0):= "10000";
	signal trigger_slope: std_logic;
	signal trigger_up_sync, trigger_up_sync2, trigger_up_sync3: std_logic;
	signal trigger_down_sync, trigger_down_sync2, trigger_down_sync3: std_logic;
	signal trigger_n_p_sync, trigger_n_p_sync2, trigger_n_p_sync3: std_logic;
	signal trigger_level_reg: std_logic_vector(8 downto 0);
	
	-- data sending vector
	signal data1_value: std_logic_vector(11 downto 0);
	-- address counter
	signal count_2560, count_2560_next: std_logic_vector(11 downto 0);
	signal vsync_reg: std_logic;
	-- memory writing flags
	signal ongoing,process_read: std_logic;

	-- sample ready register and sample flag in order to know write the next clock after an adquisition
	signal sample_flag, sample_ready_reg: std_logic;

	-- flag active when the data adquisition has finished for the current frame
	signal data_end : std_logic;

	begin 
	sync: process(clk, reset)
	begin
		if(clk'event and clk = '1') then
			if(reset = '0') then
				-- reset registers, active high
				trigger_up_sync <= '0';
				trigger_down_sync <= '0';
				trigger_n_p_sync <= '0';
				trigger_level_reg <= (others => '0');
				data_end <= '0';
			else
				-- registering
				trigger_up_sync <= trigger_up;
				trigger_up_sync2 <= trigger_up_sync;
				trigger_up_sync3 <= trigger_up_sync2;
				trigger_down_sync <= trigger_down;
				trigger_down_sync2 <= trigger_down_sync;
				trigger_down_sync3 <= trigger_down_sync2;
				trigger_n_p_sync <= trigger_n_p;
				trigger_n_p_sync2 <= trigger_n_p_sync;
				trigger_n_p_sync3 <= trigger_n_p_sync2;
				
				trigger_level <= actual_trigger;
				
				sample_ready_reg <= sample_ready;

				-- Edge detection
				vsync_reg <= vsync;
			end if;
		end if;
	end process;
	
	
	trigger_movement: process(clk, reset)
	begin
		if(clk'event and clk='1') then
			if(reset = '0') then
				actual_trigger <= "100000000";
				-- 0 negative, 1 positive
				trigger_slope <= '1';
			else
				-- falling edge detection of the buttons
				if(trigger_up_sync3 = '0' and trigger_up_sync2 = '1' and actual_trigger > trigger_unit) then
					actual_trigger <= actual_trigger - trigger_unit;
				end if;
				if (trigger_down_sync3 = '0' and trigger_down_sync2 = '1' and actual_trigger < 496) then
					actual_trigger <= actual_trigger + trigger_unit;
				end if;
				-- falling edge detection for changing the trigger slope
				if(trigger_n_p_sync3 = '0' and trigger_n_p_sync2 = '1') then
					trigger_slope <= not(trigger_slope);
				end if;
			end if;
		end if;
	end process;

	-- Process for knowing when the retrace of the following frame has started
	ongoing_process: process(clk, reset)
		begin
		if (clk'event and clk = '1') then
			if (reset = '0') then
                ongoing <= '0';
            else
            	if (data_end = '1') then
            		ongoing <= '0';
				elsif (vsync = '0' and vsync_reg = '1') then
					ongoing <= '1';
            	end if;
            end if;
        end if;
	end process;

	-- 2560 counter, conversion time for memory locations
	counter2560: process (clk, reset)
		begin
        if (clk'event and clk = '1') then
			if (reset = '0') then
            else
                if (sample_ready = '1' and ongoing = '1' and process_read = '1') then
					if(count_2560 = 2560) then 
						count_2560_next <= (others => '0');
						data_end <= '1';
					else
						count_2560_next <= count_2560 + 1;
						data_end <= '0';
					end if;
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
				-- Starting the read process
				if(sample_ready = '1' and ongoing = '1') then
					if(process_read = '0') then
						-- If negative slope
						if(trigger_slope = '0') then
							if((data1(11 downto 3) = actual_trigger) and (last_data1 > data1(11 downto 8))) then
								data1_value <= data1;
								process_read <= '1';
							end if;
						-- If positive slope
						elsif(trigger_slope = '1') then
							if((data1(11 downto 3) = actual_trigger) and (last_data1 < data1(11 downto 8))) then
								data1_value <= data1;
								process_read <= '1';
							end if;
						end if;
					else
						-- Reads until 2560 where the memory will be full of our data
						data1_value <= data1;
						if (count_2560 = 2560) then
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
				we <= '0';
				addr_in <= (others => '0');
				data_in <= (others => '0');
			else
				-- the system will write the next clock after an adquisition of new data
				if(sample_flag = '1' and process_read = '1') then
					we <= '1';
					addr_in <= count_2560 - 1;
					data_in <= data1_value;
				else
					we <= '0';
				end if;
			end if;
		end if;
	end process;
	
	count_2560 <= count_2560_next;
	sample_flag <= '1' when (sample_ready = '0' and sample_ready_reg = '1') else '0';

end arch;