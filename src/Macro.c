/**
 * Filename: Macro.c
 * Authors: William Avery (add names here)
 * Last Modified: 10/29/2021
 */
#include <stdbool.h>
#include <string.h>

#include "Macro.h"

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

                strcpy(Macro_Keybindings[i][j].numKeys, numKeys);

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

void SendKey(uint8_t HIDCode)
{
    //
    // Send the key press message.
    //
    g_eKeyboardState = STATE_SENDING;
    if (USBDHIDKeyboardKeyStateChange((void *)&g_sKeyboardDevice,
                                      1,
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
                                      1, 0, HIDCode,
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

void SendKeys(uint8_t *HIDCodes, uint8_t numKeys)
{
    for (uint8_t i = 0; i < numKeys; i++)
    {
        //
        // Send the key press message.
        //
        g_eKeyboardState = STATE_SENDING;
        if (USBDHIDKeyboardKeyStateChange((void *)&g_sKeyboardDevice,
                                        1,
                                        0,
                                        HIDCodes[i],
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
        //
        // Send the key release message.
        //
        g_eKeyboardState = STATE_SENDING;
        if (USBDHIDKeyboardKeyStateChange((void *)&g_sKeyboardDevice,
                                        1, 0, HIDCodes[i],
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

void Macro_Execute(Macro macro)
{
    // TODO send keyboard combination to computer through USB
    SendMediaKey(macro.asciiCodes[0]);
    SendKeys(macro.asciiCodes, macro.numKeys);
}
