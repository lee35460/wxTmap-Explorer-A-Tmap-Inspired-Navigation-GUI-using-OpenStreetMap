#pragma once

// Domain Layer - Core business entities and value objects
// This header provides access to all domain layer components

// Core Types and Value Objects
#include "Types.h"

// Entities (when they exist)
// #include "entities/Location.h"
// #include "entities/Route.h"
// #include "entities/MapView.h"

// Domain Services and Interfaces
// #include "services/IRoutePlanner.h"

namespace domain {
    // Re-export commonly used types for convenience
    using ::LonLat;
    using ::LocationState;
    using ::CoordinateStats;
    using ::CameraFollowMode;
    using ::LocationPuckTheme;
}