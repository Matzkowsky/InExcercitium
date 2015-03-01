//
//  AppDelegate.h
//  InExercitium
//
//  Created by David Matz on 28/02/15.
//  Copyright (c) 2015 Matzkowsky Soft. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class UserActivityWatcher;
@class UserActivityNotifier;

@interface AppDelegate : NSObject <NSApplicationDelegate> {
    UserActivityWatcher*    _userActivityWatcher;
    UserActivityNotifier*   _userActivityNotifier;
}

@property (strong, nonatomic) NSStatusItem * statusItem;
@property (assign, nonatomic) BOOL           onAirOn;

@end

