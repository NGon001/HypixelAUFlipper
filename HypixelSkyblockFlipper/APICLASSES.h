#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <iostream>

class ItemPrice {
public:
    // Default constructor
    ItemPrice() = default;

    // Constructor from JSON
    explicit ItemPrice(const nlohmann::json& json) {
        from_json(json, *this);
    }

    // Convert to JSON
    nlohmann::json to_json() const {
        nlohmann::json json;
        json["min"] = min;
        json["max"] = max;
        json["avg"] = avg;
        json["volume"] = volume;
        json["time"] = time;
        return json;
    }

    // JSON deserialization
    friend void from_json(const nlohmann::json& json, ItemPrice& itemPrice) {
        itemPrice.min = json.at("min").get<float>();
        itemPrice.max = json.at("max").get<float>();
        itemPrice.avg = json.at("avg").get<float>();
        itemPrice.volume = json.at("volume").get<float>();
        itemPrice.time = json.at("time").get<std::string>();
    }

    // Static method to parse a JSON response into a vector of ItemPrice
    static bool ParseFromJson(const std::string& jsonResponse, std::vector<ItemPrice>& itemPrices) {
        try {
            // Parse the JSON response
            nlohmann::json jsonResponseParsed = nlohmann::json::parse(jsonResponse);

            // Ensure the JSON response is an array
            if (jsonResponseParsed.is_array()) {
                // Clear the itemPrices vector
                itemPrices.clear();

                // Iterate through the JSON array and populate ItemPrice objects
                for (const auto& jsonItem : jsonResponseParsed) {
                    ItemPrice itemPrice = jsonItem.get<ItemPrice>();
                    itemPrices.push_back(itemPrice);
                }
                // For debugging, print the number of items processed
                //std::cout << "Number of ItemPrices: " << itemPrices.size() << std::endl;
            }
            else {
                //std::cerr << "Unexpected JSON format: not an array." << std::endl;
                return false;
            }
        }
        catch (const std::exception& e) {
          //  std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            return false;
        }

        return true;
    }
    
    static double CalculateFilteredAverageStartingBid(std::vector<ItemPrice>& items) {
        if (items.empty()) return 0;
        int volume = 0;
        for (auto item : items)
        {
            volume += item.volume;
        }
        
        return volume / items.size();
    }

    // Member variables
    float min;
    float max;
    float avg;
    float volume;
    std::string time;
};

class AuctionItem {
public:
    // Default constructor
    AuctionItem() = default;

    // Constructor from JSON
    explicit AuctionItem(const nlohmann::json& json) {
        from_json(json, *this);
    }

    // Convert to JSON
    nlohmann::json to_json() const {
        nlohmann::json json;
        json["uuid"] = uuid;
        json["count"] = count;
        json["startingBid"] = startingBid;
        json["tag"] = tag;
        json["itemName"] = itemName;
        json["start"] = start;
        json["end"] = end;
        json["auctioneerId"] = auctioneerId;
        json["profileId"] = profileId;

        if (coop) {
            json["coop"] = *coop;
        }

        if (!coopMembers.empty()) {
            json["coopMembers"] = coopMembers;
        }

        if (!bids.empty()) {
            json["bids"] = bids;
        }

        json["highestBidAmount"] = highestBidAmount;
        json["anvilUses"] = anvilUses;
        json["enchantments"] = enchantments;
        json["nbtData"] = nbtData;
        json["itemCreatedAt"] = itemCreatedAt;
        json["reforge"] = reforge;
        json["category"] = category;
        json["tier"] = tier;
        json["bin"] = bin;
        json["flatNbt"] = flatNbt;
        return json;
    }

    // JSON deserialization
    static void from_json(const nlohmann::json& json, AuctionItem& item) {
        item.uuid = json.at("uuid").get<std::string>();
        item.count = json.at("count").get<int>();
        item.startingBid = json.at("startingBid").get<int>();
        item.tag = json.at("tag").get<std::string>();
        item.itemName = json.at("itemName").get<std::string>();
        item.start = json.at("start").get<std::string>();
        item.end = json.at("end").get<std::string>();
        item.auctioneerId = json.at("auctioneerId").get<std::string>();
        item.profileId = json.at("profileId").get<std::string>();

        if (json.contains("coop") && !json["coop"].is_null()) {
            item.coop = std::make_shared<std::string>(json.at("coop").get<std::string>());
        }
        else {
            item.coop = nullptr;
        }

        if (json.contains("coopMembers") && !json["coopMembers"].is_null()) {
            item.coopMembers = json.at("coopMembers").get<std::vector<std::string>>();
        }
        else {
            item.coopMembers.clear();
        }

        if (json.contains("bids") && !json["bids"].is_null()) {
            item.bids = json.at("bids").get<std::vector<int>>();
        }
        else {
            item.bids.clear();
        }

        item.highestBidAmount = json.at("highestBidAmount").get<int>();
        item.anvilUses = json.at("anvilUses").get<int>();
        item.enchantments = json.at("enchantments").get<std::vector<nlohmann::json>>();
        item.nbtData = json.at("nbtData");
        item.itemCreatedAt = json.at("itemCreatedAt").get<std::string>();
        item.reforge = json.at("reforge").get<std::string>();
        item.category = json.at("category").get<std::string>();
        item.tier = json.at("tier").get<std::string>();
        item.bin = json.at("bin").get<bool>();
        item.flatNbt = json.at("flatNbt");
    }

    static bool ParseFromJson(const std::string& jsonResponse, std::vector<AuctionItem>& auctionItems) {
        try {
            // Parse the JSON response
            nlohmann::json jsonResponseParsed = nlohmann::json::parse(jsonResponse);

            // Ensure the JSON response is an array
            if (jsonResponseParsed.is_array()) {
                // Clear the auctionItems vector
                auctionItems.clear();

                int size = jsonResponseParsed.size();
                // Iterate through the JSON array and populate AuctionItem objects
                for (size_t i = 0; i < jsonResponseParsed.size(); ++i) {
                    const auto& jsonItem = jsonResponseParsed[i];
                    AuctionItem item;
                    try {
                        AuctionItem::from_json(jsonItem, item);
                        auctionItems.push_back(item);
                       // std::cout << "Parsed item at index " << i << std::endl;
                    }
                    catch (const std::exception& e) {
                        //std::cerr << "Error parsing item at index " << i << ": " << e.what() << std::endl;
                    }
                }
            }
            else {
               // std::cerr << "Unexpected JSON format: not an array." << std::endl;
                return false;
            }
        }
        catch (const std::exception& e) {
           // std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            return false;
        }

        return true;
    }

    static double CalculateFilteredAverageStartingBid(std::vector<AuctionItem>& items, double trimPercentage) {
        if (items.empty()) return 0.0;

        // Extract starting bids
        std::vector<double> bids;
        for (const auto& item : items) {
            bids.push_back(item.startingBid);
        }

        // Sort the bids
        std::sort(bids.begin(), bids.end());

        // Determine the number of elements to trim
        int trimCount = static_cast<int>(trimPercentage * bids.size());

        // Trim the outliers
        auto begin = bids.begin() + trimCount;
        auto end = bids.end() - trimCount;

        // Calculate the average of the trimmed range
        double sum = 0.0;
        for (auto it = begin; it != end; ++it) {
            sum += *it;
        }

        return sum / std::distance(begin, end);
    }

    // Member variables
    std::string uuid;
    int count;
    int startingBid;
    std::string tag;
    std::string itemName;
    std::string start;
    std::string end;
    std::string auctioneerId;
    std::string profileId;
    std::shared_ptr<std::string> coop;
    std::vector<std::string> coopMembers;
    std::vector<int> bids;
    int highestBidAmount;
    int anvilUses;
    std::vector<nlohmann::json> enchantments;
    nlohmann::json nbtData;
    std::string itemCreatedAt;
    std::string reforge;
    std::string category;
    std::string tier;
    bool bin;
    nlohmann::json flatNbt;
};

class SkyblockItem {
public:
    // Default constructor
    SkyblockItem() = default;

    // Constructor from JSON
    explicit SkyblockItem(const  nlohmann::json& json) {
        from_json(json, *this);
    }

    // Convert to JSON
    nlohmann::json to_json() const {
        nlohmann::json j;
        j["material"] = material;
        j["durability"] = durability;
        j["skin"] = skin;
        j["name"] = name;
        j["category"] = category;
        j["tier"] = tier;
        j["npc_sell_price"] = npc_sell_price;
        j["id"] = id;
        return j;
    }

    // JSON deserialization
    friend void from_json(const  nlohmann::json& j, SkyblockItem& item) {
        item.material = j.at("material").get<std::string>();
        item.durability = j.at("durability").get<int>();
        item.skin = j.at("skin").get<std::string>();
        item.name = j.at("name").get<std::string>();
        item.category = j.at("category").get<std::string>();
        item.tier = j.at("tier").get<std::string>();
        item.npc_sell_price = j.at("npc_sell_price").get<int>();
        item.id = j.at("id").get<std::string>();
    }

    // Static method to parse a JSON response
    static bool ParseFromJson(const std::string& jsonResponse, std::vector<SkyblockItem>& items) {
        try {
            // Parse the JSON response
            nlohmann::json jsonResponseParsed = nlohmann::json::parse(jsonResponse);

            // Ensure the JSON response has the "items" field and is an array
            if (jsonResponseParsed.contains("items") && jsonResponseParsed["items"].is_array()) {
                // Clear the items vector
                items.clear();

                // Iterate through the JSON array and populate SkyblockItem objects
                for (const auto& jsonItem : jsonResponseParsed["items"]) {
                    SkyblockItem item = jsonItem.get<SkyblockItem>();
                    items.push_back(item);
                }
                // For debugging, print the number of items processed
                std::cout << "Number of Items: " << items.size() << std::endl;
            }
            else {
                std::cerr << "Unexpected JSON format: no 'items' field or not an array." << std::endl;
                return false;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            return false;
        }

        return true;
    }

    // Member variables
    std::string material;
    int durability;
    std::string skin;
    std::string name;
    std::string category;
    std::string tier;
    int npc_sell_price;
    std::string id;
};

class Auction {
public:
    // Default constructor
    Auction() = default;

    // Constructor from JSON
    explicit Auction(const nlohmann::json& json) {
        from_json(json, *this);
    }

    // Convert to JSON
    nlohmann::json to_json() const {
        nlohmann::json json;
        json["uuid"] = uuid;
        json["auctioneer"] = auctioneer;
        json["profile_id"] = profile_id;
        json["coop"] = coop;
        json["start"] = start;
        json["end"] = end;
        json["item_name"] = item_name;
        json["item_lore"] = item_lore;
        json["extra"] = extra;
        json["category"] = category;
        json["tier"] = tier;
        json["starting_bid"] = starting_bid;
        json["item_bytes"] = item_bytes;
        json["claimed"] = claimed;
        json["claimed_bidders"] = claimed_bidders;
        json["highest_bid_amount"] = highest_bid_amount;
        json["bids"] = bids;
        json["bin"] = bin;
        return json;
    }

    // JSON deserialization
    friend void from_json(const nlohmann::json& json, Auction& auction) {
        auction.uuid = json.at("uuid").get<std::string>();
        auction.auctioneer = json.at("auctioneer").get<std::string>();
        auction.profile_id = json.at("profile_id").get<std::string>();
        auction.coop = json.at("coop").get<std::vector<std::string>>();
        auction.start = json.at("start").get<long long>();
        auction.end = json.at("end").get<long long>();
        auction.item_name = json.at("item_name").get<std::string>();
        auction.item_lore = json.at("item_lore").get<std::string>();
        auction.extra = json.at("extra").get<std::string>();
        auction.category = json.at("category").get<std::string>();
        auction.tier = json.at("tier").get<std::string>();
        auction.starting_bid = json.at("starting_bid").get<int>();
        auction.item_bytes = json.at("item_bytes");
        auction.claimed = json.at("claimed").get<bool>();
        auction.claimed_bidders = json.at("claimed_bidders").get<std::vector<std::string>>();
        auction.highest_bid_amount = json.at("highest_bid_amount").get<int>();
        auction.bids = json.at("bids").get<std::vector<nlohmann::json>>();
        auction.bin = json.at("bin").get<bool>();
    }

    // Static method to parse a JSON response
    static bool ParseFromJson(const std::string& jsonResponse, std::vector<Auction>& auctions) {
        try {
            // Parse the JSON response
            nlohmann::json jsonResponseParsed = nlohmann::json::parse(jsonResponse);

            // Ensure the JSON response has the "auctions" field and is an array
            if (jsonResponseParsed.contains("auctions") && jsonResponseParsed["auctions"].is_array()) {
                // Clear the auctions vector
                auctions.clear();

                // Iterate through the JSON array and populate Auction objects
                for (const auto& jsonItem : jsonResponseParsed["auctions"]) {
                    Auction auction = jsonItem.get<Auction>();
                    if (!auction.bin) continue;
                    auctions.push_back(auction);
                }
                // For debugging, print the number of items processed
                std::cout << "Number of Auctions: " << auctions.size() << std::endl;
            }
            else {
               // std::cerr << "Unexpected JSON format: no 'auctions' field or not an array." << std::endl;
                return false;
            }
        }
        catch (const std::exception& e) {
           // std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            return false;
        }

        return true;
    }

    // Member variables
    std::string uuid;
    std::string auctioneer;
    std::string profile_id;
    std::vector<std::string> coop;
    long long start;
    long long end;
    std::string item_name;
    std::string item_lore;
    std::string extra;
    std::string category;
    std::string tier;
    int starting_bid;
    nlohmann::json item_bytes;
    bool claimed;
    std::vector<std::string> claimed_bidders;
    int highest_bid_amount;
    std::vector<nlohmann::json> bids;
    bool bin;
};

class Reforges {
public:
    // Enum for Reforge options
    enum class Reforge {
        None,
        Ancient,
        Fierce,
        Wise,
        Necrotic,
        Heroic,
        Pure,
        Fabled,
        Titanic,
        Mythic,
        Spicy,
        Sharp,
        Giant,
        Withered,
        Jaded,
        Light,
        Blessed,
        Smart,
        Heavy,
        Clean,
        Bustling,
        Legendary,
        Loving,
        Spiritual,
        Auspicious,
        Rapid,
        Epic,
        Strengthened,
        Fast,
        Rooted,
        Fair,
        Glistening,
        OddSword,
        Unreal,
        Gentle,
        Waxed,
        Blooming,
        Renowned,
        Hasty,
        Precise,
        Strange,
        Deadly,
        Excellent,
        Reinforced,
        Submerged,
        Pitchin,
        Heated,
        Grand,
        Suspicious,
        Fine,
        Neat,
        RichBow,
        Salty,
        Awkward,
        Mossy,
        Robust,
        Dirty,
        Shaded,
        DoubleBit,
        Strong,
        Lumberjack,
        Godly,
        Spiked,
        Fleet,
        Refined,
        Bountiful,
        Forceful,
        Menacing,
        Perfect,
        Unpleasant,
        Brilliant,
        Fortunate,
        Lucky,
        Lush,
        Candied,
        Toil,
        Fruitful,
        Superior,
        Hurtful,
        Prospector,
        Festive,
        Treacherous,
        Itchy,
        Unyielding,
        Sturdy,
        Blended,
        Moil,
        AoteStone,
        GreenThumb,
        Stiff,
        Warped,
        Buzzing,
        WarpedOnAote,
        Snowy,
        Peasant,
        Gilded,
        Honored,
        Magnetic,
        Earthy,
        Bloody,
        Zealous,
        Great,
        Bizarre,
        Fortified,
        Headstrong,
        Unknown,
        Mithraic,
        Demonic,
        Zooming,
        Silky,
        Colossal,
        Rugged,
        Shiny,
        Ridiculous,
        Chomp,
        Ambered,
        Soft,
        Cubic,
        Empowered,
        Stellar,
        Vivid,
        Stained,
        Keen,
        Astute,
        Beady,
        BloodSoaked,
        GreaterSpook,
        Hefty,
        Pleasant,
        Simple,
        JerryStone,
        Bulky,
        Ominous,
        Pretty,
        RichSword,
        Fanged,
        Coldfusion,
        Sweet,
        OddBow,
        Any
    };


    // Helper function to convert enum to string
    static std::string reforge_to_string(Reforge reforge) {
        static const std::unordered_map<Reforge, std::string> reforge_to_string_map = {
            {Reforge::None, "None"},
            {Reforge::Ancient, "ancient"},
            {Reforge::Fierce, "Fierce"},
            {Reforge::Wise, "Wise"},
            {Reforge::Necrotic, "Necrotic"},
            {Reforge::Heroic, "Heroic"},
            {Reforge::Pure, "Pure"},
            {Reforge::Fabled, "Fabled"},
            {Reforge::Titanic, "Titanic"},
            {Reforge::Mythic, "Mythic"},
            {Reforge::Spicy, "Spicy"},
            {Reforge::Sharp, "Sharp"},
            {Reforge::Giant, "Giant"},
            {Reforge::Withered, "Withered"},
            {Reforge::Jaded, "Jaded"},
            {Reforge::Light, "Light"},
            {Reforge::Blessed, "Blessed"},
            {Reforge::Smart, "Smart"},
            {Reforge::Heavy, "Heavy"},
            {Reforge::Clean, "Clean"},
            {Reforge::Bustling, "bustling"},
            {Reforge::Legendary, "Legendary"},
            {Reforge::Loving, "Loving"},
            {Reforge::Spiritual, "Spiritual"},
            {Reforge::Auspicious, "Auspicious"},
            {Reforge::Rapid, "Rapid"},
            {Reforge::Epic, "Epic"},
            {Reforge::Strengthened, "strengthened"},
            {Reforge::Fast, "Fast"},
            {Reforge::Rooted, "rooted"},
            {Reforge::Fair, "Fair"},
            {Reforge::Glistening, "glistening"},
            {Reforge::OddSword, "odd_sword"},
            {Reforge::Unreal, "Unreal"},
            {Reforge::Gentle, "Gentle"},
            {Reforge::Waxed, "waxed"},
            {Reforge::Blooming, "blooming"},
            {Reforge::Renowned, "Renowned"},
            {Reforge::Hasty, "Hasty"},
            {Reforge::Precise, "Precise"},
            {Reforge::Strange, "Strange"},
            {Reforge::Deadly, "Deadly"},
            {Reforge::Excellent, "excellent"},
            {Reforge::Reinforced, "Reinforced"},
            {Reforge::Submerged, "submerged"},
            {Reforge::Pitchin, "pitchin"},
            {Reforge::Heated, "heated"},
            {Reforge::Grand, "Grand"},
            {Reforge::Suspicious, "suspicious"},
            {Reforge::Fine, "Fine"},
            {Reforge::Neat, "Neat"},
            {Reforge::RichBow, "rich_bow"},
            {Reforge::Salty, "Salty"},
            {Reforge::Awkward, "awkward"},
            {Reforge::Mossy, "mossy"},
            {Reforge::Robust, "robust"},
            {Reforge::Dirty, "dirty"},
            {Reforge::Shaded, "shaded"},
            {Reforge::DoubleBit, "double_bit"},
            {Reforge::Strong, "Strong"},
            {Reforge::Lumberjack, "lumberjack"},
            {Reforge::Godly, "Godly"},
            {Reforge::Spiked, "Spiked"},
            {Reforge::Fleet, "fleet"},
            {Reforge::Refined, "Refined"},
            {Reforge::Bountiful, "bountiful"},
            {Reforge::Forceful, "Forceful"},
            {Reforge::Menacing, "menacing"},
            {Reforge::Perfect, "Perfect"},
            {Reforge::Unpleasant, "Unpleasant"},
            {Reforge::Brilliant, "brilliant"},
            {Reforge::Fortunate, "fortunate"},
            {Reforge::Lucky, "lucky"},
            {Reforge::Lush, "lush"},
            {Reforge::Candied, "candied"},
            {Reforge::Toil, "toil"},
            {Reforge::Fruitful, "fruitful"},
            {Reforge::Superior, "Superior"},
            {Reforge::Hurtful, "Hurtful"},
            {Reforge::Prospector, "prospector"},
            {Reforge::Festive, "festive"},
            {Reforge::Treacherous, "Treacherous"},
            {Reforge::Itchy, "Itchy"},
            {Reforge::Unyielding, "unyielding"},
            {Reforge::Sturdy, "sturdy"},
            {Reforge::Blended, "blended"},
            {Reforge::Moil, "moil"},
            {Reforge::AoteStone, "aote_stone"},
            {Reforge::GreenThumb, "green_thumb"},
            {Reforge::Stiff, "stiff"},
            {Reforge::Warped, "warped"},
            {Reforge::Buzzing, "buzzing"},
            {Reforge::WarpedOnAote, "warped_on_aote"},
            {Reforge::Snowy, "snowy"},
            {Reforge::Peasant, "peasant"},
            {Reforge::Gilded, "Gilded"},
            {Reforge::Honored, "honored"},
            {Reforge::Magnetic, "Magnetic"},
            {Reforge::Earthy, "earthy"},
            {Reforge::Bloody, "bloody"},
            {Reforge::Zealous, "Zealous"},
            {Reforge::Great, "great"},
            {Reforge::Bizarre, "Bizarre"},
            {Reforge::Fortified, "fortified"},
            {Reforge::Headstrong, "headstrong"},
            {Reforge::Unknown, "Unknown"},
            {Reforge::Mithraic, "mithraic"},
            {Reforge::Demonic, "Demonic"},
            {Reforge::Zooming, "zooming"},
            {Reforge::Silky, "Silky"},
            {Reforge::Colossal, "colossal"},
            {Reforge::Rugged, "rugged"},
            {Reforge::Shiny, "Shiny"},
            {Reforge::Ridiculous, "ridiculous"},
            {Reforge::Chomp, "chomp"},
            {Reforge::Ambered, "ambered"},
            {Reforge::Soft, "soft"},
            {Reforge::Cubic, "cubic"},
            {Reforge::Empowered, "empowered"},
            {Reforge::Stellar, "stellar"},
            {Reforge::Vivid, "Vivid"},
            {Reforge::Stained, "stained"},
            {Reforge::Keen, "Keen"},
            {Reforge::Astute, "astute"},
            {Reforge::Beady, "beady"},
            {Reforge::BloodSoaked, "blood_soaked"},
            {Reforge::GreaterSpook, "greater_spook"},
            {Reforge::Hefty, "hefty"},
            {Reforge::Pleasant, "Pleasant"},
            {Reforge::Simple, "Simple"},
            {Reforge::JerryStone, "jerry_stone"},
            {Reforge::Bulky, "bulky"},
            {Reforge::Ominous, "Ominous"},
            {Reforge::Pretty, "Pretty"},
            {Reforge::RichSword, "rich_sword"},
            {Reforge::Fanged, "fanged"},
            {Reforge::Coldfusion, "coldfusion"},
            {Reforge::Sweet, "sweet"},
            {Reforge::OddBow, "odd_bow"},
            {Reforge::Any, "Any"}
        };
        auto it = reforge_to_string_map.find(reforge);
        if (it != reforge_to_string_map.end()) {
            return it->second;
        }
        else {
            throw std::invalid_argument("Unknown Reforge value");
        }
    }
};

struct Gemstone {
    std::string key;
    std::string quality;
    bool isUnlocked;

    bool isValid() const {
        // Define what makes a Gemstone valid
        return !key.empty() && !quality.empty();
    }
};
