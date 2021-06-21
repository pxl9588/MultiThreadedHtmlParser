#include <iomanip>
#include <iostream>
#include <thread>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <queue>
#include <unistd.h>

#include "URLParser.h"

#include <curl/curl.h>
#include <curl/easy.h>
#include <tidy/tidy.h>
#include <tidy/tidybuffio.h>

#define PBW 20

#define ERR_PARAMETERS 1
#define ERR_UNKNOWN_FILE 2

using namespace std;

void readNode(TidyDoc doc, TidyNode tnod, int indent, int& nodes, int& leafNodes, int& divNodes)
{
	// Add one if this is a valid node
	if(tnod)
	{
		nodes++;
	}
	else
	{
		return;
	}

	TidyNode child;
	int children = 0;
	for(child = tidyGetChild(tnod); child; child = tidyGetNext(child) ) {
		ctmbstr name = tidyNodeGetName(child);
		if(name) {
			if(((string)name).compare("div") == 0)
			{
				divNodes++;
			}
			children++;
		}
		readNode(doc, child, indent + 4, nodes, leafNodes, divNodes);
	}

	// If the node has 0 children it is a leaf node
	if(children == 0)
	{
		leafNodes++;
	}
}


//cURL callback function that appends the data to the Tidy Buffer
uint writeFunction(char *data, size_t size, size_t nmemb, TidyBuffer *userp)
{
	uint realSize = size * nmemb;
	tidyBufAppend(userp, data, realSize);
    return realSize;
}

string downloadAndParseFile(const char* url)
{
    CURL *curl;
    int err;
	string result = "";

    curl = curl_easy_init();
    if(curl) {
		TidyDoc tdoc;
    	TidyBuffer docbuf = {0};

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
 		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &docbuf);
        curl_easy_setopt(curl, CURLOPT_URL, url);

		tdoc = tidyCreate();
		tidyOptSetBool(tdoc, TidyForceOutput, yes);
		tidyOptSetInt(tdoc, TidyWrapLen, 4096);
		tidyOptSetBool(tdoc, TidyShowWarnings, no);
		tidyOptSetBool(tdoc, TidyShowErrors, no);
		tidyOptSetBool(tdoc, TidyHideComments, yes);
		tidyBufInit(&docbuf);

        err = curl_easy_perform(curl);

		int nodes = 0;
		int leafNodes = 0;
		int divNodes = 0;

		if(!err) {
			err = tidyParseBuffer(tdoc, &docbuf);
			if(err >= 0)
			{
				//err = tidyCleanAndRepair(tdoc); /* fix any problems */
				if(err >= 0)
				{
					//err = tidyRunDiagnostics(tdoc); /* load tidy error buffer */
					if(err >= 0)
					{
						readNode(tdoc, tidyGetRoot(tdoc), 0, nodes, leafNodes, divNodes);
						stringstream stream;
						stream << "\t" << nodes << "\t" << leafNodes << "\t" << divNodes;
						result = stream.str();
					}
				}
			}
		}

        curl_easy_cleanup(curl);
		tidyBufFree(&docbuf);
    	tidyRelease(tdoc);
    }

    return result;
}

// Parsing thread function
// Threads will attempt to grab URLs to parse until the queue is empty
void processUrls(URLParser &parser, int threadNumber)
{
	do
	{
		tuple<int,string> tup = parser.getURL();
		int count = get<0>(tup);
		string url = get<1>(tup);
		// Call parser
		if(url.compare("") != 0)
		{
			string res = url + downloadAndParseFile(url.c_str());
			parser.addResult(count, res);
		}
	}
	while(!parser.isFinished());

	return;
}

// Progress bar thread function
// Checks how many URLs have been completed
void progressBar(URLParser& parser)
{
	float totalUrls = (float)parser.getTotalURLs();
	do
	{
		float progressDecimal = (float)parser.getURLCount() / totalUrls;
		int progressPercent = progressDecimal * 100;
		cout <<"\r["<< setfill(' ') << setw(3) << progressPercent<<"%] [";

		for(int i = 0; i < PBW; i++)
		{
			if((i*(100/PBW)) <= progressPercent)
			{
				cout << "=";
			}
			else
			{
				cout << " ";
			}
		}
		cout << "]";
	}
	while(!parser.isFinished());

	// Parser can finish before the 100% is printed, so end with it

	cout << "\r[100%] [";
	for(int i = 0; i < PBW; i++)
	{
		cout << "=";
	}
	cout <<"]\n";
	return;
}

int main(int argc, char** argv)
{
	fstream newfile;

	if (argc != 3)
	{
		cout << "Expected 2 parameters, received " << argc << ". Please provide a file path and number of threads to use: '/path/to/urls.txt 5'\n";
		return ERR_PARAMETERS;
	}

	string file = argv[1];
	char* p;
	int numberOfThreads = strtol(argv[2], &p, 10);
	if(*p)
	{
		cout << "Second parameter should be a number.\n";
		return ERR_PARAMETERS;
	}


	URLParser parser(numberOfThreads);
	vector<thread> parserThreads;

	newfile.open(file.c_str(), ios::in);

	if (newfile.is_open())
	{
		string tp;
		while (getline(newfile, tp))
		{
			// Someone else might need this file, store all the urls and close it
            parser.addURL(tp);
		}

		newfile.close();
	}
	else
	{
		cout << "'" << file <<"' does not exist." <<  "\n";
		return ERR_UNKNOWN_FILE;
	}

	thread progressThread(progressBar, ref(parser));

	for(int j = 0; j < numberOfThreads; j++)
	{
		parserThreads.push_back(thread(processUrls, ref(parser), j));
	}

	// Wait for all the threads to finish
	for(int j = 0; j < numberOfThreads; j++)
	{
		parserThreads[j].join();
	}

	// Progress thread will be done once all of the parser threads are done
	progressThread.join();

	// Prints the results of the parser
	parser.printResults();

	return 0;
}


