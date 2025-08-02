!-*- f90 -*- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!  Date          : $Date$
!  Author        : $Author$
!  Created By    : dgroth
!  Created       : Sat Aug 2 10:41:34 2025
!  Last Modified : <250802.1044>
!
!  Description   :
!
!  Notes         : 
!
!  History       ;  
!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!
!  Copyright (c) 2025 dgroth.
!
!  All Rights Reserved.
!
! This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
! reproduced,  translated,  or  reduced to any  electronic  medium or machine
! readable form without prior written consent from dgroth.
!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
program addnum
    implicit none
    real :: a, b, result
    a = 12.0
    b = 15.0
    result = a + b
    print *, 'The total is', result
end program

