#include <iostream>
#include <string>
#include "LaserController.h"




int main()
{
    LaserController laser_controller{};
    char* buff = new char[laser_controller.m_required_response_buffer_size];

    // Loop forever, catching ctrl+c in windows in a pain.
    for (std::string line; std::getline(std::cin, line);)
    {
        auto response = laser_controller.Process_Command(line.c_str(), line.length(), buff, laser_controller.m_required_response_buffer_size);
        std::cout << buff << std::endl;
        //if( line.length() == 0)
        //    break;
    }
    return 0;
}