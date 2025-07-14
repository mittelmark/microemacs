# -*- perl -*- ###############################################################
#
#  System        :
#  Module        :
#  Object Name   : $RCSfile:$
#  Revision      : $Revision:$
#  Date          : $Date:$
#  Author        : $Author:$
#  Created By    : Detlef Groth
#  Created       : Sun Jul 13 18:38:16 2025
#  Last Modified : <250713.1921>
#
#  Description
#
#  Notes
#
#  History
#
##############################################################################
#
#  Copyright (c) 2025 Detlef Groth.
#
#  All Rights Reserved.
#
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from Detlef Groth.
#
##############################################################################
package Test;
use strict;
sub new {
    my ($class) = shift;
    my $self = {x=>2};
    bless $self, $class;
    return $self;
}
sub test {
    my $self = shift;
    print("Testing $self ...! x is $self->{x}\n");
}

our $y = 1;
sub main {
    my $x = 1;
    print("$y\n");
    print("Hello Perl!\n");
    my $obj = Test->new();
    $obj->test();
}
main()

