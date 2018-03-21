-- Template for combinational designs

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use STD.textio.all;
use ieee.std_logic_textio.all;

entity ecc_default_tb is
end ecc_default_tb;

architecture Behavioral of ecc_default_tb is

file file_vectors_in : text;
file file_results_out : text;

signal clock : std_logic := '0';

${UUT_SIGNALS}

begin

uut_i : entity work.${UUT_NAME}
	port map(
		${UUT_MAP}
	);

process
  variable iline : line;
  variable oline : line;
	${INPUT_VARS}
  variable space : character;
begin

  file_open(file_vectors_in, "input.txt",  read_mode);
  file_open(file_results_out, "output.txt", write_mode);

  while not endfile(file_vectors_in) loop
    readline(file_vectors_in, iline);
		${READ_VARS}
		${ASSIGN_IP}
    wait for 5 ns;
	clock <= '1';
    wait for 5 ns;
	clock <= '0';
    ${WRITE_OUT}
    writeline(file_results_out, oline);
  end loop;

  file_close(file_vectors_in);
  file_close(file_results_out);

  wait;
end process;


end architecture;
