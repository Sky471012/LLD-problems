#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <chrono>
#include <unordered_map>
#include <stdexcept>
using namespace std;

enum class VehicleType { Motorcycle, Car, Truck };
enum class SpotType { Motorcycle, Compact, Large };

string vehicleName(VehicleType type) {
    switch (type) {
        case VehicleType::Motorcycle: return "Motorcycle";
        case VehicleType::Car: return "Car";
        case VehicleType::Truck: return "Truck";
    }
    return "Unknown";
}

class Vehicle {
    string number;
    VehicleType type;

public:
    Vehicle(string number, VehicleType type)
        : number(number), type(type) {}

    string getNumber() const { return number; }
    VehicleType getType() const { return type; }
};

class ParkingSpot {
    int id;
    SpotType type;
    shared_ptr<Vehicle> vehicle;

public:
    ParkingSpot(int id, SpotType type)
        : id(id), type(type) {}

    int getId() const { return id; }
    bool isFree() const { return !vehicle; }

    bool canFit(VehicleType type) const {
        if (type == VehicleType::Motorcycle)
            return true;

        if (type == VehicleType::Car)
            return typeFitsCar();

        return this->type == SpotType::Large;
    }

private:
    bool typeFitsCar() const {
        return type == SpotType::Compact ||
               type == SpotType::Large;
    }

public:
    bool park(shared_ptr<Vehicle> v) {
        if (!isFree() || !canFit(v->getType()))
            return false;

        vehicle = v;
        return true;
    }

    void removeVehicle() {
        vehicle.reset();
    }
};

class ParkingFloor {
    int floorNumber;
    vector<ParkingSpot> spots;

public:
    ParkingFloor(int number) : floorNumber(number) {}

    void addSpot(int id, SpotType type) {
        spots.emplace_back(id, type);
    }

    ParkingSpot* findSpot(VehicleType type) {
        for (auto& spot : spots) {
            if (spot.isFree() && spot.canFit(type))
                return &spot;
        }
        return nullptr;
    }

    int getFloorNumber() const { return floorNumber; }

    ParkingSpot* getSpot(int id) {
        for (auto& spot : spots)
            if (spot.getId() == id)
                return &spot;

        return nullptr;
    }
};

class Ticket {
public:
    int id;
    string vehicleNumber;
    int floorNumber;
    int spotId;
    chrono::system_clock::time_point entryTime;

    Ticket(int id, string number, int floor, int spot)
        : id(id), vehicleNumber(number),
          floorNumber(floor), spotId(spot),
          entryTime(chrono::system_clock::now()) {}
};

class FeeCalculator {
public:
    double calculate(const Ticket& ticket) {
        auto now = chrono::system_clock::now();
        auto seconds =
            chrono::duration_cast<chrono::seconds>(
                now - ticket.entryTime).count();

        double hours = max(1.0, seconds / 3600.0);
        return hours * 30.0; // Simplified: ₹30 per hour
    }
};

class ParkingLot {
    vector<ParkingFloor> floors;
    unordered_map<int, Ticket> tickets;
    int nextTicketId = 1;
    FeeCalculator feeCalculator;

public:
    void addFloor(ParkingFloor floor) {
        floors.push_back(move(floor));
    }

    int parkVehicle(shared_ptr<Vehicle> vehicle) {
        for (auto& floor : floors) {
            ParkingSpot* spot =
                floor.findSpot(vehicle->getType());

            if (spot && spot->park(vehicle)) {
                int id = nextTicketId++;

                tickets.emplace(
                    piecewise_construct,
                    forward_as_tuple(id),
                    forward_as_tuple(
                        id, vehicle->getNumber(),
                        floor.getFloorNumber(), spot->getId()
                    )
                );

                cout << "Parked on floor "
                     << floor.getFloorNumber()
                     << ", spot " << spot->getId() << '\n';

                return id;
            }
        }

        cout << "No compatible spot available\n";
        return -1;
    }

    void exitVehicle(int ticketId) {
        auto it = tickets.find(ticketId);

        if (it == tickets.end()) {
            cout << "Invalid ticket\n";
            return;
        }

        Ticket& ticket = it->second;
        double fee = feeCalculator.calculate(ticket);

        for (auto& floor : floors) {
            if (floor.getFloorNumber() == ticket.floorNumber) {
                ParkingSpot* spot = floor.getSpot(ticket.spotId);
                if (spot)
                    spot->removeVehicle();
                break;
            }
        }

        cout << "Fee: Rs. " << fee << '\n';
        tickets.erase(it);
    }
};

int main() {
    ParkingLot lot;

    ParkingFloor floor0(0);
    floor0.addSpot(1, SpotType::Motorcycle);
    floor0.addSpot(2, SpotType::Compact);
    floor0.addSpot(3, SpotType::Large);

    lot.addFloor(move(floor0));

    auto car = make_shared<Vehicle>("DL01AB1234",
                                    VehicleType::Car);

    int ticketId = lot.parkVehicle(car);

    if (ticketId != -1)
        lot.exitVehicle(ticketId);
}



/*
classDiagram
    class Vehicle {
        -string number
        -VehicleType type
        +getNumber() string
        +getType() VehicleType
    }

    class ParkingSpot {
        -int id
        -SpotType type
        -shared_ptr~Vehicle~ vehicle
        +getId() int
        +isFree() bool
        +canFit(type) bool
        +park(vehicle) bool
        +removeVehicle() void
        -typeFitsCar() bool
    }

    class ParkingFloor {
        -int floorNumber
        -vector~ParkingSpot~ spots
        +addSpot(id, type) void
        +findSpot(type) ParkingSpot*
        +getFloorNumber() int
        +getSpot(id) ParkingSpot*
    }

    class Ticket {
        +int id
        +string vehicleNumber
        +int floorNumber
        +int spotId
        +time_point entryTime
    }

    class FeeCalculator {
        +calculate(ticket) double
    }

    class ParkingLot {
        -vector~ParkingFloor~ floors
        -unordered_map~int, Ticket~ tickets
        -int nextTicketId
        -FeeCalculator feeCalculator
        +addFloor(floor) void
        +parkVehicle(vehicle) int
        +exitVehicle(ticketId) void
    }

    ParkingLot "1" *-- "*" ParkingFloor : owns
    ParkingFloor "1" *-- "*" ParkingSpot : owns
    ParkingSpot --> Vehicle : parks
    ParkingLot --> Ticket : manages
    ParkingLot --> FeeCalculator : uses
    FeeCalculator ..> Ticket : calculates fee from


    Encapsulation: Spot occupancy is private and changed through methods.
    Composition: A floor owns its spots.
    SRP: Fee calculation is separated from parking management.
    Polymorphism: Not necessary in every class; this simple version uses an enum for vehicle types. A larger version could use vehicle subclasses or pluggable policies.
*/

