# include "images.h"
# include <pgmspace.h>

const uint8_t IMG_tank_0_data[] PROGMEM = 
{
	0x00, 0x00, 0x55, 0x54, 0x00, 0x00, //         #######        
	0x00, 0x00, 0x40, 0x04, 0x00, 0x00, //         #     #        
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                        
	0x05, 0x55, 0x55, 0x55, 0x55, 0x40, //   ###################  
	0x16, 0xAA, 0xAA, 0xAA, 0xAA, 0x50, //  ##+++++++++++++++++## 
	0x1A, 0xAA, 0xAA, 0xAA, 0xAA, 0x90, //  #+++++++++++++++++++# 
	0x5A, 0xAA, 0xAA, 0xAA, 0xAA, 0x94, // ##+++++++++++++++++++##
	0x6A, 0xAA, 0xA9, 0xAA, 0xAA, 0xA4, // #++++++++++#++++++++++#
	0x6A, 0xAA, 0xA5, 0x6A, 0xAA, 0xA4, // #+++++++++###+++++++++#
	0x6A, 0xAA, 0x96, 0x5A, 0xAA, 0xA4, // #++++++++##+##++++++++#
	0x6A, 0xAA, 0x9A, 0x9A, 0xAA, 0xA4, // #++++++++#+++#++++++++#
	0x6A, 0xAA, 0x96, 0x5A, 0xAA, 0xA4, // #++++++++##+##++++++++#
	0x5A, 0xAA, 0xA5, 0x6A, 0xAA, 0x94, // ##++++++++###++++++++##
	0x1A, 0xAA, 0xAA, 0xAA, 0xAA, 0x90, //  #+++++++++++++++++++# 
	0x15, 0x99, 0x99, 0x99, 0x99, 0x50, //  ###+#+#+#+#+#+#+#+### 
	0x05, 0x55, 0x55, 0x55, 0x55, 0x40, //   ###################  
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                        
	0x11, 0x44, 0x00, 0x00, 0x45, 0x10, //  # ## #         # ## # 
	0x15, 0x54, 0x00, 0x00, 0x55, 0x50, //  ######         ###### 
	0x05, 0x50, 0x00, 0x00, 0x15, 0x40, //   ####           ####  
};

sIMAGE IMG_tank_0 = {
  IMG_tank_0_data,
  23, /* Width */
  20, /* Height */
};