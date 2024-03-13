#include <iostream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <fstream>
#include <ctime>
#include <iomanip>

using namespace std;
using namespace std::chrono;

// Custom hash function for pair<string, int>
struct pair_hash {
    template <class T1, class T2>
    size_t operator() (const pair<T1, T2>& p) const {
        auto hash1 = hash<T1>{}(p.first);
        auto hash2 = hash<T2>{}(p.second);
        return hash1 ^ hash2;
    }
};

// Class to represent a transaction
class Transaction {
private:
    string m_url;
    int m_responseCode;

public:
    // Constructor
    Transaction(const string& u) : m_url(u) {}

    // Function to set response code
    void setResponseCode(int code) {
        m_responseCode = code;
    }

    // Function to get URL
    string getUrl() const {
        return m_url;
    }

    // Function to get response code
    int getResponseCode() const {
        return m_responseCode;
    }
};

// Class to process the input and maintain transaction and URL data
class Processor {
private:
    unordered_map<string, Transaction> m_transactionMap;
    unordered_map<pair<string, int>, int, pair_hash> m_urlMap;
    steady_clock::time_point m_startTime;

public:
    // Constructor
    Processor() {
        m_startTime = steady_clock::now();
    }

    // Function to process input
    void processInput(istream& input, ofstream& outputFile) {
        char buffer[BUFSIZ];
        string line;
        string currentTraceID;
        string currentURL;
        bool isResponse = false;

        while (input.getline(buffer, BUFSIZ)) {
            line = buffer;
            if (line.length() == 0 || line == "\n") {
                processEndOfMessage(currentTraceID, currentURL, isResponse);
                writeURLStatusCountToFile(outputFile);
            } else {
                processLine(line, currentTraceID, currentURL, isResponse);
            }
        }

        writeURLStatusCountToFile(outputFile, true);
    }

private:
    // Function to reset variables
    static void resetVariables(string& currentTraceID, string& currentURL, bool& isResponse) {
        currentTraceID.clear();
        currentURL.clear();
        isResponse = false;
    }


    // Function to parse response code
    static bool parseResponseCode(const string& url, int& responseCode) {
        char responseCodeStr[10];

        if (sscanf(url.c_str(), "%*s %s", responseCodeStr) == 1) {
            int code = atoi(responseCodeStr);
            if (code != 0 || (code == 0 && responseCodeStr[0] == '0')) {
                responseCode = code;
                return true;
            }
        }

        return false;
    }

    // Function to write URL, status code, and count to file
    void writeURLStatusCountToFile(ofstream& outputFile, bool force = false) {
        auto currentTime = steady_clock::now();
        if (duration_cast<minutes>(currentTime - m_startTime).count() >= 1 || force) {
            outputFile << endl << "Timestamp: " << getCurrentDateTime();
            for (const auto& pair : m_urlMap) {
                outputFile << "URL: " << pair.first.first << ", Response Code: " << pair.first.second << ", Count: " << pair.second << endl;
            }

            for (auto& pair : m_urlMap) {
                pair.second = 0;
            }

            m_startTime = currentTime;
        }
    }

    // Function to process the end of message
    void processEndOfMessage(string& currentTraceID, string& currentURL, bool& isResponse) {
        if (!currentTraceID.empty()) {
            if (isResponse) {
                int responseCode;
                if (!parseResponseCode(currentURL, responseCode)) {
                    resetVariables(currentTraceID, currentURL, isResponse);
                    return;
                }

                auto result = m_transactionMap.find(currentTraceID);
                if (result == m_transactionMap.end()) {
                    resetVariables(currentTraceID, currentURL, isResponse);
                    return;
                }

                currentURL = result->second.getUrl();
                m_urlMap[make_pair(currentURL, responseCode)]++;
                m_transactionMap.erase(currentTraceID);
            } else {
                m_transactionMap.insert({currentTraceID, Transaction(currentURL)});
            }
        }

        resetVariables(currentTraceID, currentURL, isResponse);
    }

    // Function to process a line
    void processLine(const string& line, string& currentTraceID, string& currentURL, bool& isResponse) {
        if (currentURL.empty()) {
            if (line.find("HTTP/1.1 ") == 0) {
                isResponse = true;
                currentURL = line;
            } else {
                char method[10], url[100];
                sscanf(line.c_str(), "%s %s", method, url);
                if (method[0] != '\0') {
                    currentURL = url;
                }
            }
        }

        if (line.find("X-Trace-ID:") == 0) {
            currentTraceID = line.substr(12);
        }
    }

    // Function to get current date and time as string
    static string getCurrentDateTime() {
        auto now = system_clock::to_time_t(system_clock::now());
        return string(ctime(&now));
    }
};

int main(int argc, char *argv[]) {
    if (argc != 3 || string(argv[1]) != "-o") {
        cout << "Usage: " << argv[0] << " -o <output_file>" << endl;
        return 1;
    }

    ofstream outputFile(argv[2], ios::app);
    if (!outputFile.is_open()) {
        cout << "Error: Unable to open file " << argv[2] << " for writing." << endl;
        return 1;
    }

    Processor processor;
    processor.processInput(cin, outputFile);

    outputFile.close();
    return 0;
}

