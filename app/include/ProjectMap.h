#pragma once

// ============================================================================
// 🗺️ wxTmap-Explorer Project File Map (간단 버전)
// ============================================================================
// 복잡한 ../../ 경로 대신 실제 파일 위치를 바로 include하는 간단한 방식
// ============================================================================

// === 🏢 DOMAIN LAYER (비즈니스 로직, 엔티티, 값 객체) ===
#define DOMAIN_TYPES                    "domain/Types.h"
#define DOMAIN_ENTITIES_LOCATION        "domain/entities/Location.h"
#define DOMAIN_ENTITIES_ROUTE           "domain/entities/Route.h"
#define DOMAIN_ENTITIES_MAPVIEW         "domain/entities/MapView.h"
#define DOMAIN_SERVICES_ROUTE_PLANNER   "domain/services/IRoutePlanner.h"

// === 🏗️ INFRASTRUCTURE LAYER (외부 시스템, 렌더링, 영속성) ===
#define INFRASTRUCTURE_RENDER_PIPELINE      "infrastructure/rendering/RenderPipeline.h"
#define INFRASTRUCTURE_RENDER_METRICS       "infrastructure/rendering/RenderMetricsExporter.h"
#define INFRASTRUCTURE_EXTERNAL_TMAP        "infrastructure/external/TmapClient.h"
#define INFRASTRUCTURE_EXTERNAL_BRIDGE      "infrastructure/external/IsimBridge.h"
#define INFRASTRUCTURE_PERSISTENCE_CACHE    "infrastructure/persistence/CacheManager.h"

// === 🎨 PRESENTATION LAYER (UI 컴포넌트, 뷰, 컨트롤러) ===
// UI Components
#define PRESENTATION_BASE_COMPONENT         "presentation/components/BaseComponent.h"
#define PRESENTATION_CAMERA_CONTROLLER      "presentation/components/CameraController.h"
#define PRESENTATION_CAMERA_PRESET          "presentation/components/CameraPreset.h"
#define PRESENTATION_TRANSITION_ANIMATOR    "presentation/components/TransitionAnimator.h"
#define PRESENTATION_LOCATION_PUCK          "presentation/components/LocationPuck.h"
#define PRESENTATION_MAP_OVERLAY_HUD        "presentation/components/MapOverlayHud.h"
#define PRESENTATION_NAVIGATION_PROGRESS    "presentation/components/NavigationProgressBar.h"
#define PRESENTATION_TURN_BANNER            "presentation/components/TurnBanner.h"
#define PRESENTATION_WAYPOINT_LIST          "presentation/components/WaypointListPanel.h"
#define PRESENTATION_POLYLINE_HIGHLIGHT     "presentation/components/PolylineHighlight.h"
#define PRESENTATION_POLYLINE_STYLER        "presentation/components/PolylineStyler.h"
#define PRESENTATION_MAP_OVERLAY_THEME      "presentation/components/MapOverlayTheme.h"
#define PRESENTATION_UI_DISPATCHER          "presentation/components/UiDispatcher.h"

// Views
#define PRESENTATION_APP_FRAME              "presentation/views/frames/AppFrame.h"
#define PRESENTATION_DEBUG_FRAME            "presentation/views/frames/DebugFrame.h"
#define PRESENTATION_MAP_PANEL              "presentation/views/panels/MapPanel.h"
#define PRESENTATION_CONTROL_PANEL          "presentation/views/panels/ControlPanel.h"

// Controllers
#define PRESENTATION_MAIN_CONTROLLER        "presentation/controllers/MainController.h"
#define PRESENTATION_MAP_CONTROLLER         "presentation/controllers/MapController.h"
#define PRESENTATION_NAVIGATION_CONTROLLER  "presentation/controllers/NavigationController.h"

// === 🔄 APPLICATION LAYER (서비스, 명령, 이벤트, 유스케이스) ===
// 향후 추가될 때 사용
// #define APPLICATION_ROUTE_SERVICE         "application/services/RouteService.h"
// #define APPLICATION_NAVIGATION_COMMAND    "application/commands/NavigationCommand.h"

// ============================================================================
// 🚀 편의 매크로들 (자주 사용되는 조합)
// ============================================================================

// 모든 Domain 타입
#define INCLUDE_ALL_DOMAIN_TYPES \
    #include DOMAIN_TYPES

// 모든 UI 컴포넌트
#define INCLUDE_ALL_UI_COMPONENTS \
    #include PRESENTATION_CAMERA_CONTROLLER \
    #include PRESENTATION_LOCATION_PUCK \
    #include PRESENTATION_MAP_OVERLAY_HUD \
    #include PRESENTATION_NAVIGATION_PROGRESS \
    #include PRESENTATION_TURN_BANNER \
    #include PRESENTATION_WAYPOINT_LIST \
    #include PRESENTATION_POLYLINE_HIGHLIGHT \
    #include PRESENTATION_POLYLINE_STYLER \
    #include PRESENTATION_MAP_OVERLAY_THEME

// 모든 렌더링 관련
#define INCLUDE_ALL_RENDERING \
    #include INFRASTRUCTURE_RENDER_PIPELINE \
    #include INFRASTRUCTURE_RENDER_METRICS

// 모든 Views
#define INCLUDE_ALL_VIEWS \
    #include PRESENTATION_APP_FRAME \
    #include PRESENTATION_DEBUG_FRAME \
    #include PRESENTATION_MAP_PANEL \
    #include PRESENTATION_CONTROL_PANEL

// ============================================================================
// 📝 참고: namespace 별칭은 각 파일에서 필요시 직접 정의
// ============================================================================
// 예시: 
// namespace wx_map = presentation::components;
// namespace core = domain;
// namespace infra = infrastructure;