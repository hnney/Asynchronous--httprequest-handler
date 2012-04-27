#ifndef _HTTPREQUEST_H__
#define _HTTPREQUEST_H__
#include "EventManager.h"

class HTTPRequest : public Singleton<HTTPRequest> {
public:
    HTTPRequest() { running_ = false; timeout_ = 0;}
    ~HTTPRequest() { }
    inline int is_running() { return running_==true;  }
    inline int timeout() {return timeout_;}
    void set_timeout (int time) {timeout_ = time;}
    
    //@brief:  dispatch event of the complete or error
    int dispatch ();
 
    //@brief:  start the http request thread
    int run();
    void stop();

    //@brief: push http request into EventManager
    int send_request(int type, const char *strurl, const char *strsave, REQUEST_CALLBACK oncomplete, REQUEST_CALLBACK onerror);
private:
    bool     running_;
    int      timeout_;
};

#endif //_HTTPREQUEST_H__
