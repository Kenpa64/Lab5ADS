	library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity ADC_control is
port(
	clk:	in std_logic;
	resetn:	in std_logic;
	sdata1:	in std_logic;
	sdata2:	in std_logic;
	ncs:	out std_logic;
	sclk:	out std_logic;
	sample_ready:	out std_logic
	);
end ADC_control;


architecture arch of ADC_control is
	constant DATALEN: integer := 12;	-- Data length

	-- counter variables
	signal sclk_count, sclk_count_next: unsigned(2 downto 0);
	signal count_17, count_17_next: unsigned(4 downto 0);
	signal sclk_reg: std_logic;
	signal sclk_actual, sclk_last, sclk_falling: std_logic;

	-- control variables
	signal ncs_reg, ncs_reg2: std_logic;
	signal outleds, outleds_reg: std_logic_vector(7 downto 0);
	signal lab5var: std_logic;

	-- Data variables
	signal sdata1_reg, sdata1_reg2: std_logic;
	signal sdata1_vector, sdata2_vector: std_logic_vector(11 downto 0);
	signal sdata2_reg, sdata2_reg2: std_logic;
	
	begin

	process (clk, resetn)
	begin
		if(rising_edge(clk)) then
			if (resetn = '0') then	
				-- Reset signals
				outleds <= (others => '0');
				ncs <= '0';
				sclk <= '0';
				leds <= (others => '0');
				
				-- reset registers
				ncs_reg <= '0';
				ncs_reg2 <= '0';
				outleds_reg <= (others => '0');
				
			else
				-- refresh the output leds value when the lab5var is equal to 1
				if lab5var = '1' then
					outleds <= sdata1_vector(11 downto 4);
				end if;
				
				-- set the ncs signal to '1' when the counter is in its maximum value (16)
				if (count_17 = 16) then
					ncs_reg <= '1';
				else
					ncs_reg <= '0';
				end if;
				
				-- Sync output control signals
				ncs_reg2 <= ncs_reg;
				ncs <= ncs_reg2;
				
				sclk <= sclk_reg;
				
				outleds_reg <= outleds;
				leds <= outleds_reg;

				-- Sync input control signals
				sdata1_reg	 <= sdata1;
				sdata1_reg2 <= sdata1_reg;

				sdata2_reg <= sdata2;
				sdata2_reg2 <= sdata2_reg;
			end if;
		end if;
	end process;
	
	-- process to handle two sclk signals, the actual and the last one in order to detect a falling edge of the signal
	falling_flag: process(clk, resetn)
	begin
		if(rising_edge(clk)) then
			if(resetn ='0') then
				sclk_actual <= '0';
				sclk_last <= '0';
			else
				sclk_last <= sclk_actual;
				sclk_actual <= sclk_reg;
			end if;
		end if;
	end process;

	-- sclk counter, at clock edge. Clock divider.
	clkdiv: process(clk, resetn)
	begin
		if(rising_edge(clk)) then
			if (resetn = '0' or sclk_count = 5) then
				sclk_count_next <= (others => '0');
			else
				sclk_count_next <= sclk_count + 1;
			end if;
		end if;
	end process;

	-- counter to 16, at sclk falling edges
	counter16: process (clk, resetn)
	begin
		if (rising_edge(clk)) then
			if (resetn = '0' ) then
				count_17_next <= (others => '0');
			else
				if (sclk_falling = '1') then
					if (count_17 = 16) then
						count_17_next <= (others => '0');
					else
						count_17_next <= count_17 + 1;
					end if;
				end if;
			end if;
		end if;
	end process;

	-- Set the data1 serial bits into a vector
	serialvector1: process (clk, resetn)
	begin
		if(rising_edge(clk)) then
			if (resetn = '0') then
				sdata1_vector <= (others => '0');
			else
				if (sclk_falling = '1') then
					if(count_17 < 16) then
						-- the first 4 zeros will not be catched
						if(count_17 < 4) then
							sdata1_vector <= (others => '0');
						else	
							-- Transformation of the following 12 serial bits into a vector of 12 positions
							sdata1_vector <= sdata1_vector(10 downto 0) & sdata1_reg2;
						end if;
					end if;
				end if;
			end if;
		end if;
	end process;
	
	-- Set the data2 serial bits into a vector
	serialvector2: process (clk, resetn)
	begin
		if(rising_edge(clk)) then
			if (resetn = '0') then
				sdata2_vector <= (others => '0');
			else
				if(sclk_falling = '1') then
					if(count_17 < 16) then
						-- the first 4 zeros will not be catched
						if(count_17 < 4) then
							sdata2_vector <= (others => '0');
						else	
							-- Transformation of the following 12 serial bits into a vector of 12 positions
							sdata2_vector <= sdata2_vector(10 downto 0) & sdata2_reg2;
						end if;
					end if;
				end if;
			end if;
		end if;
	end process;
	
	-- lab5var creation process
	var: process (clk, resetn)
	begin
		if(rising_edge(clk)) then
			if (resetn = '0') then
				lab5var <= '0';
			else
				-- this variable will be '1' during 1 fast clock while the ncs variable is '1', else '0'
				if(ncs_reg = '1' and sclk_count = "000") then
					lab5var <= '1';
				else
					lab5var <= '0';
				end if;
			end if;
		end if;
	end process;
	
	-- sclk falling edge flag
	sclk_falling <= '1' when (sclk_actual = '0' and sclk_last = '1') else '0';
	
	-- uptading the old counter values to the current ones
	sclk_count <= sclk_count_next;
	count_17 <= count_17_next;
	sample_ready <= lab5var;
	-- creation of sclk, when the sclk_counter is lower than 3 it will be '1', else '0'
	sclk_reg <= '1' when (sclk_count < 3) else '0';
	
end arch;