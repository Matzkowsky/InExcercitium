//
//  UserActivityWatcher.h
//  InExercitium
//
//  Created by David Matz on 28/02/15.
//  Copyright (c) 2015 Matzkowsky Soft. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol UserActivityNotifierDelegate;

@interface UserActivityWatcher : NSObject {
}

- (id)   initWithDelegate:(NSObject<UserActivityNotifierDelegate>*) delegate;
- (void) stopWatching;
- (void) startWatching;

@property (readwrite, atomic) BOOL notifyForCalendarEvents;
@property (readwrite, atomic) BOOL notifyForRunningApplication;
@property (readwrite, atomic) BOOL notifyForAdiumStatus;
@property (readwrite, atomic) BOOL notifyForPhoneCall;
@property (readwrite, nonatomic) NSObject<UserActivityNotifierDelegate>* userActivityDelegate;

@end
