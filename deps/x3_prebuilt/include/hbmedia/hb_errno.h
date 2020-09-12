#ifndef __HB_ERRNO_H__
#define __HB_ERRNO_H__

#define HB_ERR_APPID  (0x1000FC00L)

/*********************************************************************
|----------------------------------------------------------------|
|     APP_ID     |    MOD_ID   |   Reserved  |      ERR_ID       |
|----------------------------------------------------------------|
|<------8bits----><----8bits---><----6bit----><-----10bits------>|
**********************************************************************/

#define HB_DEF_ERR(module, errid) \
    ((int)( (HB_ERR_APPID) | ((module) << 16) | (errid) ))

/*
example, module define like this
-------------------------------------

typedef enum {
	EN_ERR_BUSY = 0,
	...
} EN_VIN_ERR_CODE_E;

#define HB_ERR_VIN_BUSY		HB_DEF_ERR(HB_ID_VIN, EN_ERR_BUSY)


int func(void)
{
	.....
	return -HB_ERR_VIN_BUSY;
}
*/

#endif
