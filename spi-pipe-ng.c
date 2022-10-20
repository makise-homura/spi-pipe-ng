#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

extern const char *vcs_ver;

static void cancel(int num, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    exit(num);
}

int main(int argc, char *argv[])
{
    char dev[PATH_MAX];
    memset(dev, 0, PATH_MAX);

    char wbuf[64];
    char rbuf[64];
    int wlen = 0;
    int rlen = 0;

    for(;;)
    {
        int opt = getopt(argc, argv, "hvd:r:");

        if(opt == -1) break;
        switch(opt)
        {
            case 'h':
                printf("Usage:\n");
                printf("\t%s -h                                :  print this help and exit\n", argv[0]);
                printf("\t%s -v                                :  show version and exit\n", argv[0]);
                printf("\t<CMD1> | %s -d <DEV>                 :  write output of CMD1 (max %lu bytes) to spidev DEV\n", argv[0], sizeof(wbuf));
                printf("\t<CMD1> | %s -d <DEV> -r <N> | <CMD2> :  same, then read N (max %lu) bytes from DEV and pipe it to CMD2\n", argv[0], sizeof(rbuf));
                exit(0);

            case 'v':
                printf("%s (spi-pipe-ng) version %s, git revision %s\n", argv[0], PROJ_VER, vcs_ver);
                exit(0);

            case 'r':
                errno = 0;
                char *endp;
                rlen = strtol(optarg, &endp, 0);
                if (errno || *endp || rlen < 0 || rlen > sizeof(rbuf)) cancel(10, "%s: %s is not a correct length\nRun %s -h for usage)\n", argv[0], optarg, argv[0]);
                break;

            case 'd':
                if (strlen(optarg) > PATH_MAX - 1) cancel(11, "%s: %s is too long to be used as device path\nRun %s -h for usage)\n", argv[0], optarg, argv[0]);
                strncpy(dev, optarg, PATH_MAX - 1);
                break;

            case '?':
                cancel(12, "Run %s -h for usage\n", argv[0]);
        }
    }

    if (!*dev) cancel(21, "%s: empty spidev device is not allowed, please specify it (e.g. /dev/spidev0.0)\n", argv[0]);

    wlen = read(STDIN_FILENO, wbuf, sizeof(wbuf));
    if (wlen == -1) cancel(20, "%s: error reading input data: %s\n", argv[0], strerror(errno));
    if (!wlen) cancel(21, "%s: empty input data is now allowed, pass at least one byte\n", argv[0]);

    struct spi_ioc_transfer spi_msg[2] =
    {
        {
            .tx_buf = (__u64)wbuf,
            .rx_buf = (__u64)NULL,
            .len = wlen,
            .speed_hz = 12500000,
            .delay_usecs = 0,
            .bits_per_word = 8,
            .cs_change = 0
        },
        {
            .tx_buf = (__u64)NULL,
            .rx_buf = (__u64)rbuf,
            .len = rlen,
            .speed_hz = 12500000,
            .delay_usecs = 0,
            .bits_per_word = 8,
            .cs_change = 0
        }
    };

    int fd;
    if((fd = open(dev, O_RDWR)) == -1) cancel(22, "%s: can't open %s: %s\n", argv[0], dev, strerror(errno));
    if (ioctl(fd, SPI_IOC_MESSAGE(rlen ? 2 : 1), spi_msg) == -1) cancel(23, "%s: can't perform SPI_IOC_MESSAGE ioctl: %s\n", argv[0], strerror(errno));;
    if(close(fd) == -1) cancel(24, "%s: can't close %s after performing ioctl: %s\n", argv[0], dev, strerror(errno));
    
    if (rlen)
    {
        int rlen2 = write(STDOUT_FILENO, rbuf, rlen);
        if (rlen2 == -1) cancel(20, "%s: error writing output data: %s\n", argv[0], strerror(errno));
        if (rlen2 != rlen) cancel(21, "%s: written only %d of %d bytes\n", argv[0], rlen2, rlen);
    }
    
    return 0;
}
