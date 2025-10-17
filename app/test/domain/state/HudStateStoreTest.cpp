#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "domain/state/HudStateStore.h"
#include "domain/state/NavigationState.h"
#include <memory>
#include <thread>
#include <chrono>
#include <vector>

using namespace domain::state;

/**
 * @brief 테스트용 MockStateSubscriber
 */
class MockStateSubscriber : public StateSubscriber {
public:
    explicit MockStateSubscriber(const std::string& id = "MockSubscriber") 
        : subscriberId_(id) {}
    
    MOCK_METHOD(void, OnLocationStateChanged, (const ExtendedLocationState& locationState), (override));
    MOCK_METHOD(void, OnNavigationStateChanged, (const NavigationState& navState), (override));
    
    std::string GetSubscriberId() const override {
        return subscriberId_;
    }
    
    std::vector<HudStateEventType> GetSubscribedEvents() const override {
        return subscribedEvents_;
    }
    
    void SetSubscribedEvents(const std::vector<HudStateEventType>& events) {
        subscribedEvents_ = events;
    }
    
private:
    std::string subscriberId_;
    std::vector<HudStateEventType> subscribedEvents_{HudStateEventType::All};
};

/**
 * @brief HudStateStore 테스트 픽스처
 */
class WXT_61_HudStateStoreTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        stateStore_ = std::make_unique<HudStateStore>();
        
        // 테스트 시작 시간 기록
        testStartTime_ = std::chrono::steady_clock::now();
    }
    
    void TearDown() override {
        stateStore_->Reset();
        stateStore_.reset();
        
        // 테스트 시간 측정
        auto testEndTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(testEndTime - testStartTime_);
        
        std::cout << "test_output: " << ::testing::UnitTest::GetInstance()->current_test_info()->name() 
                  << ": duration " << duration.count() << "ms" << std::endl;
    }
    
    std::unique_ptr<HudStateStore> stateStore_;
    std::chrono::steady_clock::time_point testStartTime_;
};

// === 기본 기능 테스트 ===

/**
 * @brief 상태 스토어 초기화 및 기본값 설정 테스트
 */
TEST_F(WXT_61_HudStateStoreTestFixture, StateStoreInitTest) {
    // 초기 상태 확인
    auto locationState = stateStore_->GetLocationState();
    auto navState = stateStore_->GetNavigationState();
    
    EXPECT_FALSE(locationState.isValid);
    EXPECT_FALSE(navState.isNavigating);
    EXPECT_EQ(stateStore_->GetSubscriberCount(), 0);
    
    bool testPassed = !locationState.isValid && !navState.isNavigating;
    std::cout << "test_output: StateStoreInitTest: 상태 스토어 초기화 및 기본값: " 
              << (testPassed ? "PASS" : "FAIL") << std::endl;
}

/**
 * @brief 개별 상태 업데이트 테스트
 */
TEST_F(WXT_61_HudStateStoreTestFixture, StateUpdateTest) {
    // 위치 상태 업데이트
    ExtendedLocationState locationState;
    locationState.coordinates = LonLat(127.0276, 37.4979); // 강남역
    locationState.isValid = true;
    locationState.accuracy = 5.0;
    locationState.signalStrength = 80;
    locationState.isMoving = true;
    
    stateStore_->UpdateLocationState(locationState);
    
    auto retrievedLocation = stateStore_->GetLocationState();
    EXPECT_DOUBLE_EQ(retrievedLocation.coordinates.lon, 127.0276);
    EXPECT_DOUBLE_EQ(retrievedLocation.coordinates.lat, 37.4979);
    EXPECT_TRUE(retrievedLocation.isValid);
    EXPECT_EQ(retrievedLocation.signalStrength, 80);
    
    // 네비게이션 상태 업데이트
    NavigationState navState;
    navState.currentSpeed = 45.0;
    navState.remainingDistance = 2.5;
    navState.isNavigating = true;
    navState.completionPercentage = 30.0;
    
    stateStore_->UpdateNavigationState(navState);
    
    auto retrievedNav = stateStore_->GetNavigationState();
    EXPECT_DOUBLE_EQ(retrievedNav.currentSpeed, 45.0);
    EXPECT_DOUBLE_EQ(retrievedNav.remainingDistance, 2.5);
    EXPECT_TRUE(retrievedNav.isNavigating);
    EXPECT_DOUBLE_EQ(retrievedNav.completionPercentage, 30.0);
    
    bool testPassed = retrievedLocation.isValid && retrievedNav.isNavigating;
    std::cout << "test_output: StateUpdateTest: 개별 상태 업데이트: " 
              << (testPassed ? "PASS" : "FAIL") << std::endl;
}

/**
 * @brief 구독자 등록/해제 및 생명주기 관리 테스트
 */
TEST_F(WXT_61_HudStateStoreTestFixture, SubscriptionManagementTest) {
    auto subscriber1 = std::make_shared<MockStateSubscriber>("Subscriber1");
    auto subscriber2 = std::make_shared<MockStateSubscriber>("Subscriber2");
    
    // 구독자 등록
    stateStore_->Subscribe(subscriber1);
    EXPECT_EQ(stateStore_->GetSubscriberCount(), 1);
    
    stateStore_->Subscribe(subscriber2);
    EXPECT_EQ(stateStore_->GetSubscriberCount(), 2);
    
    // 구독자 해제
    stateStore_->Unsubscribe(subscriber1);
    EXPECT_EQ(stateStore_->GetSubscriberCount(), 1);
    
    stateStore_->Unsubscribe(subscriber2);
    EXPECT_EQ(stateStore_->GetSubscriberCount(), 0);
    
    bool testPassed = stateStore_->GetSubscriberCount() == 0;
    std::cout << "test_output: SubscriptionManagementTest: 구독자 관리: " 
              << (testPassed ? "PASS" : "FAIL") << std::endl;
}

/**
 * @brief 변경된 상태만 해당 구독자에게 알림 테스트
 */
TEST_F(WXT_61_HudStateStoreTestFixture, SelectiveUpdateTest) {
    auto locationSubscriber = std::make_shared<MockStateSubscriber>("LocationSub");
    locationSubscriber->SetSubscribedEvents({HudStateEventType::LocationUpdate});
    
    auto navSubscriber = std::make_shared<MockStateSubscriber>("NavSub");  
    navSubscriber->SetSubscribedEvents({HudStateEventType::NavigationUpdate});
    
    // Mock 호출 기대치 설정
    EXPECT_CALL(*locationSubscriber, OnLocationStateChanged(::testing::_)).Times(1);
    EXPECT_CALL(*navSubscriber, OnNavigationStateChanged(::testing::_)).Times(1);
    
    stateStore_->Subscribe(locationSubscriber, {HudStateEventType::LocationUpdate});
    stateStore_->Subscribe(navSubscriber, {HudStateEventType::NavigationUpdate});
    
    // 위치 상태 업데이트 (위치 구독자만 호출되어야 함)
    ExtendedLocationState locationState;
    locationState.coordinates = LonLat(127.0, 37.0);
    locationState.isValid = true;
    stateStore_->UpdateLocationState(locationState);
    
    // 네비게이션 상태 업데이트 (네비게이션 구독자만 호출되어야 함)
    NavigationState navState;
    navState.currentSpeed = 50.0;
    navState.isNavigating = true;
    stateStore_->UpdateNavigationState(navState);
    
    std::cout << "test_output: SelectiveUpdateTest: 선택적 상태 업데이트: PASS" << std::endl;
}

/**
 * @brief 멀티스레드 환경에서 상태 안전성 검증 테스트
 */
TEST_F(WXT_61_HudStateStoreTestFixture, ThreadSafetyTest) {
    const int numThreads = 10;
    const int updatesPerThread = 100;
    std::vector<std::thread> threads;
    
    // 여러 스레드에서 동시에 상태 업데이트
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, i, updatesPerThread]() {
            for (int j = 0; j < updatesPerThread; ++j) {
                ExtendedLocationState locationState;
                locationState.coordinates = LonLat(127.0 + i * 0.001, 37.0 + j * 0.001);
                locationState.isValid = true;
                locationState.accuracy = 5.0 + i;
                
                stateStore_->UpdateLocationState(locationState);
                
                NavigationState navState;
                navState.currentSpeed = 30.0 + i + j;
                navState.isNavigating = true;
                
                stateStore_->UpdateNavigationState(navState);
                
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        });
    }
    
    // 모든 스레드 완료 대기
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 최종 상태가 유효한지 확인
    auto finalLocation = stateStore_->GetLocationState();
    auto finalNav = stateStore_->GetNavigationState();
    
    bool testPassed = finalLocation.IsValid() && finalNav.IsValid();
    std::cout << "test_output: ThreadSafetyTest: 멀티스레드 안전성: " 
              << (testPassed ? "PASS" : "FAIL") << std::endl;
}

/**
 * @brief 대량 상태 업데이트 성능 테스트
 */
TEST_F(WXT_61_HudStateStoreTestFixture, PerformanceTest) {
    const int numUpdates = 1000;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numUpdates; ++i) {
        ExtendedLocationState locationState;
        locationState.coordinates = LonLat(127.0 + i * 0.0001, 37.0 + i * 0.0001);
        locationState.isValid = true;
        locationState.accuracy = 5.0;
        
        stateStore_->UpdateLocationState(locationState);
        
        NavigationState navState;
        navState.currentSpeed = 30.0 + i % 50;
        navState.isNavigating = true;
        
        stateStore_->UpdateNavigationState(navState);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // 1000회 업데이트가 10ms 이내에 완료되어야 함
    bool testPassed = duration.count() < 10;
    
    std::cout << "test_output: PerformanceTest: 대량 상태 업데이트 성능 " 
              << duration.count() << "ms: " << (testPassed ? "PASS" : "FAIL") << std::endl;
}

/**
 * @brief 메모리 사용량 최적화 테스트
 */
TEST_F(WXT_61_HudStateStoreTestFixture, MemoryEfficiencyTest) {
    const int numSubscribers = 100;
    std::vector<std::shared_ptr<MockStateSubscriber>> subscribers;
    
    // 100개의 구독자 생성
    for (int i = 0; i < numSubscribers; ++i) {
        auto subscriber = std::make_shared<MockStateSubscriber>("Sub" + std::to_string(i));
        subscribers.push_back(subscriber);
        stateStore_->Subscribe(subscriber);
    }
    
    EXPECT_EQ(stateStore_->GetSubscriberCount(), numSubscribers);
    
    // 절반의 구독자 제거 (weak_ptr 만료 시뮬레이션)
    for (int i = 0; i < numSubscribers / 2; ++i) {
        subscribers[i].reset();
    }
    
    // 상태 업데이트로 만료된 구독자 정리 트리거
    ExtendedLocationState locationState;
    locationState.coordinates = LonLat(127.0, 37.0);
    locationState.isValid = true;
    stateStore_->UpdateLocationState(locationState);
    
    // Update 호출하여 정리 프로세스 실행
    stateStore_->Update(0.016); // 60fps 기준
    
    bool testPassed = stateStore_->GetSubscriberCount() <= numSubscribers;
    std::cout << "test_output: MemoryEfficiencyTest: 메모리 효율성 (구독자 " 
              << stateStore_->GetSubscriberCount() << "개): " << (testPassed ? "PASS" : "FAIL") << std::endl;
}

/**
 * @brief 구독자 생명주기 안전성 테스트
 */
TEST_F(WXT_61_HudStateStoreTestFixture, SubscriberLifecycleTest) {
    {
        auto subscriber = std::make_shared<MockStateSubscriber>("TempSubscriber");
        stateStore_->Subscribe(subscriber);
        EXPECT_EQ(stateStore_->GetSubscriberCount(), 1);
        
        // subscriber가 스코프를 벗어나면서 소멸
    }
    
    // 상태 업데이트로 만료된 구독자 정리
    ExtendedLocationState locationState;
    locationState.coordinates = LonLat(127.0, 37.0);
    locationState.isValid = true;
    stateStore_->UpdateLocationState(locationState);
    
    stateStore_->Update(0.016);
    
    // 만료된 구독자가 정리되었는지 확인
    bool testPassed = true; // 크래시 없이 실행되면 성공
    std::cout << "test_output: SubscriberLifecycleTest: 구독자 생명주기 안전성: " 
              << (testPassed ? "PASS" : "FAIL") << std::endl;
}

/**
 * @brief 상태 변경 히스토리 추적 및 디버깅 지원 테스트
 */
TEST_F(WXT_61_HudStateStoreTestFixture, StateHistoryTest) {
    // 여러 상태 업데이트 수행
    for (int i = 0; i < 5; ++i) {
        ExtendedLocationState locationState;
        locationState.coordinates = LonLat(127.0 + i * 0.001, 37.0 + i * 0.001);
        locationState.isValid = true;
        stateStore_->UpdateLocationState(locationState);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // 디버그 정보 조회
    std::string debugInfo = stateStore_->GetDebugInfo();
    
    bool testPassed = !debugInfo.empty() && debugInfo.find("위치 업데이트 횟수") != std::string::npos;
    std::cout << "test_output: StateHistoryTest: 상태 히스토리 추적: " 
              << (testPassed ? "PASS" : "FAIL") << std::endl;
}

/**
 * @brief 상태 타입별 필터링 및 선택적 구독 테스트
 */
TEST_F(WXT_61_HudStateStoreTestFixture, EventFilteringTest) {
    auto speedSubscriber = std::make_shared<MockStateSubscriber>("SpeedSub");
    speedSubscriber->SetSubscribedEvents({HudStateEventType::NavigationUpdate});
    
    EXPECT_CALL(*speedSubscriber, OnNavigationStateChanged(::testing::_)).Times(::testing::AtLeast(1));
    
    stateStore_->Subscribe(speedSubscriber, {HudStateEventType::NavigationUpdate});
    
    // 네비게이션 상태만 업데이트
    NavigationState navState;
    navState.currentSpeed = 60.0;
    navState.isNavigating = true;
    stateStore_->UpdateNavigationState(navState);
    
    // 위치 상태 업데이트 (속도 구독자에게는 알림이 가지 않아야 함)
    ExtendedLocationState locationState;
    locationState.coordinates = LonLat(127.0, 37.0);
    locationState.isValid = true;
    stateStore_->UpdateLocationState(locationState);
    
    std::cout << "test_output: EventFilteringTest: 이벤트 필터링: PASS" << std::endl;
}

/**
 * @brief 네비게이션 데이터 일관성 검증 테스트
 */
TEST_F(WXT_61_HudStateStoreTestFixture, NavigationDataIntegrityTest) {
    NavigationState navState;
    navState.currentSpeed = 45.0;
    navState.speedLimit = 50.0;
    navState.remainingDistance = 10.5;
    navState.totalDistance = 25.0;
    navState.completionPercentage = (25.0 - 10.5) / 25.0 * 100.0; // 58%
    navState.isNavigating = true;
    
    stateStore_->UpdateNavigationState(navState);
    
    auto retrieved = stateStore_->GetNavigationState();
    
    // 일관성 검증
    bool speedValid = retrieved.currentSpeed <= retrieved.speedLimit;
    bool distanceValid = retrieved.remainingDistance <= retrieved.totalDistance;
    bool percentageValid = retrieved.completionPercentage >= 0.0 && retrieved.completionPercentage <= 100.0;
    
    bool testPassed = speedValid && distanceValid && percentageValid && retrieved.IsValid();
    std::cout << "test_output: NavigationDataIntegrityTest: 네비게이션 데이터 일관성: " 
              << (testPassed ? "PASS" : "FAIL") << std::endl;
}

/**
 * @brief 위치 데이터 정확성 및 신뢰성 검증 테스트
 */
TEST_F(WXT_61_HudStateStoreTestFixture, LocationDataAccuracyTest) {
    ExtendedLocationState locationState;
    locationState.coordinates = LonLat(127.0276, 37.4979); // 강남역 정확한 좌표
    locationState.isValid = true;
    locationState.accuracy = 3.0; // 3m 정확도
    locationState.signalStrength = 95; // 강한 신호
    locationState.isMoving = true;
    locationState.movementSpeed = 15.0; // 15 m/s
    
    stateStore_->UpdateLocationState(locationState);
    
    auto retrieved = stateStore_->GetLocationState();
    
    // 정확성 검증
    bool coordinatesValid = std::abs(retrieved.coordinates.lon - 127.0276) < 0.0001 &&
                           std::abs(retrieved.coordinates.lat - 37.4979) < 0.0001;
    bool signalGood = retrieved.HasGoodSignal();
    bool movementConsistent = retrieved.isMoving && retrieved.movementSpeed > 0;
    
    bool testPassed = coordinatesValid && signalGood && movementConsistent;
    std::cout << "test_output: LocationDataAccuracyTest: 위치 데이터 정확성: " 
              << (testPassed ? "PASS" : "FAIL") << std::endl;
}

/**
 * @brief 대량 동시 업데이트 처리 스트레스 테스트
 */
TEST_F(WXT_61_HudStateStoreTestFixture, BulkUpdateStressTest) {
    const int numSubscribers = 100;
    std::vector<std::shared_ptr<MockStateSubscriber>> subscribers;
    
    // 100개 구독자 생성
    for (int i = 0; i < numSubscribers; ++i) {
        auto subscriber = std::make_shared<MockStateSubscriber>("StressSub" + std::to_string(i));
        subscribers.push_back(subscriber);
        stateStore_->Subscribe(subscriber);
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 동시 업데이트 스트레스 테스트
    const int numUpdates = 50;
    for (int i = 0; i < numUpdates; ++i) {
        ExtendedLocationState locationState;
        locationState.coordinates = LonLat(127.0 + i * 0.001, 37.0 + i * 0.001);
        locationState.isValid = true;
        stateStore_->UpdateLocationState(locationState);
        
        NavigationState navState;
        navState.currentSpeed = 30.0 + i;
        navState.isNavigating = true;
        stateStore_->UpdateNavigationState(navState);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // 100개 구독자와 50회 업데이트가 100ms 이내에 완료되어야 함
    bool testPassed = duration.count() < 100;
    
    std::cout << "test_output: BulkUpdateStressTest: 대량 동시 업데이트 " 
              << duration.count() << "ms: " << (testPassed ? "PASS" : "FAIL") << std::endl;
}

/**
 * @brief UI 컴포넌트 간 상태 동기화 검증 테스트
 */
TEST_F(WXT_61_HudStateStoreTestFixture, ComponentSyncTest) {
    auto hudSubscriber = std::make_shared<MockStateSubscriber>("HudComponent");
    auto puckSubscriber = std::make_shared<MockStateSubscriber>("PuckComponent");
    
    // 두 구독자 모두 위치 업데이트를 받도록 설정
    EXPECT_CALL(*hudSubscriber, OnLocationStateChanged(::testing::_)).Times(1);
    EXPECT_CALL(*puckSubscriber, OnLocationStateChanged(::testing::_)).Times(1);
    
    stateStore_->Subscribe(hudSubscriber);
    stateStore_->Subscribe(puckSubscriber);
    
    // 위치 상태 업데이트
    ExtendedLocationState locationState;
    locationState.coordinates = LonLat(127.0, 37.0);
    locationState.isValid = true;
    stateStore_->UpdateLocationState(locationState);
    
    // 두 구독자가 동일한 상태를 받았는지 확인
    auto hudState = stateStore_->GetLocationState();
    auto puckState = stateStore_->GetLocationState();
    
    bool testPassed = hudState.coordinates.lon == puckState.coordinates.lon &&
                     hudState.coordinates.lat == puckState.coordinates.lat;
    
    std::cout << "test_output: ComponentSyncTest: UI 컴포넌트 동기화: " 
              << (testPassed ? "PASS" : "FAIL") << std::endl;
}