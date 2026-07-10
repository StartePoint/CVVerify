#pragma once

#include "app/AppSettings.h"

namespace AppDefaultsLoader {

bool applyTo(AppSettings* settings, QString* errorMessage = nullptr);

}
