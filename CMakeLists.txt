# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.10)

# Project's name
project(MyProject)

# Set C++17 as the standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include the FetchContent module
include(FetchContent)

# Declare nlohmann_json as a fetchable content
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.10.5  # Specify the version you need
)

# Make nlohmann_json available
FetchContent_MakeAvailable(nlohmann_json)

# Add executable
# add_executable(main main.cpp AmericanTrade.cpp Bond.cpp BlackScholesPricer.cpp Date.cpp EuropeanTrade.cpp Logger.cpp Market.cpp JSONReader.cpp Pricer.cpp Swap.cpp PortfolioMaker.cpp CSVReader.cpp TenorMap.cpp Trade.cpp Utils.cpp)
add_executable(main main.cpp AmericanTrade.cpp Bond.cpp BlackScholesPricer.cpp Date.cpp EuropeanTrade.cpp Logger.cpp Market.cpp Pricer.cpp Swap.cpp PortfolioMaker.cpp CSVReader.cpp TenorMap.cpp Trade.cpp Utils.cpp RiskEngine.cpp)

# Link nlohmann_json to your project
target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json)

# Optionally, you can set the output directory for binaries
set_target_properties(main PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
