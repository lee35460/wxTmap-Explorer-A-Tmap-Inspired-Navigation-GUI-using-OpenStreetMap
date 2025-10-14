#pragma once
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <cmath>
#include <algorithm>

// 기본 지리 좌표 타입
struct LonLat {
    double lon{0.0};  // 경도 (Longitude): -180.0 ~ 180.0
    double lat{0.0};  // 위도 (Latitude): -90.0 ~ 90.0

    // 기본 생성자
    LonLat() = default;

    // 편의 생성자
    LonLat(double longitude, double latitude)
        : lon(longitude), lat(latitude) {}

    // 비교 연산자 (std::map 키로 사용하기 위해)
    bool operator<(const LonLat& other) const {
        if (lon != other.lon) return lon < other.lon;
        return lat < other.lat;
    }
    
    // 동등성 비교 (테스트용)
    bool operator==(const LonLat& other) const {
        return std::abs(lon - other.lon) < 1e-6 && 
               std::abs(lat - other.lat) < 1e-6;
    }

    // 유효성 검사 - 실제 지구 좌표 범위 체크
    bool IsValid() const {
        return lon >= -180.0 && lon <= 180.0 &&
               lat >= -90.0 && lat <= 90.0;
    }

    // 문자열 변환 - 디버깅용
    std::string ToString() const {
        return "LonLat(" + std::to_string(lon) + ", " + std::to_string(lat) + ")";
    }

    // 거리 계산 (Haversine 공식 사용, 단위: 미터)
    double DistanceTo(const LonLat& other) const {
        static const double R = 6371000.0; // 지구 반경 (미터)
        double lat1_red = lat * M_PI / 180.0;
        double lat2_red = other.lat * M_PI / 180.0;
        double delta_lat = (other.lat - lat) * M_PI / 180.0;
        double delta_lon = (other.lon - lon) * M_PI / 180.0;

        // Haversine 공식
        // a = sin²(Δφ/2) + cos φ1 ⋅ cos φ2 ⋅ sin²(Δλ/2)
        // c = 2 ⋅ atan2( √a, √(1−a) )
        // d = R ⋅ c
        double a = std::sin(delta_lat / 2) * std::sin(delta_lat / 2) +
                   std::cos(lat1_red) * std::cos(lat2_red) *
                std::sin(delta_lon / 2) * std::sin(delta_lon / 2);
        double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
        return R * c; // 거리 (미터)
    }
};  

// 웨이포인트 구조체 - 경유지 정보
struct Waypoint {
    LonLat coordinates; // gps 좌표
    std::string name; // 표시명(예: "홈", "회사")
    std::string address; // 상세 주소
    int priority{0}; // 우선순위(0=낮음, 1=보통, 2=높음)
    bool isDestination{false}; // 목적지 여부

    // 기본 생성자
    Waypoint() = default;

    // 편의 생성자
    Waypoint(double lon, double lat, const std::string& n = "")
        : coordinates(lon, lat), name(n) {}

    Waypoint(const LonLat& coords, const std::string& n = "")
        : coordinates(coords), name(n) {}

    // 비교 연산자
    bool operator==(const Waypoint& other) const {
        return coordinates == other.coordinates && name == other.name;      
    }
};

// 🎨 간단한 색상 구조체 (wxWidgets 의존성 없음)
struct SimpleColor {
    unsigned char r{0}, g{0}, b{0}, a{255};
    
    SimpleColor() = default;
    SimpleColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
        
    // 16진수 문자열로 변환
    std::string ToHexString() const {
        char hex[8]; // 7문자 (#RRGGBB) + null terminator = 8 바이트
        snprintf(hex, sizeof(hex), "#%02X%02X%02X", r, g, b);
        return std::string(hex);
    }
};

// 📊 간단한 통계 구조체
struct CoordinateStats {
    size_t total_count{0};
    size_t valid_count{0};
    double min_lon{180.0}, max_lon{-180.0};
    double min_lat{90.0}, max_lat{-90.0};
    double total_distance{0.0}; // 연속 좌표간 총 거리
    
    // 통계 업데이트
    void AddCoordinate(const LonLat& coord) {
        total_count++;
        if (coord.IsValid()) {
            valid_count++;
            min_lon = std::min(min_lon, coord.lon);
            max_lon = std::max(max_lon, coord.lon);
            min_lat = std::min(min_lat, coord.lat);
            max_lat = std::max(max_lat, coord.lat);
        }
    }
    
    // 유효율 계산
    double GetValidRate() const {
        return total_count > 0 ? static_cast<double>(valid_count) / total_count : 0.0;
    }
};

// === WXT-59: Location Puck + Camera Follow 관련 구조체 ===

struct LocationState {
    LonLat coordinates; // 현재 위치 좌표
    double accuracy{0.0}; // 위치 정확도 변경 (미터 단위)
    double bearing{0.0};  // 이동 방향 (0~360도)
    bool hasBearing{false}; // 방향 정보 유무
    bool isValid{false}; // 좌표 유효성
    std::chrono::system_clock::time_point timestamp; // 위치 업데이트 시각

    // 기본 생성자
    LocationState() = default;

    // 편의 생성자
    LocationState(const LonLat& coords, double acc = 0.0, double bear=0.0, bool hasBear = false, bool valid = false)
        : coordinates(coords), accuracy(acc), bearing(bear), hasBearing(hasBear), isValid(valid),
          timestamp(std::chrono::system_clock::now()) {};

    // 동등성 비교 (테스트용)
    bool operator==(const LocationState& other) const {
        return coordinates == other.coordinates &&
               accuracy == other.accuracy &&
               bearing == other.bearing &&
               hasBearing == other.hasBearing &&
               isValid == other.isValid &&
               timestamp == other.timestamp;
    };

    // 문자열 변환 - 디버깅용
    std::string ToString() const {
        return "LocationState(" + coordinates.ToString() +
               ", accuracy=" + std::to_string(accuracy) +
               ", bearing=" + std::to_string(bearing) +
               ", hasBearing=" + (hasBearing ? "true" : "false") +
               ", isValid=" + (isValid ? "true" : "false") + ")";
    };

    // 유효성 검사
    bool IsValid() const {
        return isValid && coordinates.IsValid();
    };

    // 거리 계산 (Haversine 공식 사용, 단위: 미터)
    double DistanceTo(const LocationState& other) const {
        return coordinates.DistanceTo(other.coordinates);

    };
};

// 카메라 Follow 모드 - Off (비활성화), Location (위치만), Bearing (위치+방향)
enum class CameraFollowMode {
    Off = 0,
    Location = 1,
    Bearing = 2
};

// LocationPuck 테마 설정 - 색상, 크기, 스타일
struct LocationPuckTheme {
    SimpleColor puckColor{0, 122, 255};        // iOS 블루
    SimpleColor accuracyColor{0, 122, 255, 50}; // 반투명 정확도 원
    SimpleColor arrowColor{255, 255, 255};      // 방향 화살표
    double puckRadius{8.0};
    // 나머지는 한 줄씩...

    double arrowLength{12.0}; // 화살표 길이
    double strokeWidth{2.0}; // 테두리 두께

    // 기본 생성자
    LocationPuckTheme() = default;

    // 편의 생성자
    LocationPuckTheme(const SimpleColor& puckCol, const SimpleColor& accCol,
                      const SimpleColor& arrowCol, double radius, double arrowLen, double strokeW)
        : puckColor(puckCol), accuracyColor(accCol), arrowColor(arrowCol),
          puckRadius(radius), arrowLength(arrowLen), strokeWidth(strokeW) {};

    // 동등성 비교 (테스트용)
    bool operator==(const LocationPuckTheme& other) const {
        return puckColor.r == other.puckColor.r && puckColor.g == other.puckColor.g &&
               puckColor.b == other.puckColor.b && puckColor.a == other.puckColor.a &&
               accuracyColor.r == other.accuracyColor.r && accuracyColor.g == other.accuracyColor.g &&
               accuracyColor.b == other.accuracyColor.b && accuracyColor.a == other.accuracyColor.a &&
               arrowColor.r == other.arrowColor.r && arrowColor.g == other.arrowColor.g &&
               arrowColor.b == other.arrowColor.b && arrowColor.a == other.arrowColor.a &&
               puckRadius == other.puckRadius &&
               arrowLength == other.arrowLength &&
               strokeWidth == other.strokeWidth;    
    };

    // 문자열 변환 - 디버깅용
    std::string ToString() const {
        return "LocationPuckTheme(puckColor=" + puckColor.ToHexString() +
               ", accuracyColor=" + accuracyColor.ToHexString() +
               ", arrowColor=" + arrowColor.ToHexString() +
               ", puckRadius=" + std::to_string(puckRadius) +
               ", arrowLength=" + std::to_string(arrowLength) +
               ", strokeWidth=" + std::to_string(strokeWidth) + ")";
   }
};

// 거리 계산 헬퍼 함수 (LocationState용)
inline double CalculateDistance(const LonLat& from, const LonLat& to) {
    return from.DistanceTo(to);
}
// }    