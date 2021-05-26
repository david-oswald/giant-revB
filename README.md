# GIAnt, the Generic Implementation ANalysis Toolkit

This is a fork of the original GIAnt project (www.sourceforge.net/projects/giant/) with a few changes.

## Changes

* The board
	* There is a GIAnt fault only design by David Oswald
	* The FPGA module is updated to `ZTEX USB-FPGA Module 2.04` with `series 2 to series 1 adapter board`

* The FPGA source code
	* Modified from `giant-20111116`
	* The FPGA selection of the project is changed to XC6SLX16
	* IP cores regenerated
	* .ucf file updated to match the new pinout of series 2 module

* The microcontroller source code is updated to fit the new board
