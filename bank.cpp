#include "bank.h"
#include <iostream>

std::string Bank::freeze_cell(int num) {
    if(num < 0 || num >= bankSize) {
        return "invalid index";
    }

    cells[num].freeze();

    return "Cell number" + std::to_string(num + 1) + "successfully frozen";
}

std::string Bank::unfreeze_cell(int num) {
    if(num < 0 || num >= bankSize) {
        return "invalid index";
    }

    cells[num].unfreeze();

    return "Cell number" + std::to_string(num + 1) + "successfully unfrozen";
}

std::string Bank::transfer(int a, int b, int amount) {
    if(a < 0 || a >= bankSize || b < 0 || b >= bankSize) {
        return "invalid index";
    }

    if(amount < 0) {
        return "invalid amount";
    }

    if(cells[a].is_frozen() || cells[b].is_frozen()) {
        return "one of the cells is frozen";
    }

    if(cells[a].send_amount(amount)) {
        if(cells[b].receive_amount(amount)) {
            return "successfully transferred";
        }

        cells[a].receive_amount(amount);
    }

    return "one of the cells is unable to complete the operation";
}

std::string Bank::add_to_all(int amount) {
    if(amount < 0) {
        return "invalid amount";
    }

    int index = 0;
    for( ; index < bankSize; ++index) {
        if(cells[index].is_frozen() || !cells[index].receive_amount(amount) ) {
            for(int i = 0; i < index; ++i) {
                cells[i].send_amount(amount);
            }

            return "one of the cells is unable to complete the operation";
        }
    }

    return "successfully added";
}

std::string Bank::sub_from_all(int amount) {
    if(amount < 0) {
        return "invalid amount";
    }

    int index = 0;
    for( ; index < bankSize; ++index) {
        if(cells[index].is_frozen() || !cells[index].send_amount(amount)) {
            for(int i = 0; i < index; ++i) {
                cells[i].receive_amount(amount);
            }

            return "one of the cells is unable to complete the operation";
        }
    }

    return "successfully subed";
}

std::string Bank::set_cell_min_amount(int num, int amount) {
    if(num < 0 || num >= bankSize) {
        return "invalid index";
    }

    if(amount > cells[num].get_curr_balance() || amount >= cells[num].get_max_balance() || cells[num].is_frozen()) {
        return "invalid amount";
    }

    cells[num].set_min_amount(amount);

    return "successfully changes the min amount";
}

std::string Bank::set_cell_max_amount(int num, int amount) {
    if(num < 0 || num >= bankSize) {
        return "invalid index";
    }

    if(amount < cells[num].get_curr_balance() || amount <= cells[num].get_min_balance() || cells[num].is_frozen()) {
        return "invalid amount";
    }

    cells[num].set_max_amount(amount);

    return "successfully changes the max amount";}

std::string Bank::get_info(int num) {
    if(num < 0 || num >= bankSize) {
        return "";
    }

    std::string res = "curr: ";
    res = res + std::to_string(cells[num].get_curr_balance()) + " | min: ";
    res = res + std::to_string(cells[num].get_min_balance()) + " | max: ";
    res = res + std::to_string(cells[num].get_max_balance()) + " | frozen: ";

    if(cells[num].is_frozen()) {
        res += "true";
    }
    else {
        res += "false";
    }

    return res;
}

BankCell& Bank::operator[](unsigned int ind)
{
    return cells[ind];
}
