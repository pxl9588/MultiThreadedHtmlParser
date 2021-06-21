#ifndef _URL_PARSER_
#define _URL_PARSER_

#include <queue>
#include <thread>
#include <string.h>
#include <mutex>
#include <tuple>
#include <map>

using namespace std;

class URLParser
{
	private:
		int _numberOfThreads = 0;
        int _totalURLs = 0;
		int _urlCounter = 0;
        bool _finished = false;
        
        // Using a queue for fast insert/delete at front
		queue<string> _urls;

        // Using mutex so threads can safely pop from the queue
        mutex _urlMutex;

        // Using map to maintain the order that URLs are printed
        map<int,string> _results;

	public:
        URLParser(int threads);
        ~URLParser();

        int getNumberThreads() {return _numberOfThreads;};
        int getTotalURLs() {return _totalURLs;};
        int getURLCount() {return _urlCounter;};
        bool isFinished() {return _finished;};

		void addURL(string url);
        tuple<int,string> getURL();
        void addResult(int position, string answer);
        void printResults();
};

#endif // #ifndef _URL_PARSER_