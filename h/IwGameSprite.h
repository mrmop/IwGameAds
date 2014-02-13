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

#if !defined(_CIW_GAME_SPRITE_H_)
#define _CIW_GAME_SPRITE_H_

#include "s3e.h"
#include "Iw2D.h"

#include "IwGameImage.h"

class CIwGameSpriteManager;

//
//
// CIwGameSprite - A sprite is the visual representation of an on screen game object
//
// Can be thought of as an interface rather than a concrete class as other sprite types are created from this
//
//
class CIwGameSprite
{
	/// Properties
protected:
	CIwGameSpriteManager*	Parent;					// Parent sprite manager
	float					Width, Height;			// Destination width and height (used to represent the visible extents of the sprite on screen)
	CIwFVec2				Position;				// Position of the sprite
	float					Angle;					// Rotation of sprite
	float					Scale;					// Scale of sprite
	CIwColour				Colour;					// Colour of sprite
	bool					Visible;				// Sprites visible state
	bool					Pooled;					// Tells system if we belong to a sprite pool or not
	bool					InUse;					// Used in a memory pooling system to mark this sprite as in use
	int						Layer;					// Depth layer
public:
	void		setParent(CIwGameSpriteManager* parent) { Parent = parent; }
	void		setDestSize(float width, float height)
	{
		Width = width;
		Height = height;
	}
	float		getDestWidth() const		{ return Width; }
	float		getDestHeight() const		{ return Height; }
	void		setPosAngScale(float x, float y, float angle, float scale)
	{
		Position.x = x;
		Position.y = y;
		Angle = angle;
		Scale = scale;
		TransformDirty = true;
	}
	void		setPosition(float x, float y)
	{
		Position.x = x;
		Position.y = y;
		TransformDirty = true;
	}
	CIwFVec2	getPosition() const			{ return Position; }
	void		setAngle(float angle)
	{
		Angle = angle;
		TransformDirty = true;
	}
	float		getAngle() const			{ return Angle; }		
	void		setScale(float scale)
	{
		Scale = scale;
		TransformDirty = true;
	}
	float		getScale() const			{ return Scale; }
	void		setColour(CIwColour& colour) { Colour = colour; }
	CIwColour	getColour() const			{ return Colour; }
	void		setVisible(bool show)		{ Visible = show; }
	bool		isVisible() const			{ return Visible; }
	void		forceTransformDirty()		{ TransformDirty = true; }
	void		setPooled(bool pooled)		{ Pooled = pooled; }
	bool		isPooled() const			{ return Pooled; }
	void		setInUse(bool in_use)		{ InUse = in_use; }
	bool		isInUse() const				{ return InUse; }
	void		setLayer(int layer)			{ Layer = layer; }
	int			getLayer() const			{ return Layer; }
	CIwFMat2D&	getTransform()				{ return Transform; }
	/// Properties End
protected:
	CIwFMat2D	Transform;					// Transform
	bool		TransformDirty;				// Dirty when transform changed
	CIwFVec2	TransformedV[4];			// Transforrmed vertices (used for hit detection)

	void		RebuildTransform();			// Rebuilds the display transform

public:
	CIwGameSprite() : Pooled(false)			{ Init(); }
	virtual ~CIwGameSprite() {}

	virtual void	Init();					// Called to initialise the sprite, used after construction or to reset the sprite in a pooled sprite system
	virtual void	Draw() = 0;				// Pure virtual, need to implement in derived classes
	bool			HitTest(int x, int y);	// Check to see if point is within area covered by transformed sprite
};

//
//
// CIwGameBitmapSprite - A BitmapSprite is a bitmapped visual representation of an on screen game object
//
//
class CIwGameBitmapSprite : public CIwGameSprite
{
	// Properties
protected:
	CIwGameImage*	Image;					// Bitmapped image that represents this sprite
	int				SrcX, SrcY;				// Top left position in source texture
	int				SrcWidth, SrcHeight;	// Width and height of sprite in source texture
public:
	void		setImage(CIwGameImage* image)
	{
		Image = image;
	}
	CIwGameImage*	getImage()				{ return Image; }
	void		setSrcDest(int x, int y, int width, int height)
	{
		setDestSize(width, height);
		SrcX = x;
		SrcY = y; 
		SrcWidth = width;
		SrcHeight = height;
	}
	void		setSrcRect(int x, int y, int width, int height)
	{
		SrcX = x;
		SrcY = y; 
		SrcWidth = width;
		SrcHeight = height;
	}
	void		setSrcRect(CIwRect* src)
	{
		SrcX = src->x;
		SrcY = src->y; 
		SrcWidth = src->w;
		SrcHeight = src->h;
	}
	int			getSrcWidth() const			{ return SrcWidth; }
	int			getSrcHeight() const		{ return SrcHeight; }
	// Properties End
public:

	CIwGameBitmapSprite() : CIwGameSprite(), Image(NULL), SrcX(0), SrcY(0), SrcWidth(0), SrcHeight(0)	{}
	virtual ~CIwGameBitmapSprite() {}
	
	void	Draw();
};

//
//
// CIwGameSpriteManager - A sprite manager 
//
// The sprite manager managers a collection of sprites, including drawing, tracking and clean up
// The sprite manager also carries its own visual transform that will be applied to all of its children, allowing the user to apply rotation, scaling ans translation to all child sprites
//
//
class CIwGameSpriteManager
{
public:
	// Provide public access to iteration of the sprite list
	typedef CIwList<CIwGameSprite*>::iterator	Iterator;
	Iterator		begin() { return Sprites.begin(); }
	Iterator		end()	{ return Sprites.end(); }

	// Properties
protected:
	CIwFMat2D					Transform;			// Transform
	CIwList<CIwGameSprite*>		Sprites;			// Our list of sprites
	CIwList<CIwGameSprite*>*	Layers;				// Visible layers used in depth sorting
public:
	void			addSprite(CIwGameSprite* sprite);
	void			removeSprite(CIwGameSprite* sprite, bool delete_sprites = true);
	void			setTransform(const CIwFMat2D& transform)	{ Transform = transform; DirtyChildTransforms(); }
	const CIwFMat2D&	getTransform() const					{ return Transform; }
	// Properties End

protected:
	void			DirtyChildTransforms();		// Dirties all child transforms to force them to update
	int				MaxLayers;					// Maximum layers
	void			ClearLayers();				// Clears all visible layers ready for next frame

public:
	CIwGameSpriteManager() : MaxLayers(0)
	{
		// Set up default rotation, scaling and translation
		Transform.SetIdentity();
		Transform.m[0][0] = IW_GEOM_ONE;
		Transform.m[1][1] = IW_GEOM_ONE;
	}
	~CIwGameSpriteManager() { Release(); }

	void			Init(int max_layers = 10);
	void			Draw();
	void			Release(bool delete_sprites = true);
};





#endif // _CIW_GAME_SPRITE_H_
