#pragma once

#include "platform.h"

#include <fried/core/build_target.h>

using fried::core::BuildTarget;
using fried::core::buildTargets;

// The page a target is built from. The web has no page of its own, so it is on the PC's, beside Windows.
inline Platform pageOf(BuildTarget target)
{
    return target == BuildTarget::Web ? Platform::Pc : platformOf(target);
}
