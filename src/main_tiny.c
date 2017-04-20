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
const char *zforthSystem;

/*
 * read image from execute file
 */

int image()
{
    char zfImg[6] = {'Z', 'F', 'I', 'M', 'G', '\0'};
    char zf[6]    = {'0', '0', '0', '0', '0', '\0'};
    int c, pos=0, img = 0;

    FILE *in, *out;
    in  = fopen( zforthSystem, "rb" );      														// open running zForthSystem (file) for read

    while( (c = getc(in)) != EOF) {                 												// read zForthSystem
            zf[pos] = c;
            pos++;
            if (pos < 5) continue;
            if (strncmp(zf, zfImg, 5) == 0) {														// image found
                img = 1;
                break;
             } else {
                zf[0] = zf[1]; zf[1] = zf[2]; zf[2] = zf[3]; zf[3] = zf[4];
                pos=4;
           	}
    }

    //
    //  read image from zForthSystem
    //

	size_t len;
	void *p = zf_dump(&len);

	if (img) {
		fread(p, 1, len, in);
	} else {
		printf("zf> ... no image found", img);
	}

	fclose(in);

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

	/* Load dict from exe file if found */

   	img = image();
   	if (!img) exit(1);
    zf_eval("FMAIN");											// call the word FMAIN inside zforth

	exit(0);
}

/*
 * End
 */

