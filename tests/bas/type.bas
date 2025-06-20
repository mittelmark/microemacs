Namespace Sample
  Type T
    x As Integer
  End Type
End Namespace

Type Parent
  Private:
    Dim As String nameParent
    Declare Constructor()
    Declare Constructor(Byref As Parent)
    Type Child
      Dim As String nameChild
      Dim As Parent Ptr ptrParent
      Declare Sub kinship()
    End Type
    Dim As Child listChild(Any)
  Public:
    Declare Constructor(Byref _nameParent As String)
    Declare Sub addChild(Byref _nameChild As String)
    Declare Sub kinship()
End Type

Constructor Parent(Byref _nameParent As String)
  This.nameParent = _nameParent
End Constructor

Sub Parent.addChild(Byref _nameChild As String)
  Redim Preserve This.listChild(Ubound(This.listChild) + 1)
  This.listChild(Ubound(This.listChild)).nameChild = _nameChild
  This.listChild(Ubound(This.listChild)).ptrParent = @This
End Sub

Sub Parent.Child.kinship()
  Print "'" & This.nameChild & "'" & " is child of " & "'" & This.ptrParent->nameParent & "'"
End Sub

Sub Parent.kinship()
  For i As Integer = 0 To Ubound(This.listChild)
    This.listChild(i).kinship()
  Next i
End Sub

Destructor Parent ()
  Deallocate This.nameParent
End Destructor

Dim As Parent p = Parent("Kennedy")
p.addChild("John Jr.")
p.addChild("Caroline")
p.addChild("Patrick")
p.kinship()

Sleep
