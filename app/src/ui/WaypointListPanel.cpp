#include "ui/WaypointListPanel.h"
#include <wx/msgdlg.h>
#include <wx/textdlg.h>

enum {
    ID_ADD_WAYPOINT = 1001,
    ID_REMOVE_WAYPOINT,
    ID_MOVE_UP,
    ID_MOVE_DOWN,
    ID_LIST_CTRL
};

// 이벤트 테이블 매핑
wxBEGIN_EVENT_TABLE(WaypointListPanel, wxPanel)
    EVT_BUTTON(ID_ADD_WAYPOINT, WaypointListPanel::OnAddWaypoint)
    EVT_BUTTON(ID_REMOVE_WAYPOINT, WaypointListPanel::OnRemoveWaypoint)
    EVT_BUTTON(ID_MOVE_UP, WaypointListPanel::OnMoveUp)
    EVT_BUTTON(ID_MOVE_DOWN, WaypointListPanel::OnMoveDown)
    EVT_LIST_ITEM_SELECTED(ID_LIST_CTRL, WaypointListPanel::OnItemSelect)
    EVT_LIST_ITEM_RIGHT_CLICK(ID_LIST_CTRL, WaypointListPanel::OnItemRightClick)
    EVT_CHAR_HOOK(WaypointListPanel::OnKeyDown)
    EVT_DPI_CHANGED(WaypointListPanel::OnDPIChanged)
wxEND_EVENT_TABLE()

/**
 * @brief 생성자 - Modern C++ 스타일의 초기화
 * 
 * 🎯 설계 포인트:
 * - 초기화 리스트를 통한 효율적인 객체 생성
 * - unique_ptr로 비즈니스 로직 객체들 안전하게 관리
 * - 단계적 초기화로 예외 안전성 보장
 */
WaypointListPanel::WaypointListPanel(wxWindow* parent)
    : wxPanel(parent, id)
    , listCtrl_(nullptr)
    , addButton_(nullptr)
    , removeButton_(nullptr)
    , editButton_(nullptr)
    , clearButton_(nullptr)
    , waypoints_(std::make_unique<std::vector<Waypoint>>()) // unique_ptr로 메모리 안전성 보장
    , validator_(std::make_unique<WaypointValidator>())       // unique_ptr로 메모리 안전성 보장
    , selectedIndex_(-1)
    , isDragging_(false)    
{
    // 단계적 초기화 - 각 단계가 독립적으로 유지보수 용이
    CreateControls();
    LayoutControls();
    BindEvents();
    UpdateButtonStates();
}

/**
 * @brief UI 컨트롤 생성
 * 
 * 🎯 설계 원칙: 관심사 분리 (Separation of Concerns)
 * - UI 생성만 담당하는 전용 메서드
 * - 각 컨트롤의 목적과 속성 명확화
 */
void WaypointListPanel::CreateControls() {
    // 리스트 컨트롤 - 웨이포인 목록 표시
    listCtrl_ = new wxListCtrl(this, ID_LIST_CTRL, wxDefaultPosition,
                                wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES); // wxWidgets가 자동 관리
    
    // 컬럼 설정 - 사용자에게 유의미한 정보 제공
    listCtrl_->
}