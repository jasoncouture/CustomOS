#pragma once
#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

typedef struct
{
	unsigned char Magic[2];
	unsigned char Mode;
	unsigned char CharacterSize;
} __attribute__((__packed__)) FontHeader;

typedef struct
{
	FontHeader *Header;
	void *GlyphBuffer;
}__attribute__((__packed__)) Font;