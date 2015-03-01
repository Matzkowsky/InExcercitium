//
//  AppDelegate.m
//  InExercitium
//
//  Created by David Matz on 28/02/15.
//  Copyright (c) 2015 Matzkowsky Soft. All rights reserved.
//

#import "AppDelegate.h"
#import "UserActivityWatcher.h"
#import "UserActivityNotifier.h"

@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    self.statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
    _statusItem.image = [NSImage imageNamed:@"On-Air_18.png"];
    [_statusItem.image setTemplate:YES];
    _statusItem.toolTip = @"Command+Click to Quit";

    NSMenu* menu = [[NSMenu alloc] init];
    [menu addItemWithTitle:@"Active" action:@selector(activeClicked:) keyEquivalent:@""];
    [menu addItemWithTitle:@"Quit"   action:@selector(quitClicked:)   keyEquivalent:@""];
    [[menu itemAtIndex:0] setState:NSOnState];

    _statusItem.menu = menu;
    [_statusItem setAction:@selector(itemClicked:)];
    
    _userActivityNotifier = [[UserActivityNotifier alloc] init];
    _userActivityWatcher  = [[UserActivityWatcher alloc] initWithDelegate:_userActivityNotifier];

    // Configuration
    _userActivityWatcher.notifyForCalendarEvents = YES;
    _userActivityWatcher.notifyForRunningApplication = YES;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (void) activeClicked:(id) sender {
    NSMenuItem* item = sender;
    [item setState:((item.state == NSOnState)?NSOffState:NSOnState)];
    if (item.state == NSOnState) {
        [_userActivityWatcher startWatching];
        _statusItem.image = [NSImage imageNamed:@"On-Air_18.png"];
        [_statusItem.image setTemplate:YES];
    } else {
        [_userActivityWatcher stopWatching];
        [_userActivityNotifier resetLedStatus];
        _statusItem.image = [NSImage imageNamed:@"On-Air_18_incative.png"];
        [_statusItem.image setTemplate:YES];
    }
}

- (void) quitClicked:(id) sender {
    [[NSApplication sharedApplication] terminate:self];
}

- (void) itemClicked:(id) sender {
    NSLog(@"On Air");
    
    NSEvent* event = [NSApp currentEvent];
    if ([event modifierFlags] & NSControlKeyMask) {
        [[NSApplication sharedApplication] terminate:self];
        return;
    }
}


@end
