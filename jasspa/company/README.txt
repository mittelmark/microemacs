> -!- document -!-
> This is part of the JASSPA MicroEmacs macro files
> Copyright (C) 2004 JASSPA (www.jasspa.com)
> See the file COPYING for copying and conditions.
>

Place  your  company  wide  information  in this  directory.  The
directory may be defined as follows:

comany.emf

    Typically  there  is a file  called  "company.emf"  which  is
    executed at start-up by all users. This file should contain a
    formal   company  name  which  is  used  for  populating  all
    templates.

        set-variable %company-name "Acme Industries Inc."

*.etf
    
    Template  files for new file  headers.  Copy the *.etf  files
    from the  "macro"  directory  and modify to the style used by
    the company.
    
*.emf
    
    Add macro files that are used on a company  basis,  typically
    private  file formats etc. The  "company.emf"  would  include
    hooks for the template i.e.
    
        add-file-hook ".myf" fhook-myfile
        
    The company directory would include the macro definition i.e.

        hkmyfile.emf
        
    This  file  type is then  defined  for all  users who use the
    company file.
