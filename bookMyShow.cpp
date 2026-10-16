
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
using namespace std;

// ---------- Enums ----------

enum class SeatStatus {
    Available,
    Booked
};

enum class BookingStatus {
    Confirmed,
    Cancelled
};

// ---------- User ----------

class User {
    string id;
    string name;

public:
    User(string id, string name)
        : id(id), name(name) {}

    string getId() const { return id; }
    string getName() const { return name; }
};

// ---------- Movie ----------

class Movie {
    string id;
    string title;
    int durationMinutes;

public:
    Movie(string id, string title, int durationMinutes)
        : id(id), title(title), durationMinutes(durationMinutes) {}

    string getId() const { return id; }
    string getTitle() const { return title; }
    int getDuration() const { return durationMinutes; }
};

// ---------- Physical Seat ----------

class Seat {
    string id;
    int price;

public:
    Seat(string id, int price) : id(id), price(price) {
        if (price <= 0)
            throw invalid_argument("Seat price must be positive");
    }

    string getId() const { return id; }
    int getPrice() const { return price; }
};

// ---------- Screen ----------

class Screen {
    string id;
    vector<Seat> seats;

public:
    Screen(string id) : id(id) {}

    string getId() const { return id; }

    void addSeat(string seatId, int price) {
        for (const auto& seat : seats) {
            if (seat.getId() == seatId)
                throw invalid_argument("Duplicate seat ID");
        }

        seats.emplace_back(seatId, price);
    }

    const vector<Seat>& getSeats() const {
        return seats;
    }

    const Seat* findSeat(const string& seatId) const {
        for (const auto& seat : seats) {
            if (seat.getId() == seatId)
                return &seat;
        }

        return nullptr;
    }
};

// ---------- Cinema ----------

class Cinema {
    string id;
    string name;
    vector<unique_ptr<Screen>> screens;

public:
    Cinema(string id, string name)
        : id(id), name(name) {}

    string getId() const { return id; }
    string getName() const { return name; }

    Screen& addScreen(string screenId) {
        for (const auto& screen : screens) {
            if (screen->getId() == screenId)
                throw invalid_argument("Duplicate screen ID");
        }

        screens.push_back(make_unique<Screen>(screenId));
        return *screens.back();
    }

    Screen* findScreen(const string& screenId) {
        for (auto& screen : screens) {
            if (screen->getId() == screenId)
                return screen.get();
        }

        return nullptr;
    }
};

// ---------- Show ----------

class Show {
    string id;
    const Movie* movie;
    Screen* screen;
    string showTime;

    // Availability is different for every show.
    unordered_map<string, SeatStatus> seatStatus;

public:
    Show(string id, const Movie* movie,
         Screen* screen, string showTime)
        : id(id), movie(movie),
          screen(screen), showTime(showTime) {

        if (movie == nullptr || screen == nullptr)
            throw invalid_argument("Show needs a movie and screen");

        // Initialize availability from physical seats.
        for (const auto& seat : screen->getSeats())
            seatStatus.emplace(seat.getId(), SeatStatus::Available);
    }

    string getId() const { return id; }
    string getMovieTitle() const { return movie->getTitle(); }
    string getShowTime() const { return showTime; }
    string getScreenId() const { return screen->getId(); }

    const Seat* findSeat(const string& seatId) const {
        return screen->findSeat(seatId);
    }

    bool isSeatAvailable(const string& seatId) const {
        auto it = seatStatus.find(seatId);

        return it != seatStatus.end() &&
               it->second == SeatStatus::Available;
    }

    bool bookSeat(const string& seatId) {
        if (!isSeatAvailable(seatId))
            return false;

        seatStatus.at(seatId) = SeatStatus::Booked;
        return true;
    }

    void releaseSeat(const string& seatId) {
        auto it = seatStatus.find(seatId);

        if (it != seatStatus.end())
            it->second = SeatStatus::Available;
    }

    void showAvailableSeats() const {
        cout << "\n" << movie->getTitle()
             << " | Screen " << screen->getId()
             << " | " << showTime << '\n';

        bool found = false;

        for (const auto& seat : screen->getSeats()) {
            if (isSeatAvailable(seat.getId())) {
                cout << seat.getId() << " (Rs. "
                     << seat.getPrice() << ")  ";
                found = true;
            }
        }

        if (!found)
            cout << "No seats available";

        cout << '\n';
    }
};

// ---------- Payment Service ----------

class PaymentService {
public:
    bool pay(int amount) {
        if (amount <= 0)
            return false;

        // Simulates a successful payment gateway.
        cout << "Payment processed: Rs. " << amount << '\n';
        return true;
    }

    bool refund(int amount) {
        if (amount <= 0)
            return false;

        // Simulates a successful refund.
        cout << "Refund processed: Rs. " << amount << '\n';
        return true;
    }
};

// ---------- Booking / Ticket ----------

class Booking {
    string id;
    string userId;
    string showId;
    vector<string> seatIds;
    int totalAmount;
    BookingStatus status;

public:
    Booking(string id, string userId, string showId,
            vector<string> seatIds, int totalAmount)
        : id(id), userId(userId), showId(showId),
          seatIds(seatIds), totalAmount(totalAmount),
          status(BookingStatus::Confirmed) {}

    string getId() const { return id; }
    string getShowId() const { return showId; }
    int getTotalAmount() const { return totalAmount; }
    BookingStatus getStatus() const { return status; }

    const vector<string>& getSeatIds() const {
        return seatIds;
    }

    void cancel() {
        status = BookingStatus::Cancelled;
    }

    void printTicket(const Show& show) const {
        cout << "\n========== BOOKING TICKET ==========\n";
        cout << "Booking ID : " << id << '\n';
        cout << "User ID    : " << userId << '\n';
        cout << "Movie      : " << show.getMovieTitle() << '\n';
        cout << "Screen     : " << show.getScreenId() << '\n';
        cout << "Show time  : " << show.getShowTime() << '\n';

        cout << "Seats      : ";
        for (const auto& seatId : seatIds)
            cout << seatId << " ";

        cout << "\nTotal      : Rs. " << totalAmount << '\n';
        cout << "====================================\n";
    }
};

// ---------- Booking Manager ----------

class BookingManager {
    // BookingManager owns each Show.
    unordered_map<string, unique_ptr<Show>> shows;

    // BookingManager stores booking records.
    unordered_map<string, Booking> bookings;

    PaymentService paymentService;
    int nextBookingId = 1;

public:
    void addShow(unique_ptr<Show> show) {
        string id = show->getId();

        auto [it, inserted] =
            shows.emplace(id, move(show));

        if (!inserted)
            throw invalid_argument("Duplicate show ID");
    }

    void showAvailableSeats(const string& showId) const {
        auto it = shows.find(showId);

        if (it == shows.end()) {
            cout << "Show not found\n";
            return;
        }

        it->second->showAvailableSeats();
    }

    string bookSeats(string userId, string showId,
                     const vector<string>& seatIds) {
        auto showIt = shows.find(showId);

        if (showIt == shows.end()) {
            cout << "Show not found\n";
            return "";
        }

        if (userId.empty() || seatIds.empty()) {
            cout << "User and seat selection are required\n";
            return "";
        }

        Show& show = *showIt->second;

        // Reject duplicate seat selections.
        unordered_set<string> uniqueSeats(
            seatIds.begin(), seatIds.end()
        );

        if (uniqueSeats.size() != seatIds.size()) {
            cout << "Duplicate seat selection\n";
            return "";
        }

        int total = 0;

        // Validate every seat before charging the user.
        for (const auto& seatId : seatIds) {
            if (!show.isSeatAvailable(seatId)) {
                cout << "Seat unavailable: " << seatId << '\n';
                return "";
            }

            const Seat* seat = show.findSeat(seatId);

            if (seat == nullptr) {
                cout << "Seat not found: " << seatId << '\n';
                return "";
            }

            total += seat->getPrice();
        }

        // Process payment before confirming the booking.
        if (!paymentService.pay(total)) {
            cout << "Payment failed\n";
            return "";
        }

        // Mark the selected seats as booked.
        // This example assumes a single-threaded simulation.
        for (const auto& seatId : seatIds) {
            if (!show.bookSeat(seatId)) {
                // Defensive rollback for this simplified example.
                for (const auto& rollbackId : seatIds)
                    show.releaseSeat(rollbackId);

                paymentService.refund(total);

                cout << "Booking failed; payment refunded\n";
                return "";
            }
        }

        string bookingId = "B" + to_string(nextBookingId++);

        bookings.emplace(
            piecewise_construct,
            forward_as_tuple(bookingId),
            forward_as_tuple(
                bookingId, userId, showId, seatIds, total
            )
        );

        bookings.at(bookingId).printTicket(show);
        return bookingId;
    }

    void cancelBooking(const string& bookingId) {
        auto bookingIt = bookings.find(bookingId);

        if (bookingIt == bookings.end()) {
            cout << "Booking not found\n";
            return;
        }

        Booking& booking = bookingIt->second;

        if (booking.getStatus() == BookingStatus::Cancelled) {
            cout << "Booking already cancelled\n";
            return;
        }

        auto showIt = shows.find(booking.getShowId());

        if (showIt == shows.end()) {
            cout << "Show not found\n";
            return;
        }

        // Refund first. Keep the booking and seats unchanged
        // if the simulated refund fails.
        if (!paymentService.refund(booking.getTotalAmount())) {
            cout << "Refund failed; cancellation not completed\n";
            return;
        }

        Show& show = *showIt->second;

        for (const auto& seatId : booking.getSeatIds())
            show.releaseSeat(seatId);

        booking.cancel();

        cout << "Booking " << bookingId
             << " cancelled successfully\n";
    }
};

// ---------- Demo ----------

int main() {
    User alice("U1", "Alice");
    User bob("U2", "Bob");

    Movie movie("M1", "Interstellar", 169);

    Cinema cinema("C1", "PVR");
    Screen& screen1 = cinema.addScreen("Screen1");

    screen1.addSeat("A1", 200);
    screen1.addSeat("A2", 200);
    screen1.addSeat("A3", 300);

    // Two screenings on the SAME physical screen.
    auto show2PM = make_unique<Show>(
        "S1", &movie, &screen1, "2:00 PM"
    );

    auto show6PM = make_unique<Show>(
        "S2", &movie, &screen1, "6:00 PM"
    );

    BookingManager manager;
    manager.addShow(move(show2PM));
    manager.addShow(move(show6PM));

    manager.showAvailableSeats("S1");
    manager.showAvailableSeats("S2");

    // Alice books A1 for the 2 PM show.
    string bookingId =
        manager.bookSeats(alice.getId(), "S1", {"A1", "A2"});

    // A1 remains available for the 6 PM show.
    manager.showAvailableSeats("S1");
    manager.showAvailableSeats("S2");

    // Bob cannot book A1 for the same 2 PM show.
    manager.bookSeats(bob.getId(), "S1", {"A1"});

    // Cancellation refunds payment and releases seats.
    if (!bookingId.empty())
        manager.cancelBooking(bookingId);

    manager.showAvailableSeats("S1");
}


/*
classDiagram
    class User {
        -string id
        -string name
    }

    class Movie {
        -string id
        -string title
        -int durationMinutes
    }

    class Cinema {
        -string id
        -string name
        -vector~Screen*~ screens
        +addScreen(screen) void
    }

    class Screen {
        -string id
        -vector~Seat~ seats
        +addSeat(seat) void
        +getSeatIds() vector
    }

    class Seat {
        -string id
        -int price
        +getId() string
        +getPrice() int
    }

    class Show {
        -string id
        -Movie* movie
        -Screen* screen
        -string showTime
        -unordered_map~string, SeatStatus~ seatStatus
        +getAvailableSeats() vector
        +bookSeat(seatId) bool
        +releaseSeat(seatId) void
    }

    class Booking {
        -string id
        -string userId
        -string showId
        -vector~string~ seatIds
        -int totalAmount
        -BookingStatus status
    }

    class PaymentService {
        +pay(amount) bool
        +refund(amount) bool
    }

    class BookingManager {
        -unordered_map~string, Show*~ shows
        -unordered_map~string, Booking~ bookings
        +addShow(show) void
        +bookSeats(userId, showId, seatIds) string
        +cancelBooking(bookingId) void
    }

    Cinema o-- Screen : has screens
    Screen *-- Seat : owns physical seats
    Show --> Movie : screens
    Show --> Screen : scheduled on
    Show --> Seat : tracks per-show availability
    BookingManager --> Show : coordinates
    BookingManager *-- Booking : stores
    Booking --> User : booked by
    Booking --> Show : for
    BookingManager --> PaymentService : uses


    SRP: Show manages show-specific seat availability, PaymentService handles payment processing, and BookingManager coordinates bookings.
    Encapsulation: Seat status is private and changed through book() and release().
    Composition: A Show contains its own seat objects, so availability is independent for each show.
    State modeling: SeatStatus and BookingStatus represent seat and booking lifecycle states. The formal State Pattern is not implemented.
    Factory Pattern — not implemented: Booking IDs are generated directly by BookingManager; a separate factory isn't needed for this simple example.
    Strategy Pattern — not implemented: Pricing and payment behavior are fixed. A PricingStrategy could support different seat categories, discounts, or dynamic pricing.
*/