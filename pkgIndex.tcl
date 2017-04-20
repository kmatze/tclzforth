# --- pkgIndex.tcl file for tclzforth 1.6
proc tclzforth:Init { dir } {
     load [file join $dir tclzforth.dll] tclzforth
     source [file join $dir tclzforth.tcl]
     }
package ifneeded tclzforth 1.6 [list tclzforth:Init $dir]

