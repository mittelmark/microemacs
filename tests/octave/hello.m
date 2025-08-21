#!/usr/bin/octave -W

#
# Copyright (c) 2025, Detlef Groth
# License:
# Date:$Date$
# Author:$Author$
# Created By:Detlef Groth
# Created:Thu Jul 24 06:25:18 2025
# Last Modified: < 250812.1316 >
#
# Description:Just test code for MicroEmacs
# verifying the Octave mode of ME
#
###################################################

%{
This is a multiline comment which can
wrap over several lines.

%}

### a greeting function

function greet

    for i = 1:5
        disp('Hello Octave World!')
    endfor

endfunction

### a test function

function test(msg)
    disp(msg)
endfunction

test('testing ...')
greet()

fprintf('4 * 5 is %d\n', 4 * 5)
x = int8(rand() * 20)

if x < 5
    disp('x is small')
elseif x < 10
    disp('x is OK')
else
    disp('x is large')
end

fprintf('x was %d\n', x)
