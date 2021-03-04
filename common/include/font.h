#pragma once
#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

struct FontHeader
{
	unsigned char Magic[2];
	unsigned char Mode;
	unsigned char CharacterSize;
};

struct Font
{
	struct FontHeader *Header;
	void *GlyphBuffer;
};