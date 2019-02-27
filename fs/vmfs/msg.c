// SPDX-License-Identifier: GPL-2.0

/*
 * Copyright 2008-2009 ARM Limited. All rights reserved.
 */

/*
 *  Utility types/functions for constructing/deconstructing blocks
 *  of data to be sent over a message box between the target and host
 *  (and vice versa) this implementation is converted to be
 *  used in the linux kernel.  These must match the behaviour in
 *  the equivalent C++ classes used on the host side
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/slab.h>

#include "msg.h"

enum msg_data_type {
	MSG_END,	/* (potential) marker for end of message data */
	MSG_UINT32,	/* 32 bit data */
	MSG_UINT64,	/* 64 bit data */
	MSG_INT32,	/* 32 bit data */
	MSG_CSTR,	/* zero terminated c string */
	MSG_DATA,	/* raw data */
	MSG_CHAR,	/* single character */
	MSG_BOOL	/* packed int? */
};

enum msg_traits {
	TYPE_SHIFT = 0,
	TYPE_BITS = 8,
	TYPE_MASK = (1 << TYPE_BITS) - 1,

	LEN_SHIFT = TYPE_BITS,
	LEN_BITS = 20,
	MAX_LEN = 1 << LEN_BITS,
	LEN_MASK = MAX_LEN - 1
};

struct message_composer {
	u8 *b_data;	/* message data */
	u32 b_size;	/* buffer size */
	u32 b_index;	/* offset to next byte to fill */
};

void msgc_init(struct message_composer *mc, void *data, u32 len)
{
	mc->b_data = data;
	mc->b_size = len;
	mc->b_index = 0;
}

void msgc_cleanup(struct message_composer *mc)
{
}

struct message_composer *msgc_new(void *data, u32 len)
{
	struct message_composer *mc =
		kmalloc(sizeof(struct message_composer), GFP_KERNEL);

	if (mc)
		msgc_init(mc, data, len);

	return mc;
}

void msgc_delete(struct message_composer *mc)
{
	msgc_cleanup(mc);
	kfree(mc);
}

static int msgc_put(struct message_composer *mc, enum msg_data_type type,
		    const void *data, u32 len)
{
	u32 tag;

	if (len >= mc->b_size)
		return 0;

	if (!mc->b_data)
		return 0;

	tag = (len << LEN_SHIFT) | ((u32)type);

	*(u32 *)(mc->b_data + mc->b_index) = tag;
	mc->b_index += 4;

	memcpy(mc->b_data + mc->b_index, data, len);
	mc->b_index += len;

	mc->b_index = (mc->b_index + 3) & ~3;	/* word align */

	return 1;
}

int msgc_put_int32(struct message_composer *mc, s32 data)
{
	return msgc_put(mc, MSG_INT32, (void *)&data, sizeof(s32));
}

int msgc_put_uint32(struct message_composer *mc, u32 data)
{
	return msgc_put(mc, MSG_UINT32, (void *)&data, sizeof(u32));
}

int msgc_put_uint64(struct message_composer *mc, u64 data)
{
	return msgc_put(mc, MSG_UINT64, (void *)&data, sizeof(u64));
}

int msgc_put_cstr(struct message_composer *mc, const char *data)
{
	return msgc_put(mc, MSG_CSTR, (void *)data, strlen(data) + 1);
}

int msgc_put_data(struct message_composer *mc, const void *data, u32 len)
{
	return msgc_put(mc, MSG_DATA, data, len);
}

u32 msgc_get_size(struct message_composer *mc)
{
	return mc->b_index;
}

struct message_decomposer {
	const u8 *b_data;	/* message data */
	u32 b_size;	/* size of buffer */
	u32 b_index;	/* current index into buffer */
};

static int msgd_get(struct message_decomposer *md, enum msg_data_type type,
		    void *data, u32 *len)
{
	u32 tag;
	u32 d_len;
	enum msg_data_type d_type;

	if (md->b_index + 4 > md->b_size)
		return 0;

	tag = *(u32 *)(md->b_data + md->b_index);

	d_type = (enum msg_data_type)((tag >> TYPE_SHIFT) & TYPE_MASK);
	d_len = ((tag >> LEN_SHIFT) & LEN_MASK);

	if (d_type != type)
		return 0;

	md->b_index += 4;

	if (md->b_index + d_len > md->b_size)
		return 0;

	if (*len > d_len)
		*len = d_len;

	memcpy(data, md->b_data + md->b_index, *len);

	md->b_index += d_len;

	md->b_index = (md->b_index + 3) & ~3;	/* word align */

	return 1;
}

void msgd_init(struct message_decomposer *md, const void *data, u32 len)
{
	md->b_data = (const unsigned char *)data;
	md->b_size = len;
	md->b_index = 0;
}

void msgd_cleanup(struct message_decomposer *md)
{
}

struct message_decomposer *msgd_new(const void *data, u32 len)
{
	struct message_decomposer *md =
		kmalloc(sizeof(struct message_decomposer), GFP_KERNEL);

	if (md)
		msgd_init(md, data, len);

	return md;
}

void msgd_delete(struct message_decomposer *md)
{
	msgd_cleanup(md);
	kfree(md);
}

/* for decomposing */

int msgd_get_int32(struct message_decomposer *md, s32 *data)
{
	u32 len = sizeof(s32);

	return msgd_get(md, MSG_INT32, (void *)data, &len);
}

int msgd_get_uint32(struct message_decomposer *md, u32 *data)
{
	u32 len = sizeof(u32);

	return msgd_get(md, MSG_UINT32, (void *)data, &len);
}

int msgd_get_uint64(struct message_decomposer *md, u64 *data)
{
	u32 len = sizeof(u64);

	return msgd_get(md, MSG_UINT64, (void *)data, &len);
}

int msgd_get_cstr(struct message_decomposer *md, char *data, unsigned int *len)
{
	return msgd_get(md, MSG_CSTR, (void *)data, len);
}

int msgd_get_data(struct message_decomposer *md, void *data, u32 *len)
{
	return msgd_get(md, MSG_DATA, data, len);
}
