#include "iostream"
#include "libraries/json/include/nlohmann/json.hpp"


#define CURL_STATICLIB
#include "libraries/curl/include/curl/curl.h"
#include "APICLASSES.h"
#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")

class API
{
private:
    std::string coflnetbase = "https://sky.coflnet.com";
    std::string coflnetMayor = "/api/mayor/";
    std::string coflnetLowestBinPrice = "/api/item/price/";
    std::string coflnetLowestLastActiveBin = "/api/auctions/tag/";
    std::string coflnetGetPriceHistoryofSold = "/api/auctions/tag/";
    std::string coflnetAuctionInfoByUUID = "/api/auction/";
    std::string coflnetItemPriceHistoryTime = "/api/item/price/";
    std::string coflnetHistory = "/history";
    std::string coflnetDay = "/day";
    std::string coflnetWeek = "/week";
    std::string coflnetMonth = "/month";
    std::string Bin= "/bin";
    std::string ActiveBin= "/active/bin";
    std::string sold= "/sold";
    std::string hypixelapikey = "HYPIXEL API KEY"; //put your api key
    std::string hypixelbase = "https://api.hypixel.net/v2";
    std::string hypixelauctionlist = "/skyblock/auctions?key=";
    std::string hypixelitemlist = "/resources/skyblock/items?key=";
public:
	using json = nlohmann::json;



    void PrintJsonResponse(const std::string& response) {

        // Try to parse and pretty-print the JSON response
        try {
            auto jsonResponse = json::parse(response);


            // Iterate through the array and print item names
            for (const auto& item : jsonResponse) {
                std::cout << "Item Name: " << item["itemName"].get<std::string>() << std::endl;
            }
           // std::cout << "Parsed JSON: " << std::endl;
            //std::cout << std::setw(4) << jsonResponse << std::endl;


        }
        catch (const json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
    }

    // Callback function to write data from response
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    // Callback function to write headers
    static size_t HeaderCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    bool CurlIn(CURL*& curl)
    {
        CURLcode res;
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (!curl)
            return 0;
        return 1;
    }

    bool CurlClean(CURL*& curl, struct curl_slist*& headers)
    {
        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 1;
    }

    bool CurlReq(const std::string& url, const std::string& postFields, std::string& response, const std::string& requestType) {
        
        CURL* curl;
        if (!CurlIn(curl)) return 0;

        std::string headerData;
        struct curl_slist* headers = nullptr;
        
        if (curl == nullptr) {
            std::cerr << "Invalid CURL handle" << std::endl;
            return false;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        if(postFields != "")
            curl_easy_setopt(curl, CURLOPT_URL, postFields.c_str());

        if (headers) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);

        if (requestType == "POST") {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        }
        else if (requestType == "GET") {
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        }
        else if (requestType == "DELETE") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        }
        else {
            std::cerr << "Unsupported request type: " << requestType << std::endl;
            CurlClean(curl, headers);
            return false;
        }

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            CurlClean(curl, headers);
            return false;
        }

        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
      //  std::cout << "HTTP Response Code: " << response_code << std::endl;

        CurlClean(curl, headers);
        return true;
    } 

    bool GetMayorInfo(int year, std::string& response)
    {
        std::string url = coflnetbase + coflnetMayor + std::to_string(year);
        

        // Make the request
        if (CurlReq(url, "", response, "GET")) {
            std::cout << "Response Data: " << response << std::endl;
        }

        return 1;
    }

    bool GetLowestPrice(std::string ItemTag, std::string& response)
    {
        std::string url = coflnetbase + coflnetLowestBinPrice + ItemTag + Bin;

        // Make the request
        if (CurlReq(url, "", response, "GET")) {
            std::cout << "Response Data: " << response << std::endl;
        }

       return 1;
    }

    bool GetLastsAuctionLowestPrices(std::string ItemTag, std::string& response, std::vector<AuctionItem>& auctionitems,std::string query = "") {
        // example ?query[Rarity]=LEGENDARY&query[PetLevel]=0

        std::string url = coflnetbase + coflnetLowestLastActiveBin + ItemTag + ActiveBin + query;

        // Make the request
        if (CurlReq(url, "", response, "GET")) {
            if (AuctionItem::ParseFromJson(response, auctionitems)) {
                return true;
            }
            else {
                //std::cerr << "Failed to parse JSON response." << std::endl;
                return false;
            }
          
        }

        
        return 1;
    }

    bool GetAuctions(std::vector<Auction>& auction)
    {
        std::string url = hypixelbase + hypixelauctionlist + hypixelapikey;
        std::string response;
        // Make the request
        if (CurlReq(url, "", response, "GET")) {
           // std::cout << "Response Data: " << response << std::endl;

            if (Auction::ParseFromJson(response, auction)) {
                // Successfully parsed and populated auctionItems
                return true;
            }
            else {
              //  std::cerr << "Failed to parse JSON response." << std::endl;
                return false;
            }
        }

        return 1;
    }

    bool GetAllItems(std::vector<SkyblockItem>& itemarray)
    {
        std::string url = hypixelbase + hypixelitemlist + hypixelapikey;
        std::string response;
        // Make the request
        if (CurlReq(url, "", response, "GET")) {
            // std::cout << "Response Data: " << response << std::endl;

            if (SkyblockItem::ParseFromJson(response, itemarray)) {
                // Successfully parsed and populated auctionItems
                return true;
            }
            else {
                std::cerr << "Failed to parse JSON response." << std::endl;
                return false;
            }
        }

        return 1;
    }

    bool GetHistoryOfSold(std::string ItemTag, std::vector<AuctionItem>& solded)
    {
        std::string url = coflnetbase + coflnetGetPriceHistoryofSold + ItemTag + sold + "?page=0&pageSize=100";
        std::string response;

        // Make the request
        if (CurlReq(url, "", response, "GET")) {
            if (AuctionItem::ParseFromJson(response, solded)) {
                return true;
            }
            else {
                //std::cerr << "Failed to parse JSON response." << std::endl;
                return false;
            }

        }
        return 1;
    }

    bool AuctionInformationByUUID(std::string UUID, AuctionItem& auction)
    {
        std::string url = coflnetbase + coflnetAuctionInfoByUUID + UUID;
        std::string response;

        if (CurlReq(url, "", response, "GET")) {
            try {
                nlohmann::json response_json = nlohmann::json::parse(response);
                AuctionItem::from_json(response_json, auction);
                // std::cout << "Parsed item at index " << i << std::endl;
            }
            catch (const std::exception& e) {
                //std::cerr << "Error parsing item at index " << i << ": " << e.what() << std::endl;
            }
        }
        
        return 1;
    }

    bool getwiki()
    {
        std::string url = "https://wiki.hypixel.net/Catacombs_Mobs";
        std::string response;
        std::string requestType = "GET";  // Specify the request type

        if (CurlReq(url, "", response, requestType)) {
            std::cout << "Response received successfully!" << std::endl;
            std::cout << "Content: " << response << std::endl;
        }
        else {
            std::cerr << "Failed to make request" << std::endl;
        }

        return 0;
    }

    bool GetItemPriceHistory(std::string ItemTag, std::vector<ItemPrice>& itemprices, std::string time,std::string query = "")
    {
        std::string url = coflnetbase + coflnetItemPriceHistoryTime + ItemTag + coflnetHistory + time + query;
        std::string response;

        // Make the request
        if (CurlReq(url, "", response, "GET")) {
            if (ItemPrice::ParseFromJson(response, itemprices)) {
                return true;
            }
            else {
                //std::cerr << "Failed to parse JSON response." << std::endl;
                return false;
            }

        }
        return 1;
    }
};