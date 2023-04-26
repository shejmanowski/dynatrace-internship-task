#include <chrono>
#include <fstream>
#include <iostream>

#include <date/date.h>
#include <nlohmann/json.hpp>
#include <boost/date_time.hpp>


#include "internship.h"

using json = nlohmann::json;
using namespace date;


namespace internship {
    int durationToInt(const std::chrono::duration<int, std::ratio<86400>> durationInDays);

    struct OperatingSystem {
        std::string name;
        std::string cycle;

        // support period expressed in days
        std::chrono::duration<int, std::ratio<86400>> supportPeriod;

        OperatingSystem(std::string name, std::string cycle, std::chrono::duration<int, std::ratio<86400>> supportPeriod) {
            this->name = name;
            this->cycle = cycle;
            this->supportPeriod = supportPeriod;
        }

        void print() {
            std::cout << name << " " << cycle << " " << durationToInt(supportPeriod);
        }
    };

    int durationToInt(const std::chrono::duration<int, std::ratio<86400>> durationInDays) {        
        return static_cast<int>(durationInDays.count());
    }

    bool compareOperatingSystems(const OperatingSystem& os1, const OperatingSystem& os2) {
        if (os1.supportPeriod > os2.supportPeriod)
            return 1;
        return 0;
    }

    bool isCorrectFormat(const json& list, const std::string attributeName) {
        try {
            boost::gregorian::date d(boost::gregorian::from_simple_string(list[attributeName]));
        }
        catch (const std::exception& ex) {
            return false;
        }
        return true;
    }

    // Created before data1.json update
    // 
    // std::string checkAndCorrectTypo(const json& version) {
    //     return (version.contains("releaseDate")) ? "releaseDate" : "releasedDate";
    // }

    std::chrono::duration<int, std::ratio<86400>> calculateSupportPeriod(const json& version, 
        const std::string& releaseDateFieldName, const std::string& eolFieldName) {

        std::istringstream iss;

        // load release date from json
        year_month_day releaseDate;
        iss.str(version[releaseDateFieldName]);
        iss >> parse("%F", releaseDate);

        // load eol date from json
        year_month_day eol;
        iss.str(version[eolFieldName]);
        iss >> parse("%F", eol);

        auto releaseDateTimePoint = sys_days(releaseDate);
        auto eolTimePoint = sys_days(eol);

        auto supportPeriod = eolTimePoint - releaseDateTimePoint;

        // adding 1 day to include end date day
        supportPeriod += days(1);

        return supportPeriod;
    }

    bool containsNeededFields(const json& version) {
        return version.contains("releaseDate") && version.contains("eol");
    }

    OperatingSystem findCycleWithMaxSupportPeriod(const json& product) {
        auto maxSupportPeriod = days(0);
        std::string cycle;

        for (const auto& version : product["versions"]) {

            std::string releaseDateFieldName = "releaseDate";
            std::string eolFieldName = "eol";

            if (containsNeededFields(version) &&
                isCorrectFormat(version, releaseDateFieldName) && 
                isCorrectFormat(version, eolFieldName)) {

                auto supportPeriod = calculateSupportPeriod(version, releaseDateFieldName, eolFieldName);

                if (supportPeriod > maxSupportPeriod) {
                    maxSupportPeriod = supportPeriod;
                    cycle = version["cycle"];
                }
            }
        }
        return OperatingSystem(product["name"], cycle, maxSupportPeriod);
    }

    void printSolution(std::vector<OperatingSystem>& operatingSystems, const int elementCount) {
        auto i = 0;
        for (auto os = operatingSystems.begin(); os != operatingSystems.end() && ++i <= elementCount; ++os)
        {
            (*os).print();
            std::cout << "\n";
        }
    }

    void findAndPrintSolution(const std::string& jsonFileName, const int elementCount) {
        std::ifstream f(jsonFileName);
        json data = json::parse(f);
        std::vector<OperatingSystem> operatingSystems;

        for (const auto& [id, product] : data.items()) {
            if (product["os"] == true) {
                OperatingSystem os = findCycleWithMaxSupportPeriod(product);
                operatingSystems.push_back(os);
            }
        }
        sort(operatingSystems.begin(), operatingSystems.end(), &compareOperatingSystems);
        printSolution(operatingSystems, elementCount);
    }

    // do not remove this function
    void solution(const std::string& jsonFileName, int elementsCount) {

        // put the call to your solution here
        findAndPrintSolution(jsonFileName, elementsCount);
    }
}