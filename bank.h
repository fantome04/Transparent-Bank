
#ifndef BANK
#define BANK

#include "bank_cell.h"
#include <vector>
#include <string>
class BankCell;

class Bank{
public:
    std::string freeze_cell(int num);
    std::string unfreeze_cell(int num);
    std::string transfer(int a, int b, int amount);
    std::string add_to_all(int amount);
    std::string sub_from_all(int amount);
    std::string set_cell_min_amount(int num, int amount);
    std::string set_cell_max_amount(int num, int amount);
    std::string get_info(int num);

    int bankSize;
    BankCell cells[];

    BankCell& operator[](unsigned int ind);
};


#endif
