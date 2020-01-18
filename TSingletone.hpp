#pragma once

#include <memory>
#include <mutex>

#include <iostream>
#include <string>
using namespace std;

template < typename T >
class TSingleton {
public:
    static T& GetInstance() {
        if (!m_instance) {
            std::call_once(m_onceflag,
                [] {
                    m_instance = new T();
                }
            );
        }
        return *m_instance;
    }

    TSingleton(const TSingleton&) = delete;
    TSingleton& operator= (const TSingleton) = delete;

protected:
    TSingleton() {
    };
    virtual ~TSingleton() {
    }

private:
    inline static T* m_instance = nullptr;
    inline static std::once_flag m_onceflag;
};

