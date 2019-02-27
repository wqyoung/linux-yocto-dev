/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _LINUX_GETOPT_H
#define _LINUX_GETOPT_H

struct option {
	const char *name;
	unsigned long flag;
	int val;
};

int vmfs_getopt(char *caller, char **options, struct option *opts,
		char **optopt, char **optarg, unsigned long *flag,
		unsigned long *value);

#endif /* _LINUX_GETOPT_H */
