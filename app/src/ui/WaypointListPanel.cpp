#include "ui/WaypointListPanel.h"
#include <wx/msgdlg.h>
#include <wx/textdlg.h>

namespace ui {

enum {
    ID_ADD_WAYPOINT = 1001,
    ID_REMOVE_WAYPOINT,
    ID_MOVE_UP,
    ID_MOVE_DOWN,
    ID_LIST_CTRL
};

// 이벤트 테이블 매핑
wxBEGIN_EVENT_TABLE(ui::WaypointListPanel, wxPanel)
    EVT_BUTTON(ID_ADD_WAYPOINT, ui::WaypointListPanel::OnAddWaypoint)
    EVT_BUTTON(ID_REMOVE_WAYPOINT, ui::WaypointListPanel::OnRemoveWaypoint)
    EVT_LIST_ITEM_SELECTED(ID_LIST_CTRL, ui::WaypointListPanel::OnListItemSelected)
    EVT_LIST_ITEM_RIGHT_CLICK(ID_LIST_CTRL, ui::WaypointListPanel::OnListItemRightClick)
    EVT_CHAR_HOOK(ui::WaypointListPanel::OnKeyDown)
wxEND_EVENT_TABLE()

/**
 * @brief 생성자 - Modern C++ 스타일의 초기화
 * 
 * 🎯 설계 포인트:
 * - 초기화 리스트를 통한 효율적인 객체 생성
 * - unique_ptr로 비즈니스 로직 객체들 안전하게 관리
 * - 단계적 초기화로 예외 안전성 보장
 */
WaypointListPanel::WaypointListPanel(wxWindow* parent, wxWindowID id)
    : wxPanel(parent, id)
    , listCtrl_(nullptr)
    , addButton_(nullptr)
    , removeButton_(nullptr)
    , editButton_(nullptr)
    , clearButton_(nullptr)
    , waypoints_(std::make_unique<std::vector<Waypoint>>()) // unique_ptr로 메모리 안전성 보장
    , validator_(std::make_unique<WaypointValidator>())       // unique_ptr로 메모리 안전성 보장
    , distanceCalculator_(std::make_unique<DistanceCalculator>())
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
    // 리스트 컨트롤 스타일 상수 정의 (가독성 및 유지보수성 향상)
    constexpr long LIST_STYLE = wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES;
    
    // 리스트 컨트롤 - 웨이포인트 목록 표시
    listCtrl_ = new wxListCtrl(this, ID_LIST_CTRL, wxDefaultPosition, wxDefaultSize, LIST_STYLE);
    
    // 컬럼 설정 - 사용자에게 유의미한 정보 제공
    listCtrl_->AppendColumn("순서", wxLIST_FORMAT_LEFT, 50);
    listCtrl_->AppendColumn("이름", wxLIST_FORMAT_LEFT, 120);
    listCtrl_->AppendColumn("경도", wxLIST_FORMAT_LEFT, 100);
    listCtrl_->AppendColumn("위도", wxLIST_FORMAT_LEFT, 100);
    listCtrl_->AppendColumn("거리(m)", wxLIST_FORMAT_LEFT, 80);
    
    // 버튼 생성 - 웨이포인트 관리 기능
    addButton_ = new wxButton(this, ID_ADD_WAYPOINT, "추가");
    removeButton_ = new wxButton(this, ID_REMOVE_WAYPOINT, "삭제");
    editButton_ = new wxButton(this, wxID_ANY, "편집");
    clearButton_ = new wxButton(this, wxID_ANY, "전체삭제");
}

void WaypointListPanel::LayoutControls() {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // 버튼 패널
    auto* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->Add(addButton_, 0, wxALL, 5);
    buttonSizer->Add(removeButton_, 0, wxALL, 5);
    buttonSizer->Add(editButton_, 0, wxALL, 5);
    buttonSizer->Add(clearButton_, 0, wxALL, 5);
    
    mainSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(listCtrl_, 1, wxEXPAND | wxALL, 5);
    
    SetSizer(mainSizer);
}

void WaypointListPanel::BindEvents() {
    // 이벤트는 이미 wxBEGIN_EVENT_TABLE에서 처리됨
}

void WaypointListPanel::UpdateButtonStates() {
    bool hasSelection = (selectedIndex_ >= 0);
    removeButton_->Enable(hasSelection);
    editButton_->Enable(hasSelection);
}

// 이벤트 핸들러 구현 (기본 Mock)
void WaypointListPanel::OnAddWaypoint(wxCommandEvent& event) {
    // Mock 구현
}

void WaypointListPanel::OnRemoveWaypoint(wxCommandEvent& event) {
    // Mock 구현
}

void WaypointListPanel::OnListItemSelected(wxListEvent& event) {
    selectedIndex_ = event.GetIndex();
    UpdateButtonStates();
}

void WaypointListPanel::OnListItemRightClick(wxListEvent& event) {
    // Mock 구현
}

void WaypointListPanel::OnKeyDown(wxKeyEvent& event) {
    // Mock 구현
    event.Skip();
}

// 헤더에서 요구하는 추가 메서드들 (Mock 구현)
void WaypointListPanel::AddWaypoint(const Waypoint& waypoint) {
    waypoints_->push_back(waypoint);
    UpdateListDisplay();
}

void WaypointListPanel::RemoveWaypoint(int index) {
    if (index >= 0 && index < static_cast<int>(waypoints_->size())) {
        waypoints_->erase(waypoints_->begin() + index);
        UpdateListDisplay();
    }
}

void WaypointListPanel::UpdateWaypoint(int index, const Waypoint& waypoint) {
    if (index >= 0 && index < static_cast<int>(waypoints_->size())) {
        (*waypoints_)[index] = waypoint;
        UpdateListDisplay();
    }
}

void WaypointListPanel::ClearWaypoints() {
    waypoints_->clear();
    UpdateListDisplay();
}

void WaypointListPanel::MoveWaypoint(int fromIndex, int toIndex) {
    // Mock 구현
}

void WaypointListPanel::SelectWaypoint(int index) {
    selectedIndex_ = index;
    UpdateButtonStates();
}

const std::vector<Waypoint>& WaypointListPanel::GetWaypoints() const {
    return *waypoints_;
}

int WaypointListPanel::GetSelectedIndex() const {
    return selectedIndex_;
}

void WaypointListPanel::SetOnWaypointChangeCallback(OnWaypointChangeCallback callback) {
    onWaypointChange_ = callback;
}

void WaypointListPanel::SetOnWaypointSelectCallback(OnWaypointSelectCallback callback) {
    onWaypointSelect_ = callback;
}

void WaypointListPanel::UpdateListDisplay() {
    listCtrl_->DeleteAllItems();
    
    for (size_t i = 0; i < waypoints_->size(); ++i) {
        const auto& wp = (*waypoints_)[i];
        
        long index = listCtrl_->InsertItem(i, wxString::Format("%zu", i + 1));
        listCtrl_->SetItem(index, 1, wp.name);
        listCtrl_->SetItem(index, 2, wxString::Format("%.6f", wp.coordinates.lon));
        listCtrl_->SetItem(index, 3, wxString::Format("%.6f", wp.coordinates.lat));
        
        if (i > 0) {
            double dist = (*waypoints_)[i-1].coordinates.DistanceTo(wp.coordinates);
            listCtrl_->SetItem(index, 4, wxString::Format("%.0f", dist));
        } else {
            listCtrl_->SetItem(index, 4, "-");
        }
    }
}

} // namespace ui