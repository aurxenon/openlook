#!/bin/bash -norc
# name:      xvf  --  start xvfilemgr and redirect its output to console
# author:    c.l.s. (cspiel@physik.tu-muenchen.de)
# last rev.: Fri Feb 14 09:36:45 MET 1997
# bash ver.: 1.14.6.(1)

# Check whether there is already an xconsole.  Start one if necessary.
{ ps | grep " xconsole $" > /dev/null 2>&1; } || xconsole &

# Launch xvfilemgr.
xvfilemgr > /dev/console 2>&1 &

