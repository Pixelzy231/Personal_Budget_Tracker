#define _WIN32_WINNT 0x0A00
#include "httplib.h"
#include <iostream>
#include <string>


template <typename T>
void swapValues(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
}

void sortExpenses(std::string* names, double* amounts, int count) {
   for(int i = 0; i < count; i++) {
    for(int j = 0; j < count - i; j++ ){
      if(amounts[j] < amounts[j+1]) {
        swapValues(amounts[j], amounts[j+1]);
        swapValues(names[j], names[j+1]);
      }
    }
   }
}


void addExpense(std::string*& names, double*& amounts, int& count, const std::string& newName, double newAmount) {
    std::string* newNames = new std::string[count + 1];
    double* newAmounts = new double[count + 1];
    for(int i = 0; i < count; i++){
      newNames[i] = names[i];
      newAmounts[i] = amounts[i];
    }
    newNames[count] = newName;
    newAmounts[count] = newAmount; 
   delete[] names;
   delete[] amounts;
   names = newNames;
   amounts = newAmounts;
   count++;
}


double* findExpense(const std::string* names, double* amounts, int count, const std::string& target) {
for (int i = 0; i < count; i++) {
        if (names[i] == target) {
            return &amounts[i];
        }
    }
    return nullptr;
}


void updateIncome(double& currentIncome, double newIncome) {
   currentIncome = newIncome;
}

double getTotalSpent(const double* amounts, int count) {
    double total = 0.0;
    for (int i = 0; i < count; i++) {
        total += amounts[i];
    }
    return total;
}


double projectSavings(double balance, double rate, int months) {
if (months <= 0) {
        return balance;
    }
    return projectSavings(balance * (1.0 + rate), rate, months - 1);
}

std::string buildStateJSON(const std::string* names, const double* amounts, int count, double income) {
    double spent = getTotalSpent(amounts, count);
    std::string json = "{\"income\":" + std::to_string(income) +
                       ",\"spent\":" + std::to_string(spent) +
                       ",\"balance\":" + std::to_string(income - spent) +
                       ",\"expenses\":[";
    for (int i = 0; i < count; i++) {
        json += "{\"name\":\"" + names[i] + "\",\"amount\":" + std::to_string(amounts[i]) + "}";
        if (i < count - 1) json += ",";
    }
    json += "]}";
    return json;
}

int main() {
    double income = 0.0;
    int count = 0;
    std::string* names = nullptr;
    double* amounts = nullptr;

    httplib::Server svr;
    svr.set_mount_point("/", "./public");

    svr.Get("/api/state", [&](const httplib::Request&, httplib::Response& res) {
        res.set_content(buildStateJSON(names, amounts, count, income), "application/json");
    });

    svr.Get("/api/income", [&](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("val")) {
            updateIncome(income, std::stod(req.get_param_value("val")));
        }
        res.set_content(buildStateJSON(names, amounts, count, income), "application/json");
    });

    svr.Get("/api/add", [&](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("name") && req.has_param("amount")) {
            addExpense(names, amounts, count, req.get_param_value("name"), std::stod(req.get_param_value("amount")));
        }
        res.set_content(buildStateJSON(names, amounts, count, income), "application/json");
    });

    svr.Get("/api/sort", [&](const httplib::Request&, httplib::Response& res) {
        sortExpenses(names, amounts, count);
        res.set_content(buildStateJSON(names, amounts, count, income), "application/json");
    });

    svr.Get("/api/edit", [&](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("name") && req.has_param("amount")) {
            double* pricePtr = findExpense(names, amounts, count, req.get_param_value("name"));
            if (pricePtr != nullptr) {
                *pricePtr = std::stod(req.get_param_value("amount"));
                res.set_content(buildStateJSON(names, amounts, count, income), "application/json");
                return;
            }
        }
        res.status = 404;
        res.set_content("{\"error\":\"Expense not found\"}", "application/json");
    });

    svr.Get("/api/project", [&](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("balance") && req.has_param("rate") && req.has_param("months")) {
            double future = projectSavings(
                std::stod(req.get_param_value("balance")),
                std::stod(req.get_param_value("rate")),
                std::stoi(req.get_param_value("months"))
            );
            res.set_content("{\"projected\":" + std::to_string(future) + "}", "application/json");
        }
    });

    std::cout << "Budget Tracker API running at http://localhost:8080\n";
    svr.listen("localhost", 8080);

    delete[] names;
    delete[] amounts;
    return 0;
}