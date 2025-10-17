#pragma once

// Presentation Layer - UI components, views, controllers
// This header provides access to all presentation layer components

// UI Components
#include "components/CameraController.h"
#include "components/LocationPuck.h"
#include "components/MapOverlayHud.h"
#include "components/NavigationProgressBar.h"
#include "components/TurnBanner.h"
#include "components/WaypointListPanel.h"
#include "components/PolylineHighlight.h"
#include "components/PolylineStyler.h"
#include "components/MapOverlayTheme.h"

// Views (when they exist)
// #include "views/frames/AppFrame.h"
// #include "views/frames/DebugFrame.h"
// #include "views/panels/MapPanel.h"

// Controllers (when they exist)
// #include "controllers/MainController.h"
// #include "controllers/MapController.h"
// #include "controllers/NavigationController.h"

namespace presentation {
    namespace components {
        // Re-export commonly used UI component types
        using ::presentation::components::CameraController;
        using ::presentation::components::LocationPuck;
        using ::presentation::components::MapOverlayHud;
        using ::presentation::components::NavigationProgressBar;
        using ::presentation::components::TurnBanner;
        using ::presentation::components::WaypointListPanel;
        using ::presentation::components::PolylineHighlightRenderer;
        using ::presentation::components::PolylineTheme;
    }
    
    // namespace views {
    //     // View types would go here
    // }
    
    // namespace controllers {
    //     // Controller types would go here
    // }
}