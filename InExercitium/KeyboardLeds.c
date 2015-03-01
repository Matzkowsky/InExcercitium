//
//  KeyboardLeds.c
//  InExercitium
//
//  Created by David Matz on 01/03/15.
//  Copyright (c) 2015 Matzkowsky Soft. All rights reserved.
//

#include "KeyboardLeds.h"

#include <IOKit/IOCFPlugIn.h>
#include <IOKit/hid/IOHIDLib.h>
#include <mach/mach_error.h>

static IOHIDElementCookie capslock_cookie    = (IOHIDElementCookie)0;
static IOHIDElementCookie numlock_cookie     = (IOHIDElementCookie)0;
static IOHIDElementCookie scrolllock_cookie  = (IOHIDElementCookie)0;

void print_errmsg_if_io_err(int expr, char* msg) {
    IOReturn err = (expr);
    
    if (err != kIOReturnSuccess) {
        fprintf(stderr, "*** %s - %s(%x, %d).\n", msg, mach_error_string(err),  err, err & 0xffffff);
        fflush(stderr);
        //exit(EX_OSERR);
    }
}
void print_errmsg_if_err(int expr, char* msg) {
    if (expr) {
        fprintf(stderr, "*** %s.\n", msg);
        fflush(stderr);
        //exit(EX_OSERR);
    }
}


io_service_t find_a_keyboard(void) {
    io_service_t result = (io_service_t)0;
    
    CFNumberRef usagePageRef = (CFNumberRef)0;
    CFNumberRef usageRef = (CFNumberRef)0;
    CFMutableDictionaryRef matchingDictRef = (CFMutableDictionaryRef)0;
    
    if (!(matchingDictRef = IOServiceMatching(kIOHIDDeviceKey))) {
        return result;
    }
    
    UInt32 usagePage = kHIDPage_GenericDesktop;
    UInt32 usage = kHIDUsage_GD_Keyboard;
    
    if (!(usagePageRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType,
                                        &usagePage))) {
        goto out;
    }
    
    if (!(usageRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType,
                                    &usage))) {
        goto out;
    }
    
    CFDictionarySetValue(matchingDictRef, CFSTR(kIOHIDPrimaryUsagePageKey),
                         usagePageRef);
    CFDictionarySetValue(matchingDictRef, CFSTR(kIOHIDPrimaryUsageKey),
                         usageRef);
    
    result = IOServiceGetMatchingService(kIOMasterPortDefault, matchingDictRef);
    
out:
    if (usageRef) {
        CFRelease(usageRef);
    }
    if (usagePageRef) {
        CFRelease(usagePageRef);
    }
    
    return result;
}

void find_led_cookies(IOHIDDeviceInterface122** handle) {
    IOHIDElementCookie cookie;
    CFTypeRef          object;
    long               number;
    long               usage;
    long               usagePage;
    CFArrayRef         elements;
    CFDictionaryRef    element;
    IOReturn           result;
    
    if (!handle || !(*handle)) {
        return;
    }
    
    result = (*handle)->copyMatchingElements(handle, NULL, &elements);
    
    if (result != kIOReturnSuccess) {
        fprintf(stderr, "Failed to copy cookies.\n");
        exit(1);
    }
    
    CFIndex i;
    
    for (i = 0; i < CFArrayGetCount(elements); i++) {
        
        element = CFArrayGetValueAtIndex(elements, i);
        
        object = (CFDictionaryGetValue(element, CFSTR(kIOHIDElementCookieKey)));
        if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID()) {
            continue;
        }
        if (!CFNumberGetValue((CFNumberRef) object, kCFNumberLongType,
                              &number)) {
            continue;
        }
        cookie = (IOHIDElementCookie)number;
        
        object = CFDictionaryGetValue(element, CFSTR(kIOHIDElementUsageKey));
        if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID()) {
            continue;
        }
        if (!CFNumberGetValue((CFNumberRef)object, kCFNumberLongType,
                              &number)) {
            continue;
        }
        usage = number;
        
        object = CFDictionaryGetValue(element,CFSTR(kIOHIDElementUsagePageKey));
        if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID()) {
            continue;
        }
        if (!CFNumberGetValue((CFNumberRef)object, kCFNumberLongType,
                              &number)) {
            continue;
        }
        usagePage = number;
        
        if (usagePage == kHIDPage_LEDs) {
            switch (usage) {
                    
                case kHIDUsage_LED_NumLock:
                    numlock_cookie = cookie;
                    break;
                    
                case kHIDUsage_LED_CapsLock:
                    capslock_cookie = cookie;
                    break;
                    
                case kHIDUsage_LED_ScrollLock:
                    scrolllock_cookie = cookie;
                    break;
                    
                default:
                    break;
            }
        }
    }
    
    return;
}

void create_hid_interface(io_object_t hidDevice, IOHIDDeviceInterface*** hdi) {
    IOCFPlugInInterface** plugInInterface = NULL;
    
    io_name_t className;
    HRESULT   plugInResult = S_OK;
    SInt32    score = 0;
    IOReturn  ioReturnValue = kIOReturnSuccess;
    
    ioReturnValue = IOObjectGetClass(hidDevice, className);
    
    print_errmsg_if_io_err(ioReturnValue, "Failed to get class name.");
    
    ioReturnValue = IOCreatePlugInInterfaceForService(
                                                      hidDevice, kIOHIDDeviceUserClientTypeID,
                                                      kIOCFPlugInInterfaceID, &plugInInterface, &score);
    if (ioReturnValue != kIOReturnSuccess) {
        return;
    }
    
    plugInResult = (*plugInInterface)->QueryInterface(plugInInterface,
                                                      CFUUIDGetUUIDBytes(kIOHIDDeviceInterfaceID), (LPVOID)hdi);
    print_errmsg_if_err(plugInResult != S_OK, "Failed to create device interface.\n");
    
    (*plugInInterface)->Release(plugInInterface);
}

int manipulate_led(UInt32 whichLED, UInt32 value) {
    io_service_t           hidService = (io_service_t)0;
    io_object_t            hidDevice = (io_object_t)0;
    IOHIDDeviceInterface **hidDeviceInterface = NULL;
    IOReturn               ioReturnValue = kIOReturnError;
    IOHIDElementCookie     theCookie = (IOHIDElementCookie)0;
    IOHIDEventStruct       theEvent;
    
    if (!(hidService = find_a_keyboard())) {
        fprintf(stderr, "No keyboard found.\n");
        return ioReturnValue;
    }
    
    hidDevice = (io_object_t)hidService;
    
    create_hid_interface(hidDevice, &hidDeviceInterface);
    
    find_led_cookies((IOHIDDeviceInterface122 **)hidDeviceInterface);
    
    ioReturnValue = IOObjectRelease(hidDevice);
    if (ioReturnValue != kIOReturnSuccess) {
        goto out;
    }
    
    ioReturnValue = kIOReturnError;
    
    if (hidDeviceInterface == NULL) {
        fprintf(stderr, "Failed to create HID device interface.\n");
        return ioReturnValue;
    }
    
    if (whichLED == kHIDUsage_LED_NumLock) {
        theCookie = numlock_cookie;
    } else if (whichLED == kHIDUsage_LED_CapsLock) {
        theCookie = capslock_cookie;
    } else if (whichLED == kHIDUsage_LED_ScrollLock) {
        theCookie = scrolllock_cookie;
    }
    
    if (theCookie == 0) {
        fprintf(stderr, "Bad or missing LED cookie.\n");
        goto out;
    }
    
    ioReturnValue = (*hidDeviceInterface)->open(hidDeviceInterface, 0);
    if (ioReturnValue != kIOReturnSuccess) {
        fprintf(stderr, "Failed to open HID device interface.\n");
        goto out;
    }
    
    ioReturnValue = (*hidDeviceInterface)->getElementValue(hidDeviceInterface,
                                                           theCookie, &theEvent);
    if (ioReturnValue != kIOReturnSuccess) {
        (void)(*hidDeviceInterface)->close(hidDeviceInterface);
        goto out;
    }
    
    fprintf(stdout, "%s -> ", (theEvent.value) ? "on" : "off");
    if (value != -1) {
        if (theEvent.value != value) {
            theEvent.value = value;
            ioReturnValue = (*hidDeviceInterface)->setElementValue(
                                                                   hidDeviceInterface, theCookie,
                                                                   &theEvent, 0, 0, 0, 0);
            if (ioReturnValue == kIOReturnSuccess) {
                fprintf(stdout, "%s\n", (theEvent.value) ? "on" : "off");
            }
        }
    }
    
    ioReturnValue = (*hidDeviceInterface)->close(hidDeviceInterface);
    
out:
    (void)(*hidDeviceInterface)->Release(hidDeviceInterface);
    
    return ioReturnValue;
}
