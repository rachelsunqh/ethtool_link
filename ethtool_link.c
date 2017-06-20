
//#include "internal.h"
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <sys/utsname.h>
#include <limits.h>
#include <ctype.h>

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/sockios.h>
#include <linux/ethtool.h>


/* Context for sub-commands */
struct cmd_context {
	const char *devname;	/* net device name */
	int fd;			/* socket suitable for ethtool ioctl */
	struct ifreq ifr;	/* ifreq suitable for ethtool ioctl */
	int argc;		/* number of arguments to the sub-command */
	char **argp;		/* arguments to the sub-command */
};


int send_ioctl(struct cmd_context *ctx, void *cmd)
{
        ctx->ifr.ifr_data = cmd;
        return ioctl(ctx->fd, SIOCETHTOOL, &ctx->ifr);
}

static int do_gset(struct cmd_context *ctx)
{
	int err;
	struct ethtool_value edata;

	fprintf(stdout, "Settings for %s:\n", ctx->devname);

	edata.cmd = ETHTOOL_GLINK;
	err = send_ioctl(ctx, &edata);
	if (err == 0) {
		fprintf(stdout, "	Link detected: %s\n",
			edata.data ? "yes":"no");
	} else if (errno != EOPNOTSUPP) {
		perror("Cannot get link status");
	}

	return 0;
}
int main(int argc, char **argp)
{
	int (*func)(struct cmd_context *);
	int want_device;
	struct cmd_context ctx;
	int k;

	/* Skip command name */
	argp++;
	argc--;

	/* First argument must be either a valid option or a device
	 * name to get settings for (which we don't expect to begin
	 * with '-').
	 */
	if (argc == 0){
		printf("%s device name\n",__func__);
		return -1;
	}

	func = do_gset;
	want_device = 1;

	if (want_device) {
		ctx.devname = *argp++;
		argc--;

		if (ctx.devname == NULL){
			printf("devname == NULL!\n");
			return -3;
		}
		if (strlen(ctx.devname) >= IFNAMSIZ){
			printf("devname len's error!\n");
			return -2;
		}

		/* Setup our control structures. */
		memset(&ctx.ifr, 0, sizeof(ctx.ifr));
		strcpy(ctx.ifr.ifr_name, ctx.devname);

		/* Open control socket. */
		ctx.fd = socket(AF_INET, SOCK_DGRAM, 0);
		if (ctx.fd < 0) {
			perror("Cannot get control socket");
			return 70;
		}
	} else {
		ctx.fd = -1;
	}

	ctx.argc = argc;
	ctx.argp = argp;

	return func(&ctx);
}
