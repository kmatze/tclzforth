#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

#include "zforth.h"

# define FLOAD 0
# define FSAVE 1

const char *zforthSystem;

/*
 * Evaluate buffer with code, check return value and report errors
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

	if(msg) { printf("zf> error: %s:%d -> %s", src, line, msg); }

	return rv;
}

/*
 * Show version information
 */

void version()
{
	printf("zf> ZForth by Ico Doornekamp (zevv on github)");
	printf("\nzf> forked with extended functions by ma.ke. 2017");
 }


/*
 * Load forth source file
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
	printf("zf> error: load source file '%s', %s", fname, strerror(errno));
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
		printf("zf> image '%s' with %5i bytes saved", fname, zf_dict_length());
		return 1;
	}
	printf("zf> error: save dictionary file '%s', %s", fname, strerror(errno));
	return 0;
}

/*
 * Load dictionary
 */

static int load(const char *fname)
{
	size_t len;
	void *p = zf_dump(&len);
	FILE *f = fopen(fname, "rb");
	if(f) {
		fread(p, 1, len, f);
		fclose(f);
		printf("zf> ... image '%s' loaded", fname);
		return 1;
	}
	printf("zf> error: load dictionary file '%s', %s", fname, strerror(errno));
	return 0;
}

/*
 * save zforth with actual dictionary in execute file zforthNew.exe
 */

int image(int fsave, const char *fin, const char *fout)
{
    char zfImg[6] = {'Z', 'F', 'I', 'M', 'G', '\0'};
    char zf[6]    = {'0', '0', '0', '0', '0', '\0'};
    int c, count=0, pos=0, img = 0;

    FILE *in, *out;
    in  = fopen( fin, "rb" );      																	// open running zForthSystem (file) for read
    if (fsave) {
    	out = fopen( fout, "wb" );																	// open new zForthSystem (file) to save
    	printf("zf> create '%s'", fout);
    }

    while( (c = getc(in)) != EOF) {                 												// read zForthSystem
            zf[pos] = c;
            pos++; count++;
            if (pos < 5) continue;
            if (strncmp(zf, zfImg, 5) == 0) {														// image found
                img = 1; count = count - 5;
                break;
             } else {
            	if (fsave == 1) { putc(zf[0], out); }												// save zForthSystem
                zf[0] = zf[1]; zf[1] = zf[2]; zf[2] = zf[3]; zf[3] = zf[4];
                pos=4;
           	}
    }

    //
    // save or read image with zForthSystem
    //

	size_t len;
	void *p = zf_dump(&len);
    if (fsave) {																					// save image?
    	if( img == 0 ) { putc(zf[0], out); putc(zf[1], out); putc(zf[2], out); putc(zf[3], out); }	// save last four bytes
		fwrite(zfImg, 1, sizeof(zfImg) - 1, out );           										// save zf img flag
   		printf("\nzf> ... saved %5i bytes system core", count);
  		fwrite(p, 1, zf_dict_length(), out);														// save image
		printf("\nzf> ... saved %5i bytes image", zf_dict_length());
		fclose(out);
	}
	if (!fsave) {																					// read image?
		if (img) {
			fread(p, 1, len, in); 																	// read image
		} else {
			printf("zf> ... no image found", img);
		}
		fclose(in);
	}
	return (img);
}

/*
 * Sys callback function
 */

zf_input_state zf_host_sys(zf_syscall_id id, const char *input)
{
	switch((int)id) {

		/* The core system callbacks */

		case ZF_SYSCALL_EMIT:
			putchar((char)zf_pop());
			fflush(stdout);
			break;

		case ZF_SYSCALL_PRINT:
			printf(ZF_CELL_FMT " ", zf_pop());
			break;

		case ZF_SYSCALL_TELL: {
			zf_cell len = zf_pop();
			void *buf = (uint8_t *)zf_dump(NULL) + (int)zf_pop();
			(void)fwrite(buf, 1, len, stdout);
			fflush(stdout); }
			break;

		/* Application specific callbacks */

		// 128: system execute, halt, bye
		case ZF_SYSCALL_USER + 0:
			exit(0);
			break;

		// 129: sinus function
		case ZF_SYSCALL_USER + 1:
			zf_push(sin(zf_pop()));
			break;

		// 130: include source file
		case ZF_SYSCALL_USER + 2:
			if(input == NULL) { return ZF_INPUT_PASS_WORD; }
			include(input);
			break;

		// 131: save image
		case ZF_SYSCALL_USER + 3:
			save("zforth.save");
			break;

		// 132: version output
		case ZF_SYSCALL_USER + 4:
			version();
    	    break;

		// 133: fsave (standalone execute file)
		case ZF_SYSCALL_USER + 5:
			// if(input == NULL) { return ZF_INPUT_PASS_WORD; }
			image(FSAVE, zforthSystem, "zfnew.exe" );
			break;

		default:
			printf("zf> error: unhandled syscall %d", id);
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
	//int r = sscanf(buf, "%f", &v);
	int r = sscanf(buf, "%i", &v);
	if(r == 0) {
		zf_abort(ZF_ABORT_NOT_A_WORD);
	}
	return v;
}

/*
 * Main
 */

int main(int argc, char **argv)
{
	int img = 0, line = 0;

	const char *fname_load = argv[1];
	zforthSystem = argv[0];

	/* Initialize zforth */

	zf_init();

	/* Load dict from from disk if requested,
	 * otherwise load dict from exe file if found,
	 * otherwise bootstrap forth dictionary
	 */

    if(fname_load) { img = load(fname_load); }							// load image from command line

   	if (!img) {															// otherwise
    	img = image(FLOAD, zforthSystem, "");							//      check image in execute file
    	if (!img) {														//		otherwise
      		zf_bootstrap();												// 			load standard bootstrap
     		printf(", standard bootstrap loaded");
     	} else {
     	    printf("zf> ... image from '%s' loaded", zforthSystem);
     	    zf_eval("FMAIN");											// call the word FMAIN inside zforth
     	}
	}
	printf("\n");
	version();


	/* Interactive interpreter
	 * and pass to zf_eval() for evaluation*/


	printf("\nzf> ");
	for(;;) {
		char buf[4096];
		if(fgets(buf, sizeof(buf), stdin)) {
			do_eval("stdin", ++line, buf);
			printf("\nzf> ");
		} else {
			break;
		}
	}

	return 0;
}

/*
 * End
 */

