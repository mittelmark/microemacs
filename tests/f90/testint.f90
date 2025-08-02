!-*- f90 -*- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!  Date          : $Date$
!  Author        : $Author$
!  Created By    : dgroth
!  Created       : Sat Aug 2 10:48:35 2025
!  Last Modified : <250802.1214>
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

program testint
    implicit none ! use this always!!
    integer :: largeval 
    integer :: x, y
    complex :: cx
    real, parameter :: pi = 3.1415927
    type person
        character (len=20) :: name
        integer :: age
    end type
    type(person) :: myself
    myself%age=17    
    print *, huge(largeval)
    cx = cmplx(1.0, -7.9)
    x = 3
    y = 2
    print *, cx
    if ( x == y) then
        print *, "x and y are equal"
    elseif  (x < y ) then
    print *, "x is smaller than y"
    else
        print *, "x is larger than y"
    end if
    select case (x)
    case (3)
        print *, "x is three"
    case (4)
        print *, "x is four"
    case default
        print *, "x is neither three or four"
    end select
    do while (x < 10)
        print *, x
        x = x + 1
    enddo
end program

