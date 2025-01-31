#include "JSONReader.h"
#include <memory>

void JSONReader::setFileName(const std::string &filename) {
    theFilename = filename;
}

std::string JSONReader::getFileName() const { return theFilename; }

void JSONReader::setMarketObject(const Market &marketObj) {
    theMarket = marketObj;
}
Market JSONReader::getMarketObject() const { return theMarket; };

void JSONReader::setPortfolio(std::vector<std::unique_ptr<Trade>>& portfolioVec) {
    thePortfolio = std::move(portfolioVec);
}


JSONReader::~JSONReader() {
    // for (Trade* trade : thePortfolio) {
    //     delete trade;
    // }
    thePortfolio.clear();
}

std::vector<std::unique_ptr<Trade>>& JSONReader::getPortfolio() { return thePortfolio; };


void JSONReader::constructPortfolio() {
    std::string lineText;
    std::ifstream MyReadFile(theFilename);
    bool buildReady = false;

    // common vars
    std::string uuid;
    std::string name;

    // common regex
    std::regex getUUID(R"(\"uuid\":\"(.*?)\")");
    std::regex getInstrument(R"(\"instrument\":\"(.*?)\")");
    std::regex getName(R"(\"name\":\"(.*?)\")");
    std::regex getStartDate(R"(\"start_date\":\"(.*?)\")");
    std::regex getEndDate(R"(\"end_date\":\"(.*?)\")");
    std::regex getExpiry(R"(\"expiry\":\"(.*?)\")");
    std::regex getNotional(R"(\"notional\":([0-9\.\-]*))");
    Date startDate;
    Date endDate;
    Date expiryDate;
    int notional;

    // BOND specific
    std::regex getPrice(R"(\"price\":([0-9\.]*))");
    std::regex getCoupon_rate_pct(R"(\"coupon_rate_pct\":([0-9\.]*))");
    double bondPrice;
    double bondCouponRate;

    // SWAP specific
    std::regex getFixed_rate_pct(R"(\"fixed_rate_pct\":([0-9\.]*))");
    std::regex getFixed_for_float(R"(\"fixed_for_float\":(.*))");
    std::regex getFrequency(R"(\"frequency\":([0-9\.]*))");
    std::regex getCurveName(R"(\"curve_name\":\"(.*?)\")");
    double fixedRate;
    bool fixed_for_float;
    double frequency;
    std::string curveName;

    // OPTION specific
    std::regex getOptionType(R"(\"type\":\"(.*?)\")");
    std::regex getStrike(R"(\"strike\":([0-9\.]*))");
    std::regex getUnderlying(R"(\"underlying\":\"(.*?)\")");
    double strike;
    std::string underlying;
    OptionType option_type;

    std::smatch match;
    int toBuild = 100;
    enum instruments { BOND, SWAP, EURO_OPTION, AMERICAN_OPTION };
    std::cout << "Construct portfolio from JSON file: " << theFilename << std::endl;

    if (MyReadFile.is_open()) {
        std::cout << "JSON file is open" << std::endl;
        std::cout<<std::endl;
    }

    while (std::getline(MyReadFile, lineText)) {
        trim(lineText); // in place trim

        // parsing logic starts here
        if (lineText.find("{") != std::string::npos && !buildReady) {
            // cue to start building something
            buildReady = true;
        } else if (lineText.find("}") != std::string::npos && buildReady) {
            // build the instrument
            switch (toBuild) {
            case BOND:
                std::cout << name << " " << startDate << " " << endDate << " "
                          << bondPrice << " " << notional << " "
                          << bondCouponRate << std::endl;
                std::cout << "building BOND" ;
                thePortfolio.push_back(std::make_unique<Bond>(startDate, endDate, referenceDate, notional, 
                                                              bondCouponRate, frequency, theMarket, name, uuid));
                std::cout << " ---> build complete" << std::endl;

                break;
            case SWAP:
                std::cout << name << " " << startDate << " " << endDate << " "
                          << notional << " " << fixedRate << " "
                          << fixed_for_float << " " << frequency << " "
                          << curveName << std::endl;
                std::cout << "building SWAP";
                thePortfolio.push_back(std::make_unique<Swap>(startDate, endDate, referenceDate, notional,
                                                              fixedRate, frequency,
                                                              fixed_for_float, theMarket, curveName, uuid));
                std::cout << " ---> build complete" << std::endl;
                break;

            case EURO_OPTION:
                std::cout << name << " " << option_type << " " << strike << " "
                          << expiryDate << " " << underlying << std::endl;
                std::cout << "building EURO_OPTION";
                thePortfolio.push_back(std::make_unique<EuropeanOption>(
                                                        option_type, strike, expiryDate, referenceDate, underlying, uuid));
                std::cout << " ---> build complete" << std::endl;
                break;

            case AMERICAN_OPTION:
                std::cout << name << " " << option_type << " " << strike << " "
                          << expiryDate << " " << underlying << std::endl;
                std::cout << "building AMERICAN_OPTION";
                thePortfolio.push_back(std::make_unique<AmericanOption>(
                                                        option_type, strike, expiryDate, referenceDate, underlying, uuid));
                std::cout << " ---> build complete" << std::endl;
                break;

            default:
                std::cout << "UNKNOWN" << std::endl;
            }
            // end of an instrument
            buildReady = false;
            std::cout << std::endl;
        } else if (buildReady) {
            // get the instrument first
            if (std::regex_search(lineText, match, getInstrument)) {
                if (match.str(1) == "BOND") {
                    toBuild = BOND;
                } else if (match.str(1) == "SWAP") {
                    toBuild = SWAP;
                } else if (match.str(1) == "EURO_OPTION") {
                    toBuild = EURO_OPTION;
                } else if (match.str(1) == "AMERICAN_OPTION") {
                    toBuild = AMERICAN_OPTION;
                } else {
                    toBuild = 100;
                }
            }
            // get the name next
            else if (std::regex_search(lineText, match, getName)) {
                name = match.str(1);
            }
            // get the uuid next
            else if (std::regex_search(lineText, match, getUUID)) {
                uuid = match.str(1);
            }
            /*
                it is product specific after this
                1. BOND
                2. SWAP
                3. EURO_OPTION
                4. AMERICAN_OPTION
            */
            else if (toBuild == BOND) {
                /*
                    BOND
                */
                if (std::regex_search(lineText, match, getPrice)) {
                    std::string temporary = match.str(1);
                    bondPrice = std::stod(match.str(1));
                } else {
                    bondPrice = bondPrice;
                }

                if (std::regex_search(lineText, match, getCoupon_rate_pct)) {
                    std::string temporary = match.str(1);
                    bondCouponRate = std::stod(match.str(1)) / 100.0;
                } else {
                    bondCouponRate = bondCouponRate;
                }

                if (std::regex_search(lineText, match, getNotional)) {
                    char neg_sign = match.str(1)[0];
                    std::cout<<"i have neg no "<<neg_sign<< " "<< match.str(1)<<std::endl;
                    if (neg_sign == '-'){
                        notional = -1 * std::stoi(match.str(1).erase(0, 1));
                    } else {
                        notional = std::stoi(match.str(1));
                    }
                } else {
                    notional = notional;
                }

                if (std::regex_search(lineText, match, getStartDate)) {
                    std::string temp_start = match.str(1);
                    std::vector<std::string> temp_parse =
                        parseRow(temp_start, '-');
                    startDate =
                        Date(std::stoi(temp_parse[0]), std::stoi(temp_parse[1]),
                             std::stoi(temp_parse[2]));
                } else {
                    startDate = startDate;
                }

                if (std::regex_search(lineText, match, getEndDate)) {
                    std::string temp_end = match.str(1);
                    std::vector<std::string> temp_parse =
                        parseRow(temp_end, '-');
                    endDate =
                        Date(std::stoi(temp_parse[0]), std::stoi(temp_parse[1]),
                             std::stoi(temp_parse[2]));
                } else {
                    endDate = endDate;
                }

                if (std::regex_search(lineText, match, getFrequency)) {
                    frequency = std::stod(match.str(1));
                } else {
                    frequency = frequency;
                }

            } else if (toBuild == SWAP) {
                /*
                    SWAP
                */
                if (std::regex_search(lineText, match, getStartDate)) {
                    std::string temp_start = match.str(1);
                    std::vector<std::string> temp_parse =
                        parseRow(temp_start, '-');
                    startDate =
                        Date(std::stoi(temp_parse[0]), std::stoi(temp_parse[1]),
                             std::stoi(temp_parse[2]));
                } else {
                    startDate = startDate;
                }

                if (std::regex_search(lineText, match, getEndDate)) {
                    std::string temp_end = match.str(1);
                    std::vector<std::string> temp_parse =
                        parseRow(temp_end, '-');
                    endDate =
                        Date(std::stoi(temp_parse[0]), std::stoi(temp_parse[1]),
                             std::stoi(temp_parse[2]));
                } else {
                    endDate = endDate;
                }

                if (std::regex_search(lineText, match, getNotional)) {
                    char neg_sign = match.str(1)[0];
                    // std::cout<<"i have neg no "<<neg_sign<< " "<< match.str(1)<<std::endl;
                    if (neg_sign == '-'){
                        notional = -1 * std::stoi(match.str(1).erase(0, 1));
                    } else {
                        notional = std::stoi(match.str(1));
                    }
                } else {
                    notional = notional;
                }

                if (std::regex_search(lineText, match, getFixed_rate_pct)) {
                    fixedRate = std::stod(match.str(1)) / 100.0;
                } else {
                    fixedRate = fixedRate;
                }

                if (std::regex_search(lineText, match, getFixed_for_float)) {
                    std::string temp = match.str(1);
                    temp.pop_back();
                    // std::cout<<"parse check get "<<temp <<std::endl;
                    if (temp == "true"){
                        fixed_for_float = true;
                    } else{
                        fixed_for_float = false;
                    }

                } else {
                    fixed_for_float = fixed_for_float;
                }

                if (std::regex_search(lineText, match, getFrequency)) {
                    frequency = std::stod(match.str(1));
                } else {
                    frequency = frequency;
                }

                if (std::regex_search(lineText, match, getCurveName)) {
                    curveName = match.str(1);
                    std::cout << "CURVENAME: " << curveName << std::endl;
                }

            } else if (toBuild == EURO_OPTION) {
                if (std::regex_search(lineText, match, getOptionType)) {
                    if (match.str(1) == "CALL") {
                        option_type = Call;
                    } else if (match.str(1) == "PUT") {
                        option_type = Put;
                    } else if (match.str(1) == "BINARY_CALL") {
                        option_type = BinaryCall;
                    } else if (match.str(1) == "BINARY_PUT") {
                        option_type = BinaryPut;
                    }
                } else {
                    option_type = option_type;
                }

                if (std::regex_search(lineText, match, getStrike)) {
                    strike = std::stod(match.str(1));
                } else {
                    strike = strike;
                }

                if (std::regex_search(lineText, match, getExpiry)) {
                    std::string temp_expiry = match.str(1);
                    std::vector<std::string> temp_parse =
                        parseRow(temp_expiry, '-');
                    expiryDate =
                        Date(std::stoi(temp_parse[0]), std::stoi(temp_parse[1]),
                             std::stoi(temp_parse[2]));
                    std::cout << "Parsed expiryDate: " << expiryDate << std::endl; // !!!
                } else {
                    expiryDate = expiryDate;
                }

                if (std::regex_search(lineText, match, getUnderlying)) {
                    underlying = match.str(1);
                }
            } else if (toBuild == AMERICAN_OPTION) {
                if (std::regex_search(lineText, match, getOptionType)) {
                    if (match.str(1) == "CALL") {
                        option_type = Call;
                    } else if (match.str(1) == "PUT") {
                        option_type = Put;
                    } else if (match.str(1) == "BINARY_CALL") {
                        option_type = BinaryCall;
                    } else if (match.str(1) == "BINARY_PUT") {
                        option_type = BinaryPut;
                    }
                } else {
                    option_type = option_type;
                }

                if (std::regex_search(lineText, match, getStrike)) {
                    strike = std::stod(match.str(1));
                } else {
                    strike = strike;
                }

                if (std::regex_search(lineText, match, getExpiry)) {
                    std::string temp_expiry = match.str(1);
                    std::vector<std::string> temp_parse =
                        parseRow(temp_expiry, '-');
                    expiryDate =
                        Date(std::stoi(temp_parse[0]), std::stoi(temp_parse[1]),
                             std::stoi(temp_parse[2]));
                    std::cout << "Parsed expiryDate: " << expiryDate << std::endl; // !!!
                } else {
                    expiryDate = expiryDate;
                }

                if (std::regex_search(lineText, match, getUnderlying)) {
                    underlying = match.str(1);
                }
            }
        }
    }
}

std::vector<std::string> JSONReader::parseRow(const std::string &row,
                                              const char &delim) {
    std::vector<std::string> result;
    std::stringstream ss(row);
    std::string item;
    while (getline(ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}

/*
int main() {
    JSONReader myJSONReader;
    myJSONReader.setFileName("./data/portfolio.json");
    myJSONReader.constructPortfolio();

    return 0;
}
*/