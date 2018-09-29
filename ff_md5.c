//
//  ff_md5.c
//  IJKMediaFramework
//
//  Created by helei on 2018/4/11.
//  Copyright © 2018年 bilibili. All rights reserved.
//

#include "ff_md5.h"
/* MD5C.C - RSA Data Security, Inc., MD5 message-digest algorithm
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
 rights reserved.
 
 License to copy and use this software is granted provided that it
 is identified as the "RSA Data Security, Inc. MD5 Message-Digest
 Algorithm" in all material mentioning or referencing this software
 or this function.
 
 License is also granted to make and use derivative works provided
 that such works are identified as "derived from the RSA Data
 Security, Inc. MD5 Message-Digest Algorithm" in all material
 mentioning or referencing the derived work.
 
 RSA Data Security, Inc. makes no representations concerning either
 the merchantability of this software or the suitability of this
 software for any particular purpose. It is provided "as is"
 without express or implied warranty of any kind.
 
 These notices must be retained in any copies of any part of this
 documentation and/or software.
 */
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
