#define CURL_STATICLIB
#include <curl.h>

#include <iostream>
#include <sstream>
#include <thread>

#pragma warning (disable : 26812)

std::string answers[256];
size_t index = 1;

namespace curl
{
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    std::string GetData(std::string url)
    {
        CURL* curl = curl_easy_init();
        std::string data;

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
        }

        return data;
    }
}

std::string GetWords(std::string data)
{
    const char start_sig[] = "\"answers\"", end_sig[] = "],";

    std::size_t start = data.find(start_sig), end;
    std::stringstream buffer;

    if (start != std::string::npos)
    {
        end = data.find(end_sig, start);
        if (end != std::string::npos) 
        {
            for (int i = start + strlen(start_sig) + strlen(end_sig); i < end; i++)
                buffer << data[i];
        }
    }

    return buffer.str();
}

void ParseWords(std::string words)
{
    std::stringstream buffer;

    char* input, * parse, * ctx = NULL;
    const char* del = ",";

    input = &words[0];
    parse = strtok_s(input, del, &ctx);

    while (parse) {
        buffer << parse;
        buffer >> answers[index];
        buffer.clear();

        parse = strtok_s(NULL, del, &ctx);
        index++;
    }

    for (int i = 0; i < index; i++) {
        for (int j = 0; j < answers[i].length(); j++)
            buffer << answers[i][j];
    }
    answers[0] = buffer.str();
}

std::string GetLetters(std::string data)
{
    std::string letters;

    for (int i = 0; i < data.length(); i++) {
        if (letters.find(data[i]) == std::string::npos)
            letters += data[i];
    }
    return letters;
}

bool isPenagram(std::string word, std::string letters)
{
    if (word.length() >= letters.length()) {
        for (int i = 0; i < letters.length(); i++) 
        {
            if (word.find(letters[i]) == std::string::npos) break;           
            else if (i >= letters.length() - 1) return true;
        }
    }

    return false;
}

int main() {
    std::string words = GetWords(curl::GetData("https://www.nytimes.com/puzzles/spelling-bee"));
    words.erase(remove(words.begin(), words.end(), '"'), words.end());
    ParseWords(words);
    std::string letters = GetLetters(answers[0]);
    int total = 0;

    for (int i = 1; i < index; i++) {
        bool penagram = isPenagram(answers[i], letters);
        int score = 0;

        if (penagram) score += 7;
        if (answers[i].length() > 4) score += answers[i].length();
        else score += 1;
        total += score;

        std::cout << "[";
        if (i < 10) std::cout << "0";
        std::cout << i << "] ";
        std::cout << answers[i];

        for (int j = answers[i].length(); j < 25; j++)
            std::cout << ".", std::this_thread::sleep_for(std::chrono::milliseconds(1));

        std::cout << " (pts. " << score;
        if (penagram) {
            if (answers[i].length() == 7) std::cout << " perfect";
            std::cout << " penagram";
        }
        std::cout << ")" << std::endl;
    }
    std::cout << "\nTotal pts.  = " << total << std::endl;
    std::cout << "Total words = " << index - 1 << std::endl << std::endl;

    system("pause");
    return 0;
}