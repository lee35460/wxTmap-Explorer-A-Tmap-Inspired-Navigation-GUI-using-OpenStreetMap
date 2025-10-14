#pragma once
#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/textctrl.h>
#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/choice.h>
#include <wx/slider.h>
#include <wx/gauge.h>
#include <wx/timer.h>
#include <wx/checkbox.h>
#include <wx/clrpicker.h>
#include <wx/spinctrl.h>
#include <memory>
#include <vector>
#include "Types.h"  // 🆕 LonLat 정의를 위해 추가

// 🆕 4단계: UI 컴포넌트 헤더 추가 (단계적 적용) - Forward declarations으로 변경
// #include "ui/MapOverlayHud.h"
// #include "ui/NavigationProgressBar.h" 
// #include "ui/WaypointListPanel.h"  // WaypointListPanel 활성화
// #include "ui/PolylineHighlight.h"  // 임시 주석 처리

// Forward declarations
class RenderPipeline;
class MapRenderPanel;  // 🆕 3단계용 forward declaration

// Forward declarations for UI and data structures
namespace ui {
    // wxWidgets 상속 UI 컴포넌트들
    class MapOverlayHud;
    class NavigationProgressBar;
    class WaypointListPanel;
    class TurnBanner;
    
    // 순수 비즈니스 로직 클래스들
    class CameraController;
    class LocationPuck;
    class PolylineHighlightRenderer;
    class UiDispatcher;
    
    // 데이터 구조체들
    struct NavigationProgress;
    struct HudState;
}
struct CoordinateStats;

// 디버그용 메인 프레임 - Modern C++ unique_ptr 적용
class DebugFrame : public wxFrame {
public:
    DebugFrame();
    virtual ~DebugFrame() = default; // unique_ptr이 자동 정리
    
    // 로그 함수 (다른 패널에서 접근 가능하도록 public)
    void AppendLog(const wxString& message);
    
private:
    // === wxWidgets 상속 UI 컴포넌트들 (화면 렌더링/사용자 입력 처리) ===
    wxNotebook* notebook_;
    wxTextCtrl* logOutput_;
    wxSplitterWindow* splitter_;
    
    // 각 탭 패널들 (wxWidgets가 자동 관리)
    wxPanel* dataTestPanel_;
    wxPanel* apiTestPanel_;
    wxPanel* renderTestPanel_;  // 3단계: MapRenderPanel을 wxPanel*로 관리
    
    // === 순수 비즈니스 로직 클래스들 (wxWidgets 비의존적) ===
    std::unique_ptr<std::vector<LonLat>> testCoordinates_;
    
    void CreateUI();
    void CreateDataTestPanel();
    void CreateApiTestPanel();
    void CreateRenderTestPanel();
    
    wxDECLARE_EVENT_TABLE();
};

// 데이터 테스트 패널 - unique_ptr로 데이터 안전성 확보
class DataTestPanel : public wxPanel {
public:
    DataTestPanel(wxWindow* parent, DebugFrame* debugFrame);
    virtual ~DataTestPanel() = default;
    
private:
    DebugFrame* debugFrame_;
    
    // === wxWidgets 상속 UI 컴포넌트들 (화면 렌더링/사용자 입력 처리) ===
    wxTextCtrl* lonInput_;
    wxTextCtrl* latInput_;
    wxButton* addCoordBtn_;
    wxListCtrl* coordList_;
    wxTextCtrl* jsonOutput_;
    
    // 추가 UI 컴포넌트들 (Step 1-2)
    wxStaticText* memoryStatus_;
    wxTextCtrl* statsOutput_;
    
    // === 순수 비즈니스 로직 클래스들 (wxWidgets 비의존적) ===
    std::unique_ptr<std::vector<LonLat>> coordinates_;
    std::unique_ptr<CoordinateStats> stats_;
    
    // 이벤트 핸들러들
    void OnAddCoordinate(wxCommandEvent& event);
    void OnClearList(wxCommandEvent& event);
    void OnCalculateDistance(wxCommandEvent& event);
    void OnAddSampleData(wxCommandEvent& event);
    
    // UI 업데이트 메서드들
    void CreateUI();
    void UpdateDisplay();
    void UpdateStatsOutput();
    void UpdateJsonOutput();
    
    wxDECLARE_EVENT_TABLE();
};

// ========================================
// 2단계: API Test 패널들 (학습용 Mock 구현)
// ========================================

// TmapClient API 테스트 패널 - POI 검색 Mock 구현
class TmapApiTestPanel : public wxPanel {
public:
    TmapApiTestPanel(wxWindow* parent, DebugFrame* debugFrame);
    virtual ~TmapApiTestPanel() = default;
    
private:
    DebugFrame* debugFrame_;
    
    // === wxWidgets 상속 UI 컴포넌트들 (화면 렌더링/사용자 입력 처리) ===
    wxTextCtrl* searchInput_;
    wxButton* searchBtn_;
    wxListCtrl* resultList_;
    wxTextCtrl* rawResponseOutput_;
    
    void OnSearchPoi(wxCommandEvent& event);
    void OnClearResults(wxCommandEvent& event);
    
    wxDECLARE_EVENT_TABLE();
};

// 경로 계획 알고리즘 테스트 패널 - 간단한 Mock 경로 생성
class RoutePlannerTestPanel : public wxPanel {
public:
    RoutePlannerTestPanel(wxWindow* parent, DebugFrame* debugFrame);
    virtual ~RoutePlannerTestPanel() = default;
    
private:
    DebugFrame* debugFrame_;
    
    // === wxWidgets 상속 UI 컴포넌트들 (화면 렌더링/사용자 입력 처리) ===
    wxChoice* plannerChoice_;        // "직선", "지그재그", "곡선" 선택
    wxChoice* algorithmChoice_;      // 알고리즘 선택 (누락된 멤버 추가)
    wxTextCtrl* startLonInput_;
    wxTextCtrl* startLatInput_;
    wxTextCtrl* endLonInput_;
    wxTextCtrl* endLatInput_;
    wxSpinCtrl* pointCountSpin_;     // 생성할 중간 점 개수
    wxButton* calculateBtn_;
    wxListCtrl* routePointsList_;
    wxTextCtrl* algorithmOutput_;    // 알고리즘 설명 및 결과
    wxTextCtrl* routeOutput_;        // 경로 결과 출력 (누락된 멤버 추가)
    
    // === 순수 비즈니스 로직 클래스들 (wxWidgets 비의존적) ===
    std::unique_ptr<std::vector<LonLat>> calculatedRoute_;
    
    // 이벤트 핸들러들
    void OnCalculateRoute(wxCommandEvent& event);
    void OnPlannerChanged(wxCommandEvent& event);
    void OnPlanRoute(wxCommandEvent& event);      // 누락된 메서드 추가
    void OnClearRoute(wxCommandEvent& event);
    
    // 헬퍼 메서드들 (학습용 Mock 알고리즘)
    void CreateUI();
    void UpdateRouteDisplay();
    std::vector<LonLat> GenerateDirectRoute(const LonLat& start, const LonLat& end, int points);
    std::vector<LonLat> GenerateZigzagRoute(const LonLat& start, const LonLat& end, int points);
    std::vector<LonLat> GenerateCurvedRoute(const LonLat& start, const LonLat& end, int points);
    
    wxDECLARE_EVENT_TABLE();
};

// ========================================
// 🆕 3단계: 지도 렌더링 패널 (MapRenderPanel)
// OpenStreetMap 타일을 사용한 실제 지도 렌더링
// ========================================
class MapRenderPanel : public wxPanel {
public:
    MapRenderPanel(wxWindow* parent, DebugFrame* debugFrame);
    
private:
    // 이벤트 핸들러
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnZoomIn(wxCommandEvent& event);
    void OnZoomOut(wxCommandEvent& event);
    void OnResetView(wxCommandEvent& event);
    void OnLoadTiles(wxCommandEvent& event);
    void OnShowRoute(wxCommandEvent& event);
    
    // 🆕 4단계: 실시간 UI 업데이트 이벤트 핸들러
    void OnStartNavigation(wxCommandEvent& event);
    void OnStopNavigation(wxCommandEvent& event);
    void OnUpdateTimer(wxTimerEvent& event);
    void OnAnimationTimer(wxTimerEvent& event); // 애니메이션 전용 타이머
    void UpdateHudDisplay();
    void UpdateProgressBar();
    
    // 좌표계 변환 함수들 (머케이터 투영법)
    wxPoint LatLonToScreen(const LonLat& coord) const;
    LonLat ScreenToLatLon(const wxPoint& point) const;
    void UpdateTileList();
    
    // 🎯 MapPanel.h 구현 가이드: 지리 계산 함수들
    double CalculateBearing(const LonLat& from, const LonLat& to) const; // Types.h로 이동 권장
    LonLat CalculateRoutePosition(double progress) const; // LocationPuck::InterpolateLocation() 활용 권장
    
    // 렌더링 관련
    void RenderMap(wxDC& dc);
    void RenderRoute(wxDC& dc);
    void RenderRouteAdvanced(wxDC& dc);  // 🆕 4단계: 향상된 경로 렌더링
    void RenderUI(wxDC& dc);
    
    // === wxWidgets 상속 UI 컴포넌트들 (화면 렌더링/사용자 입력 처리) ===
    // 기본 UI 컨트롤들
    wxButton* zoomInBtn_;
    wxButton* zoomOutBtn_;
    wxButton* resetBtn_;
    wxButton* loadTilesBtn_;
    wxButton* showRouteBtn_;
    
    // 🆕 4단계: 실시간 UI 업데이트 컨트롤들
    wxButton* startNavBtn_;
    wxButton* stopNavBtn_;
    wxTimer* updateTimer_;
    wxTimer* animationTimer_;  // LocationPuck 애니메이션 전용 타이머
    wxStaticText* hudStatusLabel_;
    wxGauge* progressGauge_;
    wxStaticText* coordLabel_;
    wxStaticText* zoomLabel_;
    
    // === wxWidgets 상속 UI 컴포넌트들 (화면 렌더링/사용자 입력 처리) ===
    // 🆕 4단계: wxPanel 상속 UI 컴포넌트들 (단계적 적용)
    ui::MapOverlayHud* hudOverlay_{nullptr};        // wxPanel 상속 - HUD 오버레이
    ui::NavigationProgressBar* progressBar_{nullptr}; // wxPanel 상속 - 진행률 표시
    ui::WaypointListPanel* waypointPanel_{nullptr};   // wxPanel 상속 - 경유지 리스트
    ui::TurnBanner* turnBanner_{nullptr};            // wxPanel 상속 - 방향 안내 배너

    // === 순수 비즈니스 로직 클래스들 (wxWidgets 비의존적) ===
    std::unique_ptr<ui::CameraController> cameraController_;      // 카메라 follow 로직
    std::unique_ptr<ui::LocationPuck> locationPuck_;             // 위치 마커 렌더링
    std::unique_ptr<ui::PolylineHighlightRenderer> polylineHighlight_; // 경로 하이라이트
    std::unique_ptr<RenderPipeline> renderPipeline_;             // 지도 타일 렌더링 파이프라인    
    // === 데이터 멤버들 ===
    // 지도 상태
    LonLat centerCoord_;      // 지도 중심 좌표 (서울 기본값)
    int zoomLevel_;           // 줌 레벨 (1-18)
    wxSize panelSize_;        // 패널 크기
    
    // 인터렉션 상태
    bool isDragging_;
    wxPoint lastMousePos_;
    
    // 경로 데이터 (2단계에서 가져올 예정)
    std::vector<LonLat> currentRoute_;
    
    // 시뮬레이션 상태
    bool isNavigating_;
    double currentProgress_;
    double currentSpeed_;
    double remainingDistance_;
    
    DebugFrame* debugFrame_;
    
    wxDECLARE_EVENT_TABLE();
};

// 이벤트 ID들
enum {
    // Data Test IDs (1000번대)
    ID_ADD_COORD = 1001,
    ID_CLEAR_LIST = 1002,
    ID_CALC_DISTANCE = 1003,
    ID_ADD_SAMPLE = 1004,
    
    // API Test IDs (2000번대) - 2단계 학습용
    ID_SEARCH_POI = 2001,
    ID_CLEAR_RESULTS = 2002,
    ID_CALCULATE_ROUTE = 2003,
    ID_PLANNER_CHOICE = 2004,
    ID_PLAN_ROUTE = 2005,      // 누락된 ID 추가
    ID_CLEAR_ROUTE = 2006,
    ID_POINT_COUNT_SPIN = 2007,
    
    // Render Test IDs (3000번대) - 🆕 3단계: 지도 렌더링
    ID_MAP_RENDER = 3001,
    ID_ZOOM_IN = 3002,
    ID_ZOOM_OUT = 3003,
    ID_RESET_VIEW = 3004,
    ID_LOAD_TILES = 3005,
    ID_SHOW_ROUTE = 3006,
    
    // 🆕 4단계: 실시간 UI 업데이트
    ID_START_NAV_SIMULATION = 3007,
    ID_STOP_NAV_SIMULATION = 3008,
    ID_UPDATE_TIMER = 3009,
    ID_ANIMATION_TIMER = 3010
};