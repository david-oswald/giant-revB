#!/bin/sh

cd $(dirname $0)
./FWLoader -c -f -uu uc_series2.ihx -uf ../../fpga/vhdl/ise/main.bit
