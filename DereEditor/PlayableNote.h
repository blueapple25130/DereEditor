#pragma once
#include "Note.h"


class PlayableNote :public Note {
private:
	NoteType m_type;

public:
	PlayableNote(int tick, int lane, NoteType type);

	PlayableNote() = default;
	~PlayableNote() = default;

	virtual void draw() const;
};