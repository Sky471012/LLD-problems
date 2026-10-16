#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <algorithm>
using namespace std;

class User {
    string id;
    string name;

public:
    User(string id, string name)
        : id(id), name(name) {}

    string getId() const { return id; }
    string getName() const { return name; }
};

class Expense {
public:
    string id;
    string description;
    string paidBy;
    int amount;
    vector<string> participants;

    Expense(string id, string description, string paidBy,
            int amount, vector<string> participants)
        : id(id), description(description), paidBy(paidBy),
          amount(amount), participants(participants) {}
};

class Group {
    string id;
    string name;

    unordered_map<string, User> members;
    vector<Expense> expenses;

    // balances[debtor][creditor] = amount owed
    map<string, map<string, int>> balances;
    int nextExpenseId = 1;

    void addDebt(string debtor, string creditor, int amount) {
        if (debtor == creditor || amount <= 0)
            return;

        int reverseDebt = balances[creditor][debtor];
        int offset = min(amount, reverseDebt);

        balances[creditor][debtor] -= offset;
        amount -= offset;

        if (amount > 0)
            balances[debtor][creditor] += amount;
    }

public:
    Group(string id, string name)
        : id(id), name(name) {}

    string getId() const { return id; }
    string getName() const { return name; }

    void addMember(const User& user) {
        members.insert_or_assign(user.getId(), user);
    }

    bool hasMember(const string& userId) const {
        return members.find(userId) != members.end();
    }

    void addExpense(string description, string payer,
                    int amount, vector<string> participants) {
        if (amount <= 0 || participants.empty())
            throw invalid_argument("Invalid expense amount or participants");

        if (!hasMember(payer))
            throw invalid_argument("Payer is not a group member");

        unordered_set<string> seen;

        for (const auto& participant : participants) {
            if (!hasMember(participant))
                throw invalid_argument(
                    "Expense participant is not a group member");

            if (!seen.insert(participant).second)
                throw invalid_argument(
                    "Duplicate expense participant");
        }

        // Record the expense in THIS group.
        expenses.emplace_back(
            to_string(nextExpenseId++), description,
            payer, amount, participants
        );

        int count = static_cast<int>(participants.size());
        int share = amount / count;
        int remainder = amount % count;

        for (int i = 0; i < count; ++i) {
            int personShare = share + (i < remainder ? 1 : 0);

            if (participants[i] != payer) {
                addDebt(participants[i], payer, personShare);
            }
        }

        cout << "Expense added to group: " << name << '\n';
    }

    bool settleDebt(string debtor, string creditor, int amount) {
        if (amount <= 0 || !hasMember(debtor) ||
            !hasMember(creditor) || debtor == creditor)
            return false;

        auto person = balances.find(debtor);

        if (person == balances.end())
            return false;

        auto debt = person->second.find(creditor);

        if (debt == person->second.end() || debt->second < amount)
            return false;

        debt->second -= amount;

        if (debt->second == 0)
            person->second.erase(debt);

        return true;
    }

    void showBalances() const {
        cout << "\nBalances for group: " << name << '\n';
        bool found = false;

        for (const auto& [debtor, creditors] : balances) {
            for (const auto& [creditor, amount] : creditors) {
                if (amount > 0) {
                    cout << debtor << " owes " << creditor
                         << " Rs. " << amount << '\n';
                    found = true;
                }
            }
        }

        if (!found)
            cout << "All balances are settled\n";
    }

    void showExpenses() const {
        cout << "\nExpenses for group: " << name << '\n';

        for (const auto& expense : expenses) {
            cout << expense.id << ". " << expense.description
                 << " | Paid by: " << expense.paidBy
                 << " | Amount: Rs. " << expense.amount << '\n';
        }
    }
};

class ExpenseManager {
    unordered_map<string, Group> groups;

public:
    void createGroup(string id, string name) {
        auto [it, inserted] =
            groups.emplace(id, Group(id, name));

        if (!inserted)
            throw invalid_argument("Group ID already exists");

        cout << "Group created: " << name << '\n';
    }

    Group& getGroup(const string& groupId) {
        auto it = groups.find(groupId);

        if (it == groups.end())
            throw invalid_argument("Group not found");

        return it->second;
    }

    void addExpense(string groupId, string description,
                    string payer, int amount,
                    vector<string> participants) {
        getGroup(groupId).addExpense(
            description, payer, amount, participants
        );
    }

    void settleDebt(string groupId, string debtor,
                    string creditor, int amount) {
        Group& group = getGroup(groupId);

        if (group.settleDebt(debtor, creditor, amount))
            cout << "Settlement recorded in "
                 << group.getName() << '\n';
        else
            cout << "Invalid settlement for this group\n";
    }

    void showBalances(string groupId) {
        getGroup(groupId).showBalances();
    }

    void showExpenses(string groupId) {
        getGroup(groupId).showExpenses();
    }
};

int main() {
    User alice("A", "Alice");
    User bob("B", "Bob");
    User charlie("C", "Charlie");

    ExpenseManager manager;

    // Create two independent groups.
    manager.createGroup("G1", "Flatmates");
    manager.createGroup("G2", "Goa Trip");

    // Add members to each group.
    manager.getGroup("G1").addMember(alice);
    manager.getGroup("G1").addMember(bob);
    manager.getGroup("G1").addMember(charlie);

    manager.getGroup("G2").addMember(alice);
    manager.getGroup("G2").addMember(bob);

    // Flatmates: Alice pays Rs. 900 for three people.
    manager.addExpense(
        "G1", "Groceries", "A", 900, {"A", "B", "C"}
    );

    // Goa Trip: Bob pays Rs. 600 shared by Alice and Bob.
    manager.addExpense(
        "G2", "Hotel", "B", 600, {"A", "B"}
    );

    manager.showBalances("G1");
    manager.showBalances("G2");

    // Bob settles Rs. 100 of his debt to Alice in Flatmates.
    manager.settleDebt("G1", "B", "A", 100);

    manager.showBalances("G1");
    manager.showExpenses("G1");
}


/*


classDiagram
    class User {
        -string id
        -string name
        +getId() string
        +getName() string
    }

    class Expense {
        +string id
        +string description
        +string paidBy
        +int amount
        +vector~string~ participants
    }

    class Group {
        -string id
        -string name
        -unordered_map~string, User~ members
        -vector~Expense~ expenses
        -map~string, map~string, int~~ balances
        +addMember(user) void
        +addExpense(description, payer, amount, participants) void
        +settleDebt(debtor, creditor, amount) bool
        +showBalances() void
    }

    class ExpenseManager {
        -unordered_map~string, Group~ groups
        +createGroup(id, name) void
        +getGroup(id) Group
        +addExpense(groupId, description, payer, amount, participants) void
        +settleDebt(groupId, debtor, creditor, amount) void
        +showBalances(groupId) void
    }

    ExpenseManager *-- Group : manages
    Group o-- User : has members
    Group *-- Expense : records group expenses
    Group ..> User : validates membership


    Composition: ExpenseManager manages multiple Group objects, and each Group contains its own expenses.
    SRP (Single Responsibility Principle): ExpenseManager coordinates group-level operations, while Group manages membership, expenses, and group-specific balances.
    Encapsulation: Group members, expenses, and balances are private. Operations such as addExpense() and settleDebt() control how they are modified.
    Data structures: unordered_map provides group and member lookups, while nested map structures track debts between users.
    Net debt calculation: Opposing debts are offset so the system tracks the net amount owed between two users.
    Strategy Pattern — not yet implemented: The current code supports only equal expense splitting. Exact-amount, percentage, and share-based splits could be added through a common SplitStrategy interface.
*/