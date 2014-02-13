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

#include "IwGameSprite.h"
#include "IwGameString.h"
#include "IwGameUtil.h"

//
//
// CIwGameSprite implementation
//
//
void CIwGameSprite::RebuildTransform()
{
	// Build the transform
	// Set the rotation transform
	Transform.SetRot(Angle * 3.14159265 / 180);
	// Scale the transform
	Transform.ScaleRot(Scale);
	// Translate the transform
	Transform.SetTrans(Position);
	// Apply parent transform if sprite is managed by a parent sprite manager
	if (Parent != NULL)
		Transform.PostMult(Parent->getTransform());

	TransformDirty = false;
}

void CIwGameSprite::Init()
{
	Parent = NULL;
	TransformDirty = true;
	Position.x = 0;
	Position.y = 0;
	Angle = 0;
	Scale = 1;
	Colour.r = 0xff;
	Colour.g = 0xff;
	Colour.b = 0xff;
	Colour.a = 0xff;
	Width = 0;
	Height = 0;
	Visible = true;
	Layer = 0;
}

bool CIwGameSprite::HitTest(int x, int y)
{
	if (TransformDirty)
		return false;

	// Generate transformed vertices
	float w = Width / 2;
	float h = Height / 2;
	TransformedV[0].x = -w;
	TransformedV[1].x = w;
	TransformedV[2].x = w;
	TransformedV[3].x = -w;
	TransformedV[0].y = -h;
	TransformedV[1].y = -h;
	TransformedV[2].y = h;
	TransformedV[3].y = h;
	for (int t = 0; t < 4; t++)
		TransformedV[t] = Transform.TransformVec(TransformedV[t]);

	int i1 = 0;
	int i2 = 3;
	for (int t = 0; t < 4; t++)
	{
		float x0 = TransformedV[i1].x - TransformedV[i2].x;
		float y0 = TransformedV[i1].y - TransformedV[i2].y;
		float x1 = x - TransformedV[i2].x;
		float y1 = y - TransformedV[i2].y;

		if ((x1 * y0 - x0 * y1) >= 0)
			return false;

		i2 = i1;
		i1++;
	}

	return true;
}

//
//
// CIwGameBitmapSprite implementation
//
//
void CIwGameBitmapSprite::Draw()
{
	// Do not render if not visible
	if (Image == NULL || !Visible || Colour.a == 0)
		return;

	if (TransformDirty)
		RebuildTransform();

	// Set this transform as the active transform for Iw2D
	Iw2DSetTransformMatrix(Transform);

	// Set colour of sprite
	Iw2DSetColour(Colour);

	// Render the sprite (centered)
	float x = -(Width / 2.0);
	float y = -(Height / 2.0);
	if (Image->getState() == CIwGameImage::CIwGameImage_State_Loaded)
		Iw2DDrawImageRegion(Image->getImage2D(), CIwFVec2(x, y), CIwFVec2(Width, Height), CIwFVec2(SrcX, SrcY), CIwFVec2(SrcWidth, SrcHeight));
//	Iw2DDrawImage(Image, CIwSVec2(x, y));
}

//
//
// CIwGameSpriteManager implementation
//
//
void CIwGameSpriteManager::addSprite(CIwGameSprite* sprite)
{
	Sprites.push_back(sprite);
	sprite->setParent(this);
}

void CIwGameSpriteManager::removeSprite(CIwGameSprite* sprite, bool delete_sprites)
{
	for (Iterator it = Sprites.begin(); it != Sprites.end(); ++it)
	{
		if (*it == sprite)
		{
			if (delete_sprites && !(*it)->isPooled())
				delete *it;
			Sprites.erase(it);
			break;
		}
	}
}

void CIwGameSpriteManager::Init(int max_layers)
{
	// Allocate layers
	Layers = new CIwList<CIwGameSprite*>[max_layers];
	MaxLayers = max_layers;
}

void CIwGameSpriteManager::Draw()
{
	// Clear layers
	ClearLayers();

	// Organise sprites into layers
	for (Iterator it = Sprites.begin(); it != Sprites.end(); ++it)
	{
		int layer = (*it)->getLayer();
		(Layers + layer)->push_back(*it);
	}

	// Draw sprite layers
	CIwList<CIwGameSprite*>* layers = Layers;
	for (int t = 0; t < MaxLayers; t++)
	{
		for (Iterator it = layers->begin(); it != layers->end(); ++it)
			(*it)->Draw();
		layers++;
	}

}

void CIwGameSpriteManager::Release(bool delete_sprites)
{
	if (delete_sprites)
	{
		// Delete all sprites in the sprite manager
		for (Iterator it = Sprites.begin(); it != Sprites.end(); ++it)
		{
			if (!(*it)->isPooled())
				delete *it;
		}
	}
	Sprites.clear();

	// Clean up layers
	SAFE_DELETE_ARRAY(Layers)
}

void CIwGameSpriteManager::DirtyChildTransforms()
{
	// Force all childreen to rebuild their transforms
	for (Iterator it = Sprites.begin(); it != Sprites.end(); ++it)
	{
		(*it)->forceTransformDirty();
	}
}

void CIwGameSpriteManager::ClearLayers()
{
	for (int t = 0; t < MaxLayers; t++)
	{
		(Layers + t)->clear();
	}
}

