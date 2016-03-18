//
//  Audio2.hpp
//  Visualizer
//
//  Created by Kyle Reis on 3/1/16.
//  Copyright © 2016 FedoraReis. All rights reserved.
//
#pragma once

#ifndef Audio3_hpp
#define Audio3_hpp

#include <essentia/algorithmfactory.h>
#include <essentia/streaming/algorithms/poolstorage.h>
#include <essentia/scheduler/network.h>
#include <SFML/Audio.hpp>

class Audio3
{
public:
    std::vector<std::vector<essentia::Real>>* audio3(std::string audioFilename, std::string outputFilename);
    std::vector<essentia::Real> getVolume();
    void loadBuffer();
    void play();
private:
    std::vector<essentia::Real> audioBuffer;
    sf::Sound sound;
    std::vector<std::vector<essentia::Real>> specOut;
    std::vector<essentia::Real> volume;
};

#endif /* Audio3_hpp */