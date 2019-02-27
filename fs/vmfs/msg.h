/* SPDX-License-Identifier: GPL-2.0 */

/*
 * Copyright 2008-2009 ARM Limited. All rights reserved.
 */

/*!
 * \file    msg.h
 * \brief   objects and functions to covert function
 * calls into messages and back
 * \todo change return codes, and define some errors
 */

#ifndef MSG_H
#define MSG_H

/*! instantiate a new message composer object that can
 *  compose a message into the supplied buffer
 *
 * \param data  buffer into which messages can be composed (may be NULL)
 * \param len   size of buffer (may be 0)
 *
 * \return message composer object
 */
struct message_composer *msgc_new(void *data, u32 len);

/*! destroy a message composer object
 *
 * \param mc    message composer object
 */
void msgc_delete(struct message_composer *mc);

/*! (re)initialise a message composer object to use a new buffer
 *
 * \param mc    message composer object
 * \param data  buffer into which messages can be composed (may be NULL)
 * \param len   size of buffer (may be 0)
 */
void msgc_init(struct message_composer *mc, void *data, u32 len);

/*! disassociate a message composer object from a buffer
 *
 * \param mc    message composer object
 */
void msgc_cleanup(struct message_composer *mc);

/*! add a signed integer to the message
 *
 * \param mc    message composer object
 * \param data  data to add to message
 *
 * \return 1 for success, 0 for fail
 */
int msgc_put_int32(struct message_composer *mc, s32 data);

/*! add an unsigned signed integer to the message
 *
 * \param mc    message composer object
 * \param data  data to add to message
 *
 * \return 1 for success, 0 for fail
 */
int msgc_put_uint32(struct message_composer *mc, u32 data);

/*! add an unsigned 64bit integer to the message
 *
 * \param mc    message composer object
 * \param data  data to add to message
 *
 * \return 1 for success, 0 for fail
 */
int msgc_put_uint64(struct message_composer *mc, u64 data);

/*! add a zero terminated string to the message
 *
 * \param mc    message composer object
 * \param data  data to add to message
 *
 * \return 1 for success, 0 for fail
 */
int msgc_put_cstr(struct message_composer *mc, const char *data);

/*! add a data block the message
 *
 * \param mc    message composer object
 * \param data  data to add to message
 * \param len   length of data to add to the message
 *
 * \return 1 for success, 0 for fail
 */
int msgc_put_data(struct message_composer *mc, const void *data, u32 len);

/*! return the current size of the message in bytes
 *
 * \param mc    message composer object
 *
 * \return size of the message in bytes
 */
u32 msgc_get_size(struct message_composer *mc);

/*! instantiate a new message decomposer object that can decompose
 *  a message from the supplied buffer
 *
 * \param data  buffer from which messages can be decomposed (may be NULL)
 * \param len   size of buffer (may be 0)
 *
 * \return message decomposer object
 */
struct message_decomposer *msgd_new(const void *data, u32 len);

/*! destroy a message decomposer object
 *
 * \param md    message decomposer object
 */
void msgd_delete(struct message_decomposer *md);

/*! (re)initialise a message decomposer object to use a new buffer
 *
 * \param md    message decomposer object
 * \param data  buffer from which messages can be decomposed (may be NULL)
 * \param len   size of buffer (may be 0)
 */
void msgd_init(struct message_decomposer *md, const void *data, u32 len);

/*! disassociate a message decomposer object from a buffer
 *
 * \param md    message decomposer object
 */
void msgd_cleanup(struct message_decomposer *md);

/*! extract a signed integer from the message
 *
 * \param md    message decomposer object
 * \param data  data to extract from message
 *
 * \return 1 for success, 0 for fail
 */
int msgd_get_int32(struct message_decomposer *md, s32 *data);

/*! extract an unsigned integer from the message
 *
 * \param md    message decomposer object
 * \param data  data to extract from message
 *
 * \return 1 for success, 0 for fail
 */
int msgd_get_uint32(struct message_decomposer *md, u32 *data);

/*! extract an unsigned 64 bit integer from the message
 *
 * \param md    message decomposer object
 * \param data  data to extract from message
 *
 * \return 1 for success, 0 for fail
 */
int msgd_get_uint64(struct message_decomposer *md, u64 *data);

/*! extract a zero terminated C string from the message
 *
 * \param md    message decomposer object
 * \param data  data to extract from message
 * \param len   in: max length to extract, out: length of string extracted
 *
 * \return 1 for success, 0 for fail
 */
int msgd_get_cstr(struct message_decomposer *md, char *data, unsigned int *len);

/*! extract a data block from the message
 *
 * \param md    message decomposer object
 * \param data  data to extract from message
 * \param len   in: max length to extract, out: length of data extracted
 *
 * \return 1 for success, 0 for fail
 */
int msgd_get_data(struct message_decomposer *md, void *data, u32 *len);

#endif /* MSG_H */
