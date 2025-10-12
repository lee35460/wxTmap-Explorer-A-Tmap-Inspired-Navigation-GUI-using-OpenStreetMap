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

// Forward declarations
struct LonLat;
class RenderPipeline;

// 디버그용 메인 프레임 - Modern C++ unique_ptr 적용
class DebugFrame : public wxFrame {
public:
    DebugFrame();
    virtual ~DebugFrame() = default; // unique_ptr이 자동 정리
    
    // 로그 함수 (다른 패널에서 접근 가능하도록 public)
    void AppendLog(const wxString& message);
    
private:
    // UI 컴포넌트들 (wxWidgets가 자동 관리)
    wxNotebook* notebook_;
    wxTextCtrl* logOutput_;
    wxSplitterWindow* splitter_;
    
    // 각 탭 패널들 (wxWidgets가 자동 관리)
    wxPanel* dataTestPanel_;
    wxPanel* apiTestPanel_;
    wxPanel* renderTestPanel_;
    
    // 비즈니스 로직 객체들 (unique_ptr로 안전한 메모리 관리) - 1단계용
    std::unique_ptr<std::vector<LonLat>> testCoordinates_;
    
    void CreateUI();
    void CreateDataTestPanel();
    void CreateApiTestPanel();
    void CreateRenderTestPanel();
    
    wxDECLARE_EVENT_TABLE();
};

// Forward declaration for CoordinateStats
struct CoordinateStats;

// 데이터 테스트 패널 - unique_ptr로 데이터 안전성 확보
class DataTestPanel : public wxPanel {
public:
    DataTestPanel(wxWindow* parent, DebugFrame* debugFrame);
    virtual ~DataTestPanel() = default;
    
private:
    DebugFrame* debugFrame_;
    
    // UI 컴포넌트들 (wxWidgets가 자동 관리)
    wxTextCtrl* lonInput_;
    wxTextCtrl* latInput_;
    wxButton* addCoordBtn_;
    wxListCtrl* coordList_;
    wxTextCtrl* jsonOutput_;
    
    // 추가 UI 컴포넌트들 (Step 1-2)
    wxStaticText* memoryStatus_;
    wxTextCtrl* statsOutput_;
    
    // 데이터 저장소 (unique_ptr로 안전한 메모리 관리)
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
    
    // UI 컴포넌트들 (2단계 학습용으로 간소화)
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
    
    // 경로 데이터 (unique_ptr로 안전한 관리)
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
// 3단계: Render Test 관련 (나중에 구현)
// ========================================
// (3단계에서 구현 예정)

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
    
    // Render Test IDs (3000번대) - 3단계에서 구현 예정
    // (나중에 추가)
};