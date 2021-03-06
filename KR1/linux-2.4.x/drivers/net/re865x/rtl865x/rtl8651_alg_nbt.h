/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : 
* Abstract : 
* Creator : 
* Author :  
* $Id: rtl8651_alg_nbt.h,v 1.2 2004/04/20 03:44:03 tony Exp $
* $Log: rtl8651_alg_nbt.h,v $
* Revision 1.2  2004/04/20 03:44:03  tony
* if disable define "RTL865X_OVER_KERNEL" and "RTL865X_OVER_LINUX", __KERNEL__ and __linux__ will be undefined.
*
* Revision 1.1  2004/02/25 14:26:33  chhuang
* *** empty log message ***
*
* Revision 1.2  2004/02/25 14:24:52  chhuang
* *** empty log message ***
*
* Revision 1.5  2004/01/09 08:03:21  tony
* make the code architecture of ALG is able to support multiple dial session.
*
* Revision 1.4  2003/10/23 07:10:04  hiwu
* arrange include file sequence
*
* Revision 1.3  2003/10/17 08:45:51  hiwu
* merge new function type
*
* Revision 1.2  2003/10/02 10:40:05  hiwu
* fix header conflict problem
*
* Revision 1.1  2003/10/02 07:08:11  hiwu
* initial version
*
*/

#ifndef _RTL8651_ALG_NBT
#define _RTL8651_ALG_NBT 


int32 rtl8651_l4AliasHandleClientUdpNbtOut(struct rtl_pktHdr *, struct ip *, struct rtl8651_tblDrv_naptTcpUdpFlowEntry_s *);
int32 rtl8651_l4AliasHandleClientUdpNbtIn(struct rtl_pktHdr *,struct ip *,struct rtl8651_tblDrv_naptTcpUdpFlowEntry_s *);
int32 rtl8651_l4AliasHandleClientUdpNbtNSOut(struct rtl_pktHdr *, struct ip *, struct rtl8651_tblDrv_naptTcpUdpFlowEntry_s *);
int32 rtl8651_l4AliasHandleClientUdpNbtNSIn(struct rtl_pktHdr *,struct ip *, struct rtl8651_tblDrv_naptTcpUdpFlowEntry_s *);	

#endif /* _RTL8651_ALG_NBT */
