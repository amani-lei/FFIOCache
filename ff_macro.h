//
//  ff_macro.h
//  ffmpeg_demo
//
//  Created by helei on 2018/9/5.
//  Copyright © 2018年 何磊. All rights reserved.
//

#ifndef ff_macro_h
#define ff_macro_h

#define BYTE0(v) ((v) & 0xFF)
#define BYTE1(v) (((v) >> 8) & 0xFF)
#define BYTE2(v) (((v) >> 16) & 0xFF)
#define BYTE3(v) (((v) >> 24) & 0xFF)
#define BYTE4(v) (((v) >> 32) & 0xFF)
#define BYTE5(v) (((v) >> 40) & 0xFF)
#define BYTE6(v) (((v) >> 48) & 0xFF)
#define BYTE7(v) (((v) >> 56) & 0xFF)

#define DATA_FROM_16(to, from) \
do{\
*(int8_t*)((to)+0)=BYTE1(from);\
*(int8_t*)((to)+1)=BYTE0(from);\
}while(0)

#define DATA_FROM_32(to, from) \
do{\
*(int8_t*)((to)+0)=BYTE3(from);\
*(int8_t*)((to)+1)=BYTE2(from);\
*(int8_t*)((to)+2)=BYTE1(from);\
*(int8_t*)((to)+3)=BYTE0(from);\
}while(0)

#define DATA_FROM_64(to, from) \
do{\
*(int8_t*)((to)+0)=BYTE7(from);\
*(int8_t*)((to)+1)=BYTE6(from);\
*(int8_t*)((to)+2)=BYTE5(from);\
*(int8_t*)((to)+3)=BYTE4(from);\
*(int8_t*)((to)+4)=BYTE3(from);\
*(int8_t*)((to)+5)=BYTE2(from);\
*(int8_t*)((to)+6)=BYTE1(from);\
*(int8_t*)((to)+7)=BYTE0(from);\
}while(0)

#define DATA_TO_16(from) \
(\
((uint16_t)*(unsigned char*)(from+0)) << 8 |\
((uint16_t)*(unsigned char*)(from+1))\
)

#define DATA_TO_32(from) \
(\
((uint32_t)*(unsigned char*)(from+0)) << 24 |\
((uint32_t)*(unsigned char*)(from+1)) << 16 |\
((uint32_t)*(unsigned char*)(from+2)) << 8 |\
((uint32_t)*(unsigned char*)(from+3))\
)

#define DATA_TO_64(from) \
(\
((uint64_t)*(unsigned char*)(from+0)) << 56 |\
((uint64_t)*(unsigned char*)(from+1)) << 48 |\
((uint64_t)*(unsigned char*)(from+2)) << 40 |\
((uint64_t)*(unsigned char*)(from+3)) << 32 |\
((uint64_t)*(unsigned char*)(from+4)) << 24 |\
((uint64_t)*(unsigned char*)(from+5)) << 16 |\
((uint64_t)*(unsigned char*)(from+6)) << 8 |\
((uint64_t)*(unsigned char*)(from+7))\
)

#define DATA_TO_I16(from) ((int16_t)DATA_TO_16(from))
#define DATA_TO_I32(from) ((int32_t)DATA_TO_32(from))
#define DATA_TO_I64(from) ((int64_t)DATA_TO_64(from))

#define DATA_TO_U16 DATA_TO_16
#define DATA_TO_U32 DATA_TO_32
#define DATA_TO_U64 DATA_TO_64


#endif /* ff_macro_h */
