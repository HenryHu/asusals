/*-
 * Copyright (c) 2017 Henry Hu <henry.hu.sh@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * Driver for the ambient light sensor found on recent Asus laptops. Inspired
 * by the als project which implements the same functionality for Linux.
 *
 *   <https://github.com/victorenator/als>
 *
 * May also work on other laptops.
 */

#include "opt_acpi.h"
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>

#include <contrib/dev/acpica/include/acpi.h>
#include <contrib/dev/acpica/include/accommon.h>

#include <dev/acpica/acpivar.h>

#define _COMPONENT	ACPI_OEM
ACPI_MODULE_NAME("ASUS")

struct acpi_asus_als_softc {
	device_t		dev;
	ACPI_HANDLE		handle;

	struct sysctl_ctx_list	sysctl_ctx;
	struct sysctl_oid	*sysctl_tree;
};

ACPI_SERIAL_DECL(asus_als, "ACPI ASUS ambient light");

static int	acpi_asus_als_probe(device_t dev);
static int	acpi_asus_als_attach(device_t dev);
static int	acpi_asus_als_detach(device_t dev);

static int  acpi_asus_als_sysctl(SYSCTL_HANDLER_ARGS);
static int  acpi_asus_als_sysctl_get(struct acpi_asus_als_softc *sc);

static device_method_t acpi_asus_als_methods[] = {
	DEVMETHOD(device_probe,  acpi_asus_als_probe),
	DEVMETHOD(device_attach, acpi_asus_als_attach),
	DEVMETHOD(device_detach, acpi_asus_als_detach),

	{ 0, 0 }
};

static driver_t acpi_asus_als_driver = {
	"acpi_asus_als",
	acpi_asus_als_methods,
	sizeof(struct acpi_asus_als_softc)
};

static devclass_t acpi_asus_als_devclass;

DRIVER_MODULE(acpi_asus_als, acpi, acpi_asus_als_driver, acpi_asus_als_devclass, 0, 0);
MODULE_DEPEND(acpi_asus_als, acpi, 1, 1, 1);

static int
acpi_asus_als_probe(device_t dev)
{
	static char		*asus_ids[] = { "ACPI0008", NULL };

    struct acpi_asus_als_softc	*sc;
    char *rstr;

	ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);

    if (acpi_disabled("asus_als"))
        return (ENXIO);

    rstr = ACPI_ID_PROBE(device_get_parent(dev), dev, asus_ids);
    if (rstr == NULL) {
        return (ENXIO);
    }

    sc = device_get_softc(dev);
    sc->dev = dev;
    sc->handle = acpi_get_handle(dev);

    device_set_desc_copy(dev, "ASUS ambient light sensor");

    return 0;
}

static int
acpi_asus_als_attach(device_t dev)
{
    struct acpi_asus_als_softc	*sc;
    struct acpi_softc	*acpi_sc;

    ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);

    sc = device_get_softc(dev);
    acpi_sc = acpi_device_get_parent_softc(dev);

    sysctl_ctx_init(&sc->sysctl_ctx);
    sc->sysctl_tree = SYSCTL_ADD_NODE(&sc->sysctl_ctx,
            SYSCTL_CHILDREN(acpi_sc->acpi_sysctl_tree),
            OID_AUTO, "asus_als", CTLFLAG_RD, 0, "");

    SYSCTL_ADD_PROC(&sc->sysctl_ctx,
            SYSCTL_CHILDREN(sc->sysctl_tree), OID_AUTO,
            "light", CTLTYPE_INT | CTLFLAG_RD | CTLFLAG_ANYBODY,
            sc, 0, acpi_asus_als_sysctl, "I", "Ambient light value");

    return (0);
}

static int
acpi_asus_als_detach(device_t dev)
{
    struct acpi_asus_als_softc	*sc;

    ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);

    sc = device_get_softc(dev);

    sysctl_ctx_free(&sc->sysctl_ctx);

    return (0);
}

static int
acpi_asus_als_sysctl(SYSCTL_HANDLER_ARGS)
{
    struct acpi_asus_als_softc	*sc;
    int value;
    int error = 0;

	ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);

    sc = (struct acpi_asus_als_softc *)oidp->oid_arg1;

	ACPI_SERIAL_BEGIN(asus_als);

    value = acpi_asus_als_sysctl_get(sc);
    error = sysctl_handle_int(oidp, &value, 0, req);

	ACPI_SERIAL_END(asus_als);

    return (error);
}

static int
acpi_asus_als_sysctl_get(struct acpi_asus_als_softc *sc)
{
    ACPI_BUFFER Buf;
    ACPI_OBJECT *Obj;
    int value = 0;

    ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);
    ACPI_SERIAL_ASSERT(asus);

    Buf.Pointer = NULL;
    Buf.Length = ACPI_ALLOCATE_BUFFER;

    AcpiEvaluateObject(sc->handle, "_ALI", NULL, &Buf);
    Obj = Buf.Pointer;

    if (Obj->Type != ACPI_TYPE_INTEGER) {
        device_printf(sc->dev, "_ALI returned non-integer\n");
    } else {
        value = Obj->Integer.Value;
    }

    AcpiOsFree(Buf.Pointer);

    return (value);
}
