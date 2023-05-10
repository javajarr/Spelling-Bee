#define CURL_STATICLIB
#include <curl.h>
#include <iostream>
#include <sstream>
#include <thread>

#pragma warning (disable : 26812)

std::string answers[256];
std::size_t index = 1;

namespace curl
{
    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    std::string get_req(std::string url)
    {
        CURL* curl = curl_easy_init();
        std::string buffer;

        if (curl)
        {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            if (res == CURLE_COULDNT_CONNECT)
                ExitProcess(EXIT_SUCCESS);
        }
        return buffer;
    }
}

std::string get_words(std::string data)
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

void parse_words(std::string words)
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

std::string get_letters(std::string penagram)
{
    std::string letters;
    for (int i = 0; i < penagram.length(); i++)
    {
        if (letters.find(penagram[i]) == std::string::npos)
            letters += penagram[i];
    }
    return letters;
}

bool is_penagram(std::string word, std::string letters)
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

int main()
{
    std::string word_list = get_words(curl::get_req("https://www.nytimes.com/puzzles/spelling-bee"));
    word_list.erase(remove(word_list.begin(), word_list.end(), '"'), word_list.end());

    parse_words(word_list);

    std::string letters = get_letters(answers[0]);
    int total_score = 0;

    for (int i = 1; i < index; i++)
    {
        bool penagram = is_penagram(answers[i], letters);
        int score = 0;

        if (penagram) score += 7;
        if (answers[i].length() > 4) score += answers[i].length();
        else score += 1;
        total_score += score;

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
    std::cout << "\nTotal pts.  = " << total_score << std::endl;
    std::cout << "Total words = " << index - 1 << std::endl << std::endl;
    std::cout << "Press ENTER to exit...";
    std::cin.get();

    return 0;
}