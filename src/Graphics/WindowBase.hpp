#pragma once

#include <string>
#include <thread>
#pragma once

#include <string>
#include <thread>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>
#include <optional>
#include <iostream>
#include <chrono>
#include <SFML/Graphics.hpp>

class WindowBase {
protected:
    unsigned int width;
    unsigned int height;
    std::string title;

    std::unique_ptr<sf::RenderWindow> win;
    std::atomic<bool> running{false};
    std::thread winThread;
    std::mutex winMutex;
    std::function<void()> onClose;

public:
    WindowBase(const std::string& title, unsigned int width, unsigned int height)
        : title(title), width(width), height(height) {}

    virtual ~WindowBase() {
        try {
            stop();
            if (winThread.joinable()) {
                if (std::this_thread::get_id() == winThread.get_id()) {
                    try { winThread.detach(); } catch (...) {}
                } else {
                    try { winThread.join(); } catch (...) {}
                }
            }
        } catch (...) {}
    }

    virtual void proceedEvent(const sf::Event& event) {}
    virtual void draw() {}

    void run() {
        bool expected = false;
        if (!running.compare_exchange_strong(expected, true)) return;
        winThread = std::thread(&WindowBase::loop, this);
    }

    void stop() {
        // Signal the loop to stop and ensure the thread is joined/detached
        running.store(false);
        if (winThread.joinable()) {
            if (std::this_thread::get_id() == winThread.get_id()) {
                // calling stop from the window thread itself: detach to avoid joining self
                try { winThread.detach(); } catch (...) {}
            } else {
                // wait for the window thread to finish
                try { winThread.join(); } catch (...) {}
            }
        }
    }

    bool isRunning() const { return running.load(); }

    void setOnClose(std::function<void()> cb) {
        onClose = std::move(cb);
    }

private:
    void loop() {
        try {
            {
                std::lock_guard<std::mutex> lk(winMutex);
                win = std::make_unique<sf::RenderWindow>(sf::VideoMode({width, height}), title);
            }

            while (running.load()) {
                std::optional<sf::Event> eventOpt;
                bool closedRequested = false;

                {
                    std::lock_guard<std::mutex> lk(winMutex);
                    if (!win || !win->isOpen()) break;

                    while ((eventOpt = win->pollEvent())) {
                        const sf::Event& ev = *eventOpt;
                        if (ev.is<sf::Event::Closed>()) { // SFML3 style
                            closedRequested = true;
                            break;
                        }
                        try {
                            proceedEvent(ev);
                        } catch (const std::exception& e) {
                            std::cerr << "Exception in proceedEvent: " << e.what() << std::endl;
                        } catch (...) {
                            std::cerr << "Unknown exception in proceedEvent" << std::endl;
                        }
                    }

                    try {
                        win->clear(sf::Color::Black);
                        draw();
                        win->display();
                    } catch (const std::exception& e) {
                        std::cerr << "Exception during draw/display: " << e.what() << std::endl;
                    } catch (...) {
                        std::cerr << "Unknown exception during draw/display" << std::endl;
                    }
                }

                if (closedRequested) {
                    // ensure we call onClose outside of the winMutex to avoid deadlocks
                    if (onClose) {
                        try {
                            onClose();
                        } catch (const std::exception& e) {
                            std::cerr << "Exception in onClose callback: " << e.what() << std::endl;
                        } catch (...) {
                            std::cerr << "Unknown exception in onClose callback" << std::endl;
                        }
                    }

                    // let the UI thread close the window (we are in UI thread here), so close it
                    std::lock_guard<std::mutex> lk(winMutex);
                    if (win && win->isOpen()) {
                        running.store(false);
                        win->close();
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            {
                std::lock_guard<std::mutex> lk(winMutex);
                if (win && win->isOpen()) win->close();
                win.reset();
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception in WindowBase::loop: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown exception in WindowBase::loop" << std::endl;
        }
    }
};