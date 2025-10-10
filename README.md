# wxTmap Explorer
*A Tmap-Inspired Navigation GUI using OpenStreetMap*

---

## 📌 Overview
wxTmap Explorer is a portfolio-ready desktop navigation GUI built with C++17, wxWidgets, and Leaflet (OpenStreetMap tiles). It mimics the Tmap UX: search bar, POI results, origin/destination selection, and a summary map.

- Tmap Open API → used only for POI/geo search (coordinates).
- Route planners → pluggable strategy pattern (direct geodesic, grid A*).
- Future extension → integration with ROS/Gazebo simulator (Project 2) and full GUI↔ROS bridge (Project 3).

---

## ✨ Features
- Tmap-like navigation UI (search, results, markers, polyline route).
- OpenStreetMap tiles by default; VWorld WMTS supported for KR maps.
- Multiple planners:
  - `DummyPlanner` (geodesic interpolation)
  - `GridAStarPlanner` (simple grid-based A*)
- Async POI search (non-blocking UI).
- Abstract `ISimBridge` for later ROS/Gazebo integration.

---

## 📂 Project Structure

```
wxTmap-Explorer/
├─ app/
│  ├─ CMakeLists.txt
│  ├─ include/
│  │  ├─ AppFrame.h
│  │  ├─ MapPanel.h
│  │  ├─ TmapClient.h
│  │  ├─ Types.h
│  │  ├─ IRoutePlanner.h
│  │  ├─ planners/
│  │  │  ├─ DummyPlanner.h        # direct geodesic interpolation
│  │  │  ├─ GridAStarPlanner.h    # simple grid A* example
│  │  │  └─ PlannerFactory.h
│  │  └─ IsimBridge.h             # reserved hook for ROS integration
│  ├─ src/
│  │  ├─ main.cpp, AppFrame.cpp, MapPanel.cpp, TmapClient.cpp
│  │  └─ planners/ (impls: DummyPlanner.cpp, GridAStarPlanner.cpp, PlannerFactory.cpp)
│  │  └─ SimBridgeDummy.cpp
│  └─ ui/                         # Leaflet static assets
│     ├─ index.html
│     ├─ app.js
│     └─ style.css
├─ config/
│  └─ appsettings.json            # API keys, planner & tiles selection
├─ LICENSE
└─ README.md
```

---

## ⚙️ Build & Run

### Requirements
- CMake ≥ 3.20
- Ninja
- wxWidgets ≥ 3.1
- C++17 compiler
- Leaflet (via CDN)
- cpr & nlohmann/json (via CMake FetchContent)

### OS Packages

- macOS (Homebrew): `brew install cmake ninja wxwidgets pkg-config git`
- Ubuntu 22.04/24.04: `sudo apt-get install -y build-essential cmake ninja-build pkg-config git libwxgtk3.2-dev libwxgtk-webview3.2-dev libcurl4-openssl-dev`
  - 대안(구버전): `libwxgtk3.0-gtk3-dev libwxgtk-webview3.0-gtk3-dev`
  - wxWidgets를 소스로 빌드 시: `libgtk-3-dev libwebkit2gtk-4.0-dev`
- Windows
  - MSYS2(UCRT64) 권장: `pacman -S --needed mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-wxWidgets git`
  - WebView2 런타임(권장, wxWebView 최신 백엔드): Microsoft Edge WebView2 Evergreen Runtime 설치
  - 대안(vcpkg + MSVC): `vcpkg install wxwidgets cpr nlohmann-json`

### macOS Example
```bash
brew install cmake ninja wxwidgets
export PKG_CONFIG_PATH="$(brew --prefix wxwidgets)/lib/pkgconfig:$PKG_CONFIG_PATH"

git clone https://github.com/<your-name>/wxTmap-Explorer.git
cd wxTmap-Explorer
mkdir -p build && cd build
cmake -G Ninja ../app
ninja
```

### Configure
Create `config/appsettings.json` (or use environment variables):

```jsonc
{
  "TMAP_APPKEY": "YOUR_TMAP_KEY",
  "PLANNER": "direct",      // direct | grid
  "TILES": "osm",           // osm | vworld
  "GRID": { "cell_deg": 0.0002 }
}
```

### Run
```bash
./tmap_app
```

---

## 🖥️ Cross-Platform Notes

- Windows (MSYS2 UCRT64): UCRT64 터미널에서 실행하세요.
  - Configure: `cmake -S app -B build -G Ninja -DCMAKE_BUILD_TYPE=Release`
  - Build: `cmake --build build -j`
  - Run: `./build/tmap_app.exe`
- Ubuntu: `sudo apt-get install` 목록 설치 후 동일한 CMake/Ninja 명령으로 빌드합니다.
- 패키지 매니저 대신 vcpkg를 쓰는 경우:
  - `cmake -S app -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake -DUSE_SYSTEM_CPR=ON -DUSE_SYSTEM_JSON=ON`

---

## 🧭 How It Works
1. Search: enter query → Tmap API (POI only).
2. Select: double-click result → marker on map.
3. Start: both origin/destination set → planner computes route → Leaflet draws polyline.

---

## 🧩 Pluggable Planners

Interface (`IRoutePlanner`):

```cpp
class IRoutePlanner {
public:
  virtual ~IRoutePlanner() = default;
  virtual bool init(const std::string& config_json_path) = 0;
  virtual std::vector<LonLat> computeRoute(LonLat start, LonLat goal) = 0;
  virtual const char* name() const = 0;
};
```

Included:
- DummyPlanner: geodesic interpolation (fast demo).
- GridAStarPlanner: basic A* on synthetic grid.

Future additions:
- OSRM/Valhalla client
- Custom OSM graph A*

---

## 🗺️ Tiles
- Default: OpenStreetMap (respect usage policy).
- Option: VWorld WMTS for KR basemap (requires API key).

---

## 🔒 Notes
- No API keys in code. Use `config/appsettings.json` or environment variables.
- Tmap API used only for POI geosearch (not routing/traffic).
- Routes are planner-based → fully controllable and extensible.

---

## 🧱 Roadmap
- Project 2: ROS2/Gazebo traffic simulator (ego + NPC vehicles).
- Project 3: GUI ↔ ROS integration (`ISimBridge`).
- Route smoothing (Bezier, Chaikin).
- Enhanced UI polish (loading indicators, address formatting).

---

## 📈 Project Management
- Jira 케이스 스터디와 스크린샷/리포트를 `docs/`에 정리했습니다.
- 자세한 내용: `docs/case-study-jira.md`
- 스크린샷/리포트 저장 위치: `docs/jira/`
- 연동/자동화 가이드: `docs/jira/integration.md`
- 샘플 프로젝트 구조: `docs/jira/sample-project.md`

---

## 📄 License

MIT. See `LICENSE` for details.

---

## 🐳 Docker/CI 베이스 패키지

- 베이스 이미지: `ubuntu:22.04` 또는 `ubuntu:24.04`
- 필수 패키지 목록:
  - build-essential cmake ninja-build pkg-config git curl ca-certificates
  - libwxgtk3.2-dev libwxgtk-webview3.2-dev
  - libcurl4-openssl-dev
  - (옵션) libgtk-3-dev libwebkit2gtk-4.0-dev  # wxWidgets를 소스로 빌드할 때

예시 Dockerfile 스니펫:

```dockerfile
FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential cmake ninja-build pkg-config git curl ca-certificates \
    libwxgtk3.2-dev libwxgtk-webview3.2-dev \
    libcurl4-openssl-dev \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY app /app
COPY config/appsettings.json /app/../config/appsettings.json
RUN cmake -S /app -B /build -G Ninja -DCMAKE_BUILD_TYPE=Release \
 && cmake --build /build -j
```
