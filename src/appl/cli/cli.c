/*
 * $Id: cli.c,v 1.5 Broadcom SDK $
 *
 * $Copyright: Copyright 2016 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 */
 
#ifdef __C51__
#ifdef CODE_USERCLASS
#pragma userclass (code = cli)
#endif /* CODE_USERCLASS */
#endif /* __C51__ */

#include "system.h"
#include "utils/ui.h"
#include "appl/cli.h"
#include "utils/net.h"
#include "boardapi/vlan.h"
#include "appl/mdns.h"
#include "utils/factory.h"
#include "appl/persistence.h"
#include "arm/mcu.h"
#include "appl/igmpsnoop.h"
#include "boardapi/mcast.h"
#if CFG_CLI_ENABLED
#define BUFLEN  64
static const uint32 Timeout=100000; 
static const uint8 commandhead=0xeb;
static const uint8 commandaddr=0x5f;
static const uint8 Errnocom=0xF1;//no this command
static const uint8 Errdataerr=0xF2;//data error
static const uint8 Errlenerr=0xF7;//data len error
static const uint8 Errtimeout=0xF8;//timeout

static uint8 liguonet_ipaddr[4];
static uint8 liguonet_netmask[4];
static uint8 liguonet_gateway[4];
#define commandbuflen BUFLEN-5
enum
{
	liguoA_command_head,
	liguoA_command_addr,
	liguoA_command_id,
	liguoA_command_len,
	liguoA_command_data,
};
struct commandbuf
{
	uint8 index;
	BOOL  flag;
}BufStatus;
typedef struct
{
	uint8 maj;
	uint8 min;
	uint8 build;
}SorftWare;
static  SorftWare sorftware={1,2,7};
typedef struct command_buf_s
{
	uint8 CommandHead;
	uint8 CommandAddr;
	uint8 CommandID;
	uint8 CommandLen;
	uint8 CommandData[commandbuflen];
	uint8 CommandCrc;	
}command_buf;
command_buf rx_Command,tx_Command;
static uint8 CommandStatus;
#define Com26Start 3
///////////////////////////////////////////////////
void StartRecCom();
void EndRecCom();
void SendMessage();
uint8 CheckCrc(command_buf com);
void Timeouthandler();
void CommandErrHandler();
BOOL CheckPortID(uint8 port);
void DataProd(uint8 i,const uint8* val,uint8 len,BOOL flag);
void uint82uint32(const uint8 *src,uint32 *des,uint8 len);
void uint8touint16(const uint8 *src,uint16 *des,uint8 len);
void uint322uint8(const uint32 *src,uint8 *des,uint8 len);
void uint162uint8(const uint16 *src,uint8 *des,uint8 len);
///////////////////////////////////////////////////
void commandbuf_add();
void CommandFormat();
void CommandIDHandler0x00();
void CommandIDHandler0x80();
void CommandIDHandler0x80_0x26();
void CommandIDHandler0x80_0x26_0x00();
void GetSorftwareVer();
void GetBuildtime();
void GetFirewareVer();
void GetMacAddress();
void GetIPv6();
void GetIPv4();
void GetSubMask();
void GetGateway();
void GetDHCPStatus();
void GetDevicePortNum();
void GetDeviceName();
void GetBonjourStatus();
void GetPasswd();
///////////////////////////////////////////////////////

void CommandIDHandler0x80_0x26_0x01();
void GetValue(uint8 index,uint8 * val,uint8 len);
void SetMacAddress();
void SetIPv6();
void SetIPv4();
void SetSubMask();
void SetGateway();
void SetDHCPStatus();
void SetBonjourStatus();
void SetPasswd();
void SetReboot();
void SetFactory();
///////////////////////////////////////////////////////
void CommandIDHandler0x80_0x26_0x03();
void GetPortEnableStatus();
void SetPortEnableStatus();
void GetPortLinkStatus();
void GetPortDuplexMode();
void SetPortDuplexMode();
void GetPortAutonegoStatus();
void SetPortAutonegoStatus();
void GetPortStomCtrStatus();
void SetPortStomCtrStatus();
void GetPortRevPackets();
void GetPortSenPackets();
void GetQVlanID();
void SetQVlanID();
void GetPortInputLimit();
void SetPortInputLimit();
void GetPortOutputLimit();
void SetPortOutputLimit();
void GetPortSpeed();
void SetPortSpeed();
void GetPortFlowCtrStatus();
///////////////////////////////////////////////////////
void CommandIDHandler0x80_0x26_0x04();
void GreateVlan();
void DestoryVlan();
void GetVlanMember();
void SetVlanMember();
void GetVlanCounts();
void GetVlanByIndex();
///////////////////////////////////////////////////////
void CommandIDHandler0x80_0x26_0x05();
void GetIGMPSnoopingStatus();
void SetIGMPSnoopingStatus();
void GetIGMPSnoopingVlan();
void SetIGMPSnoopingVlan();
void GetUnknowMuticaseStatus();
void SetUnknowMuticaseStatus();
///////////////////////////////////////////////////
void CommandIDHandler0x80_0x26_0xA6();
void SetDeviceName();
void GetMemVal();
void SetMemVal();
void GetPhyRegVal();
void SetPhyRegVal();
void GetSwitchRegVal();
void SetSwitchRegVal();
void GetTable();
void SetTable();
///////////////////////////////////////////////////////////////////
#define BUILD_YEAR ((__DATE__[7]-'0')*1000+(__DATE__[8]-'0')*100+(__DATE__[9]-'0')*10+(__DATE__[10]-'0'))

/*
#if LIGUO_DATE==32
	#define	BUILD_DATE((__DATE__[5]-'0'))
#else
	#define BUILD_DATE ((__DATE__[4]-'0')*10+(__DATE__[5]-'0'))
#endif*/
#define BUILD_HOUR ((__TIME__[0]-'0')*10+(__TIME__[1]-'0'))
#define BUILD_MIN ((__TIME__[3]-'0')*10+(__TIME__[4]-'0'))
#define BUILD_SEC ((__TIME__[6]-'0')*10+(__TIME__[7]-'0'))
uint8 Liguo2Port[17]={9,8,7,6,5,4,3,2,1,23,24,22,21,20,19,18,17};
///////////////////////////////////////////////////
extern void APIFUNC(cli_init)(void) REENTRANT;
static uint8 BUILD_MONTH=0;
static uint8 BUILD_DATE=0;
void GetMonth()
{
	if((__DATE__[0] =='J')&& (__DATE__[1] =='a') && (__DATE__[2] =='n'))
		BUILD_MONTH=1;
	else if((__DATE__[0] =='F')&& (__DATE__[1] =='e')&& (__DATE__[2] =='b'))
		BUILD_MONTH=2;
	else if((__DATE__[0] =='M')&& (__DATE__[1] =='a')&& (__DATE__[2] =='r'))
		BUILD_MONTH=3;
	else if((__DATE__[0] =='A')&& (__DATE__[1] =='p')&& (__DATE__[2] =='r'))
		BUILD_MONTH=4;
	else if((__DATE__[0] =='M')&& (__DATE__[1] =='a')&& (__DATE__[2] =='y'))
		BUILD_MONTH=5;
	else if((__DATE__[0] =='J')&& (__DATE__[1] =='u')&& (__DATE__[2] =='n'))
		BUILD_MONTH=6;
	else if((__DATE__[0] =='J')&& (__DATE__[1] =='u')&& (__DATE__[2] =='l'))
		BUILD_MONTH=7;
	else if((__DATE__[0] =='A')&& (__DATE__[1] =='u')&& (__DATE__[2] =='g'))
		BUILD_MONTH=8;
	else if((__DATE__[0] =='S')&& (__DATE__[1] =='e')&& (__DATE__[2] =='p'))
		BUILD_MONTH=9;
	else if((__DATE__[0] =='O')&& (__DATE__[1] =='c')&& (__DATE__[2] =='t'))
		BUILD_MONTH=10;
	else if((__DATE__[0] =='N')&& (__DATE__[1] =='o')&& (__DATE__[2] =='v'))
		BUILD_MONTH=11;
	else if((__DATE__[0] =='D')&& (__DATE__[1] =='e')&& (__DATE__[2] =='c'))
		BUILD_MONTH=12;
	else
		BUILD_MONTH=1;
	if(__DATE__[4]==' ')
		BUILD_DATE=(__DATE__[5]-'0');
	else
		BUILD_DATE= ((__DATE__[4]-'0')*10+(__DATE__[5]-'0'));
}

void APIFUNC(cli)(void) REENTRANT
{
    uint8 val;
    tx_Command.CommandHead=commandhead;    
    tx_Command.CommandAddr=commandaddr; 
    GetMonth();
    for(;;) 
    {
    	val=get_char();
	if(commandhead==val)
	{
		rx_Command.CommandHead=val;		
		StartRecCom();
	}
	else
	{
		if(BufStatus.index)
		{
			switch(BufStatus.index)
			{
				case liguoA_command_addr:
					rx_Command.CommandAddr=val;
					break;
				case liguoA_command_id:
					rx_Command.CommandID=val;
					tx_Command.CommandID=val;
					break;
				case liguoA_command_len:
					rx_Command.CommandLen=val;
					break;
				default:
					if(BufStatus.index-4<rx_Command.CommandLen)
					{
						rx_Command.CommandData[BufStatus.index-4]=val;
					}
					else if((BufStatus.index-4)==rx_Command.CommandLen)
					{
						EndRecCom();
						rx_Command.CommandCrc=val;
						if(0xA5==rx_Command.CommandCrc||rx_Command.CommandCrc==CheckCrc(rx_Command))
						{
							CommandFormat();
						}
						else
						{
							CommandStatus=Errdataerr;
						}
						if(CommandStatus)
						{
							CommandErrHandler();
						}
						SendMessage();
					}
			}	
		}
		else
		{
			continue;
		}	
	}
	commandbuf_add();
    }
}

BOOL APIFUNC(cli_add_cmd)(char cmd, CLI_CMD_FUNC func) REENTRANT
{
    return FALSE;
}

void APIFUNC(cli_remove_cmd)(char cmd) REENTRANT
{
   
}

void APIFUNC(cli_init)(void) REENTRANT
{
	
}
////////////////////////////////////////
void StartRecCom()
{
	BufStatus.flag=FALSE;
	BufStatus.index=0;
	CommandStatus=0;
	timer_add(Timeouthandler,NULL,Timeout);
}
void EndRecCom()
{
	timer_remove(Timeouthandler);
}
////////////////////////////////////////
void commandbuf_add()
{	
	BufStatus.index++;
}
uint8 CheckCrc(command_buf com)
{
	uint8 val=0,i;
	val=com.CommandAddr+com.CommandID+com.CommandLen;
	for(i=0;i<com.CommandLen;i++)
	{
		val+=com.CommandData[i];
	}
	return val==0xeb?0x14:val;
}
void CommandFormat()
{
	switch(rx_Command.CommandID)
	{
		case 0x80:
			CommandIDHandler0x80();
			break;
		default:
			CommandStatus=Errnocom;
			break;
	}
}
BOOL CheckPortID(uint8 port)
{
	if(port>0&&port<25)
	{
		return TRUE;
	}
	return FALSE;
}
void DataProd(uint8 i,const uint8 *val ,uint8 len,BOOL flag)
{
	int n;
	for(n=0;n<len;n++)
	{
		if(flag)
		{
			tx_Command.CommandData[i]=val[n]&0xF0;
			i++;
			tx_Command.CommandData[i]=val[n]&0x0F;	
		}
		else
		{
			tx_Command.CommandData[i]=val[n];
		}
		i++;
	}
	tx_Command.CommandLen=i;
}
void uint82uint32(const uint8 *src,uint32 *des,uint8 len)
{
	uint8 i,n;
	for(i=0;i<(len/sizeof(uint32));i++)
	{
		*(des+i)=0;
		for(n=0;n<sizeof(uint32);n++)
		{
			*(des+i)+=((*(src+(i*sizeof(uint32)+n)))<<(8*(3-n)));
		}
	}
}
void uint82uint16(const uint8 *src,uint16 *des,uint8 len)
{
	uint8 i,n;
	for(i=0;i<(len/sizeof(uint16));i++)
	{
		*(des+i)=0;
		for(n=0;n<sizeof(uint16);n++)
		{
			*(des+i)+=((*(src+(i*sizeof(uint16)+n)))<<(8*(1-n)));
		}
	}
}
void uint322uint8(const uint32 *src,uint8 *des,uint8 len)
{
	uint8 i,n;
	for(i=0;i<(len/sizeof(uint32));i++)
	{
		for(n=0;n<sizeof(uint32);n++)
		{
			*(des+((i*sizeof(uint32)+n)))=(*(src+i))/(1<<(8*(3-n)));
		}
	}
}
void uint162uint8(const uint16 *src,uint8 *des,uint8 len)
{
	uint8 i,n;
        for(i=0;i<(len/sizeof(uint16));i++)
        {
                for(n=0;n<sizeof(uint16);n++)
                {
                        *(des+((i*sizeof(uint16)+n)))=*(src+i)/(1<<(8*(1-n)));
		}
        }

}
void CommandIDHandler0x80()
{
	if(rx_Command.CommandLen<Com26Start)
	{	
		CommandStatus=Errlenerr;
		return ;
	}
	tx_Command.CommandData[0]=rx_Command.CommandData[0];
	tx_Command.CommandData[1]=rx_Command.CommandData[1];
	tx_Command.CommandData[2]=rx_Command.CommandData[2];
	switch(rx_Command.CommandData[0])
	{
		case 0x26:
			CommandIDHandler0x80_0x26();
			break;
		default:
			CommandStatus=Errnocom;
			break;
			
	}
}
void CommandIDHandler0x80_0x26()
{	
	switch(rx_Command.CommandData[1])
	{
		case 0x00:
			CommandIDHandler0x80_0x26_0x00();			
			break;
		case 0x01:
			CommandIDHandler0x80_0x26_0x01();
			break;
		case 0x03:
			CommandIDHandler0x80_0x26_0x03();
			break;
		case 0x04:
			CommandIDHandler0x80_0x26_0x04();
			break;
		case 0x05:
			CommandIDHandler0x80_0x26_0x05();
			break;
		case 0xA6:
			CommandIDHandler0x80_0x26_0xA6();
			break;
		default:
			CommandStatus=Errnocom;
			break;
	}
}
void CommandIDHandler0x80_0x26_0x00()
{
	switch(rx_Command.CommandData[2])
	{
		case 0x00:
			GetSorftwareVer();
			break;
		case 0x01:
			GetBuildtime();
			break;
		case 0x02:
			GetFirewareVer();
			break;
		case 0x03:
			GetMacAddress();
			break;
		case 0x04:
			GetIPv6();
			break;
		case 0x05:
			GetIPv4();
			break;
		case 0x06:
			GetSubMask();
			break;
		case 0x07:
			GetGateway();
			break;
		case 0x08:
			GetDHCPStatus();
			break;
		case 0x09:
			GetDevicePortNum();
			break;
		case 0x0A:
			GetDeviceName();
			break;
		case 0x0B:
			GetBonjourStatus();
			break;
		case 0x0C:
			GetPasswd();
			break;
		default:
			CommandStatus=Errnocom;
			
	}
}
void GetSorftwareVer()
{
	if(3!=rx_Command.CommandLen)
        {
                CommandStatus=Errlenerr;
                return ;
        }
	uint8 data[3];
	sal_memcpy(data,(void *)&sorftware,sizeof(sorftware));
	DataProd(Com26Start,data,sizeof(data),TRUE);
}

void GetBuildtime()
{
	if(3!=rx_Command.CommandLen)
        {
                CommandStatus=Errlenerr;
                return ;
        }
	uint8 data[7];
	data[0]=BUILD_YEAR/100;
	data[1]=BUILD_YEAR%100;
	
	data[2]=BUILD_MONTH;
	data[3]=BUILD_DATE;

	data[4]=BUILD_HOUR;
	data[5]=BUILD_MIN;
	data[6]=BUILD_SEC;
	DataProd(3,data,sizeof(data),FALSE);
}
void GetFirewareVer()
{
	if(3!=rx_Command.CommandLen)
        {
                CommandStatus=Errlenerr;
                return ;
        }
	uint8 data[3];
	
	data[0]=CFE_VER_MAJOR;
	
	data[1]=CFE_VER_MINOR;

	data[2]=CFE_VER_BUILD;
	DataProd(Com26Start,data,sizeof(data),TRUE);
}
void GetMacAddress()
{
	if(3!=rx_Command.CommandLen)
        {
                CommandStatus=Errlenerr;
                return ;
        }
	uint8 MacAddr[6];
	sys_error_t err=0;
	err=get_system_mac(MacAddr);
	if(err!=SYS_OK)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	DataProd(Com26Start,MacAddr,sizeof(MacAddr),TRUE);
	
}
void GetIPv6()
{
	if(4!=rx_Command.CommandLen)
        {
                CommandStatus=Errlenerr;
                return ;
        }
	uint8 data[16],type;
	sys_error_t err=0;
	err=get_ipv6_address(rx_Command.CommandData[3],(uip_ip6addr_t*)&data,&type);
	if(err!=SYS_OK)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	sal_printf("The type is %d\n",type);
	tx_Command.CommandData[3]=rx_Command.CommandData[3];
	DataProd(4,data,sizeof(data),TRUE);
}
void GetIPv4()
{
	if(3!=rx_Command.CommandLen)
        {
                CommandStatus=Errlenerr;
                return ;
        }
	get_network_interface_config(liguonet_ipaddr,liguonet_netmask,liguonet_gateway);
	DataProd(Com26Start,liguonet_ipaddr,sizeof(liguonet_ipaddr),TRUE);
}
void GetSubMask()
{
	if(3!=rx_Command.CommandLen)
        {
                CommandStatus=Errlenerr;
                return ;
        }
     	get_network_interface_config(liguonet_ipaddr,liguonet_netmask,liguonet_gateway);
        DataProd(Com26Start,liguonet_netmask,sizeof(liguonet_netmask),TRUE);

}
void GetGateway()
{
	if(3!=rx_Command.CommandLen)
        {
                CommandStatus=Errlenerr;
                return ;
        }
        get_network_interface_config(liguonet_ipaddr,liguonet_netmask,liguonet_gateway);
        DataProd(3,liguonet_gateway,sizeof(liguonet_gateway),TRUE);

}
void GetDHCPStatus()
{
	if(3!=rx_Command.CommandLen)
        {
                CommandStatus=Errlenerr;
                return ;
        }
       	uint8 err=0;
        err=get_network_interface_config(liguonet_ipaddr,liguonet_netmask,liguonet_gateway);
        DataProd(3,&err,sizeof(err),FALSE);

}
void GetDevicePortNum()
{
	if(3!=rx_Command.CommandLen)
	{
		CommandStatus=Errlenerr;
		return ;
	}
	uint8 num;
	num=board_uport_count();
	DataProd(3,&num,sizeof(num),FALSE);
}
void GetDeviceName()
{
	if(3!=rx_Command.CommandLen)
	{
		CommandStatus=Errlenerr;
		return ;
	}
	char name[MAX_SYSTEM_NAME_LEN+1];
	sys_error_t err=0;
	err=get_system_name(name,sizeof(name));
	if(err!=SYS_OK)
	{
		CommandStatus=Errdataerr;
		return;
	}
	name[MAX_SYSTEM_NAME_LEN+1]='\0';
	DataProd(3,(uint8 *)&name,sal_strlen(name),FALSE);
}

void GetBonjourStatus()
{

	if(3!=rx_Command.CommandLen)
	{
		CommandStatus=Errlenerr;
		return ;
	}
	uint8 bonjour=0;
#ifndef __BOOTLOADER__
	mdns_bonjour_enable_get(&bonjour);
#endif
	DataProd(3,&bonjour,sizeof(bonjour),FALSE);
}
void GetPasswd()
{
	if(3!=rx_Command.CommandLen)
	{
		CommandStatus=Errlenerr;
		return ;
	}
	sys_error_t err=0;
	char lpwd[MAX_USERNAME_LEN+1];
#ifndef __BOOTLOADER__
	err=get_login_password(lpwd,sizeof(lpwd));
#endif
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	DataProd(3,(uint8 *)&lpwd,sal_strlen(lpwd),FALSE);
}
//////////////////////////////////////////////////////////////////////////
void CommandIDHandler0x80_0x26_0x01()
{
	switch(rx_Command.CommandData[2])
	{
		case 0x00:
			SetMacAddress();
			break;
		case 0x01:
			SetIPv6();
			break;
		case 0x02:
			SetIPv4();
			break;
		case 0x03:
			SetSubMask();
			break;
		case 0x04:
			SetGateway();
			break;
		case 0x05:
			SetDHCPStatus();
			break;
		case 0x06:
			SetBonjourStatus();
			break;
		case 0x07:
			SetPasswd();
			break;
		case 0x08:
			SetReboot();
			break;
		case 0x09:
			SetFactory();
			break;
		default:
			CommandStatus=Errnocom;			
	}
	tx_Command.CommandLen=0x04;
	tx_Command.CommandData[3]=0xFA;
}
void GetValue(uint8 index,uint8 * val,uint8 len)
{
	uint8 i;
	for(i=0;i<len;i++)
	{
		val[i]=rx_Command.CommandData[2*i+index];
		val[i]+=rx_Command.CommandData[2*i+index+1];
	}
}
void SetMacAddress()
{
	if(0x0F!=rx_Command.CommandLen)
	{
		CommandStatus=Errlenerr;
		return ;
	}
	factory_config_t cfg;
	sys_error_t err;
	uint8 mac[6];
	GetValue(3,mac,sizeof(mac));
	sal_memcpy(cfg.mac,mac,sizeof(mac));
	err=factory_config_set(&cfg);
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return ;
	}
}
void SetIPv6()
{
	if(0x23!=rx_Command.CommandLen)
	{
		CommandStatus=Errlenerr;
		return ;
	}
	uint8 add[16];
	sys_error_t err;
	GetValue(3,add,sizeof(add));
	err=set_manual_ipv6_address((uip_ip6addr_t* )&add);
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return ;
	}
}
void SetIPv4()
{
	uint8 add[4];
	if(0x0B!=rx_Command.CommandLen)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	GetValue(3,add,sizeof(add));
	sys_error_t err;
	get_network_interface_config(liguonet_ipaddr,liguonet_netmask,liguonet_gateway);

	err=set_network_interface_config(INET_CONFIG_STATIC,add,liguonet_netmask,liguonet_gateway);
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return ;
	}
}
void SetSubMask()
{
	if(0x0B!=rx_Command.CommandLen)
	{
		CommandStatus=Errlenerr;
		return;
	}
	uint8 mask[4];
	GetValue(3,mask,sizeof(mask));
	sys_error_t err;
	get_network_interface_config(liguonet_ipaddr,liguonet_netmask,liguonet_gateway);
	err=set_network_interface_config(INET_CONFIG_STATIC,liguonet_ipaddr,mask,liguonet_gateway);
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return;
	}
}
void SetGateway()
{
	if(0x0B!=rx_Command.CommandLen)
        {
                CommandStatus=Errlenerr;
                return;
        }
        uint8 gate[4];
        GetValue(3,gate,sizeof(gate));
        sys_error_t err;
	get_network_interface_config(liguonet_ipaddr,liguonet_netmask,liguonet_gateway);
        err=set_network_interface_config(INET_CONFIG_STATIC,liguonet_ipaddr,liguonet_netmask,gate);
        if(SYS_OK!=err)
        {
                CommandStatus=Errdataerr;
                return;
        }

}
void SetDHCPStatus()
{
	if(0x04!=rx_Command.CommandLen)
	{
		CommandStatus=Errlenerr;
		return ;
	}
	sys_error_t err;
	INET_CONFIG config;
	if(0==rx_Command.CommandData[3])
	{
		config=INET_CONFIG_STATIC;
	}
	else
	{
		config=INET_CONFIG_DHCP_FALLBACK;
	}
	get_network_interface_config(liguonet_ipaddr,liguonet_netmask,liguonet_gateway);
	err=set_network_interface_config(config,liguonet_ipaddr,liguonet_netmask,liguonet_gateway);
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return ;
	}
}
void SetBonjourStatus()
{
	 if(4!=rx_Command.CommandLen)
        {
                CommandStatus=Errlenerr;
                return ;
        }
#ifndef __BOOTLOADER__
        uint8 bonjour=rx_Command.CommandData[3];
        mdns_bonjour_enable_set((BOOL)bonjour);
#endif

}
void SetPasswd()
{
	if(rx_Command.CommandLen<4||rx_Command.CommandLen>23)
	{
		CommandStatus=Errlenerr;
		return;
	}
	char passwd[MAX_PASSWORD_LEN+1];
	sal_memcpy((void *)&passwd,&rx_Command.CommandData[3],rx_Command.CommandLen-3);
	passwd[rx_Command.CommandLen-3]=0;
	sys_error_t err=0;
#ifndef __BOOTLOADER__
	err=set_login_password(passwd);
	persistence_save_current_settings("password");
#endif
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return;
	}
}
void SetReboot()
{
	if(0x03!=rx_Command.CommandLen)
	{
		CommandStatus=Errlenerr;
		return;
	}
	uint8 val=1;
	board_reset((void *)&val);
}
void SetFactory()
{
	if(0x03!=rx_Command.CommandLen)
	{
		CommandStatus=Errlenerr;
		return;
	}
	uint8 val=1;
#ifndef __BOOTLOADER__
	persistence_restore_factory_defaults();
	persistence_save_all_current_settings();
#endif
	board_reset((void *)&val);
}
void CommandIDHandler0x80_0x26_0x03()
{
	if(!CheckPortID(rx_Command.CommandData[Com26Start]))
	{
		CommandStatus=Errdataerr;
		return ;
	}
	switch(rx_Command.CommandData[2])
	{
		case 0x00:
			GetPortEnableStatus();
			break;
		case 0x01:
			SetPortEnableStatus();
			break;
		case 0x02:
			GetPortLinkStatus();
			break;
		case 0x03:
			GetPortDuplexMode();
			break;
		case 0x04:
			SetPortDuplexMode();
			break;
		case 0x05:
			GetPortAutonegoStatus();
			break;
		case 0x06:
			SetPortAutonegoStatus();
			break;
		case 0x07:
			GetPortStomCtrStatus();
			break;
		case 0x08:
			SetPortStomCtrStatus();
			break;
		case 0x09:
			GetPortRevPackets();
			break;
		case 0x0A:
			GetPortSenPackets();
			break;
		case 0x0B:
			GetQVlanID();
			break;
		case 0x0C:
			SetQVlanID();
			break;
		case 0x0D:
			GetPortInputLimit();
			break;
		case 0x0E:
			SetPortInputLimit();
			break;
		case 0x0F:
			GetPortOutputLimit();
			break;
		case 0x10:
			SetPortOutputLimit();
			break;
		case 0x11:
			GetPortSpeed();
			break;
		case 0x12:
			SetPortSpeed();
			break;
		case 0x13:
			GetPortFlowCtrStatus();
			break;
		default:
			CommandStatus=Errnocom;
	}
}
void GetPortEnableStatus()
{
	if(rx_Command.CommandLen!=0x04)
	{
		CommandStatus=Errlenerr;
		return ;
	}
	BOOL status;
	sys_error_t err;
	err=board_port_enable_get(rx_Command.CommandData[Com26Start],&status);
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	else
	{	
		uint8 index=Com26Start;
		tx_Command.CommandData[index++]=rx_Command.CommandData[Com26Start];
		tx_Command.CommandData[index++]=(uint8)status;
		tx_Command.CommandLen=index;
	}
	
}
void SetPortEnableStatus()
{
	if(rx_Command.CommandLen!=0x05)
	{
		CommandStatus=Errlenerr;
		return ;
	}
	sys_error_t err;
	err=board_port_enable_set((uint16)rx_Command.CommandData[Com26Start],(BOOL)rx_Command.CommandData[Com26Start+1]);
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return;
	}
	tx_Command.CommandLen=0x04;
	tx_Command.CommandData[Com26Start]=0xFA;
}
void GetPortLinkStatus()
{
	if(rx_Command.CommandLen!=0x04)	
	{
		CommandStatus=Errlenerr;
		return ;
	}
	sys_error_t err;
	BOOL status;
	err=board_get_port_link_status(rx_Command.CommandData[Com26Start],&status);
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return;
	}
	uint8 index=Com26Start;
	tx_Command.CommandData[index]=rx_Command.CommandData[index];
	index++;
	tx_Command.CommandData[index++]=status;
	tx_Command.CommandLen=index;
}
void GetPortDuplexMode()
{
	if(rx_Command.CommandLen!=0x04)
	{
		CommandStatus=Errlenerr;
		return ;
	}
	uint8 index=Com26Start,lport,unit;
	int duplex,link,ad;
	board_uport_to_lport(rx_Command.CommandData[index],&unit,&lport);
	lport=SOC_PORT_L2P_MAPPING(lport);
	PHY_LINK_GET(BMD_PORT_PHY_CTRL(unit,lport),&link,&ad);
	PHY_DUPLEX_GET(BMD_PORT_PHY_CTRL(unit,lport),&duplex);
	tx_Command.CommandData[index]=rx_Command.CommandData[index];
	index++;
	if(link)
	{
		tx_Command.CommandData[index++]=duplex;
	}
	else
	{
		tx_Command.CommandData[index++]=0x02;
	}
	tx_Command.CommandLen=index;
}
void SetPortDuplexMode()
{
	if(rx_Command.CommandLen!=0x05)
	{
		CommandStatus=Errlenerr;
		return;
	}
	uint8 index=Com26Start,lport,unit;
	int duplex,link,ad;
	duplex=rx_Command.CommandData[index+1];
	board_uport_to_lport(rx_Command.CommandData[index],&unit,&lport);
	lport=SOC_PORT_L2P_MAPPING(lport);
	PHY_LINK_GET(BMD_PORT_PHY_CTRL(unit,lport),&link,&ad);
//	if(!link)
//	{
//		CommandStatus=Errdataerr;
//		return ;
//	}
	PHY_AUTONEG_SET(BMD_PORT_PHY_CTRL(unit,lport),0);
	PHY_DUPLEX_SET(BMD_PORT_PHY_CTRL(unit,lport),duplex);
	tx_Command.CommandData[index]=rx_Command.CommandData[index];
	index++;
	tx_Command.CommandData[index++]=0xFA;
	tx_Command.CommandLen=index;
}
void GetPortAutonegoStatus()
{
	if(rx_Command.CommandLen!=0x04)
	{
		CommandStatus=Errlenerr;
		return ;
	}
	uint16 uport;
	BOOL an;
	sys_error_t err;
	uint8 index=Com26Start,lport;
	uport=rx_Command.CommandData[index];
	int link,ad;
	uint8 unit;
	board_uport_to_lport(uport,&unit,&lport);
	lport=SOC_PORT_P2L_MAPPING(lport);
	PHY_LINK_GET(BMD_PORT_PHY_CTRL(unit,lport),&link,&ad);
	lport=rx_Command.CommandData[index];
	err=board_port_an_get(lport,&an);
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	tx_Command.CommandData[index]=rx_Command.CommandData[index];
	index++;
	if(!link)
        {
                tx_Command.CommandData[index++]=0x02;
        }
	else
	{
		tx_Command.CommandData[index++]=an;
	}
	tx_Command.CommandLen=index;
}
void SetPortAutonegoStatus()
{
	if(rx_Command.CommandLen!=0x05)
        {
                CommandStatus=Errlenerr;
                return ;
        }
	uint8 unit,lport,uport,index=Com26Start;
	int	an=rx_Command.CommandData[index+1];
	uport=rx_Command.CommandData[index];
	board_uport_to_lport((uint16)uport,&unit,&lport);
	lport=SOC_PORT_P2L_MAPPING(lport);
	PHY_AUTONEG_SET(BMD_PORT_PHY_CTRL(unit,lport),an);
	tx_Command.CommandData[index++]=uport;
	tx_Command.CommandData[index++]=0xFA;
	tx_Command.CommandLen=index;
}
void GetPortStomCtrStatus()
{
#ifdef CFG_SWITCH_RATE_INCLUDED
	if(rx_Command.CommandLen!=0x04)
        {
                CommandStatus=Errlenerr;
                return ;
        }
	uint8 index=Com26Start;
	uint16 uport=rx_Command.CommandData[index];
	sys_error_t err;
	uint32 bits;
	err=board_rate_get(uport,&bits);
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	tx_Command.CommandData[index]=rx_Command.CommandData[index];
	index++;
	uint8 value[4];
	uint322uint8(&bits,value,sizeof(bits));
	DataProd(index,value,sizeof(value),TRUE);	
#endif
}
void SetPortStomCtrStatus()
{
#ifdef CFG_SWITCH_RATE_INCLUDED
	if(rx_Command.CommandLen!=0x0C)
        {
                CommandStatus=Errlenerr;
                return ;
        }
	uint8 index=Com26Start;
	uint8 uport=rx_Command.CommandData[index];
	uint8 value[4];
	uint32 bits;
	GetValue(index+1,value,sizeof(value));
	uint82uint32(value,&bits,sizeof(value));
	sys_error_t err;
	err=board_rate_set(uport,bits);
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	tx_Command.CommandData[index++]=uport;
	tx_Command.CommandData[index++]=0xFA;
	tx_Command.CommandLen=index;
#endif
}
void GetPortRevPackets()
{
	if(rx_Command.CommandLen!=0x04)
        {
                CommandStatus=Errlenerr;
                return ;
        }
	uint32 counts;
	uint8 value[4];
	uint8 uport,lport,index=Com26Start,unit;
	uport=rx_Command.CommandData[index];
	board_uport_to_lport((uint16)uport,&unit,&lport);
	lport=SOC_PORT_L2P_MAPPING(lport);
	bcm5333x_reg_get(0,SOC_PORT_BLOCK(lport),R_GRBYT(SOC_PORT_BLOCK_INDEX(lport)),&counts);
	uint322uint8(&counts,value,sizeof(uint32));
	tx_Command.CommandData[index++]=uport;
	DataProd(index,value,sizeof(value),TRUE);

}
void GetPortSenPackets()
{
	if(rx_Command.CommandLen!=0x04)
        {
                CommandStatus=Errlenerr;
                return ;
        }
        uint32 counts;
        uint8 value[4];
        uint8 uport,lport,index=Com26Start,unit;
        uport=rx_Command.CommandData[index];
        board_uport_to_lport((uint16)uport,&unit,&lport);
        lport=SOC_PORT_L2P_MAPPING(lport);
        bcm5333x_reg_get(0,SOC_PORT_BLOCK(lport),R_GTBYT(SOC_PORT_BLOCK_INDEX(lport)),&counts);
	uint322uint8(&counts,value,sizeof(uint32));
        uint322uint8(&counts,value,sizeof(uint32));
        tx_Command.CommandData[index++]=uport;
        DataProd(index,value,sizeof(value),TRUE);

}
void GetQVlanID()
{
	uint16 portID,vlanid;
	sys_error_t err;
	if(0x04!=rx_Command.CommandLen)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	portID=rx_Command.CommandData[Com26Start];
	err=board_untagged_vlan_get(portID,&vlanid);
	if(SYS_OK==err)
	{
		tx_Command.CommandLen=0x08;
		tx_Command.CommandData[3]=rx_Command.CommandData[Com26Start];	
		tx_Command.CommandData[4]=(vlanid/256)&0xF0;
		tx_Command.CommandData[5]=(vlanid/256)&0x0F;
		
		tx_Command.CommandData[6]=(vlanid%256)&0xF0;
		tx_Command.CommandData[7]=(vlanid%256)&0x0F;
	}
	else
	{
		CommandStatus=Errdataerr;
	}
	
}
void SetQVlanID()
{
	uint16 vlanid;
	sys_error_t err;
	if(0x08!=rx_Command.CommandLen)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	vlanid=(rx_Command.CommandData[4]+rx_Command.CommandData[5])*256+rx_Command.CommandData[6]+rx_Command.CommandData[7];
	if(vlanid<1||vlanid>4096)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	err=board_untagged_vlan_set(rx_Command.CommandData[Com26Start],vlanid);
	if(SYS_OK==err)
	{
		tx_Command.CommandLen=0x04;
			
	
		tx_Command.CommandData[3]=0xFA;
		
	
	}
	else
	{
		CommandStatus=Errdataerr;
	}
}
void GetPortInputLimit()
{
#ifdef CFG_SWITCH_RATE_INCLUDED
	if(rx_Command.CommandLen!=0x04)
        {
                CommandStatus=Errlenerr;
                return ;
        }
        uint8 uport,value[4],index=Com26Start;
        uint32 pps;
        uport=rx_Command.CommandData[index];
        GetValue(index+1,value,sizeof(value));
        uint82uint32(value,&pps,sizeof(value));
        sys_error_t err;
        err=board_port_rate_ingress_get((uint16) uport,&pps);
        if(SYS_OK!=err)
        {
                CommandStatus=Errdataerr;
                return;
        }
	uint322uint8(&pps,value,sizeof(pps));
        tx_Command.CommandData[index++]=uport;
        DataProd(index,value,sizeof(value),TRUE);
#endif
}
void SetPortInputLimit()
{
#ifdef CFG_SWITCH_RATE_INCLUDED
	if(rx_Command.CommandLen!=0x0C)
        {
                CommandStatus=Errlenerr;
                return ;
        }
	uint8 uport,value[4],index=Com26Start;
	uint32 pps;
	uport=rx_Command.CommandData[index];
	GetValue(index+1,value,sizeof(value));
	uint82uint32(value,&pps,sizeof(value));
	sys_error_t err;
	err=board_port_rate_ingress_set((uint16) uport,pps);
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return;
	}
	tx_Command.CommandData[index++]=uport;
	tx_Command.CommandData[index++]=0xFA;
	tx_Command.CommandLen=index;
#endif

}
void GetPortOutputLimit()
{
#ifdef CFG_SWITCH_RATE_INCLUDED
	if(rx_Command.CommandLen!=0x04)
        {
                CommandStatus=Errlenerr;
                return ;
        }
        uint8 uport,value[4],index=Com26Start;
        uint32 pps;
        uport=rx_Command.CommandData[index];
        GetValue(index+1,value,sizeof(value));
        uint82uint32(value,&pps,sizeof(value));
        sys_error_t err;
        err=board_port_rate_egress_get((uint16) uport,&pps);
        if(SYS_OK!=err)
        {
                CommandStatus=Errdataerr;
                return;
        }
	uint322uint8(&pps,value,sizeof(pps));
        tx_Command.CommandData[index++]=uport;
        DataProd(index,value,sizeof(value),TRUE);
#endif

}
void SetPortOutputLimit()
{
#ifdef CFG_SWITCH_RATE_INCLUDED
	if(rx_Command.CommandLen!=0x0C)
        {
                CommandStatus=Errlenerr;
                return ;
        }
	uint8 uport,value[4],index=Com26Start;
        uint32 pps;
        uport=rx_Command.CommandData[index];
        GetValue(index+1,value,sizeof(value));
        uint82uint32(value,&pps,sizeof(value));
        sys_error_t err;
        err=board_port_rate_egress_set((uint16) uport,pps);
        if(SYS_OK!=err)
        {
                CommandStatus=Errdataerr;
                return;
        }
        tx_Command.CommandData[index++]=uport;
        tx_Command.CommandData[index++]=0xFA;
        tx_Command.CommandLen=index;
#endif
}
void GetPortSpeed()
{
#ifdef CFG_SWITCH_RATE_INCLUDED
	if(rx_Command.CommandLen!=0x04)
        {
                CommandStatus=Errlenerr;
                return ;
        }
	uint8 unit,lport,uport,index=Com26Start,value[4];
	int link,ad;
	uint32 speed;
	uport=rx_Command.CommandData[index];
	board_uport_to_lport(uport,&unit,&lport);
	lport=SOC_PORT_L2P_MAPPING(lport);
	PHY_LINK_GET(BMD_PORT_PHY_CTRL(unit,lport),&link,&ad);
	PHY_SPEED_GET(BMD_PORT_PHY_CTRL(unit,lport),&speed);
	if(!link)
	{
		speed=0x02;
	}
	uint322uint8(&speed,value,sizeof(value));
	tx_Command.CommandData[index++]=uport;
	DataProd(index,value,sizeof(value),TRUE);
#endif
}
void SetPortSpeed()
{
#ifdef CFG_SWITCH_RATE_INCLUDED
	if(rx_Command.CommandLen!=0x0C)
        {
                CommandStatus=Errlenerr;
                return ;
        }
	uint8 unit,lport,uport,index=Com26Start,value[4];
        uint32 speed;
        uport=rx_Command.CommandData[index];
	GetValue(index+1,value,sizeof(value));
	uint82uint32(value,&speed,sizeof(value));
        board_uport_to_lport((uint16)uport,&unit,&lport);
        lport=SOC_PORT_L2P_MAPPING(lport);
	PHY_SPEED_SET(BMD_PORT_PHY_CTRL(unit,lport),speed);
        uint322uint8(&speed,value,sizeof(value));
        tx_Command.CommandData[index++]=uport;
        DataProd(index,value,sizeof(value),TRUE);
#endif
}
void GetPortFlowCtrStatus()
{
	if(rx_Command.CommandLen!=0x04)
        {
                CommandStatus=Errlenerr;
                return ;
        }

	uint8 unit,lport,index=Com26Start;
        uint16 uport=rx_Command.CommandData[index];
        int link,ad;
        BOOL tx,rx;
        board_uport_to_lport(uport,&unit,&lport);
        lport=SOC_PORT_P2L_MAPPING(lport);
        PHY_LINK_GET(BMD_PORT_PHY_CTRL(unit,lport),&link,&ad);
        board_port_pause_get(uport,&tx,&rx);
        tx_Command.CommandData[index]=rx_Command.CommandData[index];
        index++;
        if(!link)
        {
                tx_Command.CommandData[index++]=0x02;
	}
	else
	{
		 if(tx&&rx)
        	{
                	tx_Command.CommandData[index++]=1;
	        }
        	else
       	        {
               		 tx_Command.CommandData[index++]=0;
   	        }
	}
	tx_Command.CommandLen=index;
}
/////////////////////////////////////////////////////////////////////
void CommandIDHandler0x80_0x26_0x04()
{
	switch(rx_Command.CommandData[2])
	{
		case 0x02:
			GreateVlan();
			break;
		case 0x03:
			DestoryVlan();
			break;
		case 0x04:
			GetVlanMember();
			break;
		case 0x05:
			SetVlanMember();
			break;
		case 0x06:
			GetVlanCounts();
			break;
		case 0x07:
			GetVlanByIndex();
			break;
		default:
			CommandStatus=Errnocom;
			
	}
}
void GreateVlan()
{
	uint16 vlanid;
	if(0x07!=rx_Command.CommandLen)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	vlanid=(rx_Command.CommandData[3]+rx_Command.CommandData[4])*256+rx_Command.CommandData[5]+rx_Command.CommandData[6];
	if(SYS_OK==board_vlan_create(vlanid))
	{
		tx_Command.CommandLen=0x04;

		tx_Command.CommandData[3]=0xFA;
	}
	else
	{
		CommandStatus=Errdataerr;
	}
	
}
void DestoryVlan()
{
	uint16 vlanid;
	if(0x07!=rx_Command.CommandLen)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	vlanid=(rx_Command.CommandData[3]+rx_Command.CommandData[4])*256+rx_Command.CommandData[5]+rx_Command.CommandData[6];
	if(SYS_OK==board_vlan_destroy(vlanid))
	{
		tx_Command.CommandLen=0x04;
	
		tx_Command.CommandData[3]=0xFA;
	}
	else
	{
		CommandStatus=Errdataerr;
	}
}
void GetVlanMember()
{
	uint8 vlanid,portlist[3],taglist[3];
	if(0x07!=rx_Command.CommandLen)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	sal_memset(portlist,0,3);
	sal_memset(taglist,0,3);
	vlanid=(rx_Command.CommandData[3]+rx_Command.CommandData[4])*256+rx_Command.CommandData[5]+rx_Command.CommandData[6];
	if(SYS_OK==board_qvlan_port_get(vlanid,portlist,taglist))
	{
		tx_Command.CommandLen=0x13;
		
		
		tx_Command.CommandData[3]=rx_Command.CommandData[3];
		tx_Command.CommandData[4]=rx_Command.CommandData[4];
		tx_Command.CommandData[5]=rx_Command.CommandData[5];
		tx_Command.CommandData[6]=rx_Command.CommandData[6];
		
		tx_Command.CommandData[7]=portlist[0]&0xF0;
		tx_Command.CommandData[8]=portlist[0]&0x0F;
		tx_Command.CommandData[9]=portlist[1]&0xF0;
		tx_Command.CommandData[10]=portlist[1]&0x0F;
		
		tx_Command.CommandData[11]=portlist[2]&0xF0;
		tx_Command.CommandData[12]=portlist[2]&0x0F;
		

		tx_Command.CommandData[13]=taglist[0]&0xF0;
		tx_Command.CommandData[14]=taglist[0]&0x0F;
		tx_Command.CommandData[15]=taglist[1]&0xF0;
		tx_Command.CommandData[16]=taglist[1]&0x0F;

		tx_Command.CommandData[17]=taglist[2]&0xF0;
		tx_Command.CommandData[18]=taglist[2]&0x0F;		
	}

	else
	{
		CommandStatus=Errdataerr;
	}
	
}
void SetVlanMember()
{
	uint8 vlanid,portlist[3],taglist[3],i,index,temp;
	if(0x13!=rx_Command.CommandLen)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	sal_memset(portlist,0,3);
	sal_memset(taglist,0,3);
	vlanid=(rx_Command.CommandData[3]+rx_Command.CommandData[4])*256+rx_Command.CommandData[5]+rx_Command.CommandData[6];
	index=7;
	for(i=0;i<3;i++)
	{
		for(temp=0;temp<2;temp++)
		{
			if(!temp)
			{
				portlist[i]=rx_Command.CommandData[index];
			}
			else
			{
				portlist[i]+=rx_Command.CommandData[index];
			}
			index++;
		}
	}
	for(i=0;i<3;i++)
	{
		for(temp=0;temp<2;temp++)
		{
			if(!temp)
			{
				taglist[i]=rx_Command.CommandData[index];
			}
			else
			{
				taglist[i]+=rx_Command.CommandData[index];
			}
			index++;
		}
	}
	if(SYS_OK==board_qvlan_port_set(vlanid,portlist,taglist))
	{
		tx_Command.CommandLen=0x08;
		
		tx_Command.CommandData[3]=rx_Command.CommandData[3];
		tx_Command.CommandData[4]=rx_Command.CommandData[4];
		tx_Command.CommandData[5]=rx_Command.CommandData[5];
		tx_Command.CommandData[6]=rx_Command.CommandData[6];
		tx_Command.CommandData[7]=0xFA;
	}
	else
	{
		CommandStatus=Errdataerr;
	}
}
void GetVlanCounts()
{
	if(rx_Command.CommandLen!=0x03)
	{
		CommandStatus=Errlenerr;
		return ;
	}
	uint8 index=Com26Start,value[2];
	uint16 count;
	count=board_vlan_count();
	uint162uint8(&count,value,sizeof(count));
	DataProd(index,value,sizeof(value),TRUE);
}
void GetVlanByIndex()
{
	if(rx_Command.CommandLen!=0x07)
	{
		CommandStatus=Errlenerr;
		return ;
	}
	uint8 index=Com26Start,value[2],uplist[4],taglist[4];
	uint16 val,vlan;
	GetValue(index,value,sizeof(value));
	uint82uint16(value,&val,sizeof(val));
	sys_error_t err;
	err=board_qvlan_get_by_index(val,&vlan,uplist,taglist);
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return;
	}
	uint162uint8(&vlan,value,sizeof(value));
	DataProd(index,value,sizeof(value),TRUE);
}
//////////////////////////////////////////////////////////////////////////
void CommandIDHandler0x80_0x26_0x05()
{
	switch(rx_Command.CommandData[2])
	{
		case 0x00:
			GetIGMPSnoopingStatus();
			break;
		case 0x01:
			SetIGMPSnoopingStatus();
			break;
		case 0x02:
			GetIGMPSnoopingVlan();
			break;
		case 0x03:
			SetIGMPSnoopingVlan();
			break;
		case 0x04:
			GetUnknowMuticaseStatus();
			break;
		case 0x05:
			SetUnknowMuticaseStatus();
			break;
		default:
			CommandStatus=Errnocom;
	}
}
void GetIGMPSnoopingStatus()
{	
#ifdef CFG_SWITCH_MCAST_INCLUDED
	if(rx_Command.CommandLen!=0x03)
	{
		CommandStatus=Errlenerr;
		return;
	}
	uint8 enable,index=Com26Start;
	igmpsnoop_enable_get(&enable);
	tx_Command.CommandData[index++]=enable;
	tx_Command.CommandLen=index;
#endif
}
void SetIGMPSnoopingStatus()
{
#ifdef CFG_SWITCH_MCAST_INCLUDED
	if(rx_Command.CommandLen!=0x04)
        {
                CommandStatus=Errlenerr;
                return;
        }
	uint8 enable,index=Com26Start;
	enable=rx_Command.CommandData[index];
	igmpsnoop_enable_set(enable);
	tx_Command.CommandData[index++]=0xFA;
	tx_Command.CommandLen=index;
#endif
}
void GetIGMPSnoopingVlan()
{
#ifdef CFG_SWITCH_MCAST_INCLUDED
	if(rx_Command.CommandLen!=0x03)
        {
                CommandStatus=Errlenerr;
                return;
        }
	uint8 index=Com26Start,value[2];
	uint16 vlan;
	igmpsnoop_vid_get(&vlan);
	uint162uint8(&vlan,value,sizeof(vlan));
	DataProd(index,value,sizeof(vlan),TRUE);
#endif
}
void SetIGMPSnoopingVlan()
{	
#ifdef CFG_SWITCH_MCAST_INCLUDED
	if(rx_Command.CommandLen!=0x07)
        {
                CommandStatus=Errlenerr;
                return;
        }
	uint8 value[2],index=Com26Start;
	uint16 vlan;
	GetValue(index,value,sizeof(value));
	uint82uint16(value,&vlan,sizeof(value));
	igmpsnoop_vid_set(vlan);
	tx_Command.CommandData[index++]=0xFA;
	tx_Command.CommandLen=index;
#endif
}
void GetUnknowMuticaseStatus()
{
#ifdef CFG_SWITCH_MCAST_INCLUDED
	if(rx_Command.CommandLen!=0x03)
        {
                CommandStatus=Errlenerr;
                return;
        }
	uint8 enable,index=Com26Start;
	sys_error_t err;
	err=board_block_unknown_mcast_get(&enable);
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	tx_Command.CommandData[index++]=enable;
	tx_Command.CommandLen=index;
#endif
}
void SetUnknowMuticaseStatus()
{
#ifdef CFG_SWITCH_MCAST_INCLUDED
	if(rx_Command.CommandLen!=0x04)
        {
                CommandStatus=Errlenerr;
                return;
        }
	uint8 enable,index=Com26Start;
        sys_error_t err;
	enable=rx_Command.CommandData[index];
        err=board_block_unknown_mcast_set(enable);
        if(SYS_OK!=err)
        {
                CommandStatus=Errdataerr;
                return ;
        }
        tx_Command.CommandData[index++]=0xFA;
        tx_Command.CommandLen=index;
#endif
}
//////////////////////////////////////////////////////////////////////////
void CommandIDHandler0x80_0x26_0xA6()
{
	switch(rx_Command.CommandData[2])
	{
		case 0x00:
			GetDeviceName();
			break;
		case 0x01:
			SetDeviceName();
			break;
		case 0xA0:
			GetMemVal();
			break;
		case 0xA1:
			SetMemVal();
			break;
		case 0xA2:
			GetPhyRegVal();
			break;
		case 0xA3:
			SetPhyRegVal();
			break;
		case 0xA4:
			GetSwitchRegVal();
			break;
		case 0xA5:
			SetSwitchRegVal();
			break;
		case 0xA6:
			GetTable();
			break;
		case 0xA7:
			SetTable();
			break;
		default:
			CommandStatus=Errnocom;
	}
}
void SetDeviceName()
{
	if(rx_Command.CommandLen<(Com26Start+1)||rx_Command.CommandLen>(Com26Start+MAX_SYSTEM_NAME_LEN))
	{
		CommandStatus=Errlenerr;
		return ;
	}
	char name[MAX_SYSTEM_NAME_LEN+1];
	sal_memset(name,0,sizeof(name));
	sal_memcpy(name,(uint32*)&(rx_Command.CommandData[Com26Start]),(uint32)(rx_Command.CommandLen-Com26Start));
	sys_error_t err=0;
	err=set_system_name(name);
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	tx_Command.CommandLen=0x04;
	tx_Command.CommandData[Com26Start]=0xFA;
}
void GetMemVal()
{
	if(rx_Command.CommandLen!=0x0B)
	{
		CommandStatus=Errlenerr;
		return ;
	}
	uint8 addr[4],val;
	uint32 add;
	GetValue(Com26Start,addr,sizeof(addr));
	uint82uint32(addr,&add,sizeof(addr));
	val=(*(volatile uint8 *)add);
	DataProd(Com26Start,&val,sizeof(val),TRUE);	
}
void SetMemVal()
{
	if(rx_Command.CommandLen!=0x0D)
	{
		CommandStatus=Errlenerr;
		return;
	}
	uint8 addr[4],val;
	GetValue(Com26Start,addr,sizeof(addr));
	GetValue(Com26Start+2*sizeof(addr),&val,sizeof(val));
	uint32 add;
	uint82uint32(addr,&add,sizeof(addr));
	*(volatile uint8*)add=val;
	tx_Command.CommandLen=0x04;
	tx_Command.CommandData[Com26Start]=0xFA;
}
void GetPhyRegVal()
{
	if(rx_Command.CommandLen!=0x08)
	{
		CommandStatus=Errlenerr;
		return ;
	}
	uint8 port=rx_Command.CommandData[Com26Start];
	if(!CheckPortID(port))
	{
		CommandStatus=Errdataerr;
		return ;
	}
	port+=1;
	port=SOC_PORT_P2L_MAPPING(port);
	uint16 val,add;
	uint8 addr[2],value[2];
	GetValue(Com26Start+1,addr,sizeof(addr));
	sys_error_t err;
	uint82uint16(addr,&add,sizeof(addr));
	err=phy_reg_read(port,add,&val);
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	uint162uint8(&val,value,sizeof(val));
	DataProd(Com26Start,value,sizeof(value),TRUE);
}
void SetPhyRegVal()
{
	if(rx_Command.CommandLen!=0x0C)
        {
                CommandStatus=Errlenerr;
                return ;
        }
        if(!rx_Command.CommandData[rx_Command.CommandLen-1])
        {
                CommandStatus=Errdataerr;
                return ;
        }
        uint8 port=rx_Command.CommandData[Com26Start];
        if(!CheckPortID(port))
        {
                CommandStatus=Errdataerr;
                return ;
        }
        port=SOC_PORT_P2L_MAPPING(port);
        uint8 addr[2],value[2];
        GetValue(Com26Start+1,addr,sizeof(addr));
        GetValue(Com26Start+1+sizeof(addr),value,sizeof(value));
        sys_error_t err;
	uint16 add;
	uint82uint16(addr,&add,sizeof(addr));
	uint16 val;
	uint82uint16(value,&val,sizeof(value));
        err=phy_reg_write(port,add,val);
        if(SYS_OK!=err)
	{
                CommandStatus=Errdataerr;
                return ;
     	}
        tx_Command.CommandLen=0x04;
	tx_Command.CommandData[Com26Start]=0xFA;

}
void GetSwitchRegVal()
{
	if(rx_Command.CommandLen!=0x0D)
	{
		CommandStatus=Errlenerr;
		return ;
	}
	if(!rx_Command.CommandData[rx_Command.CommandLen-1])
	{
		CommandStatus=Errdataerr;
		return ;
	}
	uint8 len=rx_Command.CommandData[rx_Command.CommandLen-1];
	uint8 length=len*4;
	uint8 addr[4],value[length];
	uint8 id=rx_Command.CommandData[Com26Start];
	GetValue(Com26Start+1,addr,sizeof(addr));
	sys_error_t err=0;
	uint32 add,val[len];
	uint82uint32(addr,&add,sizeof(addr));
	err=bcm5333x_reg64_get(0,id,add,val,(int)len);
	if(SYS_OK!=err)
	{
		CommandStatus=Errdataerr;
		return ;
	}
	uint322uint8(val,value,sizeof(val));
	DataProd(Com26Start,value,sizeof(value),TRUE);
}
void SetSwitchRegVal()
{
	if(rx_Command.CommandLen<0x15||(rx_Command.CommandLen%8!=5))
        {
                CommandStatus=Errlenerr;
                return ;
        }
        if(!rx_Command.CommandData[0x14])
        {
                CommandStatus=Errdataerr;
                return ;
        }
        uint8 len=rx_Command.CommandData[0x14];
        uint8 length=len*4;
        uint8 addr[4],value[length];
	uint8 id=rx_Command.CommandData[Com26Start];
        GetValue(Com26Start+1,addr,sizeof(addr));
	GetValue(Com26Start+2+sizeof(addr),value,sizeof(value));
        sys_error_t err=0;
	uint32 add,val[len];
	uint82uint32(addr,&add,sizeof(addr));
	uint82uint32(value,val,sizeof(value));
        err=bcm5333x_reg64_set(0,id,add,val,(int)len);
        if(SYS_OK!=err)
        {
                CommandStatus=Errdataerr;
                return ;
	}
	tx_Command.CommandLen=0x04;
	tx_Command.CommandData[Com26Start]=0xFA;
}
void GetTable()
{
	if(rx_Command.CommandLen!=0x0D)
        {
                CommandStatus=Errlenerr;
                return ;
        }
        if(!rx_Command.CommandData[rx_Command.CommandLen-1])
        {
                CommandStatus=Errdataerr;
                return ;
        }
        uint8 len=rx_Command.CommandData[rx_Command.CommandLen-1];
        uint8 length=len*4;
        uint8 addr[4],value[length];
	uint8 id=rx_Command.CommandData[Com26Start];
        GetValue(Com26Start+1,addr,sizeof(addr));
        sys_error_t err=0;
	uint32 add,val[len];
	uint82uint32(addr,&add,sizeof(addr));
        err=bcm5333x_mem_get(0,id,add,val,len);
        if(SYS_OK!=err)
        {
                CommandStatus=Errdataerr;
                return ;
	}
	uint322uint8(val,value,sizeof(val));
	DataProd(Com26Start,value,sizeof(value),TRUE);

}
void SetTable()
{
	if(rx_Command.CommandLen<0x15||(rx_Command.CommandLen%8!=5))
        {
                CommandStatus=Errlenerr;
                return ;
        }
        if(!rx_Command.CommandData[0x14])
        {
                CommandStatus=Errdataerr;
                return ;
        }
        uint8 len=rx_Command.CommandData[0x14];
        uint8 length=len*4;
        uint8 addr[4],value[length];
	uint8 id=rx_Command.CommandData[Com26Start];
        GetValue(Com26Start+1,addr,sizeof(addr));
	GetValue(Com26Start+2+sizeof(addr),value,sizeof(value));
        sys_error_t err=0;
	uint32 add,val[len];
	uint82uint32(addr,&add,sizeof(addr));
	uint82uint32(value,val,sizeof(val));
        err=bcm5333x_mem_set(0,id,(uint32)add,(uint32 *)value,len);
        if(SYS_OK!=err)
        {
                CommandStatus=Errdataerr;
                return ;
	}
	tx_Command.CommandLen=0x04;
	tx_Command.CommandData[Com26Start]=0xFA;

}
//////////////////////////////////////////////////////////////////////////
void Timeouthandler()
{
	EndRecCom();

	CommandStatus=Errtimeout;
	CommandErrHandler();
	SendMessage();
}
void CommandErrHandler()
{
	tx_Command.CommandLen=1;
	tx_Command.CommandData[0]=CommandStatus;
	
}
void SendMessage()
{
	uint8 i;
	put_char(tx_Command.CommandHead);
	put_char(tx_Command.CommandAddr);
	put_char(tx_Command.CommandID);
	put_char(tx_Command.CommandLen);
	for(i=0;i<tx_Command.CommandLen;i++)
	{
		put_char(tx_Command.CommandData[i]);
	}
	put_char(CheckCrc(tx_Command));
}
////////////////////////////////////////
#endif /* CFG_CLI_ENABLED */
