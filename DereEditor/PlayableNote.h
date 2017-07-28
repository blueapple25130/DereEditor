#pragma once
#include "Note.h"


class PlayableNote :public Note {
private:
	NoteType m_type;

public:
	std::shared_ptr<PlayableNote> ConnectNote = nullptr;
	int Channel;
	int Lane;
	bool IsJudged = false;

	PlayableNote(int tick, int lane, int channel, NoteType type);

	PlayableNote() = default;
	~PlayableNote() = default;

	virtual void draw() const;

	void drawRibbon() const;

	void drawInfo() const;

	NoteType getNoteType();
};