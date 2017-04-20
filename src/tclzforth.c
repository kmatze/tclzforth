// tclzforth.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <getopt.h>
#include <math.h>

#include <tcl.h>
#include "zforth.h"

#define NS "tclzforth"
#define VERSION "1.6"
#define VERSDATUM "02-01-2017"
Tcl_Interp *interpGLOB;

/*
 * ZForth alternative output routine
 * and funtions
 *
 * example:
 *  print_zf("%s", NS " as tcl package, version " VERSION ", " VERSDATUM ", (c) 2016 ma.ke.");
 *  print_zf("%s", "\nbased on zFORTH by Ico Doornekamp (zevv on github)");
 *
 */

void print_zf( char* fmt, ...)
{
    char buf[100];
    va_list vl;
    va_start(vl, fmt);
    vsnprintf( buf, sizeof( buf), fmt, vl);
    va_end( vl);

    Tcl_AppendResult(interpGLOB, buf, (char *) NULL);
}


/*
 * ZForth host functions
 *
 */


zf_result do_eval(const char *src, int line, const char *buf)
{
	const char *msg = NULL;
	zf_result rv = zf_eval(buf);

	switch(rv)
	{
		case ZF_OK: break;
		case ZF_ABORT_INTERNAL_ERROR: msg = "internal error"; break;
		case ZF_ABORT_OUTSIDE_MEM: msg = "outside memory"; break;
		case ZF_ABORT_DSTACK_OVERRUN: msg = "dstack overrun"; break;
		case ZF_ABORT_DSTACK_UNDERRUN: msg = "dstack underrun"; break;
		case ZF_ABORT_RSTACK_OVERRUN: msg = "rstack overrun"; break;
		case ZF_ABORT_RSTACK_UNDERRUN: msg = "rstack underrun"; break;
		case ZF_ABORT_NOT_A_WORD: msg = "not a word"; break;
		case ZF_ABORT_COMPILE_ONLY_WORD: msg = "compile-only word"; break;
		case ZF_ABORT_INVALID_SIZE: msg = "invalid size"; break;
		default: msg = "unknown error";
	}

	if(msg) {
		print_zf("%s", "zf> error: ");
		if(src) print_zf("line %d, %s -> ", line, src);
		print_zf("%s", msg);
	}

	return rv;
}

/*
 * Load given forth file
 */

static int include(const char *fname)
{
	char buf[256];

	FILE *f = fopen(fname, "rb");
	int line = 1;
	if(f) {
		while(fgets(buf, sizeof(buf), f)) {
			do_eval(fname, line++, buf);
		}
		fclose(f);
		return 1;
	}
	print_zf("zf> error: load source file '%s', %s", fname, strerror(errno));
	return 0;
}

/*
 * Save dictionary
 */

static int save(const char *fname)
{
	size_t len;
	void *p = zf_dump(&len);
	FILE *f = fopen(fname, "wb");
	if(f) {
		// fwrite(p, 1, len, f);
		fwrite(p, 1, zf_dict_length(), f);
		fclose(f);
		print_zf("zf> image '%s' with %5i bytes saved", fname, zf_dict_length());
		return 1;
	}
	print_zf("zf> error: save dictionary file '%s', %s", fname, strerror(errno));
	return 0;
}

/*
 * Sys callback function
 */

zf_input_state zf_host_sys(zf_syscall_id id, const char *input)
{
	switch((int)id) {

		/* The core system callbacks */

		case ZF_SYSCALL_EMIT:
			print_zf("%c", (char)zf_pop());
			break;

		case ZF_SYSCALL_PRINT:
			print_zf(ZF_CELL_FMT " ", zf_pop());
			break;

		case ZF_SYSCALL_TELL: {
			zf_cell len = zf_pop();
			int print_zf_len = (int)len;
			char word[100];
			void *buf = (uint8_t *)zf_dump(NULL) + (int)zf_pop();
			strncpy(word, buf, print_zf_len);
			word[print_zf_len] = 0;
			print_zf("%s", word);
			}
			break;

		/* Application specific callbacks */

		// 128: bye, exit, quit
		case ZF_SYSCALL_USER + 0:
			print_zf("%s", "zf> error: 'bye' isn't implement in tcl package");
			break;

		// 129: function sinus
		case ZF_SYSCALL_USER + 1:
			zf_push(sin(zf_pop()));
			break;

		// 130: include a zf file
		case ZF_SYSCALL_USER + 2:
			if(input == NULL) {
				return ZF_INPUT_PASS_WORD;
			}
			include(input);
			break;

		// 131: dump, save memory image
		case ZF_SYSCALL_USER + 3:
			save("zforth.save");
			break;

		// 132: version output
		case ZF_SYSCALL_USER + 4:
    	    print_zf("zf> %s", "ZFORTH as tcl package, version " VERSION ", " VERSDATUM ", (c) 2017 ma.ke.\n");
    	    print_zf("zf> %s", "based on zFORTH by Ico Doornekamp (zevv on github)");
			break;

		// 133: fsave (standalone execute file)
		case ZF_SYSCALL_USER + 5:
			print_zf("%s", "zf> error: 'fsave' isn't implement in tcl package, use tcl function zfFsave");
			break;

		// default: unknown command
		default:
			print_zf("zf> error: unhandled syscall %d", id);
			break;
	}

	return ZF_INPUT_INTERPRET;
}


/*
 * Parse number
 */

zf_cell zf_host_parse_num(const char *buf)
{
	zf_cell v;
	// int r = sscanf(buf, "%f", &v);
	int r = sscanf(buf, "%i", &v);
	if(r == 0) {
		zf_abort(ZF_ABORT_NOT_A_WORD);
	}
	return v;
}

// -------------------------------------------------------------------------------
//
// tcl - procs
//
//


// run a forth command
//
static int Zforth_cmd(ClientData cdata, Tcl_Interp *interp, int argc, char *argv[])
{
	if (argc != 2) {
		Tcl_SetResult(interp, "wrong # args: zforthCmd { code }", TCL_STATIC);
		return TCL_ERROR;
	}

	interpGLOB = interp;

	zf_result rv = do_eval(NULL, 0, argv[1]);

   	return TCL_OK;
}

int DLLEXPORT Tclzforth_Init(Tcl_Interp *interp)
{
	if (Tcl_InitStubs (interp, TCL_VERSION, 0) == NULL) 	{ return TCL_ERROR; }
	if (Tcl_PkgProvide(interp, NS, VERSION) == TCL_ERROR) 	{ return TCL_ERROR; }

	// initializing zForth and load session if there

    zf_init(0);
	size_t len;
 	void *p = zf_dump(&len);
 	FILE *f = fopen("zforth.save", "rb");
 	if(f) {
 		fread(p, 1, len, f);
 		fclose(f);
 	} else {
 		zf_bootstrap();
	}

	Tcl_CreateCommand(interp, "zforthCmd",   (Tcl_CmdProc*)Zforth_cmd,    NULL, NULL);

	return TCL_OK;
}

