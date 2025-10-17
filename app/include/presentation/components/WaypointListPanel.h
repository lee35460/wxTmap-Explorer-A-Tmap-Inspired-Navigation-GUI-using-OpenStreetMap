#pragma once
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/dnd.h>
#include <vector>
#include <memory>
#include <functional>
#include <map>  // 🔧 std::map 추가
#include "Types.h"

// 🎯 설계 원칙: Modern C++의 unique_ptr를 활용한 안전한 메모리 관리
// wxWidgets 컴포넌트들은 부모에 의해 자동 관리되므로, 
// 비즈니스 로직 객체들만 unique_ptr로 관리

namespace presentation::components {

// 🔧 누락된 클래스들 Forward Declaration 또는 간단 정의
class WaypointValidator {
public:
    bool IsValid(const Waypoint& wp) const { return wp.coordinates.IsValid(); }
};

class DistanceCalculator {
public:
    double Calculate(const Waypoint& a, const Waypoint& b) const {
        return a.coordinates.DistanceTo(b.coordinates);
    }
};

/**
 * @brief 웨이포인트 목록을 관리하고 표시하는 패널
 * 
 * 주요 기능:
 * - 웨이포인트 추가/삭제/편집
 * - 드래그 앤 드롭을 통한 순서 변경
 * - 실시간 거리/시간 계산
 * - 접근성 지원 (키보드 네비게이션)
 */
class WaypointListPanel : public wxPanel {
public:
    // 🔧 콜백 함수 타입 정의 - 외부 시스템과의 결합도 최소화
    using OnWaypointChangeCallback = std::function<void(const std::vector<Waypoint>&)>;
    using OnWaypointSelectCallback = std::function<void(const Waypoint&)>;
    
    explicit WaypointListPanel(wxWindow* parent, wxWindowID id = wxID_ANY);
    virtual ~WaypointListPanel() = default; // unique_ptr이 자동으로 정리

    // 📍 핵심 기능 메서드
    void AddWaypoint(const Waypoint& waypoint);
    void RemoveWaypoint(int index);
    void UpdateWaypoint(int index, const Waypoint& waypoint);
    void ClearWaypoints();
    
    // 🎯 사용자 상호작용
    void MoveWaypoint(int fromIndex, int toIndex);
    void SelectWaypoint(int index);
    
    // 📊 데이터 접근
    const std::vector<Waypoint>& GetWaypoints() const;
    int GetSelectedIndex() const;
    
    // 🔗 콜백 설정
    void SetOnWaypointChangeCallback(OnWaypointChangeCallback callback);
    void SetOnWaypointSelectCallback(OnWaypointSelectCallback callback);

private:
    // 🎨 UI 컴포넌트들 (wxWidgets가 자동 관리)
    wxListCtrl* listCtrl_;
    wxButton* addButton_;
    wxButton* removeButton_;
    wxButton* editButton_;
    wxButton* clearButton_;
    
    // 💾 데이터 및 비즈니스 로직 (unique_ptr로 관리)
    std::unique_ptr<std::vector<Waypoint>> waypoints_;
    std::unique_ptr<WaypointValidator> validator_;
    std::unique_ptr<DistanceCalculator> distanceCalculator_;
    
    // 📞 콜백 함수들
    OnWaypointChangeCallback onWaypointChange_;
    OnWaypointSelectCallback onWaypointSelect_;
    
    // 🎯 상태 관리
    int selectedIndex_;
    bool isDragging_;
    
    // 🏗️ 초기화 메서드들
    void CreateControls();
    void LayoutControls();
    void BindEvents();
    
    // 🎮 이벤트 핸들러들
    void OnAddWaypoint(wxCommandEvent& event);
    void OnRemoveWaypoint(wxCommandEvent& event);
    void OnEditWaypoint(wxCommandEvent& event);
    void OnClearWaypoints(wxCommandEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnListItemRightClick(wxListEvent& event);
    
    // 🖱️ 드래그 앤 드롭 지원
    void OnListBeginDrag(wxListEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    
    // ⌨️ 키보드 네비게이션
    void OnKeyDown(wxKeyEvent& event);
    
    // 🔧 헬퍼 메서드들
    void UpdateListDisplay();
    void UpdateButtonStates();
    void CalculateDistances();
    void NotifyWaypointChange();
    
    wxDECLARE_EVENT_TABLE();
};

} // namespace presentation::components

// 중복 정의 제거 - 위에서 이미 간단한 버전으로 정의됨