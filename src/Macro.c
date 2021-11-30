/**
 * Filename: Macro.c
 * Authors: William Avery (add names here)
 * Last Modified: 10/29/2021
 */
#include <stdbool.h>
#include <string.h>

#include "Macro.h"
#include "RotarySwitch.h"

#include "../inc/diskio.h"
#include "../inc/ff.h"
#include "../inc/ST7735_SDC.h"
#include "../inc/tm4c123gh6pm.h"

#include "../driverlib/usb.h"
#include "../usblib/usblib.h"
#include "../usblib/usbhid.h"
#include "../usblib/device/usbdevice.h"
#include "../usblib/device/usbdhid.h"
#include "../usblib/device/usbdhidkeyb.h"
#include "../driverlib/rom_map.h"
#include "../driverlib/sysctl.h"
#include "../driverlib/gpio.h"
#include "../driverlib/fpu.h"
#include "../inc/hw_memmap.h"
#include "../inc/hw_types.h"
#include "../inc/hw_sysctl.h"
#include "../inc/hw_gpio.h"

#include "usb_keyb_structs.h"

#define MAX_SEND_DELAY 50

Macro Macro_Keybindings[ROW_COUNT][COLUMN_COUNT];
static FATFS g_sFatFs;
FIL Handle;
FRESULT MountFresult;
FRESULT Fresult;
const char inFilename[] = "mac.txt"; // 8 characters or fewer

static const int8_t g_ppi8KeyUsageCodes[][2] =
    {
        {0, HID_KEYB_USAGE_SPACE},                       //   0x20
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_1},         // ! 0x21
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_FQUOTE},    // " 0x22
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_3},         // # 0x23
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_4},         // $ 0x24
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_5},         // % 0x25
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_7},         // & 0x26
        {0, HID_KEYB_USAGE_FQUOTE},                      // ' 0x27
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_9},         // ( 0x28
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_0},         // ) 0x29
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_8},         // * 0x2a
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_EQUAL},     // + 0x2b
        {0, HID_KEYB_USAGE_COMMA},                       // , 0x2c
        {0, HID_KEYB_USAGE_MINUS},                       // - 0x2d
        {0, HID_KEYB_USAGE_PERIOD},                      // . 0x2e
        {0, HID_KEYB_USAGE_FSLASH},                      // / 0x2f
        {0, HID_KEYB_USAGE_0},                           // 0 0x30
        {0, HID_KEYB_USAGE_1},                           // 1 0x31
        {0, HID_KEYB_USAGE_2},                           // 2 0x32
        {0, HID_KEYB_USAGE_3},                           // 3 0x33
        {0, HID_KEYB_USAGE_4},                           // 4 0x34
        {0, HID_KEYB_USAGE_5},                           // 5 0x35
        {0, HID_KEYB_USAGE_6},                           // 6 0x36
        {0, HID_KEYB_USAGE_7},                           // 7 0x37
        {0, HID_KEYB_USAGE_8},                           // 8 0x38
        {0, HID_KEYB_USAGE_9},                           // 9 0x39
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_SEMICOLON}, // : 0x3a
        {0, HID_KEYB_USAGE_SEMICOLON},                   // ; 0x3b
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_COMMA},     // < 0x3c
        {0, HID_KEYB_USAGE_EQUAL},                       // = 0x3d
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_PERIOD},    // > 0x3e
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_FSLASH},    // ? 0x3f
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_2},         // @ 0x40
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_A},         // A 0x41
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_B},         // B 0x42
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_C},         // C 0x43
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_D},         // D 0x44
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_E},         // E 0x45
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_F},         // F 0x46
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_G},         // G 0x47
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_H},         // H 0x48
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_I},         // I 0x49
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_J},         // J 0x4a
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_K},         // K 0x4b
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_L},         // L 0x4c
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_M},         // M 0x4d
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_N},         // N 0x4e
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_O},         // O 0x4f
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_P},         // P 0x50
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_Q},         // Q 0x51
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_R},         // R 0x52
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_S},         // S 0x53
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_T},         // T 0x54
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_U},         // U 0x55
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_V},         // V 0x56
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_W},         // W 0x57
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_X},         // X 0x58
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_Y},         // Y 0x59
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_Z},         // Z 0x5a
        {0, HID_KEYB_USAGE_LBRACKET},                    // [ 0x5b
        {0, HID_KEYB_USAGE_BSLASH},                      // \ 0x5c
        {0, HID_KEYB_USAGE_RBRACKET},                    // ] 0x5d
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_6},         // ^ 0x5e
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_MINUS},     // _ 0x5f
        {0, HID_KEYB_USAGE_BQUOTE},                      // ` 0x60
        {0, HID_KEYB_USAGE_A},                           // a 0x61
        {0, HID_KEYB_USAGE_B},                           // b 0x62
        {0, HID_KEYB_USAGE_C},                           // c 0x63
        {0, HID_KEYB_USAGE_D},                           // d 0x64
        {0, HID_KEYB_USAGE_E},                           // e 0x65
        {0, HID_KEYB_USAGE_F},                           // f 0x66
        {0, HID_KEYB_USAGE_G},                           // g 0x67
        {0, HID_KEYB_USAGE_H},                           // h 0x68
        {0, HID_KEYB_USAGE_I},                           // i 0x69
        {0, HID_KEYB_USAGE_J},                           // j 0x6a
        {0, HID_KEYB_USAGE_K},                           // k 0x6b
        {0, HID_KEYB_USAGE_L},                           // l 0x6c
        {0, HID_KEYB_USAGE_M},                           // m 0x6d
        {0, HID_KEYB_USAGE_N},                           // n 0x6e
        {0, HID_KEYB_USAGE_O},                           // o 0x6f
        {0, HID_KEYB_USAGE_P},                           // p 0x70
        {0, HID_KEYB_USAGE_Q},                           // q 0x71
        {0, HID_KEYB_USAGE_R},                           // r 0x72
        {0, HID_KEYB_USAGE_S},                           // s 0x73
        {0, HID_KEYB_USAGE_T},                           // t 0x74
        {0, HID_KEYB_USAGE_U},                           // u 0x75
        {0, HID_KEYB_USAGE_V},                           // v 0x76
        {0, HID_KEYB_USAGE_W},                           // w 0x77
        {0, HID_KEYB_USAGE_X},                           // x 0x78
        {0, HID_KEYB_USAGE_Y},                           // y 0x79
        {0, HID_KEYB_USAGE_Z},                           // z 0x7a
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_LBRACKET},  // { 0x7b
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_BSLASH},    // | 0x7c
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_RBRACKET},  // } 0x7d
        {HID_KEYB_LEFT_SHIFT, HID_KEYB_USAGE_BQUOTE},    // ~ 0x7e
        //
        // Add characters outside of 0x20-0x7e here to avoid breaking the table
        // lookup calculations.
        //
        {0, HID_KEYB_USAGE_ENTER}, // LF 0x0A
    };

volatile bool g_bConnected = false;
volatile bool g_bSuspended = false;
volatile uint32_t g_ui32SysTickCount;
volatile bool g_bDisplayUpdateRequired;
volatile enum {
    //
    // Unconfigured.
    //
    STATE_UNCONFIGURED,

    //
    // No keys to send and not waiting on data.
    //
    STATE_IDLE,

    //
    // Waiting on data to be sent out.
    //
    STATE_SENDING
} g_eKeyboardState = STATE_UNCONFIGURED;

//*****************************************************************************
//
// Handles asynchronous events from the HID keyboard driver.
//
// \param pvCBData is the event callback pointer provided during
// USBDHIDKeyboardInit().  This is a pointer to our keyboard device structure
// (&g_sKeyboardDevice).
// \param ui32Event identifies the event we are being called back for.
// \param ui32MsgData is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the HID keyboard driver to inform the application
// of particular asynchronous events related to operation of the keyboard HID
// device.
//
// \return Returns 0 in all cases.
//
//*****************************************************************************
uint32_t
KeyboardHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgData,
                void *pvMsgData)
{
    switch (ui32Event)
    {
    //
    // The host has connected to us and configured the device.
    //
    case USB_EVENT_CONNECTED:
    {
        g_bConnected = true;
        g_bSuspended = false;
        break;
    }

    //
    // The host has disconnected from us.
    //
    case USB_EVENT_DISCONNECTED:
    {
        g_bConnected = false;
        break;
    }

    //
    // We receive this event every time the host acknowledges transmission
    // of a report.  It is used here purely as a way of determining whether
    // the host is still talking to us or not.
    //
    case USB_EVENT_TX_COMPLETE:
    {
        //
        // Enter the idle state since we finished sending something.
        //
        g_eKeyboardState = STATE_IDLE;
        break;
    }

    //
    // This event indicates that the host has suspended the USB bus.
    //
    case USB_EVENT_SUSPEND:
    {
        g_bSuspended = true;
        break;
    }

    //
    // This event signals that the host has resumed signalling on the bus.
    //
    case USB_EVENT_RESUME:
    {
        g_bSuspended = false;
        break;
    }
    //
    // We ignore all other events.
    //
    default:
    {
        break;
    }
    }
    return 0;
}

//***************************************************************************
//
// Wait for a period of time for the state to become idle.
//
// \param ui32TimeoutTick is the number of system ticks to wait before
// declaring a timeout and returning \b false.
//
// This function polls the current keyboard state for ui32TimeoutTicks system
// ticks waiting for it to become idle.  If the state becomes idle, the
// function returns true.  If it ui32TimeoutTicks occur prior to the state
// becoming idle, false is returned to indicate a timeout.
//
// \return Returns \b true on success or \b false on timeout.
//
//***************************************************************************
bool WaitForSendIdle(uint_fast32_t ui32TimeoutTicks)
{
    uint32_t ui32Start;
    uint32_t ui32Now;
    uint32_t ui32Elapsed;

    ui32Start = g_ui32SysTickCount;
    ui32Elapsed = 0;

    while (ui32Elapsed < ui32TimeoutTicks)
    {
        //
        // Is the keyboard is idle, return immediately.
        //
        if (g_eKeyboardState == STATE_IDLE)
        {
            return (true);
        }

        //
        // Determine how much time has elapsed since we started waiting.  This
        // should be safe across a wrap of g_ui32SysTickCount.
        //
        ui32Now = g_ui32SysTickCount;
        ui32Elapsed = ((ui32Start < ui32Now) ? (ui32Now - ui32Start) : (((uint32_t)0xFFFFFFFF - ui32Start) + ui32Now + 1));
    }

    //
    // If we get here, we timed out so return a bad return code to let the
    // caller know.
    //
    return false;
}

/**
 * @brief Reads and binds the macros to the appropriate keys on the Macropad.
 * Input: MicroSD card in the ST7735 with a 'MAC.txt' file
 *        
 *         --- MAC.TXT FORMAT ---
 *        - 24 Lines
 *        - Line Format: MACRO_NAME NUM_KEYS KEYS
 *        - MACRO_NAME: 3 characters long
 *        - NUM_KEYS: [1,5] (integer)
 *        - KEYS: [1,5] (characters)
 */
void Macro_Init(void)
{
    UINT successfulReads;
    uint8_t ch, x, y;

    MountFresult = f_mount(&g_sFatFs, "", 0); // mount the filesystem
    if (MountFresult)
    {
        ST7735_DrawString(122, 148, "f_mount error", ST7735_Color565(0, 0, 255));
        while (1)
        {
        };
    }
    Fresult = f_open(&Handle, inFilename, FA_READ); // open mac.txt
    x = 122, y = 130;
    if (Fresult == FR_OK)
    {
        for (uint8_t i = 0; i < ROW_COUNT; i++)
        {
            for (uint8_t j = 0; j < COLUMN_COUNT; j++)
            {
                char name[4];
                name[3] = 0x00; // null terminate the string
                uint8_t nameIdx = 0;
                // read in the name of the macro
                Fresult = f_read(&Handle, &ch, 1, &successfulReads);
                while ((Fresult == FR_OK) && (successfulReads == 1) && (ch != 0x20))
                {
                    ST7735_DrawChar(x, y, ch, ST7735_Color565(255, 255, 255), 0, 1);
                    name[nameIdx] = ch;
                    nameIdx++;
                    // go to the next column
                    x -= 6;
                    // read the next character into 'ch'
                    Fresult = f_read(&Handle, &ch, 1, &successfulReads);
                }

                strcpy(Macro_Keybindings[i][j].name, name);

                // read in how many keys are in the macro
                Fresult = f_read(&Handle, &ch, 1, &successfulReads);
                uint8_t numKeys = ch - 0x30;

                Macro_Keybindings[i][j].numKeys = numKeys;

                // read extra space character
                Fresult = f_read(&Handle, &ch, 1, &successfulReads);
                x -= 6; // add space between macro name and macro keys

                for (int k = 0; k < numKeys; k++)
                {
                    // read in the k-th key
                    Fresult = f_read(&Handle, &ch, 1, &successfulReads);
                    Macro_Keybindings[i][j].asciiCodes[k] = ch;
                }

                // read return and new line characters
                Fresult = f_read(&Handle, &ch, 1, &successfulReads);
                Fresult = f_read(&Handle, &ch, 1, &successfulReads);
                x -= 12;
            }
            x = 122;
            y -= 20;
        }
    }
    else
    {
        ST7735_DrawString(122, 148, "f_open error", ST7735_Color565(0, 0, 255));
        while (1)
        {
        };
    }

    MAP_FPULazyStackingEnable();

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    MAP_GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    //
    // Erratum workaround for silicon revision A1.  VBUS must have pull-down.
    //
    if (CLASS_IS_TM4C123 && REVISION_IS_A1)
    {
        HWREG(GPIO_PORTB_BASE + GPIO_O_PDR) |= GPIO_PIN_1;
    }

    //
    // Enable the GPIO that is used for the on-board LED.
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);
    MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);

    bool bLastSuspend;

    g_bConnected = false;
    g_bSuspended = false;
    bLastSuspend = false;

    //
    // Initialize the USB stack for device mode.  We do not operate in USB
    // device mode with active monitoring of VBUS and therefore, we will
    // specify eUSBModeForceDevice as the operating mode instead of
    // eUSBModeDevice.  To use eUSBModeDevice, the EK-TM4C123GXL LaunchPad
    // must have the R28 and R29 populated with zero ohm resistors.
    //
    USBStackModeSet(0, eUSBModeForceDevice, 0);

    //
    // Pass our device information to the USB HID device class driver,
    // initialize the USB controller, and connect the device to the bus.
    //
    USBDHIDKeyboardInit(0, &g_sKeyboardDevice);

    while (!g_bConnected)
    {
    }

    MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);

    //
    // Enter the idle state.
    //
    g_eKeyboardState = STATE_IDLE;

    //
    // Assume that the bus is not currently suspended if we have just been
    // configured.
    //
    bLastSuspend = false;
}

void SendKeys(uint8_t *HIDCodes, uint8_t numKeys)
{
    for (uint8_t i = 0; i < numKeys; i++)
    {
        uint32_t keyIdx = HIDCodes[i] - ' ';
        //
        // Send the key press message.
        //
        g_eKeyboardState = STATE_SENDING;
        if (USBDHIDKeyboardKeyStateChange((void *)&g_sKeyboardDevice,
                                        1,
                                        g_ppi8KeyUsageCodes[keyIdx][0],
                                        g_ppi8KeyUsageCodes[keyIdx][1],
                                        true) != KEYB_SUCCESS)
        {
            return;
        }

        //
        // Wait until the key press message has been sent.
        //
        if (!WaitForSendIdle(MAX_SEND_DELAY))
        {
            g_bConnected = 0;
            return;
        }
    }

    for (uint8_t i = 0; i < numKeys; i++)
    {
        uint32_t keyIdx = HIDCodes[i] - ' ';
        //
        // Send the key release message.
        //
        g_eKeyboardState = STATE_SENDING;
        if (USBDHIDKeyboardKeyStateChange((void *)&g_sKeyboardDevice,
                                        1, 
                                        0, 
                                        g_ppi8KeyUsageCodes[keyIdx][1],
                                        false) != KEYB_SUCCESS)
        {
            return;
        }

        //
        // Wait until the key release message has been sent.
        //
        if (!WaitForSendIdle(MAX_SEND_DELAY))
        {
            g_bConnected = 0;
            return;
        }
    }
}

void SendMediaKey(uint8_t HIDCode)
{
    //
    // Send the key press message.
    //
    g_eKeyboardState = STATE_SENDING;
    if (USBDHIDKeyboardKeyStateChange((void *)&g_sKeyboardDevice,
                                      2,
                                      0,
                                      HIDCode,
                                      true) != KEYB_SUCCESS)
    {
        return;
    }

    //
    // Wait until the key press message has been sent.
    //
    if (!WaitForSendIdle(MAX_SEND_DELAY))
    {
        g_bConnected = 0;
        return;
    }

    //
    // Send the key release message.
    //
    g_eKeyboardState = STATE_SENDING;
    if (USBDHIDKeyboardKeyStateChange((void *)&g_sKeyboardDevice,
                                      2, 0, HIDCode,
                                      false) != KEYB_SUCCESS)
    {
        return;
    }

    //
    // Wait until the key release message has been sent.
    //
    if (!WaitForSendIdle(MAX_SEND_DELAY))
    {
        g_bConnected = 0;
        return;
    }
}

void Macro_Execute(Macro macro, bool isMedia)
{
    if (isMedia)
    {
        SendMediaKey(macro.asciiCodes[0]);
    }
    else
    {
        SendKeys(macro.asciiCodes, macro.numKeys);
    }
}
