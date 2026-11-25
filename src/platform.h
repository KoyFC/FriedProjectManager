#pragma once

#include "core_bridge.h"

#include <fried/core/platform.h>

using fried::core::Platform;
using fried::core::Precision;

// The platforms with a page of their own, which are the first of FriedCore's, up to Android. The web comes after them and is built from the PC's page.
constexpr int s_platformCount = 7;
