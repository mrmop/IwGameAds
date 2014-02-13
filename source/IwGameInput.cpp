// 
//
// IwGame - Cross Platform Multi-purpose Game Engine using the Marmalade SDK
//
// Developed by Matthew Hopwood of Pocketeers Limited - www.pocketeers.co.uk
//
// For updates, tutorials and more details check out my blog at www.drmop.com
//
// This code is provided free of charge and without any warranty whatsoever. The only restriction to its usage is that this header must remain intact and visible in all IwGame engine files.
// If you use this engine in your product, whilst it is not mandatory, a mention of the IwGame engine would be nice or a quick email to let us know where it is being used.
//
//

#include "IwGameInput.h"
#include "IwGameString.h"
#include "s3eCompass.h"

CDECLARE_SINGLETON(CIwGameInput)

// 
//
// Input callback handlers
//
//
//
// HandleMultiTouchButtonCB - For multitouch devices the system will call this callback when the user touches the screen. This callback is called once for each screen touch
// 
void HandleMultiTouchButtonCB(s3ePointerTouchEvent* event)
{
	// Check to see if the touch already exists
	CIwGameTouch* touch = IW_GAME_INPUT->findTouch(event->m_TouchID);
    if (touch != NULL)
    {
		// Yes it does, so update the touch information
        touch->active = event->m_Pressed != 0; 
        touch->x = event->m_x;
        touch->y = event->m_y;
    }
}
//
// HandleMultiTouchMotionCB - For multitouch devices the system will call this callback when the user moves their finger on the screen. This callback is called once for each screen touch
// 
void HandleMultiTouchMotionCB(s3ePointerTouchMotionEvent* event)
{
	// Check to see if the touch already exists
	CIwGameTouch* touch = IW_GAME_INPUT->findTouch(event->m_TouchID);
    if (touch != NULL)
    {
		// Updates the touches positional information
        touch->x = event->m_x;
        touch->y = event->m_y;
    }
}
//
// HandleSingleTouchButtonCB - The system will call this callback when the user touches the screen
// 
void HandleSingleTouchButtonCB(s3ePointerEvent* event)
{
	CIwGameTouch* touch = IW_GAME_INPUT->getTouch(0);
    touch->active = event->m_Pressed != 0;
    touch->x = event->m_x;
    touch->y = event->m_y;
}
//
// HandleSingleTouchMotionCB - The system will call this callback when the user moves their finger on the screen
// 
void HandleSingleTouchMotionCB(s3ePointerMotionEvent* event)
{
	CIwGameTouch* touch = IW_GAME_INPUT->getTouch(0);
    touch->x = event->m_x;
    touch->y = event->m_y;
}

//
//
// CIwGameInput implementation
//
//
CIwGameTouch*	CIwGameInput::findTouch(int id)
{
	if (!PointerAvailable)
		return NULL;

    // Attempt to find the touch by its ID and then return it
	// If the touch does not exist then it is recorded in the touches list
	for (int t = 0; t < MAX_TOUCHES; t++)
	{
		if (Touches[t].id == id)
			return &Touches[t];
		if (!Touches[t].active)
		{
            Touches[t].id = id;
			return &Touches[t];
		}
	}

	return NULL;
}

CIwGameTouch*	CIwGameInput::getTouchByID(int id)
{
	if (!PointerAvailable)
		return NULL;

	// Find touch by its ID and return it
	for (int t = 0; t < MAX_TOUCHES; t++)
	{
//		if (Touches[t].active && Touches[t].id == id)
		if (Touches[t].id == id)
			return &Touches[t];
	}

	return NULL;
}

int CIwGameInput::getTouchCount() const
{
	if (!PointerAvailable)
		return 0;

	// Return the total number of active touches
	int count = 0;
	for (int t = 0; t < MAX_TOUCHES; t++)
	{
		if (Touches[t].active)
            count++;
	}

	return count;
}

bool CIwGameInput::Init()
{
	Tapped = false;
	Dragging = false;
	PreviousNumTouches = 0;
	BackPressed = false;
	MenuPressed = false;

	// Check to see if the device that we are running on supports the pointer
    PointerAvailable = s3ePointerGetInt(S3E_POINTER_AVAILABLE) ? true : false;

	if (PointerAvailable)
	{
		// Clear out the touches array
		for (int t = 0; t < MAX_TOUCHES; t++)
		{
			Touches[t].active = false;
			Touches[t].id = 0;
		}

		// Determine if the device supports multi-touch
		IsMultiTouch = s3ePointerGetInt(S3E_POINTER_MULTI_TOUCH_AVAILABLE) ? true : false;

		// For multi-touch devices we handle touch and motion events using different callbacks
		if (IsMultiTouch)
		{
			s3ePointerRegister(S3E_POINTER_TOUCH_EVENT, (s3eCallback)HandleMultiTouchButtonCB, NULL);
			s3ePointerRegister(S3E_POINTER_TOUCH_MOTION_EVENT, (s3eCallback)HandleMultiTouchMotionCB, NULL);
		}
		else
		{
			s3ePointerRegister(S3E_POINTER_BUTTON_EVENT, (s3eCallback)HandleSingleTouchButtonCB, NULL);
			s3ePointerRegister(S3E_POINTER_MOTION_EVENT, (s3eCallback)HandleSingleTouchMotionCB, NULL);
		}
	}

	// Check to see if the device that we are running on supports the keyboard
    KeysAvailable = ((s3eKeyboardGetInt(S3E_KEYBOARD_HAS_KEYPAD) || s3eKeyboardGetInt(S3E_KEYBOARD_HAS_ALPHA))) ? true : false;

	// Check to see if the device that we are running on supports the on screen keyboard
	OSKeyboardAvailable = s3eOSReadStringAvailable() == S3E_TRUE; 

	// Check accelerometer availability
	if (s3eAccelerometerGetInt(S3E_ACCELEROMETER_AVAILABLE))
		AccelerometerAvailable = true;
	else
		AccelerometerAvailable = false;

	// Check compass availability
	if (s3eCompassAvailable())
		CompassAvailable = true;
	else
		CompassAvailable = false;

	return true; // Pointer support
}

void CIwGameInput::Release()
{
	if (PointerAvailable)
	{
		// Unregister the pointer system callbacks
		if (IsMultiTouch)
		{
			s3ePointerUnRegister(S3E_POINTER_TOUCH_EVENT, (s3eCallback)HandleMultiTouchButtonCB);
			s3ePointerUnRegister(S3E_POINTER_TOUCH_MOTION_EVENT, (s3eCallback)HandleMultiTouchMotionCB);
		}
		else
		{
			s3ePointerUnRegister(S3E_POINTER_BUTTON_EVENT, (s3eCallback)HandleSingleTouchButtonCB);
			s3ePointerUnRegister(S3E_POINTER_MOTION_EVENT, (s3eCallback)HandleSingleTouchMotionCB);
		}
	}
}

void CIwGameInput::Update()
{
	// Update the pointer if it is available
	if (PointerAvailable)
		s3ePointerUpdate();

	// Update key system if it is available
	s3eKeyboardUpdate();

	Tapped = false;

	int num_touches = getTouchCount();
	DragDelta.x = 0;
	DragDelta.y = 0;

	if (num_touches == 0)
		Dragging = false;
	else
		Dragging = true;

	// User has just pressed screen so start timer
	if (PreviousNumTouches == 0 && num_touches == 1)
	{
		TapTimer.setDuration(TAP_TIME_MS);
		TouchedPos.x = getTouch(0)->x;
		TouchedPos.y = getTouch(0)->y;
	}

	// User has stopped pressing screen so check to see if press was a short press (a tap)
	if (PreviousNumTouches == 1 && num_touches == 0)
	{
		// Only check if touch hasnt moved much
		int dx = getTouch(0)->x - TouchedPos.x;
		int dy = getTouch(0)->y - TouchedPos.y;
		int d = dx * dx + dy * dy;
		DragDelta.x = dx;
		DragDelta.y = dy;

		if (d <= TAP_SENSITIVITY)
		{
			if (TapTimer.HasTimedOut())
			{
				TapTimer.Stop();
				Tapped = true;
			}
		}
		else
		{
			TapTimer.Stop();
		}
	}

	PreviousNumTouches = num_touches;

	// Update buttons
	BackPressed = false;
	bool back_released = (s3eKeyboardGetState(s3eKeyBack) & S3E_KEY_STATE_RELEASED) == S3E_KEY_STATE_RELEASED || (s3eKeyboardGetState(s3eKeyAbsBSK) & S3E_KEY_STATE_RELEASED) == S3E_KEY_STATE_RELEASED;
	if (back_released)
		BackPressed = true;

	MenuPressed = false;
	bool menu_released = (s3eKeyboardGetState(s3eKeyMenu) & S3E_KEY_STATE_RELEASED) == S3E_KEY_STATE_RELEASED || (s3eKeyboardGetState(s3eKeyAbsASK) & S3E_KEY_STATE_RELEASED) == S3E_KEY_STATE_RELEASED;
	if (menu_released)
		MenuPressed = true;

	// Update accelerometer
	if (AccelerometerActive)
	{
		AccelerometerPosition.x = s3eAccelerometerGetX();
		AccelerometerPosition.y = s3eAccelerometerGetY();
		AccelerometerPosition.z = s3eAccelerometerGetZ();
	}

	// Update compass
	if (CompassActive)
	{
		CompassDirection = s3eCompassGet();
		s3eCompassHeading heading = { 0, 0, 0 };
		if (s3eCompassGetHeading(&heading) != S3E_RESULT_SUCCESS)
		{
			CompassHeading.x = heading.m_X;
			CompassHeading.y = heading.m_Y;
			CompassHeading.z = heading.m_Z;
		}
	}

}

bool CIwGameInput::isKeyDown(s3eKey key) const
{
	if (!KeysAvailable)
		return false;

	// Return down state of queried key
	return (s3eKeyboardGetState(key) & S3E_KEY_STATE_DOWN) == S3E_KEY_STATE_DOWN;
}

bool CIwGameInput::isKeyUp(s3eKey key) const
{
	if (!KeysAvailable)
		return false;

	// Return up state of queried key
	return (s3eKeyboardGetState(key) & S3E_KEY_STATE_UP) == S3E_KEY_STATE_UP;
}

bool CIwGameInput::wasKeyPressed(s3eKey key) const
{
	if (!KeysAvailable)
		return false;

	// Return pressed state of queried key
	return (s3eKeyboardGetState(key) & S3E_KEY_STATE_PRESSED) == S3E_KEY_STATE_PRESSED;
}

bool CIwGameInput::wasKeyReleased(s3eKey key) const
{
	if (!KeysAvailable)
		return false;

	// Return released state of queried key
	return (s3eKeyboardGetState(key) & S3E_KEY_STATE_RELEASED) == S3E_KEY_STATE_RELEASED;
}

const char* CIwGameInput::showOnScreenKeyboard(const char* prompt, int flags, const char* default_text)
{
	if (!OSKeyboardAvailable)
		return NULL;

	// Show on screen keyboard and return the input string
	if (default_text != NULL)
		return s3eOSReadStringUTF8WithDefault(prompt, default_text, flags);
	else
		return s3eOSReadStringUTF8(prompt, flags);
}

bool CIwGameInput::startAccelerometer()
{
	AccelerometerActive = false;
	if (AccelerometerAvailable && s3eAccelerometerStart() == S3E_RESULT_SUCCESS)
	{
		AccelerometerActive = true;
		return true;
	}

	return false;
}
void CIwGameInput::stopAccelerometer()
{
	if (AccelerometerActive)
	{
		s3eAccelerometerStop();
		AccelerometerActive = false;
	}
}

bool CIwGameInput::startCompass()
{
	CompassActive = false;
	if (CompassAvailable && s3eCompassStart() == S3E_RESULT_SUCCESS)
	{
		CompassActive = true;
		return true;
	}

	return false;
}
void CIwGameInput::stopCompass()
{
	if (CompassActive)
	{
		s3eCompassStop();
		CompassActive = false;
	}
}



