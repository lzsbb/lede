/*
 * ath-pcm.h -- ALSA PCM interface for the QCA Wasp based audio interface
 *
 * Copyright (c) 2013 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _ATH_PCM_H_
#define _ATH_PCM_H_

#include <linux/sound.h>
#include <linux/list.h>
#include <linux/spinlock.h>

#define ATH79_MISC_IRQ_DMA	(ATH79_MISC_IRQ_BASE + 7)

/*
 * DMA block (MBOX/PCM)
 */
#define AR934X_DMA_REG_MBOX_FIFO			0x00
#define AR934X_DMA_REG_MBOX_FIFO_STATUS			0x08
#define AR934X_DMA_REG_SLIC_MBOX_FIFO_STATUS		0x0c
#define AR934X_DMA_REG_MBOX_DMA_POLICY			0x10

#define AR934X_DMA_MBOX_DMA_POLICY_TX_FIFO_THRESH_SHIFT	4
#define AR934X_DMA_MBOX_DMA_POLICY_TX_FIFO_THRESH_MASK	0xf
#define AR934X_DMA_MBOX_DMA_POLICY_TX_QUANTUM		BIT(3)
#define AR934X_DMA_MBOX_DMA_POLICY_RX_QUANTUM		BIT(1)

#define AR934X_DMA_REG_SLIC_MBOX_DMA_POLICY		0x14
#define AR934X_DMA_REG_MBOX0_DMA_RX_DESCRIPTOR_BASE	0x18
#define AR934X_DMA_REG_MBOX0_DMA_RX_CONTROL		0x1c
#define AR934X_DMA_REG_MBOX0_DMA_TX_DESCRIPTOR_BASE	0x20
#define AR934X_DMA_REG_MBOX0_DMA_TX_CONTROL		0x24
#define AR934X_DMA_REG_MBOX1_DMA_RX_DESCRIPTOR_BASE	0x28
#define AR934X_DMA_REG_MBOX1_DMA_RX_CONTROL		0x2c
#define AR934X_DMA_REG_MBOX1_DMA_TX_DESCRIPTOR_BASE	0x30
#define AR934X_DMA_REG_MBOX1_DMA_TX_CONTROL		0x34

#define AR934X_DMA_MBOX_DMA_CONTROL_RESUME		BIT(2)
#define AR934X_DMA_MBOX_DMA_CONTROL_START		BIT(1)
#define AR934X_DMA_MBOX_DMA_CONTROL_STOP		BIT(0)

#define AR934X_DMA_REG_MBOX_FRAME			0x38
#define AR934X_DMA_REG_SLIC_MBOX_FRAME			0x3c
#define AR934X_DMA_REG_FIFO_TIMEOUT			0x40
#define AR934X_DMA_REG_MBOX_INT_STATUS			0x44
#define AR934X_DMA_REG_SLIC_MBOX_INT_STATUS		0x48

#define AR934X_DMA_MBOX_INT_STATUS_RX_DMA_COMPLETE	BIT(10)
#define AR934X_DMA_MBOX_INT_STATUS_TX_DMA_COMPLETE	BIT(6)

#define AR934X_DMA_REG_MBOX_INT_ENABLE			0x4c
#define AR934X_DMA_REG_SLIC_MBOX_INT_ENABLE		0x50

#define AR934X_DMA_MBOX1_INT_RX_COMPLETE		BIT(11)
#define AR934X_DMA_MBOX0_INT_RX_COMPLETE		BIT(10)
#define AR934X_DMA_MBOX1_INT_TX_EOM_COMPLETE		BIT(9)
#define AR934X_DMA_MBOX0_INT_TX_EOM_COMPLETE		BIT(8)
#define AR934X_DMA_MBOX1_INT_TX_COMPLETE		BIT(7)
#define AR934X_DMA_MBOX0_INT_TX_COMPLETE		BIT(6)

#define AR934X_DMA_REG_MBOX_FIFO_RESET			0x58
#define AR934X_DMA_REG_SLIC_MBOX_FIFO_RESET		0x5c

#define AR934X_DMA_MBOX_FIFO_RESET_ALL			0xff
#define AR934X_DMA_MBOX1_FIFO_RESET_RX			BIT(3)
#define AR934X_DMA_MBOX0_FIFO_RESET_RX			BIT(2)
#define AR934X_DMA_MBOX1_FIFO_RESET_TX			BIT(1)
#define AR934X_DMA_MBOX0_FIFO_RESET_TX			BIT(0)

extern spinlock_t ath79_pcm_lock;

extern void __iomem *ath79_dma_io_base;

static inline void ath79_dma_wr(unsigned reg, u32 val)
{
	__raw_writel(val, ath79_dma_io_base + reg);
}

static inline u32 ath79_dma_rr(unsigned reg)
{
	return __raw_readl(ath79_dma_io_base + reg);
}

struct ath79_pcm_desc {
	unsigned int	OWN	:  1,    /* bit 31 */
			EOM	:  1,    /* bit 30 */
			rsvd1	:  6,    /* bit 29-24 */
			size	: 12,    /* bit 23-12 */
			length	: 12,    /* bit 11-00 */
			rsvd2	:  4,    /* bit 31-28 */
			BufPtr	: 28,    /* bit 27-00 */
			rsvd3	:  4,    /* bit 31-28 */
			NextPtr	: 28;    /* bit 27-00 */

	unsigned int Va[6];
	unsigned int Ua[6];
	unsigned int Ca[6];
	unsigned int Vb[6];
	unsigned int Ub[6];
	unsigned int Cb[6];

	/* Software specific data
	 * These data are not taken into account by the HW */
	struct list_head list; /* List linking all the buffer in virt@ space */
	dma_addr_t phys; /* Physical address of the descriptor */
};

struct ath79_pcm_rt_priv {
	struct list_head dma_head;
	struct ath79_pcm_desc *last_played;
	unsigned int elapsed_size;
	unsigned int delay_time;
	int direction;
};

/* Replaces struct ath_i2s_softc */
struct ath79_pcm_pltfm_priv {
	struct snd_pcm_substream *playback;
	struct snd_pcm_substream *capture;
};

/* platform data */
extern struct snd_soc_platform_driver ath79_soc_platform;

void ath79_mbox_interrupt_ack(u32);
void ath79_mbox_dma_start(struct ath79_pcm_rt_priv *);
void ath79_mbox_dma_stop(struct ath79_pcm_rt_priv *);
void ath79_mbox_dma_prepare(struct ath79_pcm_rt_priv *);
int ath79_mbox_dma_map(struct ath79_pcm_rt_priv *, dma_addr_t, int,int);
void ath79_mbox_dma_unmap(struct ath79_pcm_rt_priv *);
int ath79_mbox_dma_init(struct device *);
void ath79_mbox_dma_exit(void);
void ath79_mbox_dma_reset(void);

static inline unsigned int ath79_pcm_set_own_bits(struct ath79_pcm_rt_priv *rtpriv)
{
	struct ath79_pcm_desc *desc;
	unsigned int size_played = 0;

	spin_lock(&ath79_pcm_lock);
	list_for_each_entry(desc, &rtpriv->dma_head, list) {
		if (desc->OWN == 0) {
			desc->OWN = 1;
			size_played += desc->size;
		}
	}
	spin_unlock(&ath79_pcm_lock);
	return size_played;
}

static inline void ath79_pcm_clear_own_bits(struct ath79_pcm_rt_priv *rtpriv)
{
	struct ath79_pcm_desc *desc;

	spin_lock(&ath79_pcm_lock);
	list_for_each_entry(desc, &rtpriv->dma_head, list) {
		if (desc->OWN == 1) {
			desc->OWN = 0;
		}
	}
	spin_unlock(&ath79_pcm_lock);
}

static inline struct ath79_pcm_desc *ath79_pcm_get_last_played(struct ath79_pcm_rt_priv *rtpriv)
{
	struct ath79_pcm_desc *desc, *prev;

	spin_lock(&ath79_pcm_lock);
	prev = list_entry(rtpriv->dma_head.prev, struct ath79_pcm_desc, list);
	list_for_each_entry(desc, &rtpriv->dma_head, list) {
		if (desc->OWN == 1 && prev->OWN == 0) {
			return desc;
		}
		prev = desc;
	}
	spin_unlock(&ath79_pcm_lock);

	/* If we didn't find the last played buffer, return NULL */
	return NULL;
}

#endif /* _ATH_PCM_H_ */
