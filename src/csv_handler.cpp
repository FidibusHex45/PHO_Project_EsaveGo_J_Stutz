#include "csv_handler.hpp"

CSVHandler::CSVHandler() {
}

void CSVHandler::save2csv(std::string save_path, std::vector<cv::Point> dataPoints) {
    remove(save_path.c_str());

    std::ofstream file;
    file.open(save_path);

    for (auto point : dataPoints) {
        file << point.x << "," << point.y << std::endl;
    }
    file.close();
}

std::vector<splineData_proc> CSVHandler::loadSplineData(std::string load_path, int v_lower, int v_upper) {
    v_0 = v_lower;
    v_E = v_upper;
    splineData data_point;
    std::string line, value;
    int colNum = 0;

    std::fstream data(load_path, std::ios::in);
    if (data.is_open()) {
        while (getline(data, line)) {
            std::stringstream str(line);
            colNum = 0;
            while (getline(str, value, ',')) {
                switch (colNum) {
                    case 0:
                        data_point.point.x = std::stoi(value);
                        break;
                    case 1:
                        data_point.point.y = std::stoi(value);
                        break;
                    case 2:
                        data_point.velocity = std::stod(value);
                        break;
                    default:
                        std::cerr << "Invalid data point." << std::endl;
                }
                colNum++;
            }
            spline_data.push_back(data_point);
        }
    } else {
        std::cerr << "Could not open the file." << std::endl;
    }
    mapVelocity();
    return spline_data_proc;
}

std::vector<splineData_proc> CSVHandler::loadSplineDataConfig(std::string load_path) {
    splineData_proc data_point;
    std::string line, value;
    int colNum = 0;

    std::fstream data(load_path, std::ios::in);
    if (data.is_open()) {
        while (getline(data, line)) {
            std::stringstream str(line);
            colNum = 0;
            while (getline(str, value, ',')) {
                switch (colNum) {
                    case 0:
                        data_point.point.x = std::stoi(value);
                        break;
                    case 1:
                        data_point.point.y = std::stoi(value);
                        break;
                    case 2:
                        data_point.velocity = std::stoi(value);
                        break;
                    default:
                        std::cerr << "Invalid data point." << std::endl;
                }
                colNum++;
            }
            spline_data_proc.push_back(data_point);
        }
    } else {
        std::cerr << "Could not open the file." << std::endl;
    }
    return spline_data_proc;
}

void CSVHandler::saveSplineData(std::string save_path, std::vector<splineData_proc> data) {
    remove(save_path.c_str());

    std::ofstream file;
    file.open(save_path);

    for (int i = 0; i < data.size(); i++) {
        file << data.at(i).point.x << "," << data.at(i).point.y << "," << data.at(i).velocity << std::endl;
    }
    file.close();
}

void CSVHandler::mapVelocity() {
    if (v_0 >= v_E) {
        std::cerr << "lower boundary value must be smaller than upper boundary value." << std::endl;
    }
    splineData_proc dataPoint_proc;
    for (auto dataPoint : spline_data) {
        dataPoint_proc.point.x = dataPoint.point.x;
        dataPoint_proc.point.y = dataPoint.point.y;
        dataPoint_proc.velocity = int(dataPoint.velocity * double(v_E - v_0)) + v_0;
        spline_data_proc.push_back(dataPoint_proc);
    }
}
