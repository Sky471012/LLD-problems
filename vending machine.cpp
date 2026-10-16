#include <iostream>
#include <string>
#include <unordered_map>
#include <stdexcept>
using namespace std;

class Product {
public:
    string id;
    string name;
    int price; // Rupees

    Product(string id, string name, int price)
        : id(id), name(name), price(price) {}
};

class Inventory {
    struct Item {
        Product product;
        int quantity;

        Item(Product p, int q) : product(p), quantity(q) {}
    };

    unordered_map<string, Item> items;

public:
    void addProduct(Product p, int quantity) {
        items.insert_or_assign(
            p.id, Item(p, quantity)
        );
    }

    bool isAvailable(string id) {
        auto it = items.find(id);
        return it != items.end() && it->second.quantity > 0;
    }

    Product getProduct(string id) {
        auto it = items.find(id);

        if (it == items.end())
            throw runtime_error("Product not found");

        return it->second.product;
    }

    bool dispense(string id) {
        auto it = items.find(id);

        if (it == items.end() || it->second.quantity == 0)
            return false;

        it->second.quantity--;
        return true;
    }
};

class Payment {
    int balance = 0;

public:
    void insertMoney(int amount) {
        if (amount <= 0)
            throw invalid_argument("Invalid amount");

        balance += amount;
    }

    int getBalance() const {
        return balance;
    }

    bool hasEnough(int price) const {
        return balance >= price;
    }

    int collectPayment() {
        int amount = balance;
        balance = 0;
        return amount;
    }

    int refund() {
        int amount = balance;
        balance = 0;
        return amount;
    }
};

class VendingMachine {
    Inventory inventory;
    Payment payment;

public:
    void addProduct(Product p, int quantity) {
        inventory.addProduct(p, quantity);
    }

    void insertMoney(int amount) {
        payment.insertMoney(amount);
        cout << "Balance: Rs. "
             << payment.getBalance() << '\n';
    }

    void selectProduct(string id) {
        if (!inventory.isAvailable(id)) {
            cout << "Product unavailable\n";
            return;
        }

        Product product = inventory.getProduct(id);

        if (!payment.hasEnough(product.price)) {
            cout << "Insufficient money. Price: Rs. "
                 << product.price << '\n';
            return;
        }

        // Validate availability and payment before dispensing.
        int paid = payment.collectPayment();

        if (!inventory.dispense(id)) {
            cout << "Dispensing failed. Refund: Rs. "
                 << paid << '\n';
            return;
        }

        cout << "Dispensed: " << product.name << '\n';
        cout << "Change: Rs. "
             << paid - product.price << '\n';
    }

    void cancelTransaction() {
        cout << "Refund: Rs. "
             << payment.refund() << '\n';
    }
};

int main() {
    VendingMachine machine;

    machine.addProduct(
        Product("A1", "Chips", 20), 5
    );
    machine.addProduct(
        Product("B1", "Cola", 40), 3
    );

    machine.insertMoney(50);
    machine.selectProduct("B1");

    machine.insertMoney(10);
    machine.cancelTransaction();
}


/*
classDiagram
    class Product {
        +string id
        +string name
        +int price
    }

    class Inventory {
        -unordered_map~string, Item~ items
        +addProduct(p, quantity) void
        +isAvailable(id) bool
        +getProduct(id) Product
        +dispense(id) bool
    }

    class Item {
        +Product product
        +int quantity
    }

    class Payment {
        -int balance
        +insertMoney(amount) void
        +getBalance() int
        +hasEnough(price) bool
        +collectPayment() int
        +refund() int
    }

    class VendingMachine {
        -Inventory inventory
        -Payment payment
        +addProduct(p, quantity) void
        +insertMoney(amount) void
        +selectProduct(id) void
        +cancelTransaction() void
    }

    Inventory *-- Item : stores
    Item --> Product : contains
    VendingMachine *-- Inventory : owns
    VendingMachine *-- Payment : owns
    VendingMachine ..> Product : selects

    

    Encapsulation: Inventory and payment balances are private.
    Single Responsibility Principle: Product storage, payment handling, and machine coordination are separated.
    Composition: VendingMachine contains an Inventory and a Payment object.

    Where does the State Pattern fit?
    This is a particularly important LLD interview question.
    A real vending machine may have states such as:
    - Idle
    - ProductSelected
    - PaymentPending
    - Dispensing
    In the current implementation, these transitions are handled using methods and conditions. As the rules grow, we could introduce a State Pattern so each state controls which operations are valid.
    For example, inserting money while the machine is dispensing should behave differently from inserting money while it's idle.
*/