#include <iostream>
#include "App.h"
#include "Timer.h"


void App::networkLoop()
{
    std::array<uint8_t, 2048> tmp;
    Timer timer;

    while (running_) {
        if (!server_.isConnected()) {
            server_.connect(port_);
            timer.reset();
        }


        size_t n = server_.receive(tmp.data(), tmp.size());
        if (n < 0) { server_.disconnect(); continue; }
        if (n == 0) { /* optional small sleep */ continue; }

        parser_.pushBytes(tmp.data(), n);

        CanFrame f;
        while (parser_.tryPop(f, timer.elapsed())) {
            frame_queue_.push(f);
        }
    }
}

App::App(unsigned short port)
    : port_(port)
{ 
    ui_.init();
}

App::~App()
{
    bool expected = true;
    if (!running_.compare_exchange_strong(expected, false))
        return; // already stopped

    server_.disconnect();

    if (net_thread_.joinable())
        net_thread_.join();
}

void App::run()
{
    running_.store(true);
    net_thread_ = std::thread(&App::networkLoop, this);

    while (ui_.window_.isOpen())
    {
        // Event handling
        while (const auto event = ui_.window_.pollEvent())
        {
            ImGui::SFML::ProcessEvent(ui_.window_, *event);
            if (event->is<sf::Event::Closed>())
            {
                ui_.window_.close();
            }
        }

        ImGui::SFML::Update(ui_.window_, ui_.deltaClock_.restart());

        constexpr size_t MAX_FRAMES_PER_TICK = 20;
        size_t drained = frame_queue_.drain(MAX_FRAMES_PER_TICK, [&](const CanFrame& f) {model_.ingest(f);});

        ui_.render(model_);   
      
    }

    // Cleanup
    ImGui::SFML::Shutdown();
}



