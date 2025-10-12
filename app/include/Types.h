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
        char hex[8];
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

