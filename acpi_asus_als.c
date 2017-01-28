#include <sys/cdefs.h>

#include "opt_acpi.h"
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>
#include <sys/sbuf.h>

#include <contrib/dev/acpica/include/acpi.h>
#include <contrib/dev/acpica/include/accommon.h>

#include <dev/acpica/acpivar.h>

#define _COMPONENT	ACPI_OEM
ACPI_MODULE_NAME("ASUS")

static int	acpi_asus_als_probe(device_t dev);
static int	acpi_asus_als_attach(device_t dev);
static int	acpi_asus_als_detach(device_t dev);

static device_method_t acpi_asus_als_methods[] = {
	DEVMETHOD(device_probe,  acpi_asus_als_probe),
	DEVMETHOD(device_attach, acpi_asus_als_attach),
	DEVMETHOD(device_detach, acpi_asus_als_detach),

	{ 0, 0 }
};

struct acpi_asus_als_softc {
	device_t		dev;
	ACPI_HANDLE		handle;

	struct sysctl_ctx_list	sysctl_ctx;
	struct sysctl_oid	*sysctl_tree;
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

    if (acpi_disabled("asus"))
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

    ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);

    sc = device_get_softc(dev);

    return (0);
}

static int
acpi_asus_als_detach(device_t dev)
{
    struct acpi_asus_als_softc	*sc;

    ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);

    sc = device_get_softc(dev);

    return (0);
}
