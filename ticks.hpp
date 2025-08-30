#pragma once

#include <iostream>
#include <stdexcept>
#include <optional>

class MidiTicks {
private:
    std::optional<int> ticks; // MIDI Ticks, can be null

public:
    // Default Constructor
    // Initializes ticks to null (no value)
    MidiTicks() : ticks(std::nullopt) {}

    // Parameterized Constructor
    // Allows initializing ticks with a specific value
    MidiTicks(int initialTicks) {
        setTicks(initialTicks); // Use setter to leverage validation
    }

    // Getter for ticks
    // Returns the current ticks value, which may be null
    std::optional<int> getTicks() const {
        return ticks;
    }

    // Setter for ticks with a value
    // Sets the ticks to a new value after validating it
    void setTicks(int newTicks) {
        if (newTicks >= 0) {
            ticks = newTicks;
        } else {
            throw std::invalid_argument("MIDI ticks must be a non-negative integer.");
        }
    }

    // Setter to set ticks to null
    void setNull() {
        ticks.reset();
    }

    // operator= Shorthand for setter
    MidiTicks& operator=(int newTicks) {
        setTicks(newTicks);
        return *this;
    }

    // Equality operators
    bool operator==(const MidiTicks& other) const {
        return ticks == other.ticks;
    }

    bool operator==(const int& otherTicks) const {
        return ticks == otherTicks;
    }

    bool operator!=(const MidiTicks& other) const {
        return !(*this == other);
    }

    bool operator!=(const int& otherTicks) const {
        return ticks != otherTicks;
    }

    // isNull Method
    // Returns true if ticks is null (std::nullopt), false otherwise
    bool isNull() const {
        return !ticks.has_value();
    }

    // has_value Method
    // Returns true if ticks is not null, i.e., has a value
    bool has_value() {
        return ticks.has_value();
    }

    // Optional: Method to display the current ticks
    void display() const {
        if (ticks.has_value()) {
            std::cout << "Current MIDI Ticks: " << ticks.value() << std::endl;
        } else {
            std::cout << "Current MIDI Ticks: null" << std::endl;
        }
    }
};
