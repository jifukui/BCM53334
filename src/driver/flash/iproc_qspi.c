/*
 * $Id: iproc_qspi.c,v 1.19 Broadcom SDK $
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

#include "system.h"
#include "bsp_config.h"

#if CFG_FLASH_SUPPORT_ENABLED
#include "flash_table.h"

/*
 * Register and bit definitions for the BSPI control
 * registers.
 */

/* Control and Interrupt Registers */
#define BCHP_BSPI_MAST_N_BOOT_CTRL    (CFG_BSPI_BASE + 0x8)
#define BCHP_BSPI_BUSY_STATUS         (CFG_BSPI_BASE + 0xC)
#define BCHP_BSPI_INTR_STATUS         (CFG_BSPI_BASE + 0x10)
#define BCHP_BSPI_B0_STATUS           (CFG_BSPI_BASE + 0x14)
#define BCHP_BSPI_B0_CTRL             (CFG_BSPI_BASE + 0x18)
#define BCHP_BSPI_B1_STATUS           (CFG_BSPI_BASE + 0x1C)
#define BCHP_BSPI_B1_CTRL             (CFG_BSPI_BASE + 0x20)
#define BCHP_FLEX_MODE_ENABLE         (CFG_BSPI_BASE + 0x28)
#define BCHP_BITS_PER_CYCLE           (CFG_BSPI_BASE + 0x2C)
#define BCHP_BITS_PER_PHASE           (CFG_BSPI_BASE + 0x30)

/*
 * Register and bit definitions for the MSPI control
 * registers.
 */

/* Control and Interrupt Registers */
#define BCHP_MSPI_SPCR0_LSB           (CFG_BSPI_BASE + 0x200)
#define BCHP_MSPI_SPCR0_MSB           (CFG_BSPI_BASE + 0x204)
#define BCHP_MSPI_SPCR1_LSB           (CFG_BSPI_BASE + 0x208)
#define BCHP_MSPI_SPCR1_MSB           (CFG_BSPI_BASE + 0x20C)
#define BCHP_MSPI_NEWQP               (CFG_BSPI_BASE + 0x210)
#define BCHP_MSPI_ENDQP               (CFG_BSPI_BASE + 0x214)
#define BCHP_MSPI_SPCR2               (CFG_BSPI_BASE + 0x218)
#define BCHP_MSPI_MSPI_STATUS         (CFG_BSPI_BASE + 0x220)
#define BCHP_MSPI_CPTQP               (CFG_BSPI_BASE + 0x224)
#define BCHP_MSPI_TXRAM00             (CFG_BSPI_BASE + 0x240)
#define BCHP_MSPI_RXRAM00             (CFG_BSPI_BASE + 0x2C0)
#define BCHP_MSPI_RXRAM01             (CFG_BSPI_BASE + 0x2C4)
#define BCHP_MSPI_CDRAM00             (CFG_BSPI_BASE + 0x340)

/* MSPI Enable */
#define BCHP_MSPI_SPCR2_halt_MASK        (1 << (0))
/* MSPI Enable */
#define BCHP_MSPI_SPCR2_spe_MASK         (1 << (6))
/* MSPI Finished flag */
#define BCHP_MSPI_MSPI_STATUS_SPIF_MASK  (1 << (0))
/* SPI is configured to be master */
#define BCHP_MSPI_SPCR0_MSB_MSTR_MASK    (1 << (7))
/* Bits per transfer */
#define BCHP_MSPI_SPCR0_MSB_BITS_MASK    (0xF << (2))
/* 8 bits per transfer */
#define BCHP_MSPI_SPCR0_MSB_8BITS        (0x8 << (2))
/* Clock polarity */
#define BCHP_MSPI_SPCR0_MSB_CPOL_MASK    (1 << (1))
/* Clock phase */
#define BCHP_MSPI_SPCR0_MSB_CPHA_MASK    (1 << (0))

#define BSPI_BITS_FLEX_MODE_ENABLER_MASK (1 << (0))

#define BSPI_BITS_PER_PHASE_ADDR_MASK    (0x00010000)



/* MX25L-specific commands */
#define SPI_WREN_CMD        (0x06)
#define SPI_WRDI_CMD        (0x04)
#define SPI_RDSR_CMD        (0x05)
#define SPI_READ_CMD        (0x03)
#define SPI_SE_CMD          (0x20)
#define SPI_PP_CMD          (0x02)
#define SPI_RDFSR_CMD       (0x70)
#define SPI_RDID_CMD        (0x9F)
#define CMD_MX25XX_EN4B     (0xb7) /* Enter 4-byte address mode */
#define CMD_MX25XX_EX4B     (0xe9) /* Exit 4-byte address mode */

#define SPI_POLLING_INTERVAL        10      /* in usecs */
/* Bit [1:0] used to select an external device(corresponding to pin SS[i]) */
#define BSPI_Pcs_eUpgSpiPcs0        0
#define SPI_CDRAM_CONT              0x80

#define SPI_CDRAM_PCS_PCS0          0x01
#define SPI_CDRAM_PCS_PCS1          0x02
#define SPI_CDRAM_PCS_PCS2          0x00
#define SPI_CDRAM_PCS_DISABLE_ALL   (SPI_CDRAM_PCS_PCS0 | SPI_CDRAM_PCS_PCS1 | SPI_CDRAM_PCS_PCS2)

#define SPI_SYSTEM_CLK      200000000   /* 200 MHz */
#define MAX_SPI_BAUD        12500000    /* SPBR = 8, 12.5MHZ */

#define MSPI_CALC_TIMEOUT(bytes,baud)    ((((bytes * 9000)/baud) * 110)/100 + 1)

#define ENTER_BSPI_DELAY(n) do {\
                               volatile int count; \
                               for (count = 0; count < (n * 1000); count++);\
                            } while(0)

#define M25LXX_PAGE_SIZE     (256)

#define MSPI_MAX_PROGRAM_LEN  (12)

#define MIN(a, b) ((a) <= (b)? (a):(b))



/* Forwards */
static BOOL
flash_to_local_addr(flash_dev_t* dev, hsaddr_t* addr)  __attribute__((section(".2ram")));

sys_error_t spi_master_init(void) __attribute__((section(".2ram")));

sys_error_t spi_read_jedec_id(uint8* jedec_id, uint8 len)  __attribute__((section(".2ram")));

static void bspi_flush_prefetch_buffers(void)  __attribute__((section(".2ram")));

sys_error_t mspi_init(flash_dev_t *dev) __attribute__((section(".2ram")));

sys_error_t m25lxx_erase_block(flash_dev_t *dev,
                       hsaddr_t block_base)  __attribute__((section(".2ram")));

sys_error_t m25lxx_program(flash_dev_t *dev, hsaddr_t base,
               const void* data, size_t len) __attribute__((section(".2ram")));

static sys_error_t m25lxx_read(flash_dev_t *dev, const hsaddr_t base, void* data, size_t len) __attribute__((section(".2ram")));

static void *spi_memset(void *dest,int c,size_t cnt) __attribute__((section(".2ram")));
static void *spi_memcpy(void *dest,const void *src,size_t cnt) __attribute__((section(".2ram")));


static int
spi_transaction(uint8 *w_buf,
                uint8 write_len,
                uint8 *r_buf,
                uint8 read_len) __attribute__((section(".2ram")));


static int
spi_program(uint32 base, uint8 *buf, uint8 len)  __attribute__((section(".2ram")));

struct flash_dev_funs mx25l_funs = {
    mspi_init,
    m25lxx_erase_block,
    m25lxx_program,
    m25lxx_read
};



flash_dev_t current_flash_dev = { {MANUFACTURER_ID_UNKNOWN,  0x0, 0x0, 0x0}, {0x00, 0x0, 0x0, 0x0}, &mx25l_funs, 0, CFG_FLASH_START_ADDRESS, 0, 1, &uniform_4K_64Mb_info, "Unknow Flash", NULL};
flash_dev_t flashtest={{MANUFACTURER_ID_MICRON,0x71,0x15,0x10},{0xff,0xff,0xff,0x0},&mx25l_funs,0,CFG_FLASH_START_ADDRESS,0x1c25f000,1,&uniform_4K_16Mb_info,"M25PX16jfk",NULL};
static void
bspi_flush_prefetch_buffers(void)
{
    SYS_REG_WRITE32(BCHP_BSPI_B0_CTRL, 0);
    SYS_REG_WRITE32(BCHP_BSPI_B1_CTRL, 0);
    SYS_REG_WRITE32(BCHP_BSPI_B0_CTRL, 1);
    SYS_REG_WRITE32(BCHP_BSPI_B1_CTRL, 1);
}

/*
 * Strips out any device address offset to give address within device.
 */
static
BOOL flash_to_local_addr(flash_dev_t* dev, hsaddr_t* addr)
{
    /* Range check address before modifying it. */
    if ((*addr >= dev->start) && (*addr <= dev->end)) {
        *addr -= dev->start;
        return TRUE;
    }
    return FALSE;
}

void *spi_memset(void *dest,int c,size_t cnt)
{
    unsigned char *d;

    d = dest;

    while (cnt) {
    *d++ = (unsigned char) c;
    cnt--;
    }

    return d;
}
void *spi_memcpy(void *dest,const void *src,size_t cnt)
{
    unsigned char *d;
    const unsigned char *s;
    d = (unsigned char *) dest;
    s = (const unsigned char *) src;

    while (cnt) {
    *d++ = *s++;
    cnt--;
    }

    return dest;
}

sys_error_t  spi_read_jedec_id(uint8* jedec_id, uint8 len) {

    uint8 cmd[4];
    uint8 status;

    spi_memset(jedec_id, 0, sizeof(jedec_id));
    
    while(SYS_REG_READ32(BCHP_BSPI_BUSY_STATUS));
    SYS_REG_WRITE32(BCHP_BSPI_MAST_N_BOOT_CTRL,1);

    /* Wait for previous write operation done */  
    cmd[0] = SPI_RDSR_CMD;
    do {
       if (spi_transaction(cmd,1,&status,1) != SYS_OK) {
           goto out;
       }
    } while(status & 0x1);

    /* Read real ID */  
    cmd[0] = SPI_RDID_CMD;
    if (spi_transaction(cmd,1,jedec_id, len) != SYS_OK) {
        spi_memset(jedec_id, 0, len);
        goto out;
    }
out: 
    bspi_flush_prefetch_buffers();
    SYS_REG_WRITE32(BCHP_BSPI_MAST_N_BOOT_CTRL,0);
    ENTER_BSPI_DELAY(10);

    return SYS_OK;

}



static int
spi_transaction(uint8 *w_buf,
                uint8 write_len,
                uint8 *r_buf,
                uint8 read_len)
{

    uint8 i, len = write_len + read_len;;

    for (i = 0; i < len; ++i)
    {
        if (i < write_len) {
            SYS_REG_WRITE32(BCHP_MSPI_TXRAM00 + (i * 8), (uint32)w_buf[i]);
        }
        SYS_REG_WRITE32( BCHP_MSPI_CDRAM00 + (i * 4) , SPI_CDRAM_CONT | SPI_CDRAM_PCS_PCS1);
    }

    SYS_REG_WRITE32(BCHP_MSPI_CDRAM00 + ((len - 1) * 4), SPI_CDRAM_PCS_PCS1);

    /* Clear previous status */
    SYS_REG_WRITE32(BCHP_MSPI_MSPI_STATUS, 0);

    /* Set queue pointers */
    SYS_REG_WRITE32(BCHP_MSPI_NEWQP, 0);
    SYS_REG_WRITE32(BCHP_MSPI_ENDQP, len - 1);

    /* Start SPI transfer */
    SYS_REG_WRITE32(BCHP_MSPI_SPCR2, BCHP_MSPI_SPCR2_spe_MASK | BCHP_MSPI_SPCR0_MSB_MSTR_MASK);

    /* Wait for SPI to finish */
    while(!(SYS_REG_READ32(BCHP_MSPI_MSPI_STATUS) & BCHP_MSPI_MSPI_STATUS_SPIF_MASK));
    SYS_REG_WRITE32(BCHP_MSPI_MSPI_STATUS, 0);

    for (i = write_len; i < len; ++i) {
        r_buf[i-write_len] = (uint8)SYS_REG_READ32( BCHP_MSPI_RXRAM01 + (i * 8));
    }

    return SYS_OK;
}

static int
spi_program(uint32 base, uint8 *buf, uint8 len)
{
    int i, j;

    SYS_REG_WRITE32(BCHP_MSPI_TXRAM00, SPI_PP_CMD);
    SYS_REG_WRITE32(BCHP_MSPI_TXRAM00 + 8, (base >> 16) & 0xFF);
    SYS_REG_WRITE32(BCHP_MSPI_TXRAM00 + 16, (base >> 8) & 0xFF);
    SYS_REG_WRITE32(BCHP_MSPI_TXRAM00 + 24, base & 0xFF);

    for (i = 4, j = 0; j < len; i++, j++) {
        SYS_REG_WRITE32(BCHP_MSPI_TXRAM00 + (i * 8), (uint32)buf[j]);
    }

    for (i = 0; i < len + 4; i++) {
        SYS_REG_WRITE32(BCHP_MSPI_CDRAM00 + (i * 4) , SPI_CDRAM_CONT | SPI_CDRAM_PCS_PCS1);
    }

    SYS_REG_WRITE32(BCHP_MSPI_CDRAM00 + ((i - 1) * 4), SPI_CDRAM_PCS_PCS1);

    /* Clear previous status */
    SYS_REG_WRITE32(BCHP_MSPI_MSPI_STATUS, 0);

    /* Set queue pointers */
    SYS_REG_WRITE32(BCHP_MSPI_NEWQP, 0);
    SYS_REG_WRITE32(BCHP_MSPI_ENDQP, i - 1);


    /* Start SPI transfer */
    SYS_REG_WRITE32(BCHP_MSPI_SPCR2, BCHP_MSPI_SPCR2_spe_MASK | BCHP_MSPI_SPCR0_MSB_MSTR_MASK);

    /* Wait for SPI to finish */
    while(!(SYS_REG_READ32(BCHP_MSPI_MSPI_STATUS) & BCHP_MSPI_MSPI_STATUS_SPIF_MASK));
    SYS_REG_WRITE32(BCHP_MSPI_MSPI_STATUS, 0);

    return SYS_OK;
}

sys_error_t
mspi_init(flash_dev_t *dev)
{
    uint32 lval;
    uint8 jedec_id[4];
    int i, j;

    /* IPROC MSPI initialization */
    while(SYS_REG_READ32(BCHP_BSPI_BUSY_STATUS));
    SYS_REG_WRITE32(BCHP_BSPI_MAST_N_BOOT_CTRL,1);

    /* Serial clock baud rate = System clock / (2 * SPBR) */
    lval = SPI_SYSTEM_CLK / (2 * MAX_SPI_BAUD);
    SYS_REG_WRITE32(BCHP_MSPI_SPCR0_LSB, lval);

    /*
         * Master [bit 7]
         * Bits per transfer [bit 5-2]: 0 = 16
         * Clock Polarity [bit 1]
         * MSPI clock phase [bit 0]
         */
    lval = SYS_REG_READ32(BCHP_MSPI_SPCR0_MSB);
    lval &= ~(BCHP_MSPI_SPCR0_MSB_CPOL_MASK | BCHP_MSPI_SPCR0_MSB_CPHA_MASK |
              BCHP_MSPI_SPCR0_MSB_BITS_MASK);
    lval |= (BCHP_MSPI_SPCR0_MSB_MSTR_MASK | BCHP_MSPI_SPCR0_MSB_CPOL_MASK |
             BCHP_MSPI_SPCR0_MSB_CPHA_MASK | BCHP_MSPI_SPCR0_MSB_8BITS);
    SYS_REG_WRITE32(BCHP_MSPI_SPCR0_MSB, lval );

    bspi_flush_prefetch_buffers();
    SYS_REG_WRITE32(BCHP_BSPI_MAST_N_BOOT_CTRL,0);
    ENTER_BSPI_DELAY(10);

    if (dev == NULL) { /* Auto probe */
        /* read jedec id through spi driver */
        spi_read_jedec_id(jedec_id, sizeof(jedec_id));
        
        for (j=0; j<(sizeof(flash_dev_support_table)/sizeof(flash_dev_t)); j++) {
              for (i=0; i<sizeof(jedec_id); i++) {
                   if ((flash_dev_support_table[j].jedec_id[i] & flash_dev_support_table[j].jedec_id_mask[i]) != 
                       (jedec_id[i] & flash_dev_support_table[j].jedec_id_mask[i])) 
                   {
                       break;
                   }
              }
              if (i == sizeof(jedec_id)) {
                  spi_memcpy(&current_flash_dev, &flash_dev_support_table[j], sizeof(flash_dev_t));
                  if ((j+1) == (sizeof(flash_dev_support_table)/sizeof(flash_dev_t))) {
                        spi_memcpy(current_flash_dev.jedec_id, jedec_id, sizeof(current_flash_dev.jedec_id));
                  }
                  break;
              }
        }
        dev = &current_flash_dev;
    } 


    dev->end = dev->start;

    for (i=0; i < dev->num_block_infos; i++) {
        dev->end += ((hsaddr_t) dev->block_info[i].block_size *
                     (hsaddr_t) dev->block_info[i].blocks);
    }
    dev->end -= 1;
    return SYS_OK;
}

sys_error_t
bspi_set_4byte_mode(flash_dev_t *dev, int enable)
{
#if 0
    /* Disable flex mode first */
    priv->bspi_hw->flex_mode_enable = 0;

    if (enable) {

        /* Enable 32-bit address */
        priv->bspi_hw->bits_per_phase |= BSPI_BITS_PER_PHASE_ADDR_MARK;

    } else {

        /* Disable 32-bit address */
        priv->bspi_hw->bits_per_phase &= ~BSPI_BITS_PER_PHASE_ADDR_MARK;

        /* Clear upper address byte */
        priv->bspi_hw->flash_upper_addr_byte = 0;
    }

    /* Enable flex mode to take effect */
    priv->bspi_hw->flex_mode_enable = 1;

    /* Record current mode */
    priv->mode_4byte = enable;
#endif
    return SYS_OK;
}

sys_error_t m25lxx_set_4byte_mode(flash_dev_t *dev, int enable)
{
    uint8 cmd[4];
    sys_error_t rv = SYS_OK;

    while(SYS_REG_READ32(BCHP_BSPI_BUSY_STATUS));
    SYS_REG_WRITE32(BCHP_BSPI_MAST_N_BOOT_CTRL,1);
    ENTER_BSPI_DELAY(10);

    cmd[0] = enable? CMD_MX25XX_EN4B : CMD_MX25XX_EX4B;

    if (spi_transaction(cmd,1,NULL,0) != SYS_OK) {
        rv = SYS_ERR;
    }

    return rv;
}

sys_error_t m25lxx_erase_block(flash_dev_t *dev, hsaddr_t block_base)
{
    hsaddr_t local_base = block_base;
    uint8 cmd[4];
    uint8 data;
    sys_error_t rv = SYS_OK;
    /* Fix up the block address and send the sector erase command. */
    if (flash_to_local_addr(dev, &local_base)) {

        while(SYS_REG_READ32(BCHP_BSPI_BUSY_STATUS));
        SYS_REG_WRITE32(BCHP_BSPI_MAST_N_BOOT_CTRL,1);

        /* Wait for previous write operation done */  
        do {
            cmd[0] = SPI_RDSR_CMD;
            if (spi_transaction(cmd,1,&data,1) != SYS_OK) {
                rv = SYS_ERR;
                goto out;
            }
        /* Bit 0 = Write-in-Progress */
        } while(data & 0x01);
        

        cmd[0] = SPI_WREN_CMD;
        if (spi_transaction(cmd,1,NULL,0) != SYS_OK) {
            rv = SYS_ERR;
            goto out;
        }

        /* Wait for WREN done */  
        do {
            cmd[0] = SPI_RDSR_CMD;
            if (spi_transaction(cmd,1,&data,1) != SYS_OK) {
                rv = SYS_ERR;
                goto out;
            }
        /* Bit 0 = Write-in-Progress */
        } while(data & 0x01);

        cmd[0] = SPI_SE_CMD;
        cmd[1] = (uint8)(local_base >> 16);
        cmd[2] = (uint8)(local_base >> 8);
        cmd[3] = (uint8)local_base;

        if (spi_transaction(cmd,4,NULL,0) != SYS_OK) {
            rv = SYS_ERR;
            goto out;
        }

        /* Wait for Sector Erase done */  
        do {
            cmd[0] = SPI_RDSR_CMD;
            if (spi_transaction(cmd,1,&data,1) != SYS_OK) {
                rv = SYS_ERR;
                goto out;
            }
        /* Bit 0 = Write-in-Progress */
        } while(data & 0x01);

        /* special Flag register read operation for Micron */
        if (dev->flags & FLAG_FSR_POLL) {
            do {
                cmd[0] = SPI_RDFSR_CMD;
                if (spi_transaction(cmd,1,&data,1) != SYS_OK) {
                    rv = SYS_ERR;
                    goto out;
                }
            }while((data & 0x80) == 0x0);
        }
    }

out:
    cmd[0] = SPI_WRDI_CMD;
    spi_transaction(cmd,1,NULL,0);
    /* Wait for WRDI done */  
    do {
        cmd[0] = SPI_RDSR_CMD;
        if (spi_transaction(cmd,1,&data,1) != SYS_OK) {
            rv = SYS_ERR;
            goto out;
        }
    /* Bit 0 = Write-in-Progress */
    } while(data & 0x01);
    
    bspi_flush_prefetch_buffers();
    SYS_REG_WRITE32(BCHP_BSPI_MAST_N_BOOT_CTRL,0);
    ENTER_BSPI_DELAY(10);
    return rv;
}

sys_error_t m25lxx_program(flash_dev_t *dev,
                           hsaddr_t base,
                           const void* data, size_t len)
{
    hsaddr_t local_base = base;
    uint8* tx_ptr = (uint8*) data;
    uint32 tx_bytes_left = (uint32) len;
    uint32 tx_bytes, chunk;
    uint8 cmd[16], rx_data;
    
    sys_error_t rv = SYS_OK;

    /* Fix up the block address. */
    if (!flash_to_local_addr (dev, &local_base)) {
        return SYS_ERR_PARAMETER;
    }

    while(SYS_REG_READ32(BCHP_BSPI_BUSY_STATUS));
    SYS_REG_WRITE32(BCHP_BSPI_MAST_N_BOOT_CTRL,1);

    /* Perform page program operations. */
    while (tx_bytes_left) {
        tx_bytes = M25LXX_PAGE_SIZE - (((uint32)local_base) & (M25LXX_PAGE_SIZE - 1));
        tx_bytes = MIN(tx_bytes, tx_bytes_left);
        tx_bytes_left -= tx_bytes;
        while (tx_bytes) {
            chunk = MIN(tx_bytes, MSPI_MAX_PROGRAM_LEN);

             /* Wait for previous write operation done */  
            do {
                cmd[0] = SPI_RDSR_CMD;
                if (spi_transaction(cmd,1,&rx_data,1) != SYS_OK) {
                    rv = SYS_ERR;
                    goto out;
                }
            /* Bit 0 = Write-in-Progress */
            } while(rx_data & 0x01);
            

            cmd[0] = SPI_WREN_CMD;
            if (spi_transaction(cmd,1,NULL,0) != SYS_OK) {
                 rv = SYS_ERR;
                 goto out;
            }
            
            /* Wait for WREN done */ 
            do {
                cmd[0] = SPI_RDSR_CMD;
                if (spi_transaction(cmd,1,&rx_data,1) != SYS_OK) {
                    rv = SYS_ERR;
                    goto out;
                }
            /* Bit 0 = Write-in-Progress */
            } while(rx_data & 0x01);
            

            if (spi_program(local_base, tx_ptr, (uint8)chunk) != SYS_OK) {
                goto out;
            }

            /* Wait for WREN done */ 
            do {
                cmd[0] = SPI_RDSR_CMD;
                if (spi_transaction(cmd,1,&rx_data,1) != SYS_OK) {
                    rv = SYS_ERR;
                    goto out;
                }
            /* Bit 0 = Write-in-Progress */
            } while(rx_data & 0x01);

            /* special Flag register read operation for Micron */
            if (dev->flags & FLAG_FSR_POLL) {
                do {
                    cmd[0] = SPI_RDFSR_CMD;
                    if (spi_transaction(cmd,1,&rx_data,1) != SYS_OK) {
                        rv = SYS_ERR;
                        goto out;
                    }
                }while((rx_data & 0x80) == 0x0);
            }


            local_base += chunk;
            tx_ptr += chunk;
            tx_bytes -= chunk;
        }
    }
out:
    cmd[0] = SPI_WRDI_CMD;
    spi_transaction(cmd,1,NULL,0);
    /* Wait for WRDI done */  
    do {
        cmd[0] = SPI_RDSR_CMD;
        if (spi_transaction(cmd,1,&rx_data,1) != SYS_OK) {
            rv = SYS_ERR;
            goto out;
        }
    /* Bit 0 = Write-in-Progress */
    } while(rx_data & 0x01);

    bspi_flush_prefetch_buffers();
    SYS_REG_WRITE32(BCHP_BSPI_MAST_N_BOOT_CTRL,0);
    ENTER_BSPI_DELAY(10);
    return rv;
}

static
sys_error_t m25lxx_read(flash_dev_t *dev,
                        const hsaddr_t base,
                        void* data, size_t len)
{
    hsaddr_t local_base = base;
    uint8* rx_ptr = (uint8*)data;
    uint32 rx_bytes_left = (uint32) len;
    uint8 cmd[4];
    sys_error_t rv = SYS_OK;

    /* Fix up the block address. */
    if (!flash_to_local_addr (dev, &local_base)) {
        return SYS_ERR_PARAMETER;
    }

    while(SYS_REG_READ32(BCHP_BSPI_BUSY_STATUS));
    SYS_REG_WRITE32(BCHP_BSPI_MAST_N_BOOT_CTRL,1);

    cmd[0] = SPI_READ_CMD;
    /* Perform page program operations. */
    while (rx_bytes_left) {
        cmd[1] = (uint8)(local_base >> 16);
        cmd[2] = (uint8)(local_base >> 8);
        cmd[3] = (uint8)local_base;

        if (spi_transaction(cmd,4,rx_ptr,1) != SYS_OK) {
            rv = SYS_ERR;
            goto out;
        }
        /* Update counters and data pointers for the next page. */
        local_base++;
        rx_ptr++;
        rx_bytes_left--;
    }
out:
    SYS_REG_WRITE32(BCHP_BSPI_MAST_N_BOOT_CTRL,0);
    ENTER_BSPI_DELAY(10);
    return rv;
}

#endif /* CFG_FLASH_SUPPORT_ENABLED */
