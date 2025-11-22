#pragma once

#include <string>
#include <thread>
#include <memory>
#include <atomic>
#include <mutex>
#include <SFML/Graphics.hpp>
#include <functional>

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

    ~WindowBase() {
        stop();
    }

    virtual void proceedEvent(const sf::Event& event) {}

    virtual void draw() {}

private:
    void loop() {
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
                    const sf::Event& event = *eventOpt;
                    if (event.is<sf::Event::Closed>()) {
                        closedRequested = true;
                        break;
                    }
                    proceedEvent(event);
                }

                win->clear(sf::Color::Black);
                draw();
                win->display();
            }

            if (closedRequested) {
                if (onClose) {
                    onClose();
                } else {
                    std::lock_guard<std::mutex> lk(winMutex);
                    if (win && win->isOpen()) {
                        running.store(false);
                        win->close();
                    }
                }
            }

            sf::sleep(sf::milliseconds(10));
        }

        {
            std::lock_guard<std::mutex> lk(winMutex);
            if (win && win->isOpen()) win->close();
            win.reset();
        }
    }

public:
    void run() {
        bool expected = false;
        if (!running.compare_exchange_strong(expected, true)) return;
        winThread = std::thread(&WindowBase::loop, this);
    }

    void stop() {
        bool expected = true;
        if (!running.compare_exchange_strong(expected, false)) return;

        if (winThread.joinable()) {
            if (std::this_thread::get_id() == winThread.get_id()) {
                try {
                    winThread.detach();
                } catch (...) {}
            } else {
                winThread.join();
            }
        }
    }

    bool isRunning() const {
        return running.load();
    }

    void setOnClose(std::function<void()> cb) {
        onClose = std::move(cb);
    }
};