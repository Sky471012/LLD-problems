
#include <iostream>
#include <vector>
#include <set>
#include <limits>
#include <cstdlib>
#include <stdexcept>
using namespace std;

enum class Direction {
    Up,
    Down,
    Idle
};

enum class ElevatorStatus {
    Idle,
    Moving,
    DoorOpen
};

class Elevator {
    int id;
    int currentFloor;
    int minFloor;
    int maxFloor;
    Direction direction = Direction::Idle;
    ElevatorStatus status = ElevatorStatus::Idle;
    set<int> stops;

public:
    Elevator(int id, int initialFloor, int minFloor, int maxFloor)
        : id(id), currentFloor(initialFloor),
          minFloor(minFloor), maxFloor(maxFloor) {
        if (minFloor > maxFloor ||
            initialFloor < minFloor ||
            initialFloor > maxFloor) {
            throw invalid_argument("Invalid elevator floors");
        }
    }

    int getId() const {
        return id;
    }

    int getCurrentFloor() const {
        return currentFloor;
    }

    ElevatorStatus getStatus() const {
        return status;
    }

    bool isIdle() const {
        return status == ElevatorStatus::Idle && stops.empty();
    }

    bool servesFloor(int floor) const {
        return floor >= minFloor && floor <= maxFloor;
    }

    void addStop(int floor) {
        if (!servesFloor(floor))
            throw invalid_argument("Floor outside elevator range");

        stops.insert(floor);
    }

    void step() {
        // A door-open state lasts for one simulation step.
        if (status == ElevatorStatus::DoorOpen)
            status = ElevatorStatus::Moving;

        // Stop here if this floor is requested.
        auto currentStop = stops.find(currentFloor);

        if (currentStop != stops.end()) {
            stops.erase(currentStop);
            direction = Direction::Idle;
            status = ElevatorStatus::DoorOpen;

            cout << "Elevator " << id
                 << ": doors open at floor "
                 << currentFloor << '\n';
            return;
        }

        if (stops.empty()) {
            direction = Direction::Idle;
            status = ElevatorStatus::Idle;
            return;
        }

        // Simple policy: move toward the nearest requested floor.
        int target = *stops.begin();

        for (int floor : stops) {
            if (abs(floor - currentFloor) <
                abs(target - currentFloor)) {
                target = floor;
            }
        }

        if (target > currentFloor) {
            currentFloor++;
            direction = Direction::Up;
        } else if (target < currentFloor) {
            currentFloor--;
            direction = Direction::Down;
        }

        status = ElevatorStatus::Moving;

        cout << "Elevator " << id
             << " moved to floor " << currentFloor << '\n';

        if (currentFloor == target) {
            stops.erase(target);
            direction = Direction::Idle;
            status = ElevatorStatus::DoorOpen;

            cout << "Elevator " << id
                 << ": doors open at floor "
                 << currentFloor << '\n';
        }
    }
};

class ElevatorSystem {
    vector<Elevator> elevators;

public:
    ElevatorSystem(int count, int minFloor, int maxFloor) {
        if (count <= 0 || minFloor > maxFloor)
            throw invalid_argument("Invalid elevator system");

        for (int i = 0; i < count; i++) {
            elevators.emplace_back(
                i + 1, minFloor, minFloor, maxFloor
            );
        }
    }

    int requestPickup(int floor) {
        for (const auto& elevator : elevators) {
            if (!elevator.servesFloor(floor))
                throw invalid_argument("Invalid pickup floor");
        }

        Elevator* best = nullptr;
        int bestDistance = numeric_limits<int>::max();

        // Prefer an idle elevator.
        for (auto& elevator : elevators) {
            if (!elevator.isIdle())
                continue;

            int distance =
                abs(elevator.getCurrentFloor() - floor);

            if (distance < bestDistance) {
                best = &elevator;
                bestDistance = distance;
            }
        }

        // If none is idle, choose the nearest elevator.
        if (best == nullptr) {
            for (auto& elevator : elevators) {
                int distance =
                    abs(elevator.getCurrentFloor() - floor);

                if (distance < bestDistance) {
                    best = &elevator;
                    bestDistance = distance;
                }
            }
        }

        best->addStop(floor);

        cout << "Pickup at floor " << floor
             << " assigned to elevator "
             << best->getId() << '\n';

        return best->getId();
    }

    void selectDestination(int elevatorId, int floor) {
        for (auto& elevator : elevators) {
            if (elevator.getId() == elevatorId) {
                elevator.addStop(floor);

                cout << "Destination floor " << floor
                     << " added to elevator "
                     << elevatorId << '\n';
                return;
            }
        }

        throw invalid_argument("Elevator not found");
    }

    void stepAll() {
        for (auto& elevator : elevators)
            elevator.step();
    }
};

int main() {
    ElevatorSystem system(2, 0, 10);

    // A passenger requests an elevator on floor 5.
    int elevatorId = system.requestPickup(5);

    // Passenger enters and selects floor 8.
    system.selectDestination(elevatorId, 8);

    // Simulate elevator movement.
    for (int i = 0; i < 10; i++) {
        system.stepAll();
    }
}



/*

classDiagram
    class ElevatorSystem {
        -vector~Elevator~ elevators
        +requestPickup(currentFloor) int
        +selectDestination(elevatorId, floor) void
        +stepAll() void
    }

    class Elevator {
        -int id
        -int currentFloor
        -Direction direction
        -ElevatorStatus status
        -set~int~ stops
        +addStop(floor) void
        +step() void
        +getCurrentFloor() int
        +getStatus() ElevatorStatus
        +isIdle() bool
    }

    class Direction {
        <<enumeration>>
        Up
        Down
        Idle
    }

    class ElevatorStatus {
        <<enumeration>>
        Idle
        Moving
        DoorOpen
    }

    ElevatorSystem *-- Elevator : manages
    Elevator --> Direction : tracks
    Elevator --> ElevatorStatus : tracks


    SRP: Elevator handles movement and stops; ElevatorSystem assigns pickup requests and coordinates elevators.
    Encapsulation: Elevator state and requested stops are private and changed through methods.
    Composition: ElevatorSystem manages its elevator objects.
    State modeling: ElevatorStatus and Direction represent different aspects of elevator behavior.
    State Pattern — not implemented: The current code uses enums and conditionals. A full State Pattern implementation would give states their own classes and behavior.
    Strategy Pattern — not implemented: The dispatch policy is currently hardcoded. A DispatchStrategy interface could support different elevator-assignment algorithms.
*/