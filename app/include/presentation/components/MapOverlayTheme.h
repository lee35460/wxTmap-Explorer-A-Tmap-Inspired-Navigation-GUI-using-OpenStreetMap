#pragma once
#include <wx/pen.h>

namespace presentation::components {

struct PolylineTheme {
    wxColour doneColor{0, 170, 255};     // 완료(하이라이트): 파란색
    wxColour remainColor{180, 180, 180}; // 남은 구간: 회색
    wxPenCap cap{wxCAP_ROUND};
    wxPenJoin join{wxJOIN_ROUND};
};

}  // namespace presentation::components