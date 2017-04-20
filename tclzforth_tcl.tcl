catch {console show}
set zfLib [file dirname [info name]]/forth
set zfExe [file dirname [info name]]

load tclzforth.dll
source tclzforth.tcl
namespace import ::tclzforth::z*

puts "active tcl-version: [info patchlevel]\n"

if { [zforthCmd {latest .}] == "1" } {
	puts "zf> zforth with last saved session (zforth.save) loaded"
} else {
	puts "zf> none saved session, zforth is loaded with standard bootstrap ..."
	puts [zfLoad $zfLib/core.zf]
	puts [zfLoad $zfLib/dict.zf]
}
puts "\n[zf version]\n"

puts "to work with zforth try:"
puts "---------------------------------------------------------------"
puts "execute forth command   : zf { word word ... ;}  or  zf singleword"
puts "create  forth image     : zf save"
puts "create  forth autostart : zf {: FMAIN word ... word ;}"
puts "create  forth.exe       : zfFsave filename.exe image.save \[repl|norepl\]\n"