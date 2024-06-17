#include <cstdint>
#include <string>
#include <vector>


typedef struct DataPoint {
    uint8_t id;
    std::string name;
    bool valid_data;
    bool meausred;
    std::string error_msg;
    std::vector<double> x;
    std::vector<double> y;
} DataPoint;
