//
//  speaker.c
//  A2Mac
//
//  Created by Tamas Rudnai on 5/9/20.
//  Copyright © 2020 GameAlloy. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include "speaker.h"
#include "6502.h"


#define min(x,y) (x) < (y) ? (x) : (y)
#define max(x,y) (x) > (y) ? (x) : (y)
#define clamp(min,num,max) (num) < (min) ? (min) : (num) > (max) ? (max) : (num)


#define CASE_RETURN(err) case (err): return #err
const char* al_err_str(ALenum err) {
    switch(err) {
            CASE_RETURN(AL_NO_ERROR);
            CASE_RETURN(AL_INVALID_NAME);
            CASE_RETURN(AL_INVALID_ENUM);
            CASE_RETURN(AL_INVALID_VALUE);
            CASE_RETURN(AL_INVALID_OPERATION);
            CASE_RETURN(AL_OUT_OF_MEMORY);
    }
    printf("alError: 0x%04X\n", err);
    return "unknown";
}
#undef CASE_RETURN

#define __al_check_error(file,line) \
    do { \
        ALenum err = alGetError(); \
        for(; err != AL_NO_ERROR; err = alGetError()) { \
            printf( "AL Error %s at %s:%d\n", al_err_str(err), file, line ); \
        } \
    } while(0)

#define al_check_error() \
    __al_check_error(__FILE__, __LINE__)


ALCdevice *dev = NULL;
ALCcontext *ctx = NULL;
ALuint spkr_buf = 0;
ALuint spkr_src = 0;


int spkr_level = SPKR_LEVEL_ZERO;


#define BUFFER_COUNT 10
#define SOURCES_COUNT 1

ALuint spkr_buffers[BUFFER_COUNT];


const int spkr_fps = fps;
const int spkr_seconds = 1;
const unsigned spkr_sample_rate = 44100;
unsigned spkr_extra_buf = 780 / fps;
const unsigned spkr_buf_size = spkr_seconds * spkr_sample_rate * 2 / spkr_fps;
int16_t spkr_samples [ spkr_buf_size * spkr_fps * BUFFER_COUNT * 2]; // stereo
unsigned spkr_sample_idx = 0;

const unsigned spkr_play_timeout = 8;
unsigned spkr_play_time = 0;


// initialize OpenAL
void spkr_init() {
    const char *defname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    printf( "Default device: %s\n", defname );

    // restart OpenAL when restarting the virtual machine
    spkr_exit();
    
    dev = alcOpenDevice(defname);
    ctx = alcCreateContext(dev, NULL);
    alcMakeContextCurrent(ctx);
    
    // Fill buffer with zeros
    memset( spkr_samples, spkr_level, spkr_buf_size * sizeof(spkr_samples[0]) );
    
    // Create buffer to store samples
    alGenBuffers(BUFFER_COUNT, spkr_buffers);
    al_check_error();
    
    // Set-up sound source and play buffer
    alGenSources(1, &spkr_src);
    al_check_error();
    alSourcei(spkr_src, AL_LOOPING, AL_FALSE);
    al_check_error();
    alSourcef(spkr_src, AL_ROLLOFF_FACTOR, 0);
    al_check_error();
    alSource3f(spkr_src, AL_POSITION, 0.0, 8.0, 0.0);
    al_check_error();
    alListener3f(AL_POSITION, 0.0, 0.0, 0.0);
    al_check_error();
    alListener3f(AL_ORIENTATION, 0.0, -16.0, 0.0);
    al_check_error();

    // start from the beginning
    spkr_sample_idx = 0;

    // make sure we have free buffers initialized here
    freeBuffers = BUFFER_COUNT;
}

// Dealloc OpenAL
void spkr_exit() {
    if ( spkr_src ) {
        ALCdevice *dev = NULL;
        ALCcontext *ctx = NULL;
        ctx = alcGetCurrentContext();
        dev = alcGetContextsDevice(ctx);
        
        alcMakeContextCurrent(NULL);
        alcDestroyContext(ctx);
        alcCloseDevice(dev);
        
        al_check_error();
        
        spkr_src = 0;
    }
}


void spkr_toggle() {
    // TODO: This is very slow!
    //            printf("io_KBDSTRB\n");
    
    spkr_play_time = spkr_play_timeout;
    
    // push a click into the speaker buffer
    // (we will play the entire buffer at the end of the frame)
    spkr_sample_idx = (clkfrm / (default_MHz_6502 / spkr_sample_rate)) * 2;
    
    if ( spkr_level > SPKR_LEVEL_ZERO ) {
        // down edge
        while( (spkr_level = (spkr_level + SPKR_LEVEL_MIN) / 2 ) > SPKR_LEVEL_MIN + 1 ) {
            spkr_samples[ spkr_sample_idx++ ] = spkr_level;
            spkr_samples[ spkr_sample_idx++ ] = spkr_level;
        }
        spkr_level = SPKR_LEVEL_MIN;
    }
    else {
        // up edge
        while( (spkr_level = (spkr_level + SPKR_LEVEL_MAX) / 2 )  < SPKR_LEVEL_MAX - 1 ) {
            spkr_samples[ spkr_sample_idx++ ] = spkr_level;
            spkr_samples[ spkr_sample_idx++ ] = spkr_level;
        }
        spkr_level = SPKR_LEVEL_MAX;
    }
    //spkr_samples[sample_idx] = spkr_level;
    for ( int i = spkr_sample_idx; i < spkr_buf_size + spkr_extra_buf; i++ ) {
        spkr_samples[i] = spkr_level;
    }
//    memset(spkr_samples + spkr_sample_idx, spkr_level, spkr_buf_size * sizeof(spkr_samples[0]));
    
}



ALint freeBuffers = BUFFER_COUNT;
//ALuint alBuffers[BUFFER_COUNT];

void spkr_update() {
    if ( spkr_play_time ) {
        
        ALint processed = 0;
        
//        printf("freeBuffers:%d  queued:%d  processed:%d\n", freeBuffers, queued,processed);
        
        do {
            alGetSourcei (spkr_src, AL_BUFFERS_PROCESSED, &processed);
//            al_check_error();

            if ( processed ) {
                alSourceUnqueueBuffers( spkr_src, processed, &spkr_buffers[freeBuffers]);
//            al_check_error();
                freeBuffers += processed;
            }
            
        } while( freeBuffers <= 0 );
        
        freeBuffers = clamp( 1, freeBuffers, BUFFER_COUNT );
        
//        printf("freeBuffers2: %d  processed: %d\n", freeBuffers, processed);
        
        ALenum state;
        alGetSourcei( spkr_src, AL_SOURCE_STATE, &state );
//        al_check_error();

        //////////  check if there is no sound generated for long time, and fade out speaker level to avoid pops and crackles
        
        if ( --spkr_play_time == 0 ) {
            // we need to soft mute the speaker to eliminate clicking noise
            // simple linear mute
            const int steepness = 64;
            
            float step = (SPKR_LEVEL_ZERO - (float)spkr_level) / steepness;
            float fadeLevel = spkr_level - SPKR_LEVEL_ZERO;
            
            if ( step != 0 ) {
                spkr_sample_idx = 0;

                while ( ( fadeLevel < -1 ) || ( fadeLevel > 1 ) ) {
                    spkr_samples[ spkr_sample_idx++ ] = SPKR_LEVEL_ZERO + fadeLevel;
                    spkr_samples[ spkr_sample_idx++ ] = SPKR_LEVEL_ZERO + fadeLevel;
                    
                    // how smooth we want the speeker to decay, so we will hear no pops and crackles
                    fadeLevel *= 0.999;
                }
                spkr_level = SPKR_LEVEL_ZERO + fadeLevel;

                //spkr_samples[sample_idx] = spkr_level;
                memset(spkr_samples + spkr_sample_idx, SPKR_LEVEL_ZERO, spkr_extra_buf * sizeof(spkr_samples[0]));

                freeBuffers--;
                alBufferData(spkr_buffers[freeBuffers], AL_FORMAT_STEREO16, spkr_samples, spkr_sample_idx * sizeof(spkr_samples[0]), spkr_sample_rate);
                al_check_error();
                alSourceQueueBuffers(spkr_src, 1, &spkr_buffers[freeBuffers]);
                al_check_error();
            }
        }
        else {
            freeBuffers--;
            alBufferData(spkr_buffers[freeBuffers], AL_FORMAT_STEREO16, spkr_samples, (spkr_buf_size + spkr_extra_buf) * sizeof(spkr_samples[0]), spkr_sample_rate);
            al_check_error();
            alSourceQueueBuffers(spkr_src, 1, &spkr_buffers[freeBuffers]);
            al_check_error();
        }
        
        switch (state) {
            case AL_PAUSED:
                alSourcePlay(spkr_src);
                break;

            case AL_PLAYING:
                // already playing
                break;
                
            default:
                alSourcePlay(spkr_src);
                alSourcePause(spkr_src);
                break;
        }
        
        // clear the slack buffer , so we can fill it up by new data
        for ( int i = 0; i < spkr_buf_size + spkr_extra_buf; i++ ) {
            spkr_samples[i] = spkr_level;
        }
//        memset(spkr_samples, spkr_level, (spkr_buf_size + spkr_extra_buf) * sizeof(spkr_samples[0]));

        // start from the beginning
        spkr_sample_idx = 0;

    }
}


