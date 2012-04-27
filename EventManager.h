#ifndef __EVENT_MANAGER_H__
#define __EVENT_MANAGER_H__

#include <deque>
#include <string>
#include "Singleton.h"
#include "pthread.h"

typedef void (*REQUEST_CALLBACK) (std::string &strurl,std::string &strdata);
typedef void (*REQUEST_CALLBACK) (std::string &strurl, std::string &strdata);

#define EVENT_TYPE_UNKOWN   0
#define EVENT_REQUET_DATA   1
#define EVENT_DOWNLOAD_FILE 2

class Event {
public:
    Event():
    type(EVENT_TYPE_UNKOWN),
    str_url(""),
    str_data(""),
    on_error(0),
    on_complete(0){
    }
    Event(int t, std::string &url, std::string &save):
    type(t),
    str_url(url),
    str_data(save),
    on_error(0),
    on_complete(0){
    }
    int type;
    std::string         str_url;
    std::string         str_data;
    REQUEST_CALLBACK    on_error;
    REQUEST_CALLBACK    on_complete;
    /*
    void set_on_error(ERROR_CALLBACK err) {
        on_error = err;
    }
    void set_on_complete(COMPLETE_CALLBACK complete) {
        on_complete = complete;
    }
    */
};

//thread safe
class EventManager : public Singleton<EventManager> {
private:
    std::deque <Event *>    wait_queue_;
    std::deque <Event *>    error_queue_;
    std::deque <Event *>    complete_queue_;

    sp_thread_mutex_t  wait_mutex_;
    sp_thread_mutex_t  error_mutex_;
    sp_thread_mutex_t  complete_mutex_;

    void lock_wait() {
        sp_thread_mutex_lock(&wait_mutex_);
    }
    void unlock_wait() {
        sp_thread_mutex_unlock(&wait_mutex_);
    }
    void lock_error() {
        sp_thread_mutex_lock(&error_mutex_);
    }
    void unlock_error() {
        sp_thread_mutex_unlock(&error_mutex_);
    }
    void lock_complete(){
        sp_thread_mutex_lock(&complete_mutex_);
    }
    void unlock_complete() {
        sp_thread_mutex_unlock(&complete_mutex_);
    }

public:
    EventManager() {
        sp_thread_mutex_init(&wait_mutex_, NULL);
        sp_thread_mutex_init(&error_mutex_, NULL);
        sp_thread_mutex_init(&complete_mutex_, NULL);
    }
    ~EventManager() {
        sp_thread_mutex_destroy(&wait_mutex_);
        sp_thread_mutex_destroy(&error_mutex_);
        sp_thread_mutex_destroy(&complete_mutex_);
    }
    //wait queue
    void push_wait_event(Event *event) {
        lock_wait();
        wait_queue_.push_back (event);
        unlock_wait();
    }
    Event *pop_wait_event() {
        Event *event = NULL;
        lock_wait();
        if (wait_queue_.size() > 0) {
            event = wait_queue_.front();
            wait_queue_.pop_front();
        }
        unlock_wait();
        return event;
    }
    //error queue
    void push_error_event(Event *event) {
        lock_error();
        error_queue_.push_back (event);
        unlock_error();
    }
    Event *pop_error_event() {
        Event *event = NULL;
        lock_error();
        if (error_queue_.size() > 0) {
            event = error_queue_.front();
            error_queue_.pop_front();
        }
        unlock_error();
        return event;
    }
    //complete queue
    void push_complete_event(Event *event) {
        lock_complete();
        complete_queue_.push_back (event);
        unlock_complete();
    }
    Event *pop_completer_event() {
        Event *event = NULL;
        lock_complete();
        if (complete_queue_.size() > 0) {
            event = complete_queue_.front();
            complete_queue_.pop_front();
        }
        unlock_complete();
        return event;
    }
};

#endif
