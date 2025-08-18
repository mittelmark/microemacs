#!/usr/bin/env eui
----------------------------------------------------------------------------
--
--  Author        : $Author$
--  Created By    : dgroth
--  Created       : Wed Aug 13 15:58:28 2025
--  Last Modified : <250818.1010>
--
--  Description
--
--  Notes
--
--  History
--
--  Copyright (c) 2025 dgroth.
--
----------------------------------------------------------------------------

public procedure hi()
   puts(1,"Hi Euphoria!\n")
end procedure
procedure hi2()
   puts(1,"Hello Hi2!\n")
end procedure

export function max(atom a, atom b)
   if a >= b then
      return a
   else
      return b
   end if
end function
hi()
atom x = max(1,4)
object z = 3
printf(1,"Maximum of 1 and 4 is: %d, z is: %d\n",{x,z})

