#import <AudioToolbox/AudioToolbox.h>
#include <stdio.h>
#include "bo_audio.h"

static AudioUnit gUnit;
static BOMixer *gMixer;
static int gRunning;
static volatile long long gRendered;

static OSStatus render(void *refCon, AudioUnitRenderActionFlags *flags, const AudioTimeStamp *time, UInt32 bus,
                       UInt32 frames, AudioBufferList *data) {
    bo_mixer_render(gMixer, (int16_t *)data->mBuffers[0].mData, (int)frames);
    gRendered += frames;
    return noErr;
}

long long bo_audio_rendered(void) {
    return gRendered;
}

static void interruption(void *userData, UInt32 state) {
    // A call or an alarm stops the session. Start the output again when
    // it ends.
    if (state == kAudioSessionEndInterruption) {
        AudioSessionSetActive(true);
        if (gRunning) AudioOutputUnitStart(gUnit);
    }
}

int bo_audio_start(BOMixer *mixer) {
    gMixer = mixer;
    if (AudioSessionInitialize(NULL, NULL, interruption, NULL) != noErr) return 0;
    UInt32 category = kAudioSessionCategory_AmbientSound;
    AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
    AudioSessionSetActive(true);

    AudioComponentDescription desc = {kAudioUnitType_Output, kAudioUnitSubType_RemoteIO,
                                      kAudioUnitManufacturer_Apple, 0, 0};
    AudioComponent component = AudioComponentFindNext(NULL, &desc);
    if (!component || AudioComponentInstanceNew(component, &gUnit) != noErr) return 0;

    AudioStreamBasicDescription format = {0};
    format.mSampleRate = BO_AUDIO_RATE;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    format.mChannelsPerFrame = 2;
    format.mBitsPerChannel = 16;
    format.mBytesPerFrame = 4;
    format.mFramesPerPacket = 1;
    format.mBytesPerPacket = 4;
    AudioUnitSetProperty(gUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &format, sizeof(format));
    // The phone asks for 4096 frames at a time when the screen is locked.
    UInt32 maxFrames = 4096;
    AudioUnitSetProperty(gUnit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0, &maxFrames,
                         sizeof(maxFrames));
    AURenderCallbackStruct callback = {render, NULL};
    AudioUnitSetProperty(gUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &callback,
                         sizeof(callback));
    if (AudioUnitInitialize(gUnit) != noErr || AudioOutputUnitStart(gUnit) != noErr) {
        fprintf(stderr, "audio: the output did not start\n");
        return 0;
    }
    gRunning = 1;
    return 1;
}

void bo_audio_set_active(int active) {
    if (!gUnit) return;
    if (active && !gRunning) {
        AudioSessionSetActive(true);
        AudioOutputUnitStart(gUnit);
        gRunning = 1;
    } else if (!active && gRunning) {
        AudioOutputUnitStop(gUnit);
        AudioSessionSetActive(false);
        gRunning = 0;
    }
}
