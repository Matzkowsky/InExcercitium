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


@interface UserActivityNotifier () {
    LedStatus   _ledStatus;
}
- (void) setLED:(UserActivityNotifyLED) led toValue:(BOOL) value;
@end

@implementation UserActivityNotifier

- (id) init {
    self = [super init];
    if (self) {
        self.ledForAllNotifications = ledNum;
    }
    
    return self;
}

- (void) setLED:(UserActivityNotifyLED) led toValue:(BOOL) value {
    // access hardware to flag notification with appropriate LED
    manipulate_led(led, value);
}

- (void) setLedForAllNotifications:(UserActivityNotifyLED)ledForAllNotifications {
    _ledForAllNotifications = _ledForCalendarEvents = _ledForRunningApplication = _ledForAdiumStatus = _ledForPhoneCall = ledForAllNotifications;
}

#pragma mark UserActivityNotifierDelegate

- (void) notifyUserActivityFlags:(const UserActivityFlags*) userActivityFlags {
    typedef std::map<int, BOOL> LEDStatusMap;
    LEDStatusMap statusMap;
    LedStatus newStatus;

    statusMap[ledCaps]   = NO;
    statusMap[ledNum]    = NO;
    statusMap[ledScroll] = NO;
    statusMap[_ledForCalendarEvents]     |= userActivityFlags->busyByCalendarEvent;
    statusMap[_ledForRunningApplication] |= userActivityFlags->busyByRunningApplication;
    statusMap[_ledForAdiumStatus]        |= userActivityFlags->busyByAdiumStatus;
    statusMap[_ledForPhoneCall]          |= userActivityFlags->busyByPhoneCall;

    newStatus.capsLockOn   = statusMap[ledCaps];
    newStatus.numLockOn    = statusMap[ledNum];
    newStatus.scrollLockOn = statusMap[ledNum];
    
    if (newStatus.capsLockOn != _ledStatus.capsLockOn) {
        _ledStatus.capsLockOn = newStatus.capsLockOn;
        [self setLED:ledCaps toValue:_ledStatus.capsLockOn];
    }
    if (newStatus.numLockOn != _ledStatus.numLockOn) {
        _ledStatus.numLockOn = newStatus.numLockOn;
        [self setLED:ledNum toValue:_ledStatus.numLockOn];
    }
    if (newStatus.capsLockOn != _ledStatus.capsLockOn) {
        _ledStatus.capsLockOn = newStatus.capsLockOn;
        [self setLED:ledCaps toValue:_ledStatus.capsLockOn];
    }
    
    NSLog(@"LED caps,num,scroll: %d%d%d", _ledStatus.capsLockOn, _ledStatus.numLockOn, _ledStatus.scrollLockOn);
}

@end
