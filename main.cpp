#include <iostream>

#include <string>

#include <fstream>

#include <vector>
using namespace std;
#include "instruction.h"
#include "registers.h"
#include "reservationStation.h"


int const zero = 0;

int cycle = 0;

vector <string> instructions;

int dataMemory[65536];



string instructionType(string instruction);

registers* regs();

vector <instruction> instruction_table(vector<string> inst);

void variables(int index, string type, vector<string> inst, instruction& table);

void arithmetic(string instructionName, int rd, int rs1, int rs2, int imm);

void load(string instructionName, int rd, int rs1, int imm);

void store(string instructionName, int rs2, int imm, int rs1);

int findAddress(int address);

string** ROB();

reservationStation* reservation_stations();

void issue(instruction& inst, reservationStation Stations[], registers* registers, string** rob, int& currentEntries, int& tailIndex, int& index, vector <vector<string>> &output, int& noOfCycles);

void issue_load(instruction& inst, reservationStation& Stations, registers* registers, string** rob, int& tailIndex, int& index);

void issue_store(instruction& inst, reservationStation& Station, registers* registers, string** rob, int& tailIndex, int& index);

void issue_jmp_jal_ret(instruction& inst, reservationStation& Station, registers* registers, string** rob, int& tailIndex, int& index);

void issue_add_sub_addi(instruction& inst, reservationStation& Station, registers* registers, string** rob, int& tailIndex, int& index);

void issue_mul(instruction& inst, reservationStation& Station, registers* registers, string** rob, int& tailIndex, int& index);

void issue_nand(instruction& inst, reservationStation& Station, registers* registers, string** rob, int& tailIndex, int& index);

void issue_beq(instruction& inst, reservationStation& Station, registers* registers, string** rob, int& tailIndex, int& index);

void print(registers* registers, vector <instruction> inst_table, reservationStation* Stations, string** rob);

void execute(reservationStation* Stations, vector <instruction>& inst_table, string** rob, registers* registers, vector <vector<string>> &output, int& noOfCycles);

void write(reservationStation* Stations, vector <instruction>& inst_table, string** rob, registers* registers, vector <vector<string>> &output, int& noOfCycles);

void commit(vector <instruction>& inst_table, string** rob, registers* registers, int& headIndex, int& currententries, int& index, int& tailIndex, reservationStation* Stations, vector <vector<string>> &output, double& noOfCycles, double &branches, int &miss);

void printResults(vector<vector<string>>& output, int& no_of_cycles, double& branches, double& miss);

void tomasulo(vector<string> instructions);




int main(int argc, const char* argv[]) {

    struct registers reg[8];

    reg[0].value = zero;


    for (int j = 0; j < 65536; j++) { //initialize data memory

        dataMemory[j] = 0;

    }

    for (int j = 1; j < 8; j++) { //initialize registers

        reg[j].value = 0;

    }

    //reading input from files

    string instructionsFilename = "Instructions.txt";

    string dataFilename = "Data.txt";



    ifstream instructionFile;

    ifstream dataFile;

    //instructions

    instructionFile.open(instructionsFilename);

    dataFile.open(dataFilename);

    if (!instructionFile.is_open()) {

        cout << "Error opening instruction file" << endl;

        return 0;

    }

    string instructionLine;

    while (!instructionFile.eof()) {

        getline(instructionFile, instructionLine);

        instructions.push_back(instructionLine);

    }

    //data

    if (!dataFile.is_open()) {

        cout << "Error opening data file" << endl;

        return 0;

    }

    string dataLine;

    int commaPosition;

    string address, value;

    int i = 0;

    while (!dataFile.eof()) {

        getline(dataFile, dataLine);

        commaPosition = dataLine.find(',');

        value = dataLine.substr(0, commaPosition);

        address = dataLine.substr(commaPosition + 2, dataLine.length() - (commaPosition + 2));

        dataMemory[stoi(address)] = stoi(value);

        i++;

    }

    tomasulo(instructions);

    return 0;

}



string instructionType(string instruction) //returns instruction type to know which fucntion to call

{

    int spacePosition;

    string type;

    spacePosition = instruction.find(" ");

    type = instruction.substr(0, spacePosition);

    return type;

}



reservationStation* reservation_stations()

{

    reservationStation* rs = new reservationStation[15];

    rs[0].type = "LOAD";

    rs[1].type = "LOAD";

    rs[2].type = "STORE";

    rs[3].type = "STORE";

    rs[4].type = "JMP_JAL_RET";

    rs[5].type = "JMP_JAL_RET";

    rs[6].type = "BEQ";

    rs[7].type = "BEQ";

    rs[8].type = "ADD/SUB/ADDI";

    rs[9].type = "ADD/SUB/ADDI";

    rs[10].type = "ADD/SUB/ADDI";

    rs[11].type = "NAND";

    rs[12].type = "NAND";

    rs[13].type = "MUL";

    rs[14].type = "MUL";

    for (int i = 0; i < 15; i++)

    {

        rs[i].busy = false;
        rs[i].dest = -1;
    }

    return rs;

}



registers* regs()

{

    registers* r = new registers[8];

    for (int i = 0; i < 8; i++)

    {//r[i].ROB_entry = i + 1;

        r[i].ROB_entry = NULL;

        if (i == 0) {

            r[i].value = zero;

        }

        else

            r[i].value = 5;



    }

    return r;

}

vector <instruction> instruction_table(vector<string> inst)

{

    vector <instruction> inst_vec;

    instruction ins;



    for (int i = 0; i < inst.size(); i++)

    {

        ins.type = instructionType(inst[i]);

        variables(i, ins.type, inst, ins);

        ins.PC = i * 4;

        inst_vec.push_back(ins);



    }

    return inst_vec;

}

void variables(int index, string type, vector<string> inst, instruction& table)

{

    string tempinst = inst[index];

    int spacePosition;

    spacePosition = tempinst.find(" ");

    string rd, rs1, rs2, imm;

    int comma1, comma2;

    comma1 = tempinst.find(',');


    string sub = tempinst.substr(comma1 + 1, tempinst.size() - comma1);

    comma2 = sub.find(',');

    int bracket, bracket2;

    bracket = tempinst.find('(');

    bracket2 = tempinst.find(')');

    if (type == "load") {



        rd = tempinst.substr(6, 1);

        table.rd = stoi(rd);

        imm = tempinst.substr(8, bracket - 7);

        table.imm = stoi(imm);

        rs1 = tempinst.substr(bracket + 2, 1);

        table.rs1 = stoi(rs1);

        table.rs2 = NULL;

    }

    else if (type == "store")

    {

        rs2 = tempinst.substr(7, 1);

        table.rs2 = stoi(rs2);

        imm = tempinst.substr(9, bracket - 8);

        table.imm = stoi(imm);

        rs1 = tempinst.substr(bracket + 2, 1);

        table.rs1 = stoi(rs1);

        table.rd = NULL;

    }

    else if (type == "jmp")

    {

        imm = tempinst.substr(4, tempinst.size() - 4);

        table.imm = stoi(imm);

    }

    else if (type == "beq")

    {

        rs1 = tempinst.substr(5, 1);

        table.rs1 = stoi(rs1);



        rs2 = tempinst.substr(8, 1);


        table.rs2 = stoi(rs2);



        imm = tempinst.substr(11, tempinst.size() - 11);

        table.imm = stoi(imm);



        table.rd = NULL;



    }

    else if (type == "jal")

    {

        imm = tempinst.substr(4, tempinst.size() - 4);

        table.imm = stoi(imm);

    }

    else if (type == "ret")

    {



    }

    else if (type == "add")

    {

        rd = tempinst.substr(5, 1);

        table.rd = stoi(rd);



        rs1 = tempinst.substr(8, 1);

        table.rs1 = stoi(rs1);

        rs2 = tempinst.substr(11, 1);



        table.rs2 = stoi(rs2);

        table.imm = NULL;



    }

    else if (type == "sub")

    {

        rd = tempinst.substr(5, 1);

        table.rd = stoi(rd);



        rs1 = tempinst.substr(8, 1);

        table.rs1 = stoi(rs1);

        rs2 = tempinst.substr(11, 1);



        table.rs2 = stoi(rs2);

        table.imm = NULL;



    }

    else if (type == "addi")

    {

        rd = tempinst.substr(6, 1);

        table.rd = stoi(rd);

        rs1 = tempinst.substr(9, 1);

        table.rs1 = stoi(rs1);

        imm = tempinst.substr(11, tempinst.size() - 11);

        table.imm = stoi(imm);

        table.rs2 = NULL;



    }

    else if (type == "nand")

    {

        rd = tempinst.substr(6, 1);

        table.rd = stoi(rd);



        rs1 = tempinst.substr(9, 1);

        table.rs1 = stoi(rs1);

        rs2 = tempinst.substr(12, 1);



        table.rs2 = stoi(rs2);

        table.imm = NULL;



    }

    else if (type == "mul")

    {

        rd = tempinst.substr(5, 1);

        table.rd = stoi(rd);



        rs1 = tempinst.substr(8, 1);

        table.rs1 = stoi(rs1);

        rs2 = tempinst.substr(11, 1);



        table.rs2 = stoi(rs2);

        table.imm = NULL;



    }

}


string** ROB(int headIndex, int tailIndex)

{

    string** rob;

    rob = new string * [8];

    for (int i = 0; i < 8; i++)

        rob[i] = new string[5];

    for (int i = 0; i < 8; i++) {

        for (int j = 0; j < 5; j++) {

            rob[i][j] = "";

        }

    }

    for (int i = 0; i < 8; i++) {

        rob[i][0] = to_string(i + 1);

    }

    headIndex = 1;

    tailIndex = 1;

    int filled = 0;

    int robEntries = 8;

    if (filled == robEntries) {

        filled++;

    }



    return rob;

}

void issue(instruction& inst, reservationStation Stations[], registers* registers, string** rob, int& currentEntries, int& tailIndex, int& index, vector <vector<string>> &output, int& noOfCycles)

{
   
    if ((inst.state.empty()) && (rob[tailIndex - 1][1].empty()) || (inst.state == "committed"))

    {
        if (inst.type == "load")
        {
            if (Stations[0].busy == 0 && currentEntries != 8)
            {

                issue_load(inst, Stations[0], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

            else if (Stations[1].busy == 0 && currentEntries != 8)

            {

                issue_load(inst, Stations[1], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

        }

        else if (inst.type == "store")

        {

            if (Stations[2].busy == 0 && currentEntries != 8)

            {

                issue_store(inst, Stations[2], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

            else if (Stations[3].busy == 0 && currentEntries != 8)

            {

                issue_store(inst, Stations[3], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

        }

        else if (inst.type == "jmp")

        {

            if (Stations[4].busy == 0 && currentEntries != 8)

            {

                issue_jmp_jal_ret(inst, Stations[4], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

            else if (Stations[5].busy == 0 && currentEntries != 8)

            {

                issue_jmp_jal_ret(inst, Stations[5], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

        }

        else if (inst.type == "beq")

        {

            if (Stations[6].busy == 0 && currentEntries != 8)

            {

                issue_beq(inst, Stations[6], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

            else if (Stations[7].busy == 0 && currentEntries != 8)

            {

                issue_beq(inst, Stations[7], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

        }

        else if (inst.type == "jal")

        {

            if (Stations[4].busy == 0 && currentEntries != 8)

            {

                issue_jmp_jal_ret(inst, Stations[4], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

            else if (Stations[5].busy == 0 && currentEntries != 8)

            {

                issue_jmp_jal_ret(inst, Stations[5], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

        }

        else if (inst.type == "ret")

        {

            if (Stations[4].busy == 0 && currentEntries != 8)

            {

                issue_jmp_jal_ret(inst, Stations[4], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

            else if (Stations[5].busy == 0 && currentEntries != 8)

            {

                issue_jmp_jal_ret(inst, Stations[5], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

        }

        else if (inst.type == "add")

        {

            if (Stations[8].busy == 0 && currentEntries != 8)

            {

                issue_add_sub_addi(inst, Stations[8], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

            else if (Stations[9].busy == 0 && currentEntries != 8)

            {

                issue_add_sub_addi(inst, Stations[9], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

            else if (Stations[10].busy == 0 && currentEntries != 8)

            {

                issue_add_sub_addi(inst, Stations[10], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

        }

        else if (inst.type == "sub")

        {

            if (Stations[8].busy == 0 && currentEntries != 8)

            {

                issue_add_sub_addi(inst, Stations[8], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

            else if (Stations[9].busy == 0 && currentEntries != 8)

            {

                issue_add_sub_addi(inst, Stations[9], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

            else if (Stations[10].busy == 0 && currentEntries != 8)

            {

                issue_add_sub_addi(inst, Stations[10], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

        }

        else if (inst.type == "addi")

        {

            if (Stations[8].busy == 0 && currentEntries != 8)

            {

                issue_add_sub_addi(inst, Stations[8], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

            else if (Stations[9].busy == 0 && currentEntries != 8)

            {

                issue_add_sub_addi(inst, Stations[9], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

            else if (Stations[10].busy == 0 && currentEntries != 8)

            {

                issue_add_sub_addi(inst, Stations[10], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

        }

        else if (inst.type == "nand")

        {

            if (Stations[11].busy == 0 && currentEntries != 8)

            {

                issue_nand(inst, Stations[11], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

            else if (Stations[12].busy == 0 && currentEntries != 8)

            {

                issue_nand(inst, Stations[12], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

        }

        else if (inst.type == "mul")

        {

            if (Stations[13].busy == 0 && currentEntries != 8)

            {

                issue_mul(inst, Stations[13], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

            else if (Stations[14].busy == 0 && currentEntries != 8)

            {

                issue_mul(inst, Stations[14], registers, rob, tailIndex, index);
                vector<string> out;
                out.push_back(inst.type);
                out.push_back(to_string(noOfCycles));
                out.push_back(to_string(inst.PC));
                output.push_back(out);
                currentEntries++;

            }

        }

    }

}





void issue_load(instruction& inst, reservationStation& Station, registers* registers, string** rob, int& tailIndex, int& index)

{



    inst.state = "issued";

    Station.busy = 1;

    Station.op = inst.type;

    Station.No_cycles = 0;

    if (registers[inst.rs1].ROB_entry == NULL) {

        Station.Vj = inst.rs1;

    }

    else

    {



        Station.Qj = to_string(registers[inst.rs1].ROB_entry);



    }

    Station.dest = tailIndex;

    Station.A = inst.imm;

    if (tailIndex <= 8) {

        inst.ROB_ENTRY = tailIndex;

        registers[inst.rd].ROB_entry = tailIndex;

        rob[tailIndex - 1][1] = inst.type;

        rob[tailIndex - 1][2] = to_string(inst.rd); //destination not necessarily rd



        rob[tailIndex - 1][4] = "N";



        tailIndex++;

        if (tailIndex == 9)

            tailIndex = 1;

    }

    index++;

}



void issue_store(instruction& inst, reservationStation& Station, registers* registers, string** rob, int& tailIndex, int& index)

{

    inst.state = "issued";

    Station.busy = 1;

    Station.op = inst.type;

    Station.No_cycles = 0;



    if (registers[inst.rs2].ROB_entry == NULL) {

        Station.Vk = inst.rs2;

    }

    else

        Station.Qk = to_string(registers[inst.rs2].ROB_entry);





    Station.dest = tailIndex;

    Station.A = inst.imm;

    if (tailIndex <= 8) {

        inst.ROB_ENTRY = tailIndex;

        rob[tailIndex - 1][1] = inst.type;

        rob[tailIndex - 1][2] = to_string(inst.rs2);

        rob[tailIndex - 1][3] = to_string(inst.rs1 + inst.imm); //destination not necessarily rd

        rob[tailIndex - 1][4] = "N";

        tailIndex++;

        if (tailIndex == 9)

            tailIndex = 1;

    }

    index++;



}



void issue_jmp_jal_ret(instruction& inst, reservationStation& Station, registers* registers, string** rob, int& tailIndex, int& index)

{

    inst.state = "issued";

    Station.busy = 1;

    Station.op = inst.type;

    Station.No_cycles = 0;

    if (inst.type == "ret") {

        Station.A = registers[1].value;

        /*if (registers[1].ROB_entry != NULL) {

            Station.Qj = to_string(registers[1].ROB_entry);

        }*/

    }

    else

        Station.A = inst.PC + inst.imm + 4;





    Station.dest = tailIndex;

    if (tailIndex <= 8) {

        inst.ROB_ENTRY = tailIndex;


        rob[tailIndex - 1][1] = inst.type;

        if (inst.type == "jal")

        {

            rob[tailIndex - 1][2] = "1";

        }

        rob[tailIndex - 1][4] = "N";

        tailIndex++;

        if (tailIndex == 9)

            tailIndex = 1;

    }

    index = Station.A / 4;

}



void issue_beq(instruction& inst, reservationStation& Station, registers* registers, string** rob, int& tailIndex, int& index)

{

    inst.state = "issued";

    Station.busy = 1;

    Station.op = inst.type;

    Station.No_cycles = 0;

    Station.A = inst.PC + inst.imm + 4;



    if (registers[inst.rs1].ROB_entry == NULL) {

        Station.Vj = inst.rs1;

    }

    else

        Station.Qj = to_string(registers[inst.rs1].ROB_entry);

    if (registers[inst.rs2].ROB_entry == NULL) {

        Station.Vk = inst.rs2;

    }

    else

        Station.Qk = to_string(registers[inst.rs2].ROB_entry);





    Station.dest = tailIndex;

    if (tailIndex <= 8) {

        inst.ROB_ENTRY = tailIndex;

        registers[inst.rd].ROB_entry = tailIndex;

        rob[tailIndex - 1][1] = inst.type;

        if (registers[inst.rs1].value == registers[inst.rs2].value) {

            rob[tailIndex - 1][2] = "1";

        }

        else

            rob[tailIndex - 1][2] = "0";



        rob[tailIndex - 1][4] = "N";

        tailIndex++;

        if (tailIndex == 9)

            tailIndex = 1;

    }

    index = Station.A / 4;



}



void issue_add_sub_addi(instruction& inst, reservationStation& Station, registers* registers, string** rob, int& tailIndex, int& index)

{

    inst.state = "issued";

    Station.busy = 1;

    Station.op = inst.type;

    Station.No_cycles = 0;

    if (registers[inst.rs1].ROB_entry == NULL) {

        Station.Vj = inst.rs1;

    }

    else

        Station.Qj = to_string(registers[inst.rs1].ROB_entry);

    if (registers[inst.rs2].ROB_entry == NULL) {

        Station.Vk = inst.rs2;

    }

    else

        Station.Qk = to_string(registers[inst.rs2].ROB_entry);





    Station.dest = tailIndex;

    if (tailIndex <= 8) {

        inst.ROB_ENTRY = tailIndex;

        registers[inst.rd].ROB_entry = tailIndex;

        rob[tailIndex - 1][1] = inst.type;

        rob[tailIndex - 1][2] = to_string(inst.rd);//destination not necessarily rd



        rob[tailIndex - 1][4] = "N";

        tailIndex++;

        if (tailIndex == 9)

            tailIndex = 1;

    }

    index++;

}



void issue_mul(instruction& inst, reservationStation& Station, registers* registers, string** rob, int& tailIndex, int& index)

{

    inst.state = "issued";

    Station.busy = 1;

    Station.op = inst.type;

    Station.No_cycles = 0;

    if (registers[inst.rs1].ROB_entry == NULL) {

        Station.Vj = inst.rs1;

    }

    else

        Station.Qj = to_string(registers[inst.rs1].ROB_entry);

    if (registers[inst.rs2].ROB_entry == NULL) {

        Station.Vk = inst.rs2;

    }

    else

        Station.Qk = to_string(registers[inst.rs2].ROB_entry);





    Station.dest = tailIndex;

    if (tailIndex <= 8) {

        inst.ROB_ENTRY = tailIndex;

        registers[inst.rd].ROB_entry = tailIndex;

        rob[tailIndex - 1][1] = inst.type;

        rob[tailIndex - 1][2] = to_string(inst.rd); //destination not necessarily rd



        rob[tailIndex - 1][4] = "N";

        tailIndex++;

        if (tailIndex == 9)

            tailIndex = 1;

    }

    index++;

}

void issue_nand(instruction& inst, reservationStation& Station, registers* registers, string** rob, int& tailIndex, int& index)

{

    inst.state = "issued";

    Station.busy = 1;

    Station.op = inst.type;

    Station.No_cycles = 0;

    if (registers[inst.rs1].ROB_entry == NULL) {

        Station.Vj = inst.rs1;

    }

    else

        Station.Qj = to_string(registers[inst.rs1].ROB_entry);

    if (registers[inst.rs2].ROB_entry == NULL) {

        Station.Vk = inst.rs2;

    }

    else

        Station.Qk = to_string(registers[inst.rs2].ROB_entry);





    Station.dest = tailIndex;

    if (tailIndex <= 8) {

        inst.ROB_ENTRY = tailIndex;

        registers[inst.rd].ROB_entry = tailIndex;

        rob[tailIndex - 1][1] = inst.type;

        rob[tailIndex - 1][2] = to_string(inst.rd); //destination not necessarily rd



        rob[tailIndex - 1][4] = "N";

        tailIndex++;

        if (tailIndex == 9)

            tailIndex = 1;

    }

    index++;

}



void print(registers* registers, vector <instruction> inst_table, reservationStation* Stations, string** rob)

{

    cout << "INST_TABLE: " << endl;

    for (int i = 0; i < inst_table.size(); i++)

    {

        cout << "Type: " << inst_table[i].type << endl;

        cout << "rd: " << inst_table[i].rd << endl;

        cout << "rs1: " << inst_table[i].rs1 << endl;

        cout << "rs2: " << inst_table[i].rs2 << endl;

        cout << "imm: " << inst_table[i].imm << endl;

        cout << "state: " << inst_table[i].state << endl;

        cout << "ROB_ENTRY: " << inst_table[i].ROB_ENTRY << endl;

        cout << "PC: " << inst_table[i].PC << endl;

    }

    cout << "ROB:" << endl;

    for (int i = 0; i < 8; i++)

    {

        for (int j = 0; j < 5; j++)

            cout << rob[i][j] << " ";

        cout << endl;

    }





    cout << "RESERVATIONS: " << endl;

    for (int i = 0; i < 15; i++)

    {

        cout << Stations[i].type << "\t" << Stations[i].busy << "\t " << Stations[i].op << "\t " << Stations[i].Vj << " \t" << Stations[i].Vk << "\t " << Stations[i].Qj << "\t "

            << Stations[i].Qk << "\t " << Stations[i].dest << "\t " << Stations[i].A << "\t " << Stations[i].No_cycles << endl; \

    }



}





void execute(reservationStation* Stations, vector <instruction>& inst_table, string** rob, registers* registers, vector <vector<string>> &output, int& noOfCycles)

{

    for (int i = 0; i < 15; i++)

    {

        if ((Stations[i].busy == true) && (Stations[i].Qj == "") && (Stations[i].Qk == "")) //executing condtions: station is executing and isnt waiting for register to be ready

        {

            if (Stations[i].type == "LOAD")

            {

                if (Stations[i].No_cycles == 3) //finished executing

                {


                    for (int j = 0; j < inst_table.size(); j++)

                        if (inst_table[j].ROB_ENTRY == Stations[i].dest)

                        {

                            Stations[i].A = registers[inst_table[j].rs1].value + inst_table[j].imm;

                            inst_table[j].state = "executed";
                            bool entered = false;
                            for (int l = 0; l < output.size(); l++)
                            {
                                if ((inst_table[j].PC == stoi(output[l][2])) && (!entered))
                                {
                                    entered = true;
                                    output[l].push_back(to_string(noOfCycles));
                                }
                            }
                            

                        }

                }

                else

                    Stations[i].No_cycles++;

            }

            else if (Stations[i].type == "STORE") //finished executing

            {

                if (Stations[i].No_cycles == 3)

                {

                    for (int j = 0; j < inst_table.size(); j++)

                        if (inst_table[j].ROB_ENTRY == Stations[i].dest)

                        {

                            Stations[i].A = registers[inst_table[j].rs1].value + inst_table[j].imm;

                            inst_table[j].state = "executed";
                            bool entered = false;
                            for (int l = 0; l < output.size(); l++)
                            {
                                if ((inst_table[j].PC == stoi(output[l][2])) && (!entered))
                                {
                                    entered = true;
                                    output[l].push_back(to_string(noOfCycles));
                                }
                            }
                        }

                }

                else

                    Stations[i].No_cycles++;

            }

            else if (Stations[i].type == "JMP_JAL_RET") //finished executing

            {

                if (Stations[i].No_cycles == 1)

                {

                    for (int j = 0; j < inst_table.size(); j++)

                        if (inst_table[j].ROB_ENTRY == Stations[i].dest)

                        {

                            inst_table[j].state = "executed";
                            bool entered = false;
                            for (int l = 0; l < output.size(); l++)
                            {
                                if ((inst_table[j].PC == stoi(output[l][2])) && (!entered))
                                {
                                    entered = true;
                                    output[l].push_back(to_string(noOfCycles));
                                }
                            }
                        }

                }

                else

                    Stations[i].No_cycles++;

            }

            else if (Stations[i].type == "BEQ") //finished executing

            {

                if (Stations[i].No_cycles == 1)

                {

                    for (int j = 0; j < inst_table.size(); j++)

                        if (inst_table[j].ROB_ENTRY == Stations[i].dest)

                        {

                            inst_table[j].state = "executed";
                            bool entered = false;
                            for (int l = 0; l < output.size(); l++)
                            {
                                if ((inst_table[j].PC == stoi(output[l][2])) && (!entered))
                                {
                                    entered = true;
                                    output[l].push_back(to_string(noOfCycles));
                                }
                            }
                        }

                }

                else

                    Stations[i].No_cycles++;

            }

            else if (Stations[i].type == "ADD/SUB/ADDI") //finished executing

            {

                if (Stations[i].No_cycles == 2)

                {



                    for (int j = 0; j < inst_table.size(); j++)

                        if (inst_table[j].ROB_ENTRY == Stations[i].dest)

                        {

                            inst_table[j].state = "executed";
                            bool entered = false;
                            for (int l = 0; l < output.size(); l++)
                            {
                                if ((inst_table[j].PC == stoi(output[l][2])) && (!entered))
                                {
                                    entered = true;
                                    output[l].push_back(to_string(noOfCycles));
                                }
                            }
                        }

                }

                else

                    Stations[i].No_cycles++;

            }

            else if (Stations[i].type == "NAND") //finished executing

            {
                if (Stations[i].No_cycles == 1)

                {

                    for (int j = 0; j < inst_table.size(); j++)

                        if (inst_table[j].ROB_ENTRY == Stations[i].dest)

                        {

                            inst_table[j].state = "executed";
                            bool entered = false;
                            for (int l = 0; l < output.size(); l++)
                            {
                                if ((inst_table[j].PC == stoi(output[l][2])) && (!entered))
                                {
                                    entered = true;
                                    output[l].push_back(to_string(noOfCycles));
                                }
                            }
                        }

                }

                else

                    Stations[i].No_cycles++;

            }

            else if (Stations[i].type == "MUL") //finished executing

            {

                if (Stations[i].No_cycles == 10)

                {

                    for (int j = 0; j < inst_table.size(); j++)

                        if (inst_table[j].ROB_ENTRY == Stations[i].dest)

                        {

                            inst_table[j].state = "executed";
                            bool entered = false;
                            for (int l = 0; l < output.size(); l++)
                            {
                                if ((inst_table[j].PC == stoi(output[l][2])) && (!entered))
                                {
                                    entered = true;
                                    output[l].push_back(to_string(noOfCycles));
                                }
                            }
                        }

                }

                else

                    Stations[i].No_cycles++;

            }

        }

    }

}



void write(reservationStation* Stations, vector <instruction>& inst_table, string** rob, registers* registers, vector <vector<string>> &output, int& noOfCycles)

{

    for (int i = 0; i < 15; i++)

    {

        if (Stations[i].busy == true) {

            for (int j = 0; j < inst_table.size(); j++) {
                if ((inst_table[j].ROB_ENTRY == Stations[i].dest) && (inst_table[j].state == "executed")) {


                    inst_table[j].state = "written";
                    bool entered = false;
                    for (int l = 0; l < output.size(); l++)
                    {
                        if ((inst_table[j].PC == stoi(output[l][2])) && (!entered))
                        {
                            entered = true;
                            output[l].push_back(to_string(noOfCycles));
                        }
                    }

                    rob[Stations[i].dest - 1][4] = "Y";

                    for (int k = 0; k < 15; k++) {

                        if (Stations[k].Qj == rob[Stations[i].dest - 1][0]) {

                            try { Stations[k].Vj = stoi(rob[Stations[i].dest - 1][2]); }

                            catch (...) { cout << "ERROR"; }

                            Stations[k].Qj = "";

                        }

                        if (Stations[k].Qk == rob[Stations[i].dest - 1][0]) {

                            try { Stations[k].Vk = stoi(rob[Stations[i].dest - 1][2]); }

                            catch (...) { cout << "ERROR"; }

                            Stations[k].Qk = "";

                        }

                    }

                    if (inst_table[j].type == "load") {

                        rob[Stations[i].dest - 1][3] = to_string(dataMemory[Stations[i].A]);
                    }

                    else if (inst_table[j].type == "store") {

                        rob[Stations[i].dest - 1][3] = to_string(Stations[i].A);

                    }

                    else if (inst_table[j].type == "jmp") {

                        rob[Stations[i].dest - 1][3] = to_string(Stations[i].A);

                    }

                    else if (inst_table[j].type == "ret") {

                        rob[Stations[i].dest - 1][3] = to_string(Stations[i].A);

                    }

                    else if (inst_table[j].type == "jal") {

                        rob[Stations[i].dest - 1][3] = to_string(inst_table[j].PC + 4);

                    }

                    else if (inst_table[j].type == "beq") {

                        rob[Stations[i].dest - 1][3] = to_string(Stations[i].A);

                    }

                    else if (inst_table[j].type == "add") {



                        rob[Stations[i].dest - 1][3] = to_string(registers[Stations[i].Vj].value + registers[Stations[i].Vk].value);



                    }

                    else if (inst_table[j].type == "sub") {



                        rob[Stations[i].dest - 1][3] = to_string(registers[Stations[i].Vj].value - registers[Stations[i].Vk].value);



                    }

                    else if (inst_table[j].type == "addi") {



                        rob[Stations[i].dest - 1][3] = to_string(registers[Stations[i].Vj].value + inst_table[j].imm);



                    }

                    else if (inst_table[j].type == "nand") {

                        rob[Stations[i].dest - 1][3] = to_string(!(registers[Stations[i].Vj].value && registers[Stations[i].Vk].value));
     }

                    else if (inst_table[j].type == "mul") {



                        rob[Stations[i].dest - 1][3] = to_string(registers[Stations[i].Vj].value * registers[Stations[i].Vk].value);



                    }
                    Stations[i].busy = 0;

                    Stations[i].op = "";

                    Stations[i].Vj = NULL;

                    Stations[i].Vk = NULL;

                    Stations[i].Qj = "";

                    Stations[i].Qk = "";

                    Stations[i].dest = NULL;

                    Stations[i].A = NULL;

                    Stations[i].No_cycles = NULL;

                }

            }

        }

    }





}

void commit(vector <instruction>& inst_table, string** rob, registers* registers, int& headIndex, int& currententries, int& index, int& tailIndex, reservationStation* Stations, vector <vector<string>> &output, int& noOfCycles, double &branches, double &miss)

{
 if (rob[headIndex - 1][4] == "Y") {

        if ((rob[headIndex - 1][1] != "jmp") && (rob[headIndex - 1][1] != "ret") && (rob[headIndex - 1][1] != "store"))

        {

            registers[stoi(rob[headIndex - 1][2])].value = stoi(rob[headIndex - 1][3]);

            registers[stoi(rob[headIndex - 1][2])].ROB_entry = NULL;

        }

        if ((rob[headIndex - 1][1] == "store")) {

            dataMemory[stoi(rob[headIndex - 1][3])] = registers[stoi(rob[headIndex - 1][2])].value;

        }

        if ((rob[headIndex - 1][1] == "beq")) {
            branches++;
            if (rob[headIndex - 1][2] == "0") {
                miss++;
                for (int j = 0; j < inst_table.size(); j++) {

                    if (stoi(rob[headIndex - 1][0]) == inst_table[j].ROB_ENTRY) {

                        inst_table[j].state = "committed";
                        bool entered = false;
                        for (int l = 0; l < output.size(); l++)
                        {
                            if ((inst_table[j].PC == stoi(output[l][2])) && (!entered))
                            {
                                entered = true;
                                output[l].push_back(to_string(noOfCycles));
                            }
                        }
                    }
                }

                while (index != ((stoi(rob[headIndex - 1][3])) / 4) - 1) {

                    inst_table[index].state = "";

                    index--;

                }

                for (int r = 0; r < 8; r++) {

                    rob[r][1] = "";

                    rob[r][2] = "";

                    rob[r][3] = "";

                    rob[r][4] = "N";

                }

                headIndex = 1;

                tailIndex = 1;

                currententries = 0;

                for (int rs = 0; rs < 15; rs++) {

                    Stations[rs].busy = 0;

                    Stations[rs].op = "";

                    Stations[rs].Vj = NULL;

                    Stations[rs].Vk = NULL;

                    Stations[rs].Qj = "";

                    Stations[rs].Qk = "";

                    Stations[rs].dest = NULL;

                    Stations[rs].A = NULL;

                    Stations[rs].No_cycles = NULL;



                }

                for (int reg = 0; reg < 8; reg++) {

                    registers[reg].ROB_entry = NULL;

                }

                for (int a = 0; a < inst_table.size(); a++) {

                    if (inst_table[a].ROB_ENTRY == stoi(rob[headIndex - 1][0])) {

                        index = inst_table[a].PC / 4 + 2;

                    }

                }

            }

            else

            {



                currententries--;

                rob[headIndex - 1][1] = "";

                rob[headIndex - 1][2] = "";

                rob[headIndex - 1][3] = "";

                rob[headIndex - 1][4] = "N";

                for (int j = 0; j < inst_table.size(); j++) {

                    if (stoi(rob[headIndex - 1][0]) == inst_table[j].ROB_ENTRY) {

                        inst_table[j].state = "committed";
                        bool entered = false;
                        for (int l = 0; l < output.size(); l++)
                        {
                            if ((inst_table[j].PC == stoi(output[l][2])) && (!entered))
                            {
                                entered = true;
                                output[l].push_back(to_string(noOfCycles));
                            }
                        }
                    }
                }

                headIndex++;

            }

        }

        else

        {

            currententries--;

            rob[headIndex - 1][1] = "";

            rob[headIndex - 1][2] = "";

            rob[headIndex - 1][3] = "";

            rob[headIndex - 1][4] = "N";

            for (int j = 0; j < inst_table.size(); j++) {

                if (stoi(rob[headIndex - 1][0]) == inst_table[j].ROB_ENTRY) {

                    inst_table[j].state = "committed";
                    bool entered = false;
                            for (int l = 0; l < output.size(); l++)
                            {
                                if ((inst_table[j].PC == stoi(output[l][2])) && (!entered))
                                {
                                    entered = true;
                                    output[l].push_back(to_string(noOfCycles));
                                }
                            }
                }

            }

            headIndex++;

        }

    }

}





bool check_if_rob_is_empty(string** rob)

{

    bool empty = true;

    for (int i = 0; i < 8; i++)

        if (rob[i][1] != "")

            empty = false;

    return empty;

}

void printResults(vector<vector<string>> &output,int &no_of_cycles, double &branches, double &miss)

{
    cout << "INSTRUCTION" << "    " << "ISSUED" << "\t\t" << "EXECUTED" << "\t" << "WRITTEN" << "\t\t" << "COMMITTED" << endl;
    for (int i = 0; i < output.size(); i++)
    {
        for (int j = 0; j < output[i].size(); j++)
        {
            if(j != 2)
                cout << output[i][j] << "     \t\t";
        }
        cout << endl;
    }
    cout << endl;
    cout << "IPC : " << (double)output.size() / (double)no_of_cycles << endl;

    if(branches == 0)
        cout << "BRANCH MISPREDICITON: " << 0 << endl;
    else
        cout << "BRANCH MISPREDICITON: " << miss / branches << endl;
}

void tomasulo(vector<string> instructions)

{

    int NUMBER_OF_CYCLES = 0;

    int headIndex = 1;

    int tailIndex = 1;

    string** rob = ROB(headIndex, tailIndex);

    vector <instruction> inst_table = instruction_table(instructions);

    reservationStation* Stations = reservation_stations();

    registers* registers = regs();

    vector <vector<string>> output;

    int maxEntries = 8;

    int currentEntries = 0;

    int index = 0;

    double branches = 0;

    double miss = 0;







    while ((index < inst_table.size()) || check_if_rob_is_empty(rob) == false)

    {

        commit(inst_table, rob, registers, headIndex, currentEntries, index, tailIndex, Stations, output, NUMBER_OF_CYCLES,branches,miss);

        write(Stations, inst_table, rob, registers, output, NUMBER_OF_CYCLES);



        execute(Stations, inst_table, rob, registers, output, NUMBER_OF_CYCLES);



        if (index < inst_table.size())

        {

            issue(inst_table[index], Stations, registers, rob, currentEntries, tailIndex, index, output, NUMBER_OF_CYCLES);



        }

        registers[0].value = 0;

        registers[0].ROB_entry = NULL;
        NUMBER_OF_CYCLES++;
    }
    printResults(output,NUMBER_OF_CYCLES,branches,miss);
}
