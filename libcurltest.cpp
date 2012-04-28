// libcurltest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "curl/curl.h"

#include "HTTPRequest.h"

#include <iostream>
using namespace std;

#pragma comment(lib, "libcurld_imp.lib")

void complete(string &strurl, string &data) {
    //cout<<str<<endl;
    cout<<strurl<<" complete\n";
}
void error(std::string &strurl, string &data) {
    cout<<strurl<<" error "<<data<<endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
//  HTTPRequest::instance()->set_timeout(500);
    HTTPRequest::instance()->run();
    HTTPRequest::instance()->send_request(EVENT_REQUET_DATA, "http://www.baidu.com", NULL,  complete, error);
    HTTPRequest::instance()->send_request(EVENT_DOWNLOAD_FILE, "http://www.baidu.com/", "baidu.html",  complete, error);
    HTTPRequest::instance()->send_request(EVENT_REQUET_DATA, "http://www.example.com/", NULL,  complete, error);
    while (1) {
        if (HTTPRequest::instance()->dispatch() ) {
            //TODO
        }
        else {
            sp_sleep (1000);
            cout<<"main loop\n";
        }
    }
	return 0;
}

