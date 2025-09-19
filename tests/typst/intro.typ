#set text(size: 22pt)
#set align(center)

Typst Tutorial

#set text(size: 15pt)

Max Musterman\
University of Mustercity

#set text(size: 11pt)

#set align(left)

= Introduction

In this report, we will  explore the various  factors  that  influence  
_fluid dynamics_ in glaciers and how they  contribute  to the formation and behaviour
of these natural structures.

+ The climate
+ The topography
+ The geology

+ The climate
  - Temperature
  - Precipitation
+ The topography
+ The geology

As an example for a *glacier* see Figure @glaciers. 

#figure(
        image("glacier.png",width: 70%),
        caption: [
                  _Glaciers_ form an important part of the earth's climate
                  system.
                  ]
        ) <glaciers> 


= Methods

We follow the glacier melting models  established in @glacier-melt.  Some more
text can be written.

Typst as well supports comments:

/*
 * 
 * This is a multiline comment
 */

= Math

The equation $Q = rho A v + C$
defines the glacial flow rate.

Which we can write even more $ x = 3 $.

The flow rate of a glacier is given
by the following equation:

$ Q = rho A v + "time offset" $

= Lorem Lipsum

#lorem(50)

#lorem(50)

= Code embedding

```py
def hello():
     print("Hello Python!")
hello()
```

#bibliography("works.bib")


