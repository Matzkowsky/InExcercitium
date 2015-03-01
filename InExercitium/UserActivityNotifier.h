//
//  UserActivityNotifier.h
//  InExercitium
//
//  Created by David Matz on 28/02/15.
//  Copyright (c) 2015 Matzkowsky Soft. All rights reserved.
//

#import <Foundation/Foundation.h>
#include <IOKit/hid/IOHIDUsageTables.h>

typedef enum {
    ledCaps   = kHIDUsage_LED_CapsLock,
    ledNum    = kHIDUsage_LED_NumLock,
    ledScroll = kHIDUsage_LED_ScrollLock
} UserActivityNotifyLED;

typedef struct {
    BOOL busyByCalendarEvent;
    BOOL busyByRunningApplication;
    BOOL busyByAdiumStatus;
    BOOL busyByPhoneCall;
} UserActivityFlags;

@protocol UserActivityNotifierDelegate
- (void) notifyUserActivityFlags:(const UserActivityFlags*) userActivityFlags;
@end

@interface UserActivityNotifier : NSObject<UserActivityNotifierDelegate>

@property (readwrite, nonatomic) UserActivityNotifyLED ledForCalendarEvents;
@property (readwrite, nonatomic) UserActivityNotifyLED ledForRunningApplication;
@property (readwrite, nonatomic) UserActivityNotifyLED ledForAdiumStatus;
@property (readwrite, nonatomic) UserActivityNotifyLED ledForPhoneCall;
@property (readwrite, nonatomic) UserActivityNotifyLED ledForAllNotifications;

@end
