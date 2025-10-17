#pragma once

#include "ProjectMap.h"
#include "domain/state/NavigationState.h"
#include PRESENTATION_BASE_COMPONENT
#include <memory>
#include <string>

namespace presentation::components {

/**
 * @brief HUD 상태 구독자 베이스 컴포넌트
 * 
 * BaseComponent를 상속받아 wxWidgets와 통합되며, 
 * StateSubscriber 인터페이스를 구현하여 상태 변경을 구독합니다.
 */
template<typename WxBase>
class StateSubscriberComponent : public BaseComponent<WxBase>, 
                                public domain::state::StateSubscriber {
public:
    explicit StateSubscriberComponent(const std::string& componentName, WxBase* parent = nullptr)
        : BaseComponent<WxBase>(componentName, parent) {
    }
    
    virtual ~StateSubscriberComponent() = default;
    
    // === StateSubscriber 인터페이스 구현 ===
    std::string GetSubscriberId() const override {
        return this->GetComponentName();
    }
    
    // 기본적으로 모든 이벤트 구독
    std::vector<domain::state::HudStateEventType> GetSubscribedEvents() const override {
        return {domain::state::HudStateEventType::All};
    }
    
protected:
    /**
     * @brief 상태 스토어 구독 시작
     */
    void StartSubscription() {
        auto sharedThis = std::static_pointer_cast<domain::state::StateSubscriber>(
            this->shared_from_this());
        domain::state::HudStateManager::GetInstance().Subscribe(sharedThis);
    }
    
    /**
     * @brief 상태 스토어 구독 해제
     */
    void StopSubscription() {
        auto sharedThis = std::static_pointer_cast<domain::state::StateSubscriber>(
            this->shared_from_this());
        domain::state::HudStateManager::GetInstance().Unsubscribe(sharedThis);
    }
};

/**
 * @brief wxWidgets와 연동된 상태 구독자 컴포넌트
 */
using StateSubscriberWxComponent = StateSubscriberComponent<wxWindow>;

/**
 * @brief wxWidgets 없는 상태 구독자 컴포넌트
 */
using StateSubscriberNoWxComponent = StateSubscriberComponent<NoWxBase>;

} // namespace presentation::components