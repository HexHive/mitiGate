# RAP plugin

[RAP](https://grsecurity.net/rap_faq.php) is a PaX feature to harden the kernel
and user-land by implementing CFI for C programs through a gcc plugin. The main
documentation of RAP is a
[presentation](https://pax.grsecurity.net/docs/PaXTeam-H2HC15-RAP-RIP-ROP.pdf)
and random tweets by [@PaXTeam](https://twitter.com/paxteam) when they "inform"
people about different aspects of the tool.

Using RAP is somewhat difficult and we refer to a [blog
post](https://nebelwelt.net/blog/20181226-CFIeval.html) on how to evaluate
RAP and LLVM-CFI.

This directory contains a pre-compiled version of the RAP plugin for gcc 6.3.0
(for Debian 9). If you want to compile your own, you'll have to download the
[kernel patch](https://www.grsecurity.net/~paxguy1/pax-linux-4.9.24-test7.patch)
and the [Linux kernel
4.9.24](http://ftp.ntu.edu.tw/linux/kernel/v4.x/linux-4.9.24.tar.gz), unpack
both, unpack the [kernel config](./config) and copy it to the Linux directory.
Then run make to compile the RAP plugin and it'll be placed in
`linux-4.9.24-pax/scripts/gcc-plugins/rap_plugin/rap_plugin.so` for you to use.
The important kernel flags are:

```C
CONFIG_HAVE_GCC_PLUGINS=y
CONFIG_GCC_PLUGINS=y
CONFIG_PAX_RAP=y
```

The Linux kernel is under the GPL and so is the patch (confirmed by PaXTeam). We
therefore provide the precompiled binary, the sources, links to the sources, and
a quick note on how to compile your own. Happy open-source!
