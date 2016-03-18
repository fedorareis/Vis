/*
 * Copyright (C) 2006-2013  Music Technology Group - Universitat Pompeu Fabra
 *
 * This file is part of Essentia
 *
 * Essentia is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation (FSF), either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * version 3 along with this program.  If not, see http://www.gnu.org/licenses/
 */

#include <iostream>
#include "Audio3.hpp"

using namespace std;
using namespace essentia;
using namespace essentia::standard;

#define SAMPLE_RATE 44100

std::vector<vector<Real>>* Audio3::audio3(std::string audioFilename, std::string outputFilename)
{
    // register the algorithms in the factory(ies)
    essentia::init();
    
    Pool pool;
    
    /////// PARAMS //////////////
    int frameSize = 2048;
    int hopSize = 1024;
    
    // we want to compute the MFCC of a file: we need the create the following:
    // audioloader -> framecutter -> windowing -> FFT -> MFCC
    
    AlgorithmFactory& factory = standard::AlgorithmFactory::instance();
    
    Algorithm* audio = factory.create("MonoLoader",
                                      "filename", audioFilename,
                                      "sampleRate", SAMPLE_RATE);
    
    Algorithm* fc    = factory.create("FrameCutter",
                                      "frameSize", frameSize,
                                      "hopSize", hopSize);
    
    Algorithm* w     = factory.create("Windowing",
                                      "type", "blackmanharris62");
    
    Algorithm* lext   = factory.create("LevelExtractor",
                                       "frameSize", frameSize,
                                       "hopSize", hopSize);
    
    Algorithm* autoCor = factory.create("AutoCorrelation");
    
    Algorithm* spec  = factory.create("Spectrum");
    Algorithm* mfcc  = factory.create("MFCC");
    Algorithm* loud  = factory.create("Loudness");
    
    
    
    
    /////////// CONNECTING THE ALGORITHMS ////////////////
    cout << "-------- connecting algos ---------" << endl;
    
    // Audio -> FrameCutter
    
    audio->output("audio").set(audioBuffer);
    fc->input("signal").set(audioBuffer);
    
    // FrameCutter -> Windowing -> Spectrum
    vector<Real> frame, windowedFrame;
    Real loudness;
    
    fc->output("frame").set(frame);
    w->input("frame").set(frame);
    
    w->output("frame").set(windowedFrame);
    spec->input("frame").set(windowedFrame);
    
    loud->input("signal").set(windowedFrame);
    loud->output("loudness").set(loudness);
    
    // Spectrum -> MFCC
    vector<Real> spectrum, mfccCoeffs, mfccBands, LoudnessExt, Correlation;
    
    autoCor->input("array").set(windowedFrame);
    autoCor->output("autoCorrelation").set(Correlation);
    
    lext->input("signal").set(windowedFrame);
    lext->output("loudness").set(LoudnessExt);
    
    spec->output("spectrum").set(spectrum);
    mfcc->input("spectrum").set(spectrum);
    
    mfcc->output("bands").set(mfccBands);
    mfcc->output("mfcc").set(mfccCoeffs);
    
    
    
    /////////// STARTING THE ALGORITHMS //////////////////
    cout << "-------- start processing " << audioFilename << " --------" << endl;
    
    audio->compute();
    
    while (true) {
        
        // compute a frame
        fc->compute();
        
        // if it was the last one (ie: it was empty), then we're done.
        if (!frame.size()) {
            break;
        }
        
        w->compute();
        autoCor->compute();
        loud->compute();
        //lext->compute();
        spec->compute();
        mfcc->compute();
        
        //cout << spectrum << endl;
        specOut.push_back(spectrum);
        volume.push_back(loudness);
        
        //cout << specOut << "\n\n\n\n" << endl;
    }
    
    delete audio;
    delete fc;
    delete w;
    delete spec;
    delete mfcc;
    
    essentia::shutdown();
    
    //return volume;
    return &specOut;
}

std::vector<Real> Audio3::getVolume()
{
    //cout << volume << endl;
    return volume;
}

void Audio3::loadBuffer()
{
    //std::vector<short> audio(audioBuffer.begin(), audioBuffer.end());
    sf::SoundBuffer buffer;
    std::vector<sf::Int16> samples(audioBuffer.begin(), audioBuffer.end());
    buffer.loadFromSamples(&samples[0], samples.size(), 2, SAMPLE_RATE);
    //buffer.loadFromMemory(&samples[0], samples.size());
    sound.setBuffer(buffer);
}

void Audio3::play()
{
    //sound.play();
    //music.play();
}