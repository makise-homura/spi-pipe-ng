A more configurable replacement for `spi-pipe` that allows different read and write sizes

# Build

## General build flow

```
meson build
cd build
ninja
ninja install
```

## Options

* You may specify `-Dprefix=<PREFIX>` to override default install prefix. Resulting binary will be installed to `<PREFIX>/bin/spi-pipe-ng`.
* You may specify `-Dstatic=enabled` if you wish to build static binary.

## Build requirements

* `<limits.h>` should have `PATH_MAX` defined.

# Usage

```
spi-pipe-ng -h
```

```
spi-pipe-ng -v
```

```
<CMD1> | spi-pipe-ng -d <DEV> 
```

```
<CMD1> | spi-pipe-ng -d <DEV> -r <N> | <CMD2>
```

Writes output of `<CMD1>` (max 64 bytes) to spidev `<DEV>`. If `-r` specified, then, after write, reads `<N>` (max 64) bytes from `<DEV>` and pipe it to `<CMD2>`.

* `-v`: show version and exit
* `-h`: show help and exit
* `-d <DEV>`: spidev device to use (e.g. `/dev/spidev0.0`)
* `-r <N>`: read `<N>` bytes back after write

# Examples

## Identify SPI flash

Other words, issue RDID (0x9F) command and print out 3 identification bytes in hex:

```
echo -ne '\x9f' | spi-pipe-ng -d /dev/spidev0.0 -r 3 | hexdump -C
```

## Read SPI flash status register

Other words, issue RDSR (0x05) command and print out 1 byte:

```
echo -ne '\x05' | spi-pipe-ng -d /dev/spidev0.0 -r 1 | hexdump -C
```

## Set BP bytes in SPI flash status register:

Other words, issue WREN (0x06) command, then WRSR (0x01) with 0x1C (0b00011100, so set BP{0,1,2} to 1) as payload, and then WRDI (0x04):

```
echo -ne '\x06' | spi-pipe-ng -d /dev/spidev0.0
echo -ne '\x01\x1C' | spi-pipe-ng -d /dev/spidev0.0
echo -ne '\x04' | spi-pipe-ng -d /dev/spidev0.0
```
