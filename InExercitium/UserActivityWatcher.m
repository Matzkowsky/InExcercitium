//
//  UserActivityWatcher.m
//  InExercitium
//
//  Created by David Matz on 28/02/15.
//  Copyright (c) 2015 Matzkowsky Soft. All rights reserved.
//

#import "UserActivityWatcher.h"
#import "UserActivityNotifier.h"
#import <EventKit/EventKit.h> // Calendar access


@interface UserActivityWatcher () {
    NSTimer*            _timer;
    EKEventStore*       _store;
    UserActivityFlags   _userActivityFlags;
}
- (void) watchForUserActivity:(id) sender;
- (void) watchForCalendarEvents;
- (void) watchForRunningApplication;
- (void) watchForAdiumStatus;
- (void) watchForPhoneCall;

@end

@implementation UserActivityWatcher

- (id) initWithDelegate:(NSObject<UserActivityNotifierDelegate>*) delegate {
    self = [super init];
    if (self) {
        _userActivityDelegate = delegate;
        [self startWatching];
    }
    
    return self;
}

- (id) init {
    self = [super init];
    if (self) {
        [self startWatching];
    }
    
    return self;
}

- (void) dealloc {
    [self stopWatching];
    _store = nil;
}

- (void) startWatching {
    if (_timer) {
        return;
    }
    _timer = [NSTimer scheduledTimerWithTimeInterval:2.0 target:self selector:@selector(watchForUserActivity:) userInfo:nil repeats:YES];
}

- (void) stopWatching {
    [_timer invalidate];
    _timer = nil;
}

- (void) watchForUserActivity:(id)sender {

    if (_notifyForCalendarEvents) {
        [self watchForCalendarEvents];
    } else {
        _userActivityFlags.busyByCalendarEvent = NO;
    }
    if (_notifyForRunningApplication) {
        [self watchForRunningApplication];
    } else {
        _userActivityFlags.busyByRunningApplication = NO;
    }
    if (_notifyForAdiumStatus) {
        [self watchForAdiumStatus];
    } else {
        _userActivityFlags.busyByAdiumStatus = NO;
    }
    if (_notifyForPhoneCall) {
        [self watchForPhoneCall];
    } else {
        _userActivityFlags.busyByPhoneCall = NO;
    }

    [_userActivityDelegate notifyUserActivityFlags:&_userActivityFlags];
}

- (void) watchForCalendarEvents {
    if (_store == nil) {
        _store = [[EKEventStore alloc] initWithAccessToEntityTypes:EKEntityMaskEvent];
    }
    // Fetch event with a predicate
    //NSCalendar *calendar = [NSCalendar currentCalendar];
    NSPredicate *predicate = [_store predicateForEventsWithStartDate:[NSDate date] endDate:[NSDate date] calendars:nil];
    NSArray *events = [_store eventsMatchingPredicate:predicate];

    if ([events count] > 0) {
        for (EKEvent* event in events) {
            NSLog(@"%@", [event title]);
        }
        
        _userActivityFlags.busyByCalendarEvent = YES;
    } else {
        _userActivityFlags.busyByCalendarEvent = NO;
    }
}

- (void) watchForRunningApplication {
    _userActivityFlags.busyByRunningApplication = NO;
}

- (void) watchForAdiumStatus {
    _userActivityFlags.busyByAdiumStatus = NO;
}

- (void) watchForPhoneCall {
    _userActivityFlags.busyByPhoneCall = NO;
}

//- (void) notifyForEvent:(SEL) function withObject:(id) obj {
//    [_userActivityDelegate performSelector:@selector(function) withObject:obj];
//    _eventNotified = YES;
//}

@end
