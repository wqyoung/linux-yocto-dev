// SPDX-License-Identifier: GPL-2.0

/*
 * Copyright 2008-2009 ARM Limited. All rights reserved.
 */

/*!
 * \file    vfs.cpp
 * \brief   target side vfs implementation in C
 *
 * The vfs functions have been renamed to vfsop to avoid
 * symbol clashes in linux. We should standardise on one or the other.
 */

/* linux kernel requires different includes */

#include <linux/mm.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/slab.h>

#include "messagebox.h"
#include "msg.h"
#include "vfs.h"

/********************************************************************
 * vfs layer implementation
 *
 * VFS operations - these must match those defined in VFS.h
 ********************************************************************/
enum vfs_op {
	VFS_OPENMOUNTS,
	VFS_READMOUNTS,
	VFS_CLOSEMOUNTS,

	VFS_OPENDIR,
	VFS_READDIR,
	VFS_CLOSEDIR,
	VFS_MKDIR,
	VFS_RMDIR,
	VFS_REMOVE,
	VFS_RENAME,
	VFS_GETATTR,
	VFS_SETATTR,

	VFS_OPENFILE,
	VFS_CLOSEFILE,
	VFS_WRITEFILE,
	VFS_READFILE,
	VFS_GETFILESIZE,
	VFS_SETFILESIZE,
	VFS_FILESYNC,

	VFS_SYMLINK,
	VFS_READLINK
};

/********************************************************************
 * maximum _data_ transfer in a message, this must allow for
 * other message parameters
 * \todo it should be derived from the maximum message size
 ********************************************************************/
#define VFS_MAX_DATA 4096
#define VFS_MAX_MSG  8192

struct VFS {
	struct message_box *mb;
	struct message_composer *mc;
	struct message_decomposer *md;

	int last_err;
};

void vfsop_init(struct VFS *vfs, struct message_box *mb)
{
	vfs->mb = mb;
	vfs->mc = msgc_new(NULL, 0);
	vfs->md = msgd_new(NULL, 0);

	vfs->last_err = 0;
}

void vfsop_cleanup(struct VFS *vfs)
{
	vfs->mb = NULL;
	msgc_delete(vfs->mc);
	vfs->mc = NULL;
	msgd_delete(vfs->md);
	vfs->md = NULL;
}

struct VFS *vfsop_new(struct message_box *mb)
{
	struct VFS *vfs = kmalloc(sizeof(*vfs), GFP_KERNEL);

	/* vfs should check that MB is actually a VFS mb */

	if (vfs)
		vfsop_init(vfs, mb);

	return vfs;
}

void vfsop_delete(struct VFS *vfs)
{
	vfsop_cleanup(vfs);
	kfree(vfs);
}

int vfsop_startcall(struct VFS *vfs, u32 op)
{
	void *buffer;

	if (mb_lock(vfs->mb) < 0)
		return -1;

	buffer = mb_start(vfs->mb, VFS_MAX_MSG);

	msgc_init(vfs->mc, buffer, VFS_MAX_MSG);

	msgc_put_uint32(vfs->mc, 0);	/* message id */
	msgc_put_uint32(vfs->mc, op);	/* vfs operation */

	return 0;
}

void vfsop_call(struct VFS *vfs)
{
	void *buffer;
	u32 blen;
	u32 id;

	/* int ret = */ mb_end(vfs->mb, msgc_get_size(vfs->mc));

	msgc_cleanup(vfs->mc);

	/* todo - this can currently return -1 if the thread was interrupted.
	 * we probably don't want to support interruption during the call
	 */
	mb_wait(vfs->mb);

	buffer = mb_receive(vfs->mb, &blen);

	msgd_init(vfs->md, buffer, blen);

	msgd_get_uint32(vfs->md, &id);	/* message id inserted above */

	/* todo - check the id's match */
}

void vfsop_endcall(struct VFS *vfs)
{
	msgd_cleanup(vfs->md);

	mb_unlock(vfs->mb);
}

s32 vfsop_openmounts(struct VFS *vfs)
{
	s32 handle;

	vfsop_startcall(vfs, VFS_OPENMOUNTS);

	vfsop_call(vfs);
	msgd_get_int32(vfs->md, &handle);

	vfsop_endcall(vfs);

	return handle;
}

s32 vfsop_readmounts(struct VFS *vfs, s32 handle, u32 attr,
		     u8 *attrdata, u32 attrdatalen)
{
	s32 ret;

	vfsop_startcall(vfs, VFS_READMOUNTS);

	msgc_put_int32(vfs->mc, handle);
	msgc_put_uint32(vfs->mc, attr);
	msgc_put_uint32(vfs->mc, attrdatalen);

	vfsop_call(vfs);

	msgd_get_int32(vfs->md, &ret);
	msgd_get_data(vfs->md, attrdata, &attrdatalen);

	vfsop_endcall(vfs);

	return ret;
}

s32 vfsop_closemounts(struct VFS *vfs, s32 handle)
{
	s32 ret;

	ret = vfsop_startcall(vfs, VFS_READMOUNTS);
	if (ret < 0)
		return ret;

	msgc_put_int32(vfs->mc, handle);

	vfsop_call(vfs);

	msgd_get_int32(vfs->md, &ret);

	vfsop_endcall(vfs);

	return ret;
}

s32 vfsop_opendir(struct VFS *vfs, const char *dirname)
{
	s32 ret;

	vfsop_startcall(vfs, VFS_OPENDIR);

	msgc_put_cstr(vfs->mc, dirname);

	vfsop_call(vfs);

	msgd_get_int32(vfs->md, &ret);

	vfsop_endcall(vfs);

	return ret;
}

s32 vfsop_readdir(struct VFS *vfs, s32 handle, u32 attr,
		  u8 *attrdata, u32 attrdatalen)
{
	s32 ret;

	vfsop_startcall(vfs, VFS_READDIR);

	msgc_put_int32(vfs->mc, handle);
	msgc_put_uint32(vfs->mc, attr);
	msgc_put_uint32(vfs->mc, attrdatalen);

	vfsop_call(vfs);

	msgd_get_int32(vfs->md, &ret);
	msgd_get_data(vfs->md, attrdata, &attrdatalen);

	vfsop_endcall(vfs);

	return ret;
}

s32 vfsop_closedir(struct VFS *vfs, s32 handle)
{
	s32 ret;

	vfsop_startcall(vfs, VFS_CLOSEDIR);

	msgc_put_int32(vfs->mc, handle);

	vfsop_call(vfs);

	msgd_get_int32(vfs->md, &ret);

	vfsop_endcall(vfs);

	return ret;
}

s32 vfsop_mkdir(struct VFS *vfs, const char *name)
{
	s32 ret;

	vfsop_startcall(vfs, VFS_MKDIR);

	msgc_put_cstr(vfs->mc, name);

	vfsop_call(vfs);

	msgd_get_int32(vfs->md, &ret);

	vfsop_endcall(vfs);

	return ret;
}

s32 vfsop_rmdir(struct VFS *vfs, const char *name)
{
	s32 ret;

	vfsop_startcall(vfs, VFS_RMDIR);

	msgc_put_cstr(vfs->mc, name);

	vfsop_call(vfs);

	msgd_get_int32(vfs->md, &ret);

	vfsop_endcall(vfs);

	return ret;
}

s32 vfsop_remove(struct VFS *vfs, const char *name)
{
	s32 ret;

	vfsop_startcall(vfs, VFS_REMOVE);

	msgc_put_cstr(vfs->mc, name);

	vfsop_call(vfs);

	msgd_get_int32(vfs->md, &ret);

	vfsop_endcall(vfs);

	return ret;
}

s32 vfsop_rename(struct VFS *vfs, const char *oldname, const char *newname)
{
	s32 ret;

	vfsop_startcall(vfs, VFS_RENAME);

	msgc_put_cstr(vfs->mc, oldname);
	msgc_put_cstr(vfs->mc, newname);

	vfsop_call(vfs);

	msgd_get_int32(vfs->md, &ret);

	vfsop_endcall(vfs);

	return ret;
}

s32 vfsop_getattr(struct VFS *vfs, const char *name, u32 attr,
		  u8 *attrdata, u32 attrdatalen)
{
	s32 ret;

	vfsop_startcall(vfs, VFS_GETATTR);

	msgc_put_cstr(vfs->mc, name);
	msgc_put_uint32(vfs->mc, attr);
	msgc_put_uint32(vfs->mc, attrdatalen);

	vfsop_call(vfs);

	msgd_get_int32(vfs->md, &ret);
	msgd_get_data(vfs->md, attrdata, &attrdatalen);

	vfsop_endcall(vfs);

	return ret;
}

s32 vfsop_setattr(struct VFS *vfs, const char *name, u32 attr,
		  const u8 *attrdata, u32 attrdatalen)
{
	s32 ret;

	vfsop_startcall(vfs, VFS_SETATTR);

	msgc_put_cstr(vfs->mc, name);
	msgc_put_uint32(vfs->mc, attr);
	msgc_put_data(vfs->mc, attrdata, attrdatalen);

	vfsop_call(vfs);

	msgd_get_int32(vfs->md, &ret);

	vfsop_endcall(vfs);

	return ret;
}

s32 vfsop_openfile(struct VFS *vfs, const char *name, u32 flags)
{
	s32 ret;

	vfsop_startcall(vfs, VFS_OPENFILE);

	msgc_put_cstr(vfs->mc, name);
	msgc_put_uint32(vfs->mc, flags);

	vfsop_call(vfs);

	msgd_get_int32(vfs->md, &ret);

	vfsop_endcall(vfs);

	return ret;
}

s32 vfsop_closefile(struct VFS *vfs, s32 handle)
{
	s32 ret;

	vfsop_startcall(vfs, VFS_CLOSEFILE);

	msgc_put_int32(vfs->mc, handle);

	vfsop_call(vfs);

	msgd_get_int32(vfs->md, &ret);

	vfsop_endcall(vfs);

	return ret;
}

s32 vfsop_writefile(struct VFS *vfs, s32 handle, u64 offset,
		    const void *data, s32 len)
{
	s32 ret;
	s32 residual = len;

	/* Transfer has to be broken into manageable chunks */

	while (residual > 0) {
		s32 t_len =
			(residual > VFS_MAX_DATA) ? VFS_MAX_DATA : residual;

		vfsop_startcall(vfs, VFS_WRITEFILE);

		msgc_put_int32(vfs->mc, handle);
		msgc_put_uint64(vfs->mc, offset);
		msgc_put_data(vfs->mc, data, t_len);
		msgc_put_uint32(vfs->mc, t_len);

		vfsop_call(vfs);

		msgd_get_int32(vfs->md, &ret);

		vfsop_endcall(vfs);

		if (ret < 0)
			return ret;

		offset += ret;
		residual -= ret;
		data = (u8 *)data + ret;

		if (ret < t_len)
			break;
	}

	return len - residual;
}

s32 vfsop_readfile(struct VFS *vfs, s32 handle, u64 offset, void *data,
		   s32 len)
{
	s32 ret;
	s32 residual = len;
	u32 rlen = len;

	/* data must be sent in manageable chunks */

	while (residual > 0) {
		s32 t_len =
			(residual > VFS_MAX_DATA) ? VFS_MAX_DATA : residual;

		vfsop_startcall(vfs, VFS_READFILE);

		msgc_put_int32(vfs->mc, handle);
		msgc_put_uint64(vfs->mc, offset);
		msgc_put_uint32(vfs->mc, t_len);

		vfsop_call(vfs);

		msgd_get_int32(vfs->md, &ret);
		msgd_get_data(vfs->md, data, &rlen);

		vfsop_endcall(vfs);

		if (ret < 0)
			return ret;

		offset += ret;
		residual -= ret;
		data = (u8 *)data + ret;

		if (ret < t_len)
			break;
	}

	return len - residual;
}

s32 vfsop_getfilesize(struct VFS *vfs, s32 handle, u64 *size)
{
	s32 ret;

	vfsop_startcall(vfs, VFS_GETFILESIZE);

	msgc_put_int32(vfs->mc, handle);

	vfsop_call(vfs);

	msgd_get_int32(vfs->md, &ret);
	msgd_get_uint64(vfs->md, size);

	vfsop_endcall(vfs);

	return ret;
}

s32 vfsop_setfilesize(struct VFS *vfs, s32 handle, u64 size)
{
	s32 ret;

	vfsop_startcall(vfs, VFS_SETFILESIZE);

	msgc_put_int32(vfs->mc, handle);
	msgc_put_uint64(vfs->mc, size);

	vfsop_call(vfs);

	msgd_get_int32(vfs->md, &ret);

	vfsop_endcall(vfs);

	return ret;
}

s32 vfsop_filesync(struct VFS *vfs, s32 handle)
{
	s32 ret;

	vfsop_startcall(vfs, VFS_FILESYNC);

	msgc_put_int32(vfs->mc, handle);

	vfsop_call(vfs);

	msgd_get_int32(vfs->md, &ret);

	vfsop_endcall(vfs);

	return ret;
}

s32 vfsop_symlink(struct VFS *vfs, const char *filename, const char *symlink)
{
	s32 ret;

	vfsop_startcall(vfs, VFS_SYMLINK);

	msgc_put_cstr(vfs->mc, filename);
	msgc_put_cstr(vfs->mc, symlink);

	vfsop_call(vfs);

	msgd_get_int32(vfs->md, &ret);

	vfsop_endcall(vfs);

	return ret;
}

s32 vfsop_readlink(struct VFS *vfs, const char *filename, char *buf,
		   s32 bufsiz)
{
	s32 ret;
	u32 rlen;

	vfsop_startcall(vfs, VFS_READLINK);

	msgc_put_cstr(vfs->mc, filename);
	msgc_put_int32(vfs->mc, bufsiz);

	vfsop_call(vfs);

	msgd_get_int32(vfs->md, &ret);
	msgd_get_data(vfs->md, buf, &rlen);

	vfsop_endcall(vfs);

	return ret;
}
