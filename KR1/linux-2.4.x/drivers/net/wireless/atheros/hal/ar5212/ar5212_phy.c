/*
 * Copyright (c) 2002-2004 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2004 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/releases/SuperG_Linux_AP/src/802_11/madwifi/hal/ar5212/ar5212_phy.c#3 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5212

#include "ah.h"
#include "ah_internal.h"

#include "ar5212/ar5212.h"

/* shorthands to compact tables for readability */
#define	OFDM	IEEE80211_T_OFDM
#define	CCK	IEEE80211_T_CCK
#define	TURBO	IEEE80211_T_TURBO
#define	XR	ATHEROS_T_XR

HAL_RATE_TABLE ar5212_11a_table = {
	8,  /* number of rates */
	{ 0 },
	{
/*                                                  short            ctrl  */
/*                valid                 rateCode Preamble  dot11Rate Rate */
/*   6 Mb */ {  AH_TRUE, OFDM,    6000,     0x0b,    0x00, (0x80|12),   0 },
/*   9 Mb */ {  AH_TRUE, OFDM,    9000,     0x0f,    0x00,        18,   0 },
/*  12 Mb */ {  AH_TRUE, OFDM,   12000,     0x0a,    0x00, (0x80|24),   2 },
/*  18 Mb */ {  AH_TRUE, OFDM,   18000,     0x0e,    0x00,        36,   2 },
/*  24 Mb */ {  AH_TRUE, OFDM,   24000,     0x09,    0x00, (0x80|48),   4 },
/*  36 Mb */ {  AH_TRUE, OFDM,   36000,     0x0d,    0x00,        72,   4 },
/*  48 Mb */ {  AH_TRUE, OFDM,   48000,     0x08,    0x00,        96,   4 },
/*  54 Mb */ {  AH_TRUE, OFDM,   54000,     0x0c,    0x00,       108,   4 }
	},
};

HAL_RATE_TABLE ar5212_turbo_table = {
	8,  /* number of rates */
	{ 0 },
	{
/*                                                 short            ctrl  */
/*                valid                rateCode Preamble  dot11Rate Rate */
/*   6 Mb */ {  AH_TRUE, TURBO,   6000,    0x0b,    0x00, (0x80|12),   0 },
/*   9 Mb */ {  AH_TRUE, TURBO,   9000,    0x0f,    0x00,        18,   0 },
/*  12 Mb */ {  AH_TRUE, TURBO,  12000,    0x0a,    0x00, (0x80|24),   2 },
/*  18 Mb */ {  AH_TRUE, TURBO,  18000,    0x0e,    0x00,        36,   2 },
/*  24 Mb */ {  AH_TRUE, TURBO,  24000,    0x09,    0x00, (0x80|48),   4 },
/*  36 Mb */ {  AH_TRUE, TURBO,  36000,    0x0d,    0x00,        72,   4 },
/*  48 Mb */ {  AH_TRUE, TURBO,  48000,    0x08,    0x00,        96,   4 },
/*  54 Mb */ {  AH_TRUE, TURBO,  54000,    0x0c,    0x00,       108,   4 }
	},
};

HAL_RATE_TABLE ar5212_11b_table = {
	4,  /* number of rates */
	{ 0 },
	{
/*                                                 short            ctrl  */
/*                valid                rateCode Preamble  dot11Rate Rate */
/*   1 Mb */ {  AH_TRUE,  CCK,    1000,    0x1b,    0x00, (0x80| 2),   0 },
/*   2 Mb */ {  AH_TRUE,  CCK,    2000,    0x1a,    0x04, (0x80| 4),   1 },
/* 5.5 Mb */ {  AH_TRUE,  CCK,    5500,    0x19,    0x04, (0x80|11),   1 },
/*  11 Mb */ {  AH_TRUE,  CCK,   11000,    0x18,    0x04, (0x80|22),   1 }
	},
};


/* Venice TODO: roundUpRate() is broken when the rate table does not represent rates
 * in increasing order  e.g.  5.5, 11, 6, 9.    
 * An average rate of 6 Mbps will currently map to 11 Mbps. 
 */
HAL_RATE_TABLE ar5212_11g_table = {
	12,  /* number of rates */
	{ 0 },
	{
/*                                                 short            ctrl  */
/*                valid                rateCode Preamble  dot11Rate Rate */
/*   1 Mb */ {  AH_TRUE, CCK,     1000,    0x1b,    0x00,         2,   0 },
/*   2 Mb */ {  AH_TRUE, CCK,     2000,    0x1a,    0x04,         4,   1 },
/* 5.5 Mb */ {  AH_TRUE, CCK,     5500,    0x19,    0x04,        11,   2 },
/*  11 Mb */ {  AH_TRUE, CCK,    11000,    0x18,    0x04,        22,   3 },
/* Hardware workaround - remove rates 6, 9 from rate ctrl */
/*   6 Mb */ { AH_FALSE, OFDM,    6000,    0x0b,    0x00,        12,   4 },
/*   9 Mb */ { AH_FALSE, OFDM,    9000,    0x0f,    0x00,        18,   4 },
/*  12 Mb */ {  AH_TRUE, OFDM,   12000,    0x0a,    0x00,        24,   6 },
/*  18 Mb */ {  AH_TRUE, OFDM,   18000,    0x0e,    0x00,        36,   6 },
/*  24 Mb */ {  AH_TRUE, OFDM,   24000,    0x09,    0x00,        48,   8 },
/*  36 Mb */ {  AH_TRUE, OFDM,   36000,    0x0d,    0x00,        72,   8 },
/*  48 Mb */ {  AH_TRUE, OFDM,   48000,    0x08,    0x00,        96,   8 },
/*  54 Mb */ {  AH_TRUE, OFDM,   54000,    0x0c,    0x00,       108,   8 }
	},
};

HAL_RATE_TABLE ar5212_xr_table = {
	12,		/* number of rates */
	{ 0 },
	{
/*                                                 short            ctrl  */
/*                valid                rateCode Preamble  dot11Rate Rate */
/* 0.5 Mb */ {  AH_TRUE,   XR,    500,     0x07,    0x00, (0x80| 1),   0 },
/*   1 Mb */ {  AH_TRUE,   XR,   1000,     0x02,    0x00, (0x80|11),   1 },
/*   2 Mb */ {  AH_TRUE,   XR,   2000,     0x06,    0x00, (0x80|22),   2 },
/*   3 Mb */ {  AH_TRUE,   XR,   3000,     0x01,    0x00, (0x80|22),   3 },
/*   6 Mb */ {  AH_TRUE, OFDM,   6000,     0x0b,    0x00, (0x80|12),   4 },
/*   9 Mb */ {  AH_TRUE, OFDM,   9000,     0x0f,    0x00,        18,   4 },
/*  12 Mb */ {  AH_TRUE, OFDM,  12000,     0x0a,    0x00, (0x80|24),   6 },
/*  18 Mb */ {  AH_TRUE, OFDM,  18000,     0x0e,    0x00,        36,   6 },
/*  24 Mb */ {  AH_TRUE, OFDM,  24000,     0x09,    0x00, (0x80|48),   8 },
/*  36 Mb */ {  AH_TRUE, OFDM,  36000,     0x0d,    0x00,        72,   8 },
/*  48 Mb */ {  AH_TRUE, OFDM,  48000,     0x08,    0x00,        96,   8 },
/*  54 Mb */ {  AH_TRUE, OFDM,  54000,     0x0c,    0x00,       108,   8 }
	},
};

#undef	OFDM
#undef	CCK
#undef	TURBO
#undef	XR

const HAL_RATE_TABLE *
ar5212GetRateTable(struct ath_hal *ah, u_int mode)
{
	HAL_RATE_TABLE *rt;
	switch (mode) {
	case HAL_MODE_11A:
		rt = &ar5212_11a_table;
		break;
	case HAL_MODE_11B:
		rt = &ar5212_11b_table;
		break;
	case HAL_MODE_11G:
#ifdef notdef
	case HAL_MODE_PUREG:
#endif
		rt =  &ar5212_11g_table;
		break;
	case HAL_MODE_TURBO:
	case HAL_MODE_108G:
		rt =  &ar5212_turbo_table;
		break;
	case HAL_MODE_XR:
		rt = &ar5212_xr_table;
		break;
	default:
		HALDEBUG(ah, "%s: invalid mode 0x%x\n", __func__, mode);
		return AH_NULL;
	}
	ath_hal_setupratetable(ah, rt);
	return rt;
}
#endif /* AH_SUPPORT_AR5212 */
