/*
 * GParser.cpp
 *
 *  Created on: Sep 5, 2019
 *      Author: embedded
 */

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

enum Command{
    M1, //0,
    M2, //1,
    M4, //2,
    M5, //= 3,
    M10, // = 4,
    M11, // = 5,
    G1, // = 6,
    G28 // = 7
 };

void selectCommandParameter(const std::string gcode_input_line){
    /*extract Command from line and turn it into int value, then read the line and fill a std::map with numeric-parameter value
        //
        //gcode_input_line is the original input
        //input_sstream copys the input in a stringstream to cut it down
        //parameter will be used to temp. store the string-parameter-block from the input to then extract the numeric value
        //comand_int see enum Command
        //
   */
   std::string command_string, parameter;
   std::stringstream input_sstream(gcode_input_line);
   int command_int;
   getline(input_sstream, command_string, ' ');
   if(command_string[0] == 'M'){
        switch(stoi(command_string)){
            case 1: command_int = 0;
                break;
            case 2: command_int = 1;
                break;
            case 4: command_int = 2;
                break;
            case 5: command_int = 3;
                break;
            case 10: command_int = 4;
                break;
            case 11: command_int = 5;
                break;
        }
   }else if(command_string[0] == 'G'){
        switch(stoi(command_string)){
            case 1: command_int = 6;
                break;
            case 2: command_int = 7;
                break;
        }
   }

   if(command_int == 6){
        std::vector<float> parameterMap;
        for(auto it = parameterMap.begin(); getline(input_sstream, parameter, ' '); it++){
               parameterMap.insert(it, stof(parameter));
        }
    }else{
        std::vector<int> parameterMap;
        for(auto it = parameterMap.begin(); getline(input_sstream, parameter, ' '); it++){
               parameterMap.insert(it, stoi(parameter));
        }

    }
//maybe safe the map somehwere?
}

int GParser(void){
    std::string gcode_input_line;
    std::ifstream input_file;
    input_file.open("gcode.txt");
    while(!input_file.eof()){
        getline(input_file, gcode_input_line, '\n');
        selectCommandParameter(gcode_input_line);
    }
    input_file.close();
    return 0;
}
