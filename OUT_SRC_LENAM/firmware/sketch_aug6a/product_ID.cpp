#include "product_ID.h"
#include <Preferences.h>
#include "config.h"

bool save_product_id(void)
{
    Preferences preferences;

    if (!preferences.begin("product", false))
        return false;

    const size_t written = preferences.putString("device_id", PRODUCT_ID);

    preferences.end();

    return written > 0;
}