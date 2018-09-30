//
//  ff_md5.c
//
//  Created by helei on 2018/4/11.
//  Copyright 何磊 <helei0908@hotmail.com>. All rights reserved.
//

#include "ff_md5.h"
#include <string.h>
#include <stdio.h>
#include "md5.h"
void ff_str_md5(const char * str, char md5[36]){
    md5_byte_t digest[16];
    md5_state_t md5StateT;
    md5_init(&md5StateT);
    md5_append(&md5StateT, (const unsigned char *)str, (int)strlen(str));
    md5_finish(&md5StateT, digest);
    ff_md5_hex2str(digest, md5);
}

void ff_md5(const unsigned char * data, int data_len, char md5[36]){
    md5_byte_t digest[16];
    md5_state_t md5StateT;
    md5_init(&md5StateT);
    md5_append(&md5StateT, data, data_len);
    md5_finish(&md5StateT, digest);
    ff_md5_hex2str(digest, md5);
}

void ff_md5_hex2str(unsigned char hex_md5[16], char str_md5[36]){
    //memset(str_md5, 0, 36);//32 used
    str_md5[32] = 0;//32 used
    int i = 0;
    char h, l;
    for(i = 0; i < 16; i++){
        h = (hex_md5[i] >> 4) & 0xF;
        l = hex_md5[i] & 0xF;
        if(h < 10){
            str_md5[i * 2] = h + '0';
        }else {
            str_md5[i * 2] = h - 10 + 'a';
        }
        if(l < 10){
            str_md5[i * 2 + 1] = l + '0';
        }else {
            str_md5[i * 2 + 1] = l - 10 + 'a';
        }
    }
}
