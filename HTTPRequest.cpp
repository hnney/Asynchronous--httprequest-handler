#include "HTTPRequest.h"
#include "curl/curl.h"
#include <stdio.h>

#include <iostream>

static char errorBuffer[CURL_ERROR_SIZE];
typedef size_t (*write_callback)(void *ptr, size_t size, size_t nmemb, void *stream);
//curl  Ïà¹Ø
size_t write_file(void *ptr, size_t size, size_t nmemb, void *stream){
    int written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    std::string *str = (std::string *)stream;
    size_t sizes = size * nmemb;
    str->append((char*)ptr, size*nmemb);
    return sizes;
}

int use_curl(Event *event, write_callback callback, void *stream) {
    CURLcode code = CURLE_OK;
    CURL *curl = curl_easy_init();
    do {
        if (!curl) {       
            break;
        }
        code = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
        if (code != CURLE_OK) {
            break;
        }
        code = curl_easy_setopt(curl, CURLOPT_URL, event->str_url.c_str());
        if (code != CURLE_OK) {
            break;
        }
        code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
        if (code != CURLE_OK) {
            break;
        }
        code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, stream);
        if (code != CURLE_OK) {
            break;
        }
        code = curl_easy_setopt(curl, CURLOPT_TIMEOUT, HTTPRequest::instance()->timeout());
        if (code != CURLE_OK) {
            break;
        }
        code = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, HTTPRequest::instance()->timeout()*3);
        if (code != CURLE_OK) {
            break;
        }
        code = curl_easy_perform(curl);
        if (code != CURLE_OK) {
            break;
        }
        //check http response code
        int response = 0;
        code = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response); 
        if (code != CURLE_OK || response / 100 != 2) {
            code = CURLE_HTTP_RETURNED_ERROR;
        }
    }while (0);
    if (curl) {
        curl_easy_cleanup(curl);
    }
    return (code==CURLE_OK?0:1);
}


int get_data(Event *event) {
    return use_curl(event, write_data, &event->str_data);
}

int download_file(Event *event) {
    //openfile
    if (event->str_data.length() == 0) {
        return 1;
    }
    FILE *file = fopen(event->str_data.c_str(), "wb");
    if (!file) {
        return 1;
    }
    int code = use_curl(event, write_file, file);
    if (file) {
        fclose(file);
    }
    return code;
}

sp_thread_result_t SP_THREAD_CALL http_request_thread(void *arg)
{
    while (HTTPRequest::instance()->is_running()) {
        Event *event =  HTTPRequest::instance()->get_event().pop_wait_event();
        if(event) {
            int ret = -1;
            if (event->type == EVENT_REQUET_DATA) {
                ret = get_data(event);
            }
            else if (event->type == EVENT_DOWNLOAD_FILE) {
                ret = download_file(event);
            }
            if (ret == 0) {
                HTTPRequest::instance()->get_event().push_complete_event(event);
            }
            else {
                event->str_data = errorBuffer;
                HTTPRequest::instance()->get_event().push_error_event(event);
            }   
        }
        else {
            sp_sleep(1000);
            std::cout<<"thread loop\n";
        }
    }
    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////
DECLARE_SINGLETON_CLASS(HTTPRequest)
EventManager HTTPRequest::event_manager_;

int HTTPRequest::send_request(int type, const char *strurl, const char *strsave, REQUEST_CALLBACK oncomplete, REQUEST_CALLBACK onerror) {
    Event *event = new Event;
    event->type = type;
    if (strurl) {
        event->str_url = strurl;
    }
    if (strsave) {
        event->str_data = strsave;
    }
    event->on_complete = oncomplete;
    event->on_error = onerror;
    HTTPRequest::instance()->get_event().push_wait_event(event);
    return 0;
}

int HTTPRequest::dispatch() {
    int ret = 0;
    Event *event = HTTPRequest::instance()->get_event().pop_completer_event();
    if (event) {
        if (event->on_complete) {
            (event->on_complete)(event->str_url, event->str_data);
        }
        ret++;
        delete event;
    }
    event = HTTPRequest::instance()->get_event().pop_error_event();
    if (event) {
        if ( event->on_error) {
            (event->on_error)(event->str_url, event->str_data);
        }
        delete event;
        ret++;
    }
    return ret;
}

int HTTPRequest::run() {
    int ret = 0;
    if  (running_) {
        return ret;
    }
    curl_global_init(CURL_GLOBAL_ALL);
    running_ = true;
    sp_thread_attr_t attr;
    sp_thread_t thread;
    sp_thread_attr_init(&attr);
    sp_thread_attr_setdetachstate(&attr, SP_THREAD_CREATE_DETACHED);
    if (sp_thread_create(&thread, &attr, http_request_thread, NULL) != 0) {
        ret = -1;
    }
    sp_thread_attr_destroy(&attr);
    return ret;
}
void HTTPRequest::stop() {
    running_ = false;
    curl_global_cleanup();
}