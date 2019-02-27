/* SPDX-License-Identifier: GPL-2.0 */

/*
 * Copyright 2008-2009 ARM Limited. All rights reserved.
 */

/*!
 * \file    messagebox.h
 * \brief   driver for simple messagebox device
 *
 */

/*! Defines the interface to a simple messagebox device driver
 *
 * The intention with the messagebox device is to provide a very simple
 * interface for sending and receiving packets of information to the host
 * side device. It should be possible to encapsulate all target OS locking
 * and barriers within the messagebox.
 *
 * The current implementation is very basic, supporting enough functionality
 * for the VFS blocking model to work correctly.
 *
 * TODO:
 * split the interface so that the buffer is allocated/freed
 * separate to send/receive?
 */

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

/*! Instantiate a new messagebox driver
 *
 * \param dev_base physical base address of device registers+buffer
 * \param dev_irq  irq number of device
 *
 * \return opaque message box structure for use in other calls
 */
struct message_box *mb_new(phys_addr_t dev_base, u32 dev_irq);

/*! free resources assocuated with the messagebox handle
 *
 * \param mb    messagebox handle
 */
void mb_delete(struct message_box *mb);

/*! Reserve resources for sending a message
 *
 * \param mb    messagebox handle
 * \param len   maximum length of message that will be sent
 *
 * \returns     pointer to buffer to fill with message
 */
void *mb_start(struct message_box *mb, u32 len);

/*! Send and release the buffer obtained with mb_start
 *
 * \param mb    messagebox handle
 * \param len   actual length of message in buffer
 *
 * \return      non-zero if a reply message is ready to be received
 */
int mb_end(struct message_box *mb, u32 len);

/*! Check whether receive data is available
 *
 * \param mb    messagebox handle
 *
 * \return      1 for data available. 0 for none available
 *
 * Allows drivers to poll for receive data (rather than using interrupts)
 */
int mb_ready(struct message_box *mb);

/*! Wait for receive data to become available
 *
 * \param mb    messagebox handle
 *
 * \return      0 for data available. -ve for error (e.g. -EINTR)
 *
 * This function will block until there is receive data available
 */
int mb_wait(struct message_box *mb);

/*! lock the messagebox for exclusive access by a thread
 *
 * \param mb    messagebox handle
 *
 * \return      0 for lockable, -ve for error (e.g. -EINTR)
 *
 * This function will block until the message box is free and locked,
 * or until the thread is interrupted
 */
int mb_lock(struct message_box *mb);

/*! unlock the messagebox and allow it to be used by another thread
 *
 * \param mb    messagebox handle
 */
void mb_unlock(struct message_box *mb);

/*! Receive an incoming (reply) message
 *
 * \param mb    messagebox handle
 * \param len   pointer to instance to receive length of message
 *
 * \return      pointer to buffer containing received message
 *
 * Message data should be copied from the buffer before the call returns
 */
void *mb_receive(struct message_box *mb, u32 *len);

/*! Get the device configured id
 *
 * \param mb    messagebox handle
 *
 * \return      value configured in the 'id' parameter in the lisa model
 */
u32 mb_id(struct message_box *mb);

#endif /* MESSAGEBOX_H */
