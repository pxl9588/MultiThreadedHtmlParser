#include "URLParser.h"
#include <queue>
#include <thread>
#include <string.h>
#include <tuple>
#include <map>
#include <iostream>

using namespace std;

URLParser::URLParser(int threads)
{
    _numberOfThreads = threads;
}

URLParser::~URLParser()
{
}

void URLParser::addURL(string url)
{
    _urls.push(url);
    _totalURLs++;
}

// Attempts to grab the lock and remove a URL from the queue
tuple<int,string> URLParser::getURL()
{
    tuple<int,string> tup;
    if(_finished)
        return tuple<int,string>(-1, "");

    _urlMutex.lock();

    if(_urls.size() != 0)
    {
        string url = _urls.front();  
        _urls.pop();
        tup = tuple<int,string>(_urlCounter++, url);
    }

    if(_urls.size() == 0)
    {
        _finished = true;
    }

    _urlMutex.unlock();

    return tup;
}

void URLParser::addResult(int position, string answer)
{
    _results.insert({position, answer});
}

void URLParser::printResults()
{
    for (map<int, string>::iterator it = _results.begin(); it != _results.end(); it++)
    {
        std::cout << it->second << '\n';
    }
}