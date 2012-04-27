#ifndef __SINGLETON_H__
#define __SINGLETON_H__

template <class T>
class Singleton {
public:
    static inline T* instance() {
        if ( !instance_ ) {
            instance_ = new T;
        }
        return instance_;
    }
    static void release() {
        if (!instance_) {
            delete instance_;
            instance_ = NULL;
        }
    }
protected:
    Singleton() {}
    virtual ~Singleton(){}
private:
    static T*   instance_;
};

#define DECLARE_SINGLETON_CLASS(T) \
    template <class T> \
    T* Singleton<T>::instance_ = NULL;

#endif
