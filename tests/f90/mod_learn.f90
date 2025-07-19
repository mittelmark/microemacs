! https://raw.githubusercontent.com/da-james/djs-office-hours/refs/heads/master/fortran-learning-src/mod_learn.f90
! Fortran Tutorial by David James, License of code: MIT

module mod_learn

    ! provides precision for data types
    use iso_fortran_env

    ! artifact of legacy code
    implicit none

contains

    function basics()
        ! ***** initialize data types
        real(real64) :: a, b, carr(5)
        real(real64), allocatable :: darr(:)
        integer(int64) :: i, j
        complex(real64) :: c
        logical :: basics
        character(len=12) :: msg

        a = 0
        b = 3
        i = 4
        j = 5
        c = (1,2)
        basics = .true.
        msg = "hello world!"

        print *, "floats", a, b
        print *, "ints", i, j
        print *, "complex", c%re, c%im
        print *, "bools", basics
        print *, msg

        ! ***** control flow
        if(a < 0) then
            print *, "a is negative"
        else if(a > 0) then
            print *, "a is positive"
        else
            print *, "a is zero"
        end if

        ! ***** do/while loop
        do i = 1,10
            print *, i
        end do

        i = 1
        do while(i < 10)
            i = i + 1
        end do

        ! arrays
        allocate(darr(i))
        darr(:) = 1
        print *, darr
        darr(3) = 3
        print *, darr
        print *, carr
        carr = [1, 2, 3, 4, 5]
        print *, carr

        carr(2) = 1
        print *, carr

        ! subroutines
        print *, i, j
        call sub_a(a, i, j)
        print *, i, j

    end function basics

    subroutine sub_a(x, y, z)
        real(real64), intent(in) :: x
        integer(int64), intent(inout) :: y, z

        print *, x
        y = y + 1
        z = z - 1

    end subroutine sub_a

end module

program main

    use mod_learn, only : basics
    
    implicit none
    logical :: flag
    
    flag = basics()
    print *, flag
end program

