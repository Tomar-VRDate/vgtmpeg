/* @@--
 * 
 * Copyright (C) 2010-2011 Alberto Vigata
 *       
 * This file is part of vgtmpeg
 * 
 * a Versed Generalist Transcoder
 * 
 * vgtmpeg is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * vgtmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __NLINPUT_H
#define __NLINPUT_H

#include "nlffmsg.h"


#include <pthread.h>
#include <stdio.h>


#define sl0(x) (x)
#define sl8(x) ((x)<<8)
#define sl16(x) ((x)<<16)
#define sl24(x) ((x)<<24)

#define VGTM_W  ( sl24('V') | sl16('G') | sl8('T') | sl0('M') )
#define MTGV_W  ( sl24('M') | sl16('T') | sl8('G') | sl0('V') )

#define CB2INT(x) ( sl24(x[0]) | sl16(x[1]) | sl8(x[2]) | sl0(x[3]) ) 

//static int nlinput_readint(int *val) {
//    char b4[4];
//    int read = fread( b4, 1, 4, stdin );
//    *val = CB2INT(b4);
//    return read==4;
//}

static int nlinput_readbyte(char *val) {
    int read = fread( val, 1, 1, stdin );
    return read==1;
}


/* message code definitions */
#define EXIT                101
#define CANCEL_TRANSCODE    99


/* cross thread signal struct */
typedef struct {
    int exit;
    int cancel_transcode;
} nlinput_t;

static void *nlinput_start(void *c) {
    nlinput_t *ctx = (nlinput_t *)c;
    unsigned char b;
    int loop = 1;

    while(loop) {

       printf("about to read\n");
       if(!nlinput_readbyte(&b)) 
           break;

       printf("read %x\n",b);
       switch(b) {
           case EXIT:
               printf("nlinput: exiting\n");
               ctx->exit = 1;
               loop = 0;
               break;
           case CANCEL_TRANSCODE:
               printf("nlinput: canceling transcode\n");
               ctx->cancel_transcode = 1;
               break;
       }
    }

    pthread_exit(NULL);
}

/* static objects to be imported on module */
static nlinput_t nli;
static pthread_t nlin_th;
static pthread_attr_t nlin_attr;

/* fires up input thread */
static void nlinput_prepare(void) {
    /* starting nlinput */
    memset( &nli, 0, sizeof (nlinput_t) );

    pthread_attr_init(&nlin_attr);
    pthread_attr_setdetachstate(&nlin_attr, PTHREAD_CREATE_JOINABLE );
    pthread_create( &nlin_th, &nlin_attr, nlinput_start, (void *)&nli );
}

/* shutdown input thread */
static void nlinput_cancel(void) {
    void *status;
    printf("nlinput_cancel\n");

    pthread_join( nlin_th, &status );
    pthread_attr_destroy(&nlin_attr);
}


#endif /* __NLINPUT_H */
