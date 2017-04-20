######################################################################
#
# BEGINN namespace
#
namespace eval ::tclzforth:: {
#---------------------------------------------------------------------
#
# zf
#
proc zf { zfWords } {
	set zfResult [split [zforthCmd $zfWords] "\n"]
	set res {}
	set linecounter 0
	set newline ""
	foreach line $zfResult {
		incr linecounter
		if { [regexp "zf> error:" $line] > 0 } {
			puts stderr $line
		} else {
			append res $newline $line
			if { $linecounter == 1 } { set newline "\n" }
		}
	}
	return $res
 }
#---------------------------------------------------------------------
#
# zfLoad
#
proc zfLoad { FileName } {
	set fileid [open $FileName r]
	set file_data [read $fileid]
	set data [split $file_data "\n"]
	close $fileid
	set counter 0
    foreach line $data {
    	incr counter
        set res [zf $line]
        if { $res != "" } { puts $res }
	}
    return "    $counter lines read from [file tail $FileName]"
}
#---------------------------------------------------------------------
#
# zfFsave
#
proc zfFsave { FileName FileImage {Type "repl"} } {
	global zfExe
    set FileZforth "$zfExe/zforth.exe"
    if { $Type ne "repl" } { set FileZforth "$zfExe/zftiny.exe" }

	set fOut [open $FileName w]
	fconfigure $fOut -translation binary

	set fIn [open $FileZforth r]
	fconfigure $fIn -translation binary
	puts -nonewline $fOut [read $fIn]
	close $fIn

	puts -nonewline $fOut {ZFIMG}

	set fIn [open "$FileImage" r]
	fconfigure $fIn -translation binary
	puts -nonewline $fOut [read $fIn]
	close $fIn

	close $fOut
}

######################################################################
#
# END namespace
#
namespace export zf zfLoad zfFsave
}