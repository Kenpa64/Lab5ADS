library IEEE;
use IEEE.STD_LOGIC_1164.all;

entity portMap is
  port (
		clk:	in std_logic;
		resetn:	in std_logic;
		sdata1:	in std_logic;
		sdata2:	in std_logic;
		sw0: in std_logic;
		gpio_in: in std_logic_vector(22 downto 0);
		temp_up, temp_down, trigger_up, trigger_down, trigger_n_p: in std_logic;
		ncs, sclk:	out std_logic;
		hsync:	out std_logic;
		red:	out std_logic_vector(3 downto 0);
		green:	out std_logic_vector(3 downto 0);
		blue:	out std_logic_vector(3 downto 0)
		);
end;

architecture loquequierasmedaigual of portMap is

signal sample_ready, we, clk_in, clk_out, reset, vsync_trigger: std_logic;
signal data1, addr_in, addr_out, data_in, data_out: std_logic_vector(11 downto 0);
signal trigger_level: std_logic_vector(8 downto 0);

component trigger
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
		addr_in:	out std_logic_vector(11 downto 0);
		data_in:	out std_logic_vector(11 downto 0);
		trigger_level:	out std_logic_vector(8 downto 0)
	);
end component;

component vga_control
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
	vsync_trigger: out std_logic;
	addr_out: out std_logic_vector(11 downto 0)
	);
end component;

component ADC_control
port(
	clk:	in std_logic;
	resetn:	in std_logic;
	sdata1:	in std_logic;
	sdata2:	in std_logic;
	ncs:	out std_logic;
	sclk:	out std_logic;
	data1:  out std_logic_vector(11 downto 0);
	sample_ready:	out std_logic
	);
end component;

component sync_ram_dualport
port(
		clk_in : in std_logic;
		clk_out : in std_logic;
		we : in std_logic ;
		addr_in : in std_logic_vector(11 downto 0) ;
		addr_out : in std_logic_vector(11 downto 0) ;
		data_in : in std_logic_vector(11 downto 0) ;
		data_out : out std_logic_vector(11 downto 0)
);
end component;
signal resetVGA, vsync : std_logic;
begin
  resetVGA <= not resetn;
 -- vsyncout <= vsync;
 -- vsync <= vsyncin;
  G1: vga_control port map (data_out => data_out, trigger_level=> trigger_level, clk=> clk, reset => resetVGA, gpio_in => gpio_in, 
  sw0=>sw0, vsync=> vsync, vsync_trigger => vsync_trigger, hsync=>hsync, red=>red, green=>green, blue=>blue, addr_out=>addr_out);
  G2: trigger port map (vsync => vsync_trigger, sample_ready=>sample_ready, clk=>clk, reset=>resetn, trigger_up=>trigger_up, 
  trigger_down=>trigger_down, trigger_n_p=>trigger_n_p, data1=>data1, we=>we, addr_in=>addr_in, data_in=>data_in, trigger_level=>trigger_level, 
  period_clks=>period_clks);
  G3: ADC_control port map(clk=>clk, resetn=>resetn, sdata1=>sdata1, sdata2=>sdata2, ncs=>ncs, sclk=>sclk, data1=>data1, sample_ready=>sample_ready);
  G4: sync_ram_dualport port map(clk_in=>clk, clk_out=>clk, we=>we, addr_in=>addr_in, addr_out=>addr_out, data_in=>data_in, data_out=>data_out);
end; 