tclzForth
=========

tclzForth is a tcl package based on zForth by Ico Doornekamp alias zevv (see github)
for the windows system. For zForth inside read the documentation in zForth-master.

I've changed a little bit the command line of zForth system:

**Deleted features**
```
   - command line arguments -h -t -l
   - trace (to make the exe file smallest)
```
**Added features**
```
   - single comand line argument to load saved dictionary
   - new start process
   - version   info about zForth
   - FMAIN     this word will be started after successful image loaded (autostart)
   - fsave     save the actual workspace as image in zfnew.exe
```
**The start process of zForth.exe is changed to this steps:**
```
   1. load image (e.g. zforth.save) from disk if requested as first command line argument,
   2. otherwise load image inside from exe file if found,
   3. otherwise load standard forth bootstrap (system forth)
```
To compile I used tcc (and gcc 3.4.5 too) with a simple batch script, compile_zforth.bat.

To make the execute files smallest, I use strip.exe and upx3.93.

**First start of zForth should be:**
```
   1. start zforth.exe
   2. zforth cli:
      130 sys forth\core.zf           // load system core
      130 sys forth\dict.zf           // load words for handling dictionary
      words                           // show words in dictionary
      save                            // save work space as image in file zforth.save
      fsave                           // save workspace as a extended zforth system in file zfnew.exe
      bye                             // end the zForth session
 ```

**Now you can start with zForth in diffrent ways:**
```
   1. zforth.exe zforth.save          // start zForth system and load saved image zforth.save
   2. zfnew.exe                       // start extended zForth system
```
tclzforth you can compile with gcc. See the batch file compile_tclzforth.bat

**To start tclforth follow the next steps:**
```
   0. start tcl/tk shell, inside do:
   1. in the start directory should be the files zforth.exe, zftiny.exe and the subdir forth with core.zf and dict.zf
   2. source tclzforth_tcl.tcl                    // load tclzforth.dll, set the environment and load tclforth.tcl
                                                  // show "welcome-screen" of tclzforth
```
**With the next steps you can create your own execute zForth (no tcl system!):**
```
   1. zf {: FMAIN 1 1 + . ; }                     // your are inside zForth -> define the "autostart" word
   2. zf save                                     // your are inside zForth -> save workspace as image in file zforth.save
   3a. zfFsave yourZf1.exe zforth.save repl       // create yourZf1.exe with repl
   3b. zfFsave yourZf2.exe zforth.save norepl     // create yourZf1.exe without repl
```
TRY IT ;-)

I hope you have fun.

Greetings - kmatze (aka ma.ke.) - 20.04.2017






