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

void create_hid_interface(io_object_t hidDevice, IOHIDDeviceInterface*** hdi);
void find_led_cookies(IOHIDDeviceInterface122** handle);

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


IOHIDDeviceInterface** find_a_keyboard(void) {
    IOHIDDeviceInterface** resultHIDDevice = NULL;
    io_iterator_t ioIt = (io_iterator_t)0;
    kern_return_t kernReturn = 0;
    
    CFNumberRef usagePageRef = (CFNumberRef)0;
    CFNumberRef usageRef = (CFNumberRef)0;
    CFMutableDictionaryRef matchingDictRef = (CFMutableDictionaryRef)0;
    
    if (!(matchingDictRef = IOServiceMatching(kIOHIDDeviceKey))) {
        return NULL;
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
    
    CFDictionarySetValue(matchingDictRef, CFSTR(kIOHIDPrimaryUsagePageKey), usagePageRef);
    CFDictionarySetValue(matchingDictRef, CFSTR(kIOHIDPrimaryUsageKey), usageRef);
    
    kernReturn = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDictRef, &ioIt);

    if (!kernReturn) {
        io_service_t  hidService =  0;
        while ( (hidService = IOIteratorNext(ioIt)) ) {
            io_object_t            hidDevice = (io_object_t)hidService;
            io_name_t              hidDeviceName = "";
            IOReturn               ioRetVal = kIOReturnError;
            IOHIDDeviceInterface** hidDeviceInterface = NULL;

            create_hid_interface(hidDevice, &hidDeviceInterface);
            if (hidDeviceInterface == NULL) {
                fprintf(stderr, "Failed to create HID device interface.\n");
                continue;
            }
            IORegistryEntryGetName(hidDevice, hidDeviceName);
            
            find_led_cookies((IOHIDDeviceInterface122**)hidDeviceInterface);
            
            ioRetVal = IOObjectRelease(hidDevice);
            if (ioRetVal != kIOReturnSuccess) {
                (*hidDeviceInterface)->Release(hidDeviceInterface);
                continue;
            }
            
            ioRetVal = (*hidDeviceInterface)->open(hidDeviceInterface, kIOHIDOptionsTypeNone);
            if (ioRetVal != kIOReturnSuccess) {
                fprintf(stderr, "Failed to open HID device %s (0x%8X)\n", hidDeviceName, ioRetVal);
                (*hidDeviceInterface)->Release(hidDeviceInterface);
                continue;
            } else {
                // exclude Karabiner virtual keyboard driver
                if (strncmp(hidDeviceName, "org_pqrs_driver_Karabiner_VirtualHIDDevice_VirtualHIDKeyboard", 61) == 0) {
                    fprintf(stdout, "Skip HID device %s\n", hidDeviceName);
                    (*hidDeviceInterface)->Release(hidDeviceInterface);
                    continue;
                }
                // take first device which we can open
                fprintf(stdout, "Opened HID device %s\n", hidDeviceName);
                resultHIDDevice = hidDeviceInterface;
                break;
            }
        }
    }
    IOObjectRelease(ioIt);
    
out:
    if (usageRef) {
        CFRelease(usageRef);
    }
    if (usagePageRef) {
        CFRelease(usagePageRef);
    }
    
    return resultHIDDevice;
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
        fprintf(stderr, "Failed to copy cookies ((0x%8X)\n", result);
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
    
    CFRelease(elements);
    
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
    
    ioReturnValue = IOCreatePlugInInterfaceForService(hidDevice, kIOHIDDeviceUserClientTypeID,
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
    IOHIDDeviceInterface **hidDeviceInterface = NULL;
    IOReturn               ioReturnValue = kIOReturnError;
    IOHIDElementCookie     theCookie = (IOHIDElementCookie)0;
    IOHIDEventStruct       theEvent;
    
    if (!(hidDeviceInterface = find_a_keyboard())) {
        fprintf(stderr, "No keyboard found.\n");
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
            ioReturnValue = (*hidDeviceInterface)->setElementValue(hidDeviceInterface, theCookie,
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
