//
//  ff_md5.h
//
//  Created by helei on 2018/4/11.
//  Copyright 何磊 <helei0908@hotmail.com>. All rights reserved.
//

#ifndef ff_md5_h
#define ff_md5_h

#include <stdio.h>

void ff_str_md5(const char * str, char md5[36]);
void ff_md5(const unsigned char * data, int data_len, char md5[36]);
void ff_md5_hex2str(unsigned char hex_md5[16], char str_md5[36]);
#endif /* ff_md5_h */
