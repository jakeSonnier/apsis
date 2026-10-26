#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

int main ()
{
    std::string satName;
    int satelliteNumber;
    char classification;
    std::string internationalDesignator;
    int epochDay = 0;
    int epochYear = 0;
    double julianDayFraction;
    double ballisticCoefficient;
    std::string secondDerivativeMeanMotion;
    int dragTerm;
    int ephemerisType;
    int elementNumber;
    double inclination;
    double rightAscension;
    double eccentricity;
    double perigee;
    double meanAnomaly;
    double meanMotion;
    int revolutionNumber;

    std::string ignored;

    std::ifstream inputFile("tles.csv");

    if (!inputFile.is_open())
    {
        std::cerr << "Error opening file\n";
        return 1;
    }

    std::string name;
    std::string line1;
    std::string line2;

    if (!std::getline(inputFile, name) ||
        !std::getline(inputFile, line1) ||
        !std::getline(inputFile, line2))
    {
        std::cerr << "Error: incomplete TLE\n";
        return 1;
    }

    // Extract fields from line 1
    satelliteNumber = std::stoi(line1.substr(2, 5));
    classification = line1[7];
    internationalDesignator = line1.substr(9, 8);

    epochYear = std::stoi(line1.substr(18, 2));
    epochDay = std::stod(line1.substr(20, 12));

    // Extract fields from line 2
    inclination = std::stod(line2.substr(8, 8));
    rightAscension = std::stod(line2.substr(17, 8));

    // Eccentricity has an implied decimal point
    std::string eccentricityText =
        "0." + line2.substr(26, 7);
    eccentricity = std::stod(eccentricityText);

    perigee =
        std::stod(line2.substr(34, 8));

    meanAnomaly =
        std::stod(line2.substr(43, 8));

    meanMotion =
        std::stod(line2.substr(52, 11));

    std::cout << std::setprecision(10);

    // Display the parsed values
    std::cout << "Satellite: " << name << '\n';
    std::cout << "Catalog number: " << satelliteNumber << '\n';
    std::cout << "Classification: " << classification << '\n';
    std::cout << "International designator: "
              << internationalDesignator << '\n';

    std::cout << "Epoch year (2-digit): " << epochYear << '\n';
    std::cout << "Epoch day: " << epochDay << '\n';

    std::cout << "Inclination: " << inclination << " degrees\n";
    std::cout << "RAAN: " << rightAscension << " degrees\n";
    std::cout << "Eccentricity: " << eccentricity << '\n';
    std::cout << "Argument of perigee: "
              << perigee << " degrees\n";
    std::cout << "Mean anomaly: " << meanAnomaly << " degrees\n";
    std::cout << "Mean motion: " << meanMotion
              << " revolutions/day\n";

    return 0;
}