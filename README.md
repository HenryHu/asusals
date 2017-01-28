# asusals
ASUS ambient light sensor driver for FreeBSD

# Install
    # make obj
    # make depend
    # make
    # make install
    # kldload acpi_asus_als
    # sysctl hw.acpi.asus_als.light
    
# Note

Submitted as PR 216552 (https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=216552).

# Reference

Inspired by the Linux ALS driver
https://github.com/victorenator/als
