#pragma once

// Infrastructure Layer - External systems, rendering, persistence
// This header provides access to all infrastructure layer components

// Rendering Infrastructure
#include "rendering/RenderPipeline.h"
#include "rendering/RenderMetricsExporter.h"

// External Systems (when they exist)
// #include "external/TmapClient.h"
// #include "external/IsimBridge.h"

// Persistence (when it exists)
// #include "persistence/CacheManager.h"

namespace infrastructure {
    namespace rendering {
        // Re-export commonly used rendering types
        using ::RenderPipeline;
    }
    
    // namespace external {
    //     // External system types would go here
    // }
    
    // namespace persistence {
    //     // Persistence types would go here  
    // }
}