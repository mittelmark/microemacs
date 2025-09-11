## -!- ps1 -!-
<#
Here follows a multiline comment
which can have many lines
#>

Write-Host 'Hello, World!'
'Hello, World!' | Write-Host

function Get-Fibonacci ($n) {
    if ($n -lt 2) {
        $n
    }
    else {
        (Get-Fibonacci ($n - 1)) + (Get-Fibonacci ($n - 2))
    }
}

# TODO: Check - If this works below

Get-Fibonacci 3 | Write-Host 
