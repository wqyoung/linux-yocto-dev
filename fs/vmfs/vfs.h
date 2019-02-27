/* SPDX-License-Identifier: GPL-2.0 */

/*!
 * \file    vfs.h
 * \brief   target side vfs interface in C
 *
 * This interface has been renamed to VFS (and the operations to vfsop) to
 * avoid symbol clashes in linux. We should standardise on one or the other.
 */

#ifndef VFS_H
#define VFS_H

#include "messagebox.h"

/* get these definitions from a shared header used on both host/target side */

/* Objects types that can exist on a filesystem */
enum vfs_type {
	VFS_TYPE_NONE,		/* file not found */
	VFS_TYPE_FILE,		/* regular file */
	VFS_TYPE_DIR,		/* directory */
	VFS_TYPE_LINK,		/* symbolic link */
	VFS_TYPE_UNKNOWN,	/* unknown object type */
	VFS_TYPE_MOUNT		/* mount point */
};

/* \todo these should probably be +ve and return as -VFS_ERR_ etc. */
enum vfs_error {
	VFS_ERR_OK = 0,		/* all ok (actually 0 or +ve means ok) */
	VFS_ERR_BADHANDLE = -1,	/* invalid or wrong type of handle */
	VFS_ERR_NOENTRY = -2,	/* no more entries in a directory */
	VFS_ERR_NOROOM = -3,	/* ran out of memory/buffer/disk space */
	VFS_ERR_MAXHANDLE = -4,	/* ran out of handles */
	VFS_ERR_NOMOUNT = -5,	/* no such mount exists */
	VFS_ERR_NOTFOUND = -6,	/* object not found */
	VFS_ERR_PERM = -7,	/* permission error */
	VFS_ERR_NOTDIR = -8,	/* path element wasn't a directory */
	VFS_ERR_TOOLONG = -9,	/* path or path element too long */
	VFS_ERR_EXIST = -10,	/* an object with the name already exists */
	VFS_ERR_NOTEMPTY = -11,	/* tried to remove dir that wasn't empty */
	VFS_ERR_INVALID = -12,	/* invalid operation or operand */
	VFS_ERR_ISDIR = -13,	/* object is a directory */
	VFS_ERR_TOOBIG = -14,	/* return value too large to represent */
	VFS_ERR_UNIMPL = -15,	/* unimplemented feature */
	VFS_ERR_UNKNOWN = -100	/* unexpected host error */
};

enum vfs_attr {
	VFS_ATTR_MTIME = 0x0001,  /* u64 modification time */
	VFS_ATTR_ACCESS = 0x0002, /* u32 access permissions (r/w/e) */
	VFS_ATTR_TYPE = 0x0004,	  /* u32 obj type (as above) */
	VFS_ATTR_SIZE = 0x0008,	  /* u64 obj size in bytes */
	VFS_ATTR_CTIME = 0x0010,  /* u64 obj creation time */
	VFS_ATTR_ATIME = 0x0020,  /* u64 obj access time */
	VFS_ATTR_RTIME = 0x0040,  /* u64 current real time */
	VFS_ATTR_DISKSIZE = 0x0100,  /* u64 size of disk in bytes */
	VFS_ATTR_DISKFREE = 0x0200,  /* u64 free space on disk in bytes */
	VFS_ATTR_NAME = 0x8000,	  /* char* last to make variable length easy */
};

/* flags passed to Mount::openFile */
enum vfs_open_flags {
	VFS_OPEN_RDONLY = 1,
	VFS_OPEN_WRONLY = 2,
	VFS_OPEN_RDWR = VFS_OPEN_RDONLY | VFS_OPEN_WRONLY,
	VFS_OPEN_CREATE = 4,
	VFS_OPEN_NEW = 8,
	VFS_OPEN_TRUNCATE = 16
};

/*! instantiate a new vfs object
 *
 * \param mb         message box instance to use as a transport layer
 *
 * \return           vfs instance handle to use in vfs calls
 */
struct VFS *vfsop_new(struct message_box *mb);

/*! delete a vfs instance
 *
 * \param vfs       instance to delete
 */
void vfsop_delete(struct VFS *vfs);

/*! Open an iterator on the list of mounts added with add Mount
 *
 * \param vfs      vfs instance
 *
 * \return a handle to be used with readmounts/closemounts or vfs_error code
 */
s32 vfsop_openmounts(struct VFS *vfs);

/* Read the next entry in a list of mounts
 *
 * \param vfs       vfs instance
 * \param id        mount iterator handle
 * \param attr      bit mask of attributes to return (one or more vfs_attr)
 * \param attrdata  data block to receive attributes
 * \param attrlen   size of attribute block
 *
 * \return vfs_error code
 *
 * The attribute block is packed with data in vfs_attr order (lowest to
 * highest). Be careful to unpack the attribute block using the correct
 * data sizes. Not all attributes are relavent to mount data
 *
 */
s32 vfsop_readmounts(struct VFS *vfs, s32 handle, u32 attr,
		     u8 *attrdata, u32 attrdatalen);

/* Close a mount iterator handle
 *
 * \param vfs      vfs instance
 * \param id        mount iterator handle
 *
 * \return vfs_error code
 */
s32 vfsop_closemounts(struct VFS *vfs, s32 handle);

/* Open a directory iterator handle
 *
 * \param vfs      vfs instance
 * \param name      full (vfs) path name to directory
 *
 * \return directory iterator handle for use with readdir/closedir
 * or a vfs_error code
 */
s32 vfsop_opendir(struct VFS *vfs, const char *dirname);

/* Read an entry form a directory iterator
 *
 * \param vfs      vfs instance
 * \param id        directory iterator handle
 * \param attr      bit mask of attributes to return (one or more vfs_attr)
 * \param attrdata  data block to receive attributes
 * \param attrlen   size of attribute block
 *
 * \return vfs_error code
 *
 * The attribute block is packed with data in vfs_attr order (lowest to
 * highest). Be careful to unpack the attribute block using the correct
 * data sizes
 *
 * \todo pass attrlen by reference so it can be updated with the size used
 * \todo pass attr by reference so that the actual returned attributes can
 * be indicated
 */
s32 vfsop_readdir(struct VFS *vfs, s32 handle, u32 attr,
		  u8 *attrdata, u32 attrdatalen);

/* Close a directory iterator
 *
 * \param vfs      vfs instance
 * \param id        directory iterator handle
 *
 * \return vfs_error code
 */
s32 vfsop_closedir(struct VFS *vfs, s32 handle);

/* Create a directory
 *
 * \param vfs      vfs instance
 * \param name      (vfs) directory name to create
 *
 * \return vfs_error code
 */
s32 vfsop_mkdir(struct VFS *vfs, const char *name);

/* Remove a directory
 *
 * \param vfs      vfs instance
 * \param name      (vfs) directory name to create
 *
 * \return vfs_error code
 */
s32 vfsop_rmdir(struct VFS *vfs, const char *name);

/* Remove a file
 *
 * \param vfs      vfs instance
 * \param name      (vfs) file to remove (may also work on other
 * object types)
 *
 * \return vfs_error code
 */
s32 vfsop_remove(struct VFS *vfs, const char *name);

/* Rename an object
 *
 * \param vfs      vfs instance
 * \param oldname   (vfs) object to rename
 * \param newname   (vfs) new name of object
 *
 * \return vfs_error code
 */
s32 vfsop_rename(struct VFS *vfs, const char *oldname, const char *newname);

/* Retrieve attributes of an object on the filesystem
 *
 * \param vfs      vfs instance
 * \param name      (vfs) object name
 * \param attr      bit mask of attributes to return (one or more vfs_attr)
 * \param attrdata  data block to receive attributes
 * \param attrlen   size of attribute block
 *
 * \return vfs_error code
 *
 * The attribute block is packed with data in vfs_attr order (lowest to
 * highest). Be careful to unpack the attribute block using the
 * correct data sizes
 *
 * \todo pass attrlen by reference so it can be updated with the size used
 * \todo pass attr by reference so that the actual returned attributes
 * can be indicated
 */
s32 vfsop_getattr(struct VFS *vfs, const char *name, u32 attr,
		  u8 *attrdata, u32 attrdatalen);

/* Retrieve attributes of an object on the filesystem
 *
 * \param vfs      vfs instance
 * \param name      (vfs) object name
 * \param attr      bit mask of attributes to modify (one or more vfs_attr)
 * \param attrdata  data block containing packed attributes
 * \param attrlen   size of attribute block
 *
 * \return vfs_error code
 *
 * The attribute block should be packed with data in vfs_attr order
 * (lowest to highest). Be careful to pack the attribute block using
 * the correct data sizes
 *
 * Not all attributes can be modified using this (e.g. file size/disk
 * free/file name)
 *
 * \todo pass attr by reference so that the actual modified
 * attributes can be indicated
 */
s32 vfsop_setattr(struct VFS *vfs, const char *name, u32 attr,
		  const u8 *attrdata, u32 attrdatalen);

/* Open a file object on the filesystem for reading/writing
 *
 * \param vfs      vfs instance
 * \param filename  (vfs) file name
 * \param flags     vfs_open_flags value indicating how to open the file
 *
 * \return file handle to use with readfile/writefile/closefile etc or
 * a vfs_error code
 */
s32 vfsop_openfile(struct VFS *vfs, const char *name, u32 flags);

/* Close a file object by a handle returned from openfile
 *
 * \param vfs      vfs instance
 * \param id        file handle
 *
 * \return vfs_error code
 */
s32 vfsop_closefile(struct VFS *vfs, s32 handle);

/* Write data to a file
 *
 * \param vfs      vfs instance
 * \param id        file handle returned from openfile
 * \param offset    offset into file from where to start writing
 * \param data      pointer to data block containing data to be written
 * \param len       length of data to be written
 *
 * \return length of data actually written to the file or a vfs_error code
 */
s32 vfsop_writefile(struct VFS *vfs, s32 handle, u64 offset,
		    const void *data, s32 len);

/* Read data from a file
 *
 * \param vfs      vfs instance
 * \param id        file handle returned from openfile
 * \param offset    offset into file from where to start reading
 * \param data      pointer to data block to receive data read from file
 * \param len       size of data block to receive data
 *
 * \return length of data actually read from the file or a vfs_error code
 */
s32 vfsop_readfile(struct VFS *vfs, s32 handle, u64 offset, void *data,
		   s32 len);

/* Get the size of an open file
 *
 * \param vfs      vfs instance
 * \param id        file handle returned from openfile
 * \param size      pointer to instance data to receive file size
 *
 * \return vfs_error code
 */
s32 vfsop_getfilesize(struct VFS *vfs, s32 handle, u64 *size);

/* Set the size of an open file
 *
 * \param vfs      vfs instance
 * \param id        file handle returned from openfile
 * \param size      new size of file
 *
 * \return vfs_error code
 *
 * this will truncate or extend the file depending on whether the new
 * size is smaller or larger than the current file size
 */
s32 vfsop_setfilesize(struct VFS *vfs, s32 handle, u64 size);

/* Force modified parts of a file back to persistent storage
 *
 * \param vfs      vfs instance
 * \param id        file handle returned from openfile
 *
 * \return vfs_error code
 */
s32 vfsop_filesync(struct VFS *vfs, s32 handle);

/* Linux target support functions */

/* Create a symbolic link object
 *
 * \param vfs      vfs instance
 * \param filename  (vfs) name of link object to be created
 * \param data      content of link object (typically a path to
 * another object)
 *
 * \return vfs_error code
 *
 * \todo this is not yet implemented
 */

s32 vfsop_symlink(struct VFS *vfs, const char *filename, const char *symlink);

/* Read the contents of a symbolic link object
 *
 * \param vfs      vfs instance
 * \param filename  (vfs) name of link object to be read
 * \param data      data block to receive link object contents
 * \param bufsiz    size of data block to receive link object contents
 *
 * \return vfs_error code
 *
 * \todo this is not yet implemented
 */
s32 vfsop_readlink(struct VFS *vfs, const char *filename, char *buf,
		   s32 bufsiz);

#endif /* VFS_H */
