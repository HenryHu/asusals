# $FreeBSD: head/sys/modules/acpi/acpi_asus/Makefile 193530 2009-06-05 18:44:36Z jkim $

.PATH:	.

KMOD=	acpi_asus_als
SRCS=	acpi_asus_als.c opt_acpi.h acpi_if.h bus_if.h device_if.h

.include <bsd.kmod.mk>

MAN=	acpi_asus_als.4
.include <bsd.man.mk>

all: all-man

install: maninstall
