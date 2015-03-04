//
//  UserActivityNotifier.m
//  InExercitium
//
//  Created by David Matz on 28/02/15.
//  Copyright (c) 2015 Matzkowsky Soft. All rights reserved.
//

#import "UserActivityNotifier.h"
#include <map>

extern "C" {
#include "KeyboardLeds.h"
}

typedef struct {
    BOOL capsLockOn;
    BOOL numLockOn;
    BOOL scrollLockOn;
} LedStatus;

typedef std::map<int, BOOL> LEDStatusMap;

@interface UserActivityNotifier () {
    LedStatus    _ledStatus;
    LEDStatusMap _ledCustomState;
}
- (void) setLED:(UserActivityNotifyLED) led toValue:(BOOL) value;
@end

@implementation UserActivityNotifier

- (id) init {
    self = [super init];
    if (self) {
        self.ledForAllNotifications = ledNum;
        [self resetLedStatus];
    }
    
    return self;
}

- (void) resetLedStatus {
    _ledStatus.capsLockOn = _ledStatus.numLockOn = _ledStatus.scrollLockOn = NO;
    [self setLED:ledCaps toValue:NO];
    [self setLED:ledNum  toValue:NO];
    [self setLED:ledCaps toValue:NO];
    _ledCustomState[ledCaps]   = NO;
    _ledCustomState[ledNum]    = NO;
    _ledCustomState[ledScroll] = NO;
}

- (void) setLED:(UserActivityNotifyLED) led toValue:(BOOL) value {
    // access hardware to flag notification with appropriate LED
    manipulate_led(led, value);
}

- (void) setLedForAllNotifications:(UserActivityNotifyLED)ledForAllNotifications {
    _ledForAllNotifications = _ledForCalendarEvents = _ledForRunningApplication = _ledForAdiumStatus = _ledForPhoneCall = ledForAllNotifications;
}

- (void) toggleCustomStateForLed:(UserActivityNotifyLED) led {
    _ledCustomState[led] = !_ledCustomState[led];
}

- (BOOL) customStateForLed:(UserActivityNotifyLED) led {
    return _ledCustomState[led];
}

#pragma mark UserActivityNotifierDelegate

- (void) notifyUserActivityFlags:(const UserActivityFlags*) userActivityFlags {
    LEDStatusMap statusMap;
    LedStatus newStatus;

    statusMap[ledCaps]   = _ledCustomState[ledCaps];
    statusMap[ledNum]    = _ledCustomState[ledNum];
    statusMap[ledScroll] = _ledCustomState[ledScroll];
    statusMap[_ledForCalendarEvents]     |= userActivityFlags->busyByCalendarEvent;
    statusMap[_ledForRunningApplication] |= userActivityFlags->busyByRunningApplication;
    statusMap[_ledForAdiumStatus]        |= userActivityFlags->busyByAdiumStatus;
    statusMap[_ledForPhoneCall]          |= userActivityFlags->busyByPhoneCall;

    newStatus.capsLockOn   = statusMap[ledCaps];
    newStatus.numLockOn    = statusMap[ledNum];
    newStatus.scrollLockOn = statusMap[ledScroll];
    
    if (newStatus.capsLockOn != _ledStatus.capsLockOn) {
        _ledStatus.capsLockOn = newStatus.capsLockOn;
        [self setLED:ledCaps toValue:_ledStatus.capsLockOn];
    }
    if (newStatus.numLockOn != _ledStatus.numLockOn) {
        _ledStatus.numLockOn = newStatus.numLockOn;
        [self setLED:ledNum toValue:_ledStatus.numLockOn];
    }
    if (newStatus.scrollLockOn != _ledStatus.scrollLockOn) {
        _ledStatus.scrollLockOn = newStatus.scrollLockOn;
        [self setLED:ledScroll toValue:_ledStatus.scrollLockOn];
    }
    
    NSLog(@"LED caps,num,scroll: %d%d%d", _ledStatus.capsLockOn, _ledStatus.numLockOn, _ledStatus.scrollLockOn);
}

@end
