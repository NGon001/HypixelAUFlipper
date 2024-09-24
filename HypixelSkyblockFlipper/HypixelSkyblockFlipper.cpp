#include "Api.hpp"
#include <chrono>
#include <thread>
#include <locale>
#include <codecvt>
#include <fstream>
#include <regex>
#include <tchar.h>

std::wstring symbolstart = L"⚚";
std::wstring symbolstar = L"✪";

// Convert UTF-8 to UTF-16
std::wstring utf8_to_utf16(const std::string& utf8) {
    std::wstring utf16;
    size_t i = 0;

    while (i < utf8.size()) {
        unsigned char ch = static_cast<unsigned char>(utf8[i]);
        unsigned long uni;
        size_t todo;

        // Determine the number of bytes to follow
        if (ch <= 0x7F) {
            uni = ch;
            todo = 0;
        }
        else if (ch <= 0xBF) {
            // Invalid UTF-8 sequence, return original string
            return std::wstring(utf8.begin(), utf8.end());
        }
        else if (ch <= 0xDF) {
            uni = ch & 0x1F;
            todo = 1;
        }
        else if (ch <= 0xEF) {
            uni = ch & 0x0F;
            todo = 2;
        }
        else if (ch <= 0xF7) {
            uni = ch & 0x07;
            todo = 3;
        }
        else {
            // Invalid UTF-8 sequence, return original string
            return std::wstring(utf8.begin(), utf8.end());
        }

        // Move to the next byte
        i++;

        // Process continuation bytes
        for (size_t j = 0; j < todo; ++j) {
            if (i >= utf8.size()) {
                // Incomplete sequence, return original string
                return std::wstring(utf8.begin(), utf8.end());
            }
            unsigned char next_ch = static_cast<unsigned char>(utf8[i]);
            if (next_ch < 0x80 || next_ch > 0xBF) {
                // Invalid continuation byte, return original string
                return std::wstring(utf8.begin(), utf8.end());
            }
            uni = (uni << 6) | (next_ch & 0x3F);
            i++;
        }

        // Check for valid Unicode code points
        if (uni >= 0xD800 && uni <= 0xDFFF) {
            // Surrogate pair not allowed in UTF-16, return original string
            return std::wstring(utf8.begin(), utf8.end());
        }
        if (uni > 0x10FFFF) {
            // Code point out of range, return original string
            return std::wstring(utf8.begin(), utf8.end());
        }

        // Convert to UTF-16
        if (uni <= 0xFFFF) {
            utf16 += static_cast<wchar_t>(uni);
        }
        else {
            uni -= 0x10000;
            utf16 += static_cast<wchar_t>((uni >> 10) + 0xD800);
            utf16 += static_cast<wchar_t>((uni & 0x3FF) + 0xDC00);
        }
    }

    return utf16;
}
// Function to help debug
void print_utf8_as_utf16(const std::string& utf8) {
    std::wstring utf16 = utf8_to_utf16(utf8);
    if (utf16 == std::wstring(utf8.begin(), utf8.end())) {
        std::cout << "Original string (not valid UTF-8): " << utf8 << std::endl;
    }
    else {
        std::wcout << L"Converted UTF-16 string: " << utf16 << std::endl;
    }
}

// Function to check if a wide string starts with a specific symbol
bool StartsWithSymbol(const std::wstring& str, const std::wstring& symbol) {
    return str.find(symbol) == 0; // Check if symbol is at the start of the string
}

// Function to count occurrences of a wide character symbol in a wide string
int CountSymbolOccurrences(const std::wstring& str, const std::wstring& symbol) {
    int count = 0;
    size_t pos = 0;

    // Search for the symbol in the string and count occurrences
    while ((pos = str.find(symbol, pos)) != std::wstring::npos) {
        ++count;
        pos += symbol.length(); // Move past the current symbol
    }

    return count;
}

// Convert std::string to std::wstring
std::wstring string_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}
// Convert std::wstring to std::string (UTF-16 to UTF-8)
std::string wstring_to_string(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

// Check if UTF-16 string contains a specific Reforge value
bool contains_reforge(const std::wstring& utf16_string, Reforges::Reforge reforge) {
    std::wstring reforge_str = string_to_wstring(Reforges::reforge_to_string(reforge));
    return utf16_string.find(reforge_str) != std::wstring::npos;
}

// Remove unwanted symbols from the start and end of a wide string
std::wstring RemoveSymbols(const std::wstring& str, const std::wstring& symbols) {
    std::wstring result = str;
    size_t start = result.find_first_not_of(symbols);
    if (start == std::wstring::npos) return L""; // All characters are symbols

    size_t end = result.find_last_not_of(symbols);
    result = result.substr(start, end - start + 1);
    return result;
}

// Normalize the name by removing symbols and stars
std::wstring NormalizeItemName(const std::wstring& name) {
    // Define symbols and stars
    std::wstring symbols = L"⚚✪";
    std::wstring normalized = RemoveSymbols(name, symbols);
    return normalized;
}
// Function to remove a reforge from a wide string
std::wstring RemoveReforge(std::wstring& utf16_string, const std::wstring& reforge) {
    size_t pos = utf16_string.find(reforge);
    while (pos != std::wstring::npos) {
        utf16_string.erase(pos, reforge.length());
        pos = utf16_string.find(reforge);
    }
    return utf16_string;
}

std::wstring TrimLeadingTrailingWhitespace(const std::wstring& str) {
    size_t start = str.find_first_not_of(L" \t\n\r\f\v");
    size_t end = str.find_last_not_of(L" \t\n\r\f\v");
    return (start == std::wstring::npos) ? L"" : str.substr(start, end - start + 1);
}

// Main function to process the item name
std::wstring ProcessItemName(const std::wstring& itemName) {
    std::wstring removedsymbol = RemoveSymbols(itemName, symbolstart);
    removedsymbol = NormalizeItemName(removedsymbol);

    const int REFORGE_COUNT = static_cast<int>(Reforges::Reforge::Any);

    for (int i = 0; i < REFORGE_COUNT; i++)
    {
        Reforges::Reforge reforge = static_cast<Reforges::Reforge>(i);
        bool found = contains_reforge(removedsymbol, reforge);
        if (found)
        {
            return TrimLeadingTrailingWhitespace(RemoveReforge(removedsymbol, string_to_wstring(Reforges::reforge_to_string(reforge))));
        }
    }
    return removedsymbol;
}
bool StartsWith(const std::wstring& str, const std::wstring& prefix) {
    return str.rfind(prefix, 0) == 0;
}
// Find the item ID by name in a JSON file
void FindItemIdByName(const std::string& filename, const std::wstring& targetNameW,std::string& id) {
    // Open the file
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    // Read the entire file into a string
    std::string fileContent((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    // Close the file
    file.close();

    try {
        // Parse the JSON data
        nlohmann::json root = nlohmann::json::parse(fileContent);

        // Ensure the JSON object contains the "items" key
        if (!root.contains("items") || !root["items"].is_array()) {
            //std::cerr << "Expected 'items' array in JSON data." << std::endl;
            return;
        }

        // Iterate through each item in the "items" array
        for (const auto& item : root["items"]) {
            // Extract and normalize the item name
            std::wstring nameW = string_to_wstring(item.at("name").get<std::string>());
            std::wstring normalizedItemName = NormalizeItemName(nameW);

            // Compare with the target name
            if (normalizedItemName == targetNameW) {
                 id = item.value("id", "");

                // Skip if ID starts with "STARRED"
                if (StartsWith(string_to_wstring(id), L"STARRED")) {
                    continue;
                }
                if (id == "GOD_POTION_2")
                    id = "GOD_POTION";
              //  std::cout << "ID for " << wstring_to_string(targetNameW) << " is: " << id << std::endl;
                return;
            }
        }

        //std::cout << "Item with name " << wstring_to_string(targetNameW) << " not found." << std::endl;
    }
    catch (const  nlohmann::json::exception& e) {
        //std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }
}

double CalculateTimeDifference(std::time_t current_time, std::time_t session_create_time) {
    return std::abs(std::difftime(current_time, session_create_time));
}

void CopyToClipboard(const std::string& text) {
    // Open the clipboard
    if (!OpenClipboard(nullptr)) {
        return;
    }

    // Empty the clipboard
    EmptyClipboard();

    // Allocate global memory for the text
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    if (hGlobal == nullptr) {
        CloseClipboard();
        return;
    }

    // Lock the memory and copy the text to it
    LPVOID pGlobal = GlobalLock(hGlobal);
    if (pGlobal != nullptr) {
        memcpy(pGlobal, text.c_str(), text.size() + 1);
        GlobalUnlock(hGlobal);

        // Set the clipboard data
        SetClipboardData(CF_TEXT, hGlobal);
    }

    // Close the clipboard
    CloseClipboard();
}

float ProcentageAddOrSub(float number, float procent, const std::string& operation)
{
    static const std::map<std::string, std::function<double(double, double)>> operations = {
   {"+", std::plus<double>()},
   {"-", std::minus<double>()},
    };
    auto it = operations.find(operation);
    if (it != operations.end()) {
        // Apply the operation
        float percentageDecimal = procent / 100.0;
        float increase = number * percentageDecimal;
        return it->second(number, increase);
    }
    else {
        // Handle invalid operation
        std::cerr << "Invalid operation: " << operation << std::endl;
        return 0;
    }
}

// Function to play sound
void playSound(const std::wstring& soundFile) {
    PlaySound(soundFile.c_str(), NULL, SND_FILENAME | SND_ASYNC);
}

std::vector<Gemstone> getGemstonesFromNBT(const nlohmann::json& nbtData) {
    std::vector<Gemstone> gemstones;

    // Check if NBT data has "data" and inside it "gems"
    if (nbtData.contains("data") && nbtData["data"].contains("gems")) {
        const auto& data = nbtData["data"];
        const auto& gems = data["gems"];

        // Extract unlocked_slots
        std::vector<std::string> unlockedSlots;
        if (gems.contains("unlocked_slots")) {
            for (const auto& slot : gems["unlocked_slots"]) {
                unlockedSlots.push_back(slot.get<std::string>());
            }
        }

        // Iterate over all gems (excluding "unlocked_slots")
        for (auto it = gems.begin(); it != gems.end(); ++it) {
            std::string gemKey = it.key();

            // Skip the unlocked_slots entry
            if (gemKey == "unlocked_slots") continue;

            // Determine whether the gem is unlocked
            bool isUnlocked = (std::find(unlockedSlots.begin(), unlockedSlots.end(), gemKey) != unlockedSlots.end());

            // Gem quality (value of the gem) or a gem stone if it ends in "_gem"
            std::string gemQuality;
            if (it.value().is_string()) {
                gemQuality = it.value().get<std::string>();
            }
            else {
                gemQuality = "UNKNOWN";
            }

            // Store the gemstone details
            gemstones.push_back({ gemKey, gemQuality, isUnlocked });
        }
    }

    return gemstones;
}

int GetVolume(std::vector<ItemPrice> pricehistory)
{
    int volume = 0;
    for (auto item : pricehistory)
    {
        volume += item.volume;
    }
    return volume;
}

bool PetDetect(std::string name,int& level) //[Lvl 1] Baby Yeti
{
    std::regex pattern(R"(\[Lvl (\d+)\] .+)");
    std::smatch match;

    if (std::regex_search(name, match, pattern)) {
        level = std::stoi(match[1].str());
        return true;
    }

    return false;
}

std::string transformName(const std::string& input) {
    // Regular expression pattern to capture the level and item name
    std::regex pattern(R"(\[Lvl \d+\] (.+))");
    std::smatch match;

    if (std::regex_search(input, match, pattern)) {
        // Extract the item name (e.g., "Baby Yeti")
        std::string itemName = match[1].str();

        // Convert item name to uppercase and replace spaces with underscores
        std::transform(itemName.begin(), itemName.end(), itemName.begin(), ::toupper);
        std::replace(itemName.begin(), itemName.end(), ' ', '_');

        // Prefix with "PET_" and return
        return "PET_" + itemName;
    }

    return "";  // Return empty if no match is found
}

// Function to simulate keyboard input
void SimulateKeyInput(WORD keyCode, bool keyDown) {
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keyCode; // Virtual-Key code for the key

    if (!keyDown) {
        input.ki.dwFlags = KEYEVENTF_KEYUP; // Key release
    }

    SendInput(1, &input, sizeof(INPUT));
}

// Function to simulate typing a string (quickly, without delays)
void SimulateFastTyping(const std::string& text) {
    for (char c : text) {
        SHORT key = VkKeyScan(c); // Get virtual key code for the character
        SimulateKeyInput(LOBYTE(key), true);  // Key down
        SimulateKeyInput(LOBYTE(key), false); // Key up
    }
}

// Function to retrieve clipboard text as a string
std::string GetClipboardText() {
    if (!OpenClipboard(nullptr)) return "";

    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr) {
        CloseClipboard();
        return "";
    }

    char* pszText = static_cast<char*>(GlobalLock(hData));
    if (pszText == nullptr) {
        GlobalUnlock(hData);
        CloseClipboard();
        return "";
    }

    std::string text(pszText);

    GlobalUnlock(hData);
    CloseClipboard();

    return text;
}

// Function to simulate mouse click at absolute screen coordinates
void SimulateMouseClick(int x, int y) {
    // Set up the INPUT structure
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;

    // Convert screen coordinates to absolute coordinates
    // Windows uses a coordinate system where (0, 0) is top-left and (65535, 65535) is bottom-right of the screen
    input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    input.mi.dx = (x * 65535) / GetSystemMetrics(SM_CXSCREEN); // X coordinate
    input.mi.dy = (y * 65535) / GetSystemMetrics(SM_CYSCREEN); // Y coordinate
    SendInput(1, &input, sizeof(INPUT));

    // Simulate left mouse button down
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));

    // Simulate left mouse button up
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}



void MainAlgo() {
    API api;
    std::vector<Auction> auctions;
    std::vector<AuctionItem> itemprices;
    std::vector<AuctionItem> solded;
    std::vector<AuctionItem> soldedsorted;
    std::vector<Gemstone> gemstemp;
    std::vector<Gemstone> gemsauction;
    std::vector<ItemPrice> pricehistory;
    std::string response;
    std::time_t timeofreq = 0;
    float balance = 30000000;
    float profitvalue = 300000;
    int petlevel = 0;
    bool ispet = false;

    while (true) {
        auctions.clear();
        api.GetAuctions(auctions);

        if (!auctions.empty()) {
            for (const auto& auction : auctions) {


                AuctionItem auctionitembyit;
                api.AuctionInformationByUUID(auction.uuid, auctionitembyit);
                auto now = std::chrono::system_clock::now();

                if (!timeofreq) {
                    timeofreq = std::chrono::system_clock::to_time_t(now);
                }

                std::time_t currenttime_t = std::chrono::system_clock::to_time_t(now);
                auto differencetime = CalculateTimeDifference(currenttime_t, timeofreq);

                if (differencetime >= 70) {
                    timeofreq = 0;
                    break;
                }

                std::string itemfullname = auction.item_name;
                std::wstring itemfullnameutf16 = utf8_to_utf16(itemfullname);
                std::wstring itemfullnameFixed = ProcessItemName(itemfullnameutf16);

                std::string itemid;
                itemid.clear();
                FindItemIdByName("AllItems.txt", itemfullnameFixed, itemid);

                std::string queryParams;
                int auctionitemstar = 0;
                petlevel = 0;
                ispet = PetDetect(auction.item_name, petlevel);

                if (ispet && itemid.empty()) {
                    itemid = transformName(auction.item_name);
                    if (itemid.empty()) continue;
                    queryParams = "?query[Rarity]=" + auction.tier + "&query[Stars]=" + std::to_string(auctionitemstar) + "&query[PetLevel]=" + std::to_string(petlevel);
                }
                else {
                    if (itemid.empty()) continue;
                    auctionitemstar = CountSymbolOccurrences(itemfullnameutf16, symbolstar);
                    queryParams = "?query[Rarity]=" + auction.tier + "&query[Stars]=" + std::to_string(auctionitemstar);
                }

                gemstemp.clear();
                gemsauction.clear();
                itemprices.clear();
                response.clear();
                pricehistory.clear();

                api.GetLastsAuctionLowestPrices(itemid, response, itemprices, queryParams);
                api.GetItemPriceHistory(itemid, pricehistory, "/week", queryParams);

                int volumeday = GetVolume(pricehistory);
                int volumedayaverage = ItemPrice::CalculateFilteredAverageStartingBid(pricehistory);

                if (auctionitemstar == 0 && !ispet) {
                    if (volumeday < 500) continue;
                    if (volumedayaverage < 5) continue;
                }

                if (itemprices.empty()) continue;

                float pricedifference1 = 0;
                float pricedifference2 = 0;
                float auctionfee1 = 0;
                float auctionfee2 = 0;
                int count = 0;

                for (const auto& item : itemprices) {
                    if (item.bin && count == 0) {
                        auctionfee1 = ProcentageAddOrSub(item.startingBid, 3, "-");
                        pricedifference1 = auctionfee1 - auction.starting_bid;
                        count++;
                    }
                    else if (item.bin && count == 1) {
                        auctionfee2 = ProcentageAddOrSub(item.startingBid, 3, "-");
                        pricedifference2 = auctionfee2 - auction.starting_bid;
                        count++;
                    }
                    else if (count > 1) {
                        break;
                    }
                }

                solded.clear();
                soldedsorted.clear();
                api.GetHistoryOfSold(itemid, solded);

                for (const auto& item : solded) {
                    std::wstring itemsoldfullname = utf8_to_utf16(item.itemName);
                    int solditemstar = CountSymbolOccurrences(itemsoldfullname, symbolstar);
                    bool auctionhaveupgrade = StartsWith(itemfullnameutf16, symbolstar);
                    bool soldhaveupgrade = StartsWith(itemsoldfullname, symbolstar);

                    if (solditemstar == auctionitemstar && auctionhaveupgrade == soldhaveupgrade && item.tier == auction.tier) {
                        if (ispet) {
                            int itempetlvl;
                            PetDetect(item.itemName, itempetlvl);
                            if (itempetlvl != petlevel) continue;
                            if (!item.bin) continue;
                        }

                        if (!item.bin) continue;

                        if (!item.nbtData.empty() || !auctionitembyit.nbtData.empty())
                        {
                            gemstemp = getGemstonesFromNBT(item.nbtData);
                            gemsauction = getGemstonesFromNBT(auctionitembyit.nbtData);

                            if (!gemsauction.empty() && !gemstemp.empty() && gemsauction.size() == gemstemp.size()) {
                                bool valid = true;
                                for (size_t i = 0; i < gemsauction.size(); i++) {
                                    if (gemsauction[i].quality != gemstemp[i].quality) {
                                        valid = false;
                                        break;
                                    }
                                }
                                if (!valid) continue;
                            }
                        }
                        soldedsorted.push_back(item);
                    }
                }

                double average = AuctionItem::CalculateFilteredAverageStartingBid(soldedsorted, 0.4);
                double averagedif = average - auction.starting_bid;

                if (averagedif < profitvalue || (pricedifference1 <= profitvalue || pricedifference2 <= profitvalue) || !auction.bids.empty()) continue;

                if (auction.tier != "COMMON" && auction.tier != "UNCOMMON" && itemid != "GOD_POTION") {
                    std::cout << "----------------------------------------" << std::endl;
                    std::cout << "average price is: " << std::to_string(average) << std::endl;
                    std::cout << "Lowest bin price:\n"
                        << "tag: " << itemprices[0].tag << "\n"
                        << "startingBid: " << std::to_string(itemprices[0].startingBid) << "\n"
                        << "tier: " << itemprices[0].tier << "\n"
                        << "uid: /viewauction " << itemprices[0].uuid << "\n"
                        << "pricedifference1: " << std::to_string(pricedifference1) << "\n"
                        << "pricedifference2: " << std::to_string(pricedifference2) << "\n";

                    std::cout << "Auction info:\n"
                        << "tag: " << auction.item_name << "\n"
                        << "stars: " << auctionitemstar << "\n"
                        << "volumeavg: " << volumedayaverage << "\n"
                        << "startingBid: " << auction.starting_bid << "\n";

                    for (const auto& gem : gemsauction) {
                        std::cout << "gemstones, quality: " << gem.quality << "\n";
                    }

                    std::cout << "tier: " << auction.tier << "\n"
                        << "uid: /viewauction " << auction.uuid << "\n";

                    std::cout << "----------------------------------------" << std::endl;
                    CopyToClipboard("/viewauction " + auction.uuid);
                    playSound(L"C:\\Users\\1\\source\\repos\\HypixelSkyblockFlipper\\HypixelSkyblockFlipper\\news-ting-6832.wav");


                    //uncomment if you want full automatic trade (but it is banneble by hypixel rules);

                  /*  // Step 1: Simulate pressing the "/" key
                    SimulateKeyInput(VK_OEM_2, true);  // "/" key down
                    SimulateKeyInput(VK_OEM_2, false); // "/" key up

                    Sleep(50); // Small delay

                    // Step 2: Simulate pressing the Backspace key
                    SimulateKeyInput(VK_BACK, true);   // Backspace down
                    SimulateKeyInput(VK_BACK, false);  // Backspace up

                    Sleep(50); // Small delay

                    // Step 3: Retrieve clipboard text and simulate typing each character quickly
                    std::string clipboardText = GetClipboardText();
                    if (!clipboardText.empty()) {
                        SimulateFastTyping(clipboardText);
                    }

                    Sleep(300); // Small delay

                    // Step 4: Simulate pressing the Enter key
                    SimulateKeyInput(VK_RETURN, true);  // Enter key down
                    SimulateKeyInput(VK_RETURN, false); // Enter key up

                    Sleep(1000); // Small delay

                    // Simulate mouse click at (959, 440)
                    SimulateMouseClick(959, 440);
                    Sleep(500); // Wait for 2 seconds

                    // Simulate mouse click at (851, 422)
                    SimulateMouseClick(851, 422);*/
                   
                }
            }
        }
    }
}

int main()
{
	 MainAlgo(); 
    //api.getwiki();
}