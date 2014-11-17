/*
 * Copyright (C) 2013 Felix Fietkau <nbd@openwrt.org>
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "../log.h"
#include "init.h"

static void
early_mounts(void)
{
	mount("proc", "/proc", "proc", MS_NOATIME, 0);//内核proc设备；挂载到/proc目录下；文件系统类型为proc;挂载模式；文件系统权限及读写标志。
	mount("sysfs", "/sys", "sysfs", MS_NOATIME, 0);

	mount("tmpfs", "/tmp", "tmpfs", MS_NOSUID | MS_NODEV | MS_NOATIME, NULL);
	mkdir("/tmp/run", 0777);
	mkdir("/tmp/lock", 0777);
	mkdir("/tmp/state", 0777);
	symlink("/tmp", "/var");//为/tmp目录建立一符号链接到/var目录，访问/tmp目录即是访问/var目录。

	mount("tmpfs", "/dev", "tmpfs", MS_NOATIME, "mode=0755,size=512K");
	mkdir("/dev/shm", 0755);
	mkdir("/dev/pts", 0755);
	mount("devpts", "/dev/pts", "devpts", MS_NOATIME, "mode=600");
}

static void
early_dev(void)
{
	mkdev("*", 0600);
	mknod("/dev/null", 0666, makedev(1, 3));//参数1:设备节点的完整路径及名称；访问权限；makedev在内核中创建设备文件(主设备号，次设备号)
}///dev/null这一设备文件在此处创建，/dev/console这一设备文件在何处创建??

static void
early_console(const char *dev)
{
	struct stat s;
	int dd;

	if (stat(dev, &s)) {
		ERROR("Failed to stat %s\n", dev);
		return;
	}

	dd = open(dev, O_RDWR);
	if (dd < 0)
		dd = open("/dev/null", O_RDWR);

	dup2(dd, STDIN_FILENO);//dup2()，为文件句柄赋值(此处将linux的标准输入标准输出都设置成/dev/console文件)
	dup2(dd, STDOUT_FILENO);
	dup2(dd, STDERR_FILENO);

	if (dd != STDIN_FILENO &&
	    dd != STDOUT_FILENO &&
	    dd != STDERR_FILENO)
		close(dd);
}

static void
early_env(void)
{
	setenv("PATH", "/bin:/sbin:/usr/bin:/usr/sbin", 1);
}

void
early(void)
{
	if (getpid() != 1)//getpid()获取当前进程的pid号，1为procd进程的pid号。
		return;

	early_mounts();
	early_dev();
	early_env();
	early_console("/dev/console");

	LOG("Console is alive\n");
}
