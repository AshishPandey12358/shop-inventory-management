#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include "sqlite3.h"

using namespace std;

// ================= DATABASE =================
class Database {
    sqlite3* db;

public:
    Database() {
        sqlite3_open("shop.db", &db);

        const char* sql =
            "CREATE TABLE IF NOT EXISTS items ("
            "category TEXT,"
            "name TEXT PRIMARY KEY,"
            "count INTEGER);";

        sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
    }

    void saveCategory(const string& category, const map<string, int>& items) {
        string del = "DELETE FROM items WHERE category='" + category + "';";
        sqlite3_exec(db, del.c_str(), nullptr, nullptr, nullptr);

        for (auto& it : items) {
            string sql =
                "INSERT INTO items VALUES('" + category + "','" +
                it.first + "'," + to_string(it.second) + ");";
            sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
        }
    }

    void loadCategory(const string& category, map<string, int>& items) {
        string sql =
            "SELECT name, count FROM items WHERE category='" + category + "';";
        sqlite3_stmt* stmt;

        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            string name = (const char*)sqlite3_column_text(stmt, 0);
            int count = sqlite3_column_int(stmt, 1);
            items[name] = count;
        }
        sqlite3_finalize(stmt);
    }

    ~Database() {
        sqlite3_close(db);
    }
};

// ================= SHOP =================
class Shop {
    map<string, int> vegetables;
    map<string, int> fruits;
    Database db;

public:
    Shop() {
        db.loadCategory("vegetable", vegetables);
        db.loadCategory("fruit", fruits);
    }

    void addItem(const string& category, const string& name, int count) {
        if (category == "vegetable")
            vegetables[name] += count;
        else
            fruits[name] += count;

        persist(category);
    }

    void sellItem(const string& name) {
        if (vegetables.count(name)) {
            if (--vegetables[name] == 0)
                vegetables.erase(name);
            persist("vegetable");
            return;
        }
        if (fruits.count(name)) {
            if (--fruits[name] == 0)
                fruits.erase(name);
            persist("fruit");
            return;
        }
        cout << "Item not found\n";
    }

    void removeItem(const string& name) {
        if (vegetables.erase(name)) {
            persist("vegetable");
            return;
        }
        if (fruits.erase(name)) {
            persist("fruit");
            return;
        }
        cout << "Item not found\n";
    }

    void display(const string& category) {
        map<string, int>& items =
            (category == "vegetable") ? vegetables : fruits;

        if (items.empty()) {
            cout << "No items\n";
            return;
        }

        vector<pair<string, int>> sorted(items.begin(), items.end());
        sort(sorted.begin(), sorted.end(),
             [](auto& a, auto& b) {
                 return a.second > b.second;
             });

        for (auto& it : sorted)
            cout << it.first << "\t" << it.second << endl;
    }

private:
    void persist(const string& category) {
        if (category == "vegetable")
            db.saveCategory(category, vegetables);
        else
            db.saveCategory(category, fruits);
    }
};

// ================= MAIN =================
int main() {
    Shop shop;
    int ch, cnt;
    string name;

    do {
        cout << "\n====== SHOP SYSTEM (STL VERSION) ======\n";
        cout << "1. Add Vegetable\n";
        cout << "2. Add Fruit\n";
        cout << "3. Display Vegetables\n";
        cout << "4. Display Fruits\n";
        cout << "5. Sell Item\n";
        cout << "6. Remove Item Completely\n";
        cout << "7. Exit\n";
        cout << "Choice: ";
        cin >> ch;

        switch (ch) {
        case 1:
            cin >> name >> cnt;
            shop.addItem("vegetable", name, cnt);
            break;
        case 2:
            cin >> name >> cnt;
            shop.addItem("fruit", name, cnt);
            break;
        case 3:
            shop.display("vegetable");
            break;
        case 4:
            shop.display("fruit");
            break;
        case 5:
            cin >> name;
            shop.sellItem(name);
            break;
        case 6:
            cin >> name;
            shop.removeItem(name);
            break;
        }
    } while (ch != 7);

    return 0;
}
