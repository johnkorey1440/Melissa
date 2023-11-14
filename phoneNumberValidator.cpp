#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>

const std::string MELISSA_API_URL = "https://api.melissa.com/v1/phone/";

// Function to perform HTTP request using libcurl
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

// Function to get the Melissa API key from the user without echoing it to the console
std::string getMelissaApiKeyFromUser() {
    std::string apiKey;
    std::cout << "Enter your Melissa API key: ";
    std::getline(std::cin, apiKey);
    return apiKey;
}

// Function to validate a phone number using Melissa API
bool validatePhoneNumber(const std::string& phoneNumber, const std::string& apiKey, std::string& carrier) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Error initializing libcurl." << std::endl;
        return false;
    }

    std::string url = MELISSA_API_URL + phoneNumber;
    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Add Melissa API key to the request headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Basic " + apiKey).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        std::cerr << "Failed to perform HTTP request: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    // Parse the response to extract carrier information
    // Modify this part based on the actual response format from Melissa API
    // You might need to use a JSON parsing library for a more complex response
    // For simplicity, assume the response is in JSON format and contains a "carrier" field
    size_t pos = response.find("\"carrier\"");
    if (pos != std::string::npos) {
        size_t start = response.find("\"", pos + 9) + 1;
        size_t end = response.find("\"", start);
        carrier = response.substr(start, end - start);
        return true;
    } else {
        std::cerr << "Failed to extract carrier information from the response." << std::endl;
        return false;
    }
}

int main() {
    std::string apiKey = getMelissaApiKeyFromUser();

    // Open Leads.txt for reading
    std::ifstream leadsFile("Leads.txt");
    if (!leadsFile.is_open()) {
        std::cerr << "Error opening Leads.txt." << std::endl;
        return 1;
    }

    // Create results folder if it doesn't exist
    system("mkdir -p results");

    // Process each phone number in Leads.txt
    std::string phoneNumber;
    while (std::getline(leadsFile, phoneNumber)) {
        std::string carrier;

        if (validatePhoneNumber(phoneNumber, apiKey, carrier)) {
            // Write the carrier information to the corresponding results file
            std::ofstream resultFile("results/" + carrier + ".txt", std::ios::app);
            if (resultFile.is_open()) {
                resultFile << phoneNumber << std::endl;
                resultFile.close();
            } else {
                std::cerr << "Error writing to results/" << carrier << ".txt." << std::endl;
            }
        }
    }

    leadsFile.close();

    return 0;
}
