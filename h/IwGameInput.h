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

#if !defined(_IW_GAME_INPUT_H_)
#define _IW_GAME_INPUT_H_

#include "IwGeom.h"
#include "s3ePointer.h"
#include "s3eKeyboard.h"
#include "s3eOSReadString.h"

#include "IwGameUtil.h"

#define MAX_TOUCHES		4
#define TAP_TIME_MS		20
#define TAP_SENSITIVITY	(16 * 16 + 16 * 16)

#define	IW_GAME_INPUT	(CIwGameInput::getInstance())


//
//
// CIwGameTouch - Represents a single touch
//
//
struct CIwGameTouch
{
public:
    int		x, y;		// Touch position
    bool	active;		// Touch active state
    int		id;			// ID of touch - The system tracks multiple touches by assigning each one a unique ID
};

//
//
// CIwGameInput - The IwGameInput class is responsible for handling all keyboard and pointer input
//
//
class CIwGameInput
{
	CDEFINE_SINGLETON(CIwGameInput)

	// Properties
private:
	bool			PointerAvailable;				// true if a pointer is present
	bool			KeysAvailable;					// true if a key input is present
	bool			OSKeyboardAvailable;			// true if on screen keyboard is available
	bool			AccelerometerAvailable;			// true if accelerometer is available
	bool			CompassAvailable;				// true if compass is available
	bool			IsMultiTouch;					// true if multitouch is enabled
	CIwGameTouch	Touches[MAX_TOUCHES];			// List of potential touches
	bool			Dragging;						// true when user drags the pointer
	CIwVec2			DragDelta;						// Amcount dragged
	CIwVec2			TouchedPos;						// Position user touched initially
	bool			Tapped;							// true when user taps screen (cleared in next update)
	bool			BackPressed;					// Back key pressed state
	bool			MenuPressed;					// Menu key pressed state
	bool			AccelerometerActive;			// Active state of accelerometer
	bool			CompassActive;					// Active state of compass
	CIwVec3			AccelerometerReference;			// Accelerometer reference position
	CIwVec3			AccelerometerPosition;			// Current accelerometer position
	CIwVec3			CompassHeading;					// Current compass heading
	int				CompassDirection;				// Current compass direction
public:
	// Availability query
	bool			isPointerAvailable() const		{ return PointerAvailable; }	// Returns availability of the pointer
	bool			isKeysAvailable() const			{ return KeysAvailable; }		// Returns availability of keys
	bool			isOSKeyboardAvailable() const	{ return OSKeyboardAvailable; }	// Returns availability of on screen keyboard
	bool			isAccelerometerAvailable() const { return AccelerometerAvailable; } // Returns availability of accelerometer
	bool			isCompassAvailable() const		{ return CompassAvailable; }	// Returns true if compass is available
	bool			isMultiTouch() const			{ return IsMultiTouch; }		// Returns multitouch capability

	// Pointer
	CIwGameTouch*	getTouchByID(int id);											// Returns the touch identified by its id
	CIwGameTouch*	getTouch(int index)				{ return &Touches[index]; }		// Gets a specific touch
	CIwGameTouch*	findTouch(int id);												// Finds a specific touch by its id
	int				getTouchCount() const;											// Get number of touches this frame
	bool			hasTapped() const				{ return Tapped; }				// Returns tapped status
	bool			isTouching() const				{ return Dragging; }			// Returns touching status
	CIwVec2			getTouchedPos() const			{ return TouchedPos; }
	CIwVec2			getDragDelta() const			{ return DragDelta; }

	// keys / Buttons
	bool			isKeyDown(s3eKey key) const;									// Tests if a key is down
	bool			isKeyUp(s3eKey key) const;										// Tests if a key is up
	bool			wasKeyPressed(s3eKey key) const;								// Tests if a key was pressed
	bool			wasKeyReleased(s3eKey key) const;								// Tests if a key was released
	const char*		showOnScreenKeyboard(const char* prompt, int flags = 0, const char* default_text = NULL);
	bool			isBackPressed()					{ return BackPressed; }
	void			resetBackPressed()				{ BackPressed = false; }
	bool			isMenuPressed()					{ return MenuPressed; }
	void			resetMenuPressed()				{ MenuPressed = false; }

	// Accelerometer
	bool			startAccelerometer();											// Start accelerometer input
	void			stopAccelerometer();											// Stop accelerometer input
	void			setAccelerometerReference();									// Sets the current accelerometer position as a reference posisition 
	CIwVec3			getAccelerometerPosition() const { return AccelerometerPosition; }	// Get current accelerometer position
	CIwVec3			getAccelerometerOffset() const { return AccelerometerPosition - AccelerometerReference; }	// Get current accelerometer offset from the reference position

	// Compass
	bool			startCompass();													// Start compass input
	void			stopCompass();													// Stop compass input
	CIwVec3			getCompassHeading() const { return CompassHeading; }			// Get current compass heading
	int				getCompassDirection() const { return CompassDirection; }		// Get current compass direction (0 to 360 degrees)

	// Properties end

private:
	CIwGameTimer	TapTimer;
	int				PreviousNumTouches;

public:
	bool			Init();							// Initialises the input system (returns true if pointer is supported)
	void			Release();						// Releases data used by the input system
	void			Update();						// Updates the input system, called every frame
};




#endif	// _IW_GAME_INPUT_H_